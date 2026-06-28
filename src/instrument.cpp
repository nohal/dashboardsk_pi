/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  DashboardSK Plugin
 * Author:   Pavel Kalian
 *
 ******************************************************************************
 * This file is part of the DashboardSK plugin
 * (https://github.com/nohal/dashboardsk_pi).
 *   Copyright (C) 2022 by Pavel Kalian
 *   https://github.com/nohal
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3, or (at your option) any later
 * version of the license.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#if defined(DASHBOARDSK_USE_GIOMM)
#include <giomm/settings.h>
#endif
#include "dashboard.h"
#include "instrument.h"

PLUGIN_BEGIN_NAMESPACE

Instrument::~Instrument()
{
    if (m_parent_dashboard) {
        m_parent_dashboard->Unsubscribe(this);
    }
}

double Instrument::GetSystemFontScalingFactor()
{
#if defined(DASHBOARDSK_USE_GIOMM)
    Glib::RefPtr<Gio::Settings> s
        = Gio::Settings::create("org.gnome.desktop.interface");
    return s->get_double("text-scaling-factor");
#else
    return 1.0;
#endif
}

wxColor Instrument::GetDimedColor(const wxColor& c) const
{
    switch (m_color_scheme) {
    case 3:
        return (
            wxColor(wxMax(0, wxMin(c.Red() + c.Red() * LUMIMOSITY_NIGHT, 255)),
                wxMax(0, wxMin(c.Green() + c.Green() * LUMIMOSITY_NIGHT, 255)),
                wxMax(0, wxMin(c.Blue() + c.Blue() * LUMIMOSITY_NIGHT, 255))));
    case 2:
        return (
            wxColor(wxMax(0, wxMin(c.Red() + c.Red() * LUMIMOSITY_DUSK, 255)),
                wxMax(0, wxMin(c.Green() + c.Green() * LUMIMOSITY_DUSK, 255)),
                wxMax(0, wxMin(c.Blue() + c.Blue() * LUMIMOSITY_DUSK, 255))));
    default:
        return c;
    }
}

void Instrument::ReadConfig(Json::Value& config)
{
    if (config.isMember("name")) {
        m_name = fromJsonVal(config["name"].asString());
    }
    if (config.isMember("title")) {
        m_title = fromJsonVal(config["title"].asString());
    }
    if (config.isMember("allowed_age")) {
        m_allowed_age_sec = config["allowed_age"].asInt();
    }
    if (config.isMember(DSK_SETTING_ZONES)) {
        m_zones = Zone::ParseZonesFromString(
            fromJsonVal(config[DSK_SETTING_ZONES].asString()));
    }
    if (config.isMember("locked_source")) {
        m_locked_source = fromJsonVal(config["locked_source"].asString());
    }
    if (config.isMember("locked_source_path")) {
        m_locked_source_path
            = fromJsonVal(config["locked_source_path"].asString());
    }
}

Json::Value Instrument::GenerateJSONConfig()
{
    Json::Value v;
    v["name"] = toJson(m_name);
    v["title"] = toJson(m_title);
    v["class"] = toJson(GetClass());
    v["allowed_age"] = m_allowed_age_sec;
    v[DSK_SETTING_ZONES] = toJson(Zone::ZonesToString(m_zones));
    const wxString key = GetPrimarySKKey();
    if (!m_locked_source.IsEmpty() && key.EndsWith(".SRC:lockpersist")
        && (m_locked_source_path.IsEmpty()
            || m_locked_source_path.IsSameAs(key))) {
        v["locked_source"] = toJson(m_locked_source);
        v["locked_source_path"] = toJson(key);
    }
    return v;
}

void Instrument::SetSetting(const wxString& key, const wxString& value)
{
    if (key == "name") {
        m_name = value;
    } else if (key == "title") {
        m_title = value;
    } else if (key == "allowed_age") {
        m_allowed_age_sec = IntFromString(value);
    } else if (key == DSK_SETTING_ZONES) {
        m_zones = Zone::ParseZonesFromString(value);
    } else {
        m_config_vals[UNORDERED_KEY(key)] = value;
    }
    m_needs_redraw = true;
}

void Instrument::SetSetting(const wxString& key, const wxColor& value)
{
    m_config_vals[UNORDERED_KEY(key)] = value.GetAsString(wxC2S_HTML_SYNTAX);
}

void Instrument::SetSetting(const wxString& key, const int& value)
{
    if (key == "allowed_age") {
        m_allowed_age_sec = value;
    } else {
        m_config_vals[UNORDERED_KEY(key)] = wxString::Format("%i", value);
    }
}

void Instrument::SetColorScheme(int scheme)
{
    m_color_scheme = scheme;
    m_needs_redraw = true;
}

wxBitmap Instrument::ScaleBitmap(
    wxBitmap& bmpIn, double scale, bool antialiasing)
{
    if (scale <= 0.0)
        return wxNullBitmap;
    wxSize sizeIn = bmpIn.GetSize();
    wxSize sizeOut((int)sizeIn.GetX() * scale, (int)sizeIn.GetY() * scale);

    wxBitmap bmpOut;
    bmpOut.Create(sizeOut); // the final size

    wxMemoryDC inDC(bmpIn);

    if (antialiasing && scale < 0.4) {
#if wxUSE_GRAPHICS_CONTEXT
        wxGCDC outDC(bmpOut);
#else
        wxMemoryDC outDC(bmpOut);
#endif
        outDC.StretchBlit(wxPoint(0, 0), sizeOut, &inDC, wxPoint(0, 0), sizeIn);
    } else {
        wxMemoryDC outDC(bmpOut);
        outDC.StretchBlit(wxPoint(0, 0), sizeOut, &inDC, wxPoint(0, 0), sizeIn);
        outDC.SelectObject(wxNullBitmap);
    }
    inDC.SelectObject(wxNullBitmap);
    return bmpOut;
}

const wxColor Instrument::AdjustColorForZone(const double& val,
    const wxColor& nominal_color, const wxColor& normal_color,
    const wxColor& alert_color, const wxColor& warn_color,
    const wxColor& alarm_color, const wxColor& emergency_color)
{
    wxColor c = nominal_color;
    Zone::state high_state = Zone::state::nominal;
    for (auto& zone : m_zones) {
        if (high_state < Zone::state::emergency && val >= zone.GetLowerLimit()
            && val <= zone.GetUpperLimit() && high_state < zone.GetState()) {
            high_state = zone.GetState();
            switch (high_state) {
            case Zone::state::normal:
                c = normal_color;
                break;
            case Zone::state::alert:
                c = alert_color;
                break;
            case Zone::state::warn:
                c = warn_color;
                break;
            case Zone::state::alarm:
                c = alarm_color;
                break;
            case Zone::state::emergency:
                c = emergency_color;
                break;
            default:
                c = nominal_color;
            }
        }
    }
    return c;
}

wxBitmap Instrument::Render(double scale)
{
    m_new_data = false;
    return wxNullBitmap;
}

const wxColor Instrument::ColorFromString(const wxString& color)
{
    wxColor clr;
    clr.Set(color);
    return clr;
}

const int Instrument::IntFromString(const wxString& str)
{
    int i;
#if (wxCHECK_VERSION(3, 1, 6))
    if (str.ToInt(&i)) {
        return i;
    }
#else
    return wxAtoi(str);
#endif
    return 0;
}

const double Instrument::DoubleFromString(const wxString& str)
{
    double i;
    if (str.ToDouble(&i)) {
        return i;
    }
    return 0.0;
}

wxString Instrument::GetStringSetting(const wxString& key)
{
    if (key.IsSameAs(DSK_SETTING_ZONES)) {
        return Zone::ZonesToString(m_zones);
    }
    if (m_config_vals.find(UNORDERED_KEY(key)) != m_config_vals.end()) {
        return m_config_vals[UNORDERED_KEY(key)];
    }
    return wxEmptyString;
}

int Instrument::GetIntSetting(const wxString& key)
{
    int i = 0;
    if (m_config_vals.find(UNORDERED_KEY(key)) != m_config_vals.end()) {
#if (wxCHECK_VERSION(3, 1, 6))
        if (!m_config_vals[UNORDERED_KEY(key)].ToInt(&i)) {
            i = 0;
        }
#else
        i = wxAtoi(m_config_vals[UNORDERED_KEY(key)]);
#endif
    }
    return i;
}

double Instrument::GetDoubleSetting(const wxString& key)
{
    double i = 0.0;
    if (m_config_vals.find(UNORDERED_KEY(key)) != m_config_vals.end()) {
#if (wxCHECK_VERSION(3, 1, 6))
        if (!m_config_vals[UNORDERED_KEY(key)].ToDouble(&i)) {
            i = 0.0;
        }
#else
        i = wxAtof(m_config_vals[UNORDERED_KEY(key)]);
#endif
    }
    return i;
}

wxColor Instrument::GetColorSetting(const wxString& key)
{
    if (m_config_vals.find(UNORDERED_KEY(key)) != m_config_vals.end()) {
        wxColor col;
        wxFromString(m_config_vals[UNORDERED_KEY(key)], &col);
        return col;
    }
    return *wxCYAN;
}

double Instrument::Transform(const double& val, const transformation& formula)
{
    switch (formula) {
    case transformation::none:
        return val;
    case transformation::rad2deg:
        return rad2deg(val);
    case transformation::ms2kn:
        return 1.943844 * val;
    case transformation::ms2kmh:
        return 3.6 * val;
    case transformation::ms2mph:
        return 2.236936 * val;
    case transformation::m2ft:
        return 3.28084 * val;
    case transformation::m2fm:
        return 0.546807 * val;
    case transformation::m2nm:
        return val / 1852;
    case transformation::degk2degc:
        return val - 273.15;
    case transformation::degk2degf:
        return val * 1.8 - 459.67;
    case transformation::ratio2perc:
        return val * 100;
    case transformation::pa2hpa:
        return val / 100;
    case transformation::pa2kpa:
        return val / 1000;
    case transformation::pa2mpa:
        return val / 1000000;
    case transformation::pa2atm:
        return val / 101325;
    case transformation::pa2mmhg:
        return val / 133.3223684;
    case transformation::pa2psi:
        return val / 6894.757;
    case transformation::hz2rpm:
        return val * 60;
    case transformation::rads2degm:
        return val * 3437.7467747131;
    case transformation::joul2kwh:
        return val / 3600000;
    case transformation::joul2wh:
        return val / 3600;
    case transformation::sec2hour:
        return val / 3600;
    case transformation::m3s2lh:
        return val * 3600000;
    default:
        return val;
    }
}

const wxString Instrument::ConcatChoiceStrings(wxArrayString arr)
{
    wxString s = wxEmptyString;
    for (auto& str : arr) {
        if (!s.IsEmpty()) {
            s.Append(";");
        }
        s.Append(str);
    }
    return s;
}

void Instrument::ConfigureFromMeta(Json::Value& sk_meta)
{
    if (sk_meta.isMember("shortName") && m_title != DUMMY_TITLE) {
        m_title = fromJsonVal(sk_meta["shortName"].asString());
    }
    if (sk_meta.isMember("displayName") && m_name.StartsWith("New ")) {
        m_name = fromJsonVal(sk_meta["displayName"].asString());
    } else if (sk_meta.isMember("longName") && m_name.StartsWith("New ")) {
        m_name = fromJsonVal(sk_meta["longName"].asString());
    }
    if (sk_meta.isMember("zones") && sk_meta["zones"].isArray()) {
        for (int i = 0; i < (int)sk_meta["zones"].size(); i++) {
            m_zones.emplace_back(Zone(sk_meta["zones"][i]["lower"].asDouble(),
                sk_meta["zones"][i]["upper"].asDouble(),
                Zone::StateFromString(
                    fromJsonVal(sk_meta["zones"][i]["state"].asString())),
                sk_meta["zones"][i].isMember("message")
                    ? fromJsonVal(sk_meta["zones"][i]["message"].asString())
                    : wxString()));
        }
    }
    // TODO: displayScale (not universal, do in SimpleGauge where we need it or
    // make universal as it may be needed on many places?)
}

void Instrument::ConfigureFromKey(const wxString& key)
{
    if (!key.IsEmpty() && m_title == DUMMY_TITLE) {
        m_title = key.AfterLast('.');
    }
    if (!key.IsEmpty() && m_name.StartsWith("New ")) {
        m_name = key.AfterLast('.');
    }
}

const Json::Value* Instrument::GetSKDataResolved(const wxString& path)
{
    // ponytail: check for magic source modes and apply locking logic
    if (!m_parent_dashboard) {
        return nullptr;
    }

    int srcPos = path.Find("SRC:");
    if (srcPos == wxNOT_FOUND) {
        // No source designation - use base path
        return m_parent_dashboard->GetSKData(path);
    }

    wxString srcDesignation = path.Mid(srcPos + 4); // Skip "SRC:"

    if (srcDesignation == "any") {
        // ponytail: for "any" mode, just scan and return first available source
        // (no locking, returns first available each time)
        return m_parent_dashboard->GetSKData(path);
    } else if (srcDesignation == "lockfirst"
        || srcDesignation == "lockpersist") {
        wxString basePath = path.Left(srcPos - 1);

        if (!m_locked_source_path.IsEmpty()
            && !m_locked_source_path.IsSameAs(path)) {
            m_locked_source.Clear();
        }
        m_locked_source_path = path;

        if (!m_locked_source.IsEmpty()) {
            const Json::Value* lockedValue = m_locked_source.IsSameAs("direct")
                ? m_parent_dashboard->GetSKData(basePath)
                : m_parent_dashboard->GetSKData(
                      basePath + ".SRC:" + m_locked_source);
            if (lockedValue) {
                m_locked_source_time = std::chrono::system_clock::now();
                return lockedValue;
            }
            if (srcDesignation == "lockfirst") {
                return nullptr;
            }

            const auto age = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - m_locked_source_time);
            if (age.count() < m_allowed_age_sec) {
                return nullptr;
            }
            m_locked_source.Clear();
        }

        // Lock not established - find first available source and lock to it
        const Json::Value* baseValue = m_parent_dashboard->GetSKData(basePath);
        if (!baseValue) {
            return nullptr;
        }

        // Try direct value first, then scan for SRC: entries
        if (baseValue->isMember("value")) {
            // Direct value exists - lock to it (empty source = direct)
            m_locked_source = "direct";
            m_locked_source_time = std::chrono::system_clock::now();
            return baseValue;
        }

        // Scan for first available SRC: entry
        for (const auto& name : baseValue->getMemberNames()) {
            if (name.find("SRC:") == 0) {
                const Json::Value* candidate = &(*baseValue)[name];
                // Return this source container if it has any data (value for
                // scalars, or nested data for complex types like position with
                // latitude/longitude)
                if (candidate->isMember("value")
                    || !candidate->getMemberNames().empty()) {
                    // Found first available source - lock to it
                    wxString srcName(name);
                    wxString srcValue = srcName.Mid(4); // Skip "SRC:"
                    m_locked_source = srcValue;
                    m_locked_source_time = std::chrono::system_clock::now();
                    return candidate;
                }
            }
        }
        return nullptr;
    } else {
        // Exact source designation - delegate to base GetSKData
        return m_parent_dashboard->GetSKData(path);
    }
}

PLUGIN_END_NAMESPACE
