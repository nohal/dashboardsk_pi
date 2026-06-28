/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  DashboardSK Plugin
 * Author:   Pavel Kalian
 *
 ******************************************************************************
 * This file is part of the DashboardSK plugin
 * (https://github.com/nohal/dashboardsk_pi).
 *   Copyright (C) 2026 by Pavel Kalian
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

#include "combinedgaugeinstrument.h"
#include "dashboard.h"
#include "instrument.h"
#include <limits>

PLUGIN_BEGIN_NAMESPACE

void CombinedGaugeInstrument::InitExtra()
{
    m_name = _("New Combined Gauge");
    m_center_sk_key = wxEmptyString;
    m_center_value = std::numeric_limits<double>::min();
    m_center_transformation = transformation::none;
    // The center value shares the dial with the needle, so render it smaller
    // than the simple gauge's full-size central value.
    m_value_font_divisor = 6;

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_CGI_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_CGI_SETTINGS
#undef X

    // Order the dialog: primary source (key, gauge type, transformation) first,
    // then the center value source (key, transformation, and the inherited
    // format/suffix that apply to it), then the gauge appearance settings.
    // Controls not listed below keep their original relative order at the end.
    const wxString order[]
        = { DSK_SETTING_SK_KEY, DSK_SGI_GAUGE_TYPE, DSK_SETTING_TRANSFORMATION,
              DSK_CGI_CENTER_KEY, DSK_CGI_CENTER_TRANSFORM, DSK_SETTING_FORMAT,
              DSK_SETTING_VALUE_SUFFIX };
    vector<config_control> ordered;
    for (const auto& key : order) {
        for (auto it = m_config_controls.begin(); it != m_config_controls.end();
            ++it) {
            if (key.IsSameAs(it->key)) {
                ordered.push_back(*it);
                m_config_controls.erase(it);
                break;
            }
        }
    }
    ordered.insert(
        ordered.end(), m_config_controls.begin(), m_config_controls.end());
    m_config_controls = std::move(ordered);
}

void CombinedGaugeInstrument::SubscribeAll()
{
    if (!m_parent_dashboard) {
        return;
    }
    m_parent_dashboard->Unsubscribe(this);
    if (!m_sk_key.IsEmpty()) {
        m_parent_dashboard->Subscribe(m_sk_key, this);
    }
    if (!m_center_sk_key.IsEmpty()) {
        m_parent_dashboard->Subscribe(m_center_sk_key, this);
    }
}

void CombinedGaugeInstrument::SetSetting(
    const wxString& key, const wxString& value)
{
    SimpleGaugeInstrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_SK_KEY)) {
        // The base only (re)subscribed the primary key, dropping the center
        // value subscription in the process. Restore both.
        SubscribeAll();
    } else if (key.IsSameAs(DSK_CGI_CENTER_KEY)
        && !m_center_sk_key.IsSameAs(value)) {
        m_center_sk_key = wxString(value);
        SubscribeAll();
    } else if (key.IsSameAs(DSK_CGI_CENTER_TRANSFORM)) {
        SetSetting(key, IntFromString(value));
    }
}

void CombinedGaugeInstrument::SetSetting(const wxString& key, const int& value)
{
    SimpleGaugeInstrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_CGI_CENTER_TRANSFORM)) {
        m_center_transformation = static_cast<transformation>(value);
    }
}

void CombinedGaugeInstrument::ProcessData()
{
    SimpleGaugeInstrument::ProcessData();
    if (m_new_data && !m_center_sk_key.IsEmpty()) {
        const Json::Value* val = GetSKDataResolved(m_center_sk_key);
        if (val) {
            Json::Value v = val->get("value", *val);
            m_center_value = Transform(v.isDouble() ? v.asDouble()
                    : v.isInt64()                   ? v.asInt64()
                                                    : 0.0,
                m_center_transformation);
        }
    }
}

wxString CombinedGaugeInstrument::FormatCenterValue()
{
    // The center always shows the center value; the primary value only drives
    // the needle and is never displayed as text. The inherited format and value
    // suffix are applied to the center value.
    if (m_center_sk_key.IsEmpty()) {
        return "---";
    }
    wxString value = wxString::Format(
        m_format_strings[m_format_index], abs(m_center_value));
    if (m_center_value < 0
        && !m_supported_formats[m_format_index].StartsWith("ABS")) {
        value.Prepend("-");
    }
    return value.Append(m_value_suffix);
}

void CombinedGaugeInstrument::ReadConfig(Json::Value& config)
{
    SimpleGaugeInstrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.isMember(b)) {                                                  \
        SetSetting(b, fromJsonVal(config[b].g()));                             \
    }
    DSK_CGI_SETTINGS
#undef X
}

Json::Value CombinedGaugeInstrument::GenerateJSONConfig()
{
    Json::Value v = SimpleGaugeInstrument::GenerateJSONConfig();
#define X(a, b, c, d, e, f, g, h) v[b] = toJson(h(b));
    DSK_CGI_SETTINGS
#undef X
    return v;
}

PLUGIN_END_NAMESPACE
