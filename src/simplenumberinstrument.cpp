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

#include "simplenumberinstrument.h"
#include "dashboard.h"
#include "instrument.h"
#include <limits>

PLUGIN_BEGIN_NAMESPACE

void SimpleNumberInstrument::Init()
{
    // Define formatting and transformation data to be shared between settings
    // GUI and instrument logic (We need it to be consistent)
#define X(a, b, c) m_supported_formats.Add(b);
    DSK_VALUE_FORMATS
#undef X
#define X(a, b, c) m_format_strings.Add(c);
    DSK_VALUE_FORMATS
#undef X
#define X(a, b) m_supported_transforms.Add(b);
    DSK_UNIT_TRANSFORMATIONS
#undef X

    // Basic settings inherited from Instrument class
    m_title = "???";
    m_name = _("New Simple Number");
    m_sk_key = wxEmptyString;
    // SimpleNumberInstrument's own settings
    m_timed_out = false;
    m_needs_redraw = true;
    m_title_font = wxFont(
        15, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_body_font
        = wxFont(30, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    m_smoothing = 0;
    m_old_value = std::numeric_limits<double>::min();

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_SNI_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_SNI_SETTINGS
#undef X
}

void SimpleNumberInstrument::SetSetting(
    const wxString& key, const wxString& value)
{
    Instrument::SetSetting(key, value);
    if (key == DSK_SETTING_SK_KEY && !m_sk_key.IsSameAs(value)) {
        m_sk_key = wxString(value);
        if (m_parent_dashboard) {
            m_parent_dashboard->Unsubscribe(this);
            m_parent_dashboard->Subscribe(m_sk_key, this);
        }
    } else if (key.IsSameAs(DSK_SETTING_FORMAT)
        || key.IsSameAs(DSK_SETTING_TRANSFORMATION)
        || key.IsSameAs(DSK_SETTING_SMOOTHING)
        || key.IsSameAs(DSK_SETTING_BODY_FONT)
        || key.IsSameAs(DSK_SETTING_TITLE_FONT)) {
        // TODO: The above manually maintained list should be replaced with
        // something using the information from the DSK_SNI_SETTINGS macro
        int i = 0;
#if (wxCHECK_VERSION(3, 1, 6))
        value.ToInt(&i);
#else
        i = wxAtoi(value);
#endif
        SetSetting(key, i);
    }
}

void SimpleNumberInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_FORMAT)) {
        m_format_index = value;
    } else if (key.IsSameAs(DSK_SETTING_TRANSFORMATION)) {
        m_transformation = static_cast<transformation>(value);
    } else if (key.IsSameAs(DSK_SETTING_TITLE_FONT)) {
        m_title_font.SetPointSize(value);
    } else if (key.IsSameAs(DSK_SETTING_BODY_FONT)) {
        m_body_font.SetPointSize(value);
    } else if (key.IsSameAs(DSK_SETTING_SMOOTHING)) {
        m_smoothing = value;
    }
}

wxBitmap SimpleNumberInstrument::Render(double scale)
{
    wxString value;
    wxColor ctb = GetDimedColor(GetColorSetting(DSK_SETTING_TITLE_BG));
    wxColor ctf = GetDimedColor(GetColorSetting(DSK_SETTING_TITLE_FG));
    wxColor cbb = GetDimedColor(GetColorSetting(DSK_SETTING_BODY_BG));
    wxColor cbf = GetDimedColor(GetColorSetting(DSK_SETTING_BODY_FG));
    wxColor cb = GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR));
    if (!m_new_data) {
        value = "-----";
        cbb = GetDimedColor(GetColorSetting(DSK_SETTING_BODY_BG));
        cbf = GetDimedColor(GetColorSetting(DSK_SETTING_BODY_FG));
        if (!m_timed_out
            && (m_allowed_age_sec > 0
                && (m_last_change.IsValid()
                    && !wxDateTime::Now().IsEqualUpTo(
                        m_last_change, wxTimeSpan(m_allowed_age_sec))))) {
            m_needs_redraw = true;
            m_timed_out = true;
            m_old_value = std::numeric_limits<double>::min();
            cbb = GetDimedColor(GetColorSetting(DSK_SETTING_ALERT_BG));
            cbf = GetDimedColor(GetColorSetting(DSK_SETTING_ALERT_FG));
        }
    } else {
        m_new_data = false;
        m_needs_redraw = true;
        m_last_change = wxDateTime::Now();
        m_timed_out = false;
        const wxJSONValue* val = m_parent_dashboard->GetSKData(m_sk_key);
        if (val) {
            wxJSONValue v = val->Get("value", *val);
            if (m_format_index >= m_format_strings.GetCount()) {
                value = wxString::Format(
                    "E: format", m_format_index, m_format_strings.GetCount());
                cbb = GetDimedColor(GetColorSetting(DSK_SETTING_ALERT_BG));
                cbf = GetDimedColor(GetColorSetting(DSK_SETTING_ALERT_FG));
            } else {
                double dval = Transform(v.AsDouble());
                if (m_old_value > std::numeric_limits<double>::min()) {
                    dval = (m_smoothing * m_old_value
                               + (DSK_SNI_SMOOTHING_MAX - m_smoothing + 1)
                                   * dval)
                        / (DSK_SNI_SMOOTHING_MAX + 1);
                }
                m_old_value = dval;
                value = wxString::Format(
                    m_format_strings[m_format_index], abs(dval));
                if (dval < 0) {
                    value.Prepend("-");
                }
                ctb = GetDimedColor(GetColor(dval, color_item::title_bg));
                ctf = GetDimedColor(GetColor(dval, color_item::title_fg));
                cbb = GetDimedColor(GetColor(dval, color_item::body_bg));
                cbf = GetDimedColor(GetColor(dval, color_item::body_fg));
                cb = GetDimedColor(GetColor(dval, color_item::border));
            }
        } else {
            value = _("Error!");
            cbb = GetDimedColor(GetColorSetting(DSK_SETTING_ALERT_BG));
            cbf = GetDimedColor(GetColorSetting(DSK_SETTING_ALERT_FG));
        }
    }

    if (!m_needs_redraw) {
        return m_bmp;
    }
    m_needs_redraw = false;
    wxString dummy_str(
        "9999"); // dummy string to size the instrument consistently
    wxCoord size_x, size_y;
    wxCoord title_x, title_y;
    wxCoord body_x, body_y;
    wxCoord dummy_x, dummy_y;
    wxMemoryDC mdc;

    wxFont tf = m_title_font;
    tf.SetPointSize(m_title_font.GetPointSize() * scale);
    mdc.SetFont(tf);
    mdc.GetTextExtent(m_title, &title_x, &title_y);
    wxFont bf = m_body_font;
    bf.SetPointSize(m_body_font.GetPointSize() * scale);
    mdc.SetFont(bf);
    mdc.GetTextExtent(dummy_str, &dummy_x, &dummy_y);
    mdc.GetTextExtent(value, &body_x, &body_y);
    size_x = (wxMax(title_x, wxMax(body_x, dummy_x)) + 2 * BORDER_SIZE);
    size_y = (title_y + body_y + 4 * BORDER_SIZE);
    m_bmp = wxBitmap(size_x, size_y);
    mdc.SelectObject(m_bmp);
    wxGCDC dc(mdc);
    // Draw stuff
    dc.SetBrush(wxBrush(cbb));
    dc.DrawRectangle(0, 0, size_x, size_y);
    dc.SetBrush(wxBrush(ctb));
    dc.DrawRectangle(0, 0, size_x, title_y + 2 * BORDER_SIZE);
    dc.SetFont(tf);
    dc.SetTextForeground(ctf);
    dc.DrawText(m_title, (size_x - title_x) / 2, BORDER_SIZE);
    dc.SetFont(bf);
    dc.SetTextForeground(cbf);
    dc.DrawText(value, (size_x - body_x) / 2, title_y + 3 * BORDER_SIZE);
    dc.SetPen(wxPen(cb, BORDER_LINE_WIDTH, wxPENSTYLE_SOLID));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(BORDER_LINE_WIDTH / 2, BORDER_LINE_WIDTH / 2,
        size_x - BORDER_LINE_WIDTH, size_y - BORDER_LINE_WIDTH);
    // Done drawing
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
}

void SimpleNumberInstrument::ReadConfig(wxJSONValue& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.HasMember(b)) {                                                 \
        SetSetting(b, config[b].g());                                          \
    }
    DSK_SNI_SETTINGS
#undef X
}

wxJSONValue SimpleNumberInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    wxJSONValue v = Instrument::GenerateJSONConfig();
    // my own parameters
#define X(a, b, c, d, e, f, g, h)                                              \
    if (!wxString(b).IsSameAs(DSK_SETTING_ZONES)) {                            \
        v[b] = h(b);                                                           \
    }
    DSK_SNI_SETTINGS
#undef X
    return v;
}

double SimpleNumberInstrument::Transform(const double& val)
{
    return Instrument::Transform(val, m_transformation);
}

const wxColor SimpleNumberInstrument::GetColor(
    const double& val, const color_item item)
{
    wxColor c;
    switch (item) {
    case color_item::title_bg:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_TITLE_BG),
            GetColorSetting(DSK_SETTING_TITLE_BG),
            GetColorSetting(DSK_SETTING_TITLE_BG),
            GetColorSetting(DSK_SETTING_TITLE_BG),
            GetColorSetting(DSK_SETTING_TITLE_BG),
            GetColorSetting(DSK_SETTING_TITLE_BG));
        break;
    case color_item::title_fg:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG));
        break;
    case color_item::body_bg:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_BODY_BG),
            GetColorSetting(DSK_SETTING_BODY_BG),
            GetColorSetting(DSK_SETTING_ALERT_BG),
            GetColorSetting(DSK_SETTING_WARN_BG),
            GetColorSetting(DSK_SETTING_ALRM_BG),
            GetColorSetting(DSK_SETTING_EMERG_BG));
        break;
    case color_item::body_fg:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_BODY_FG),
            GetColorSetting(DSK_SETTING_BODY_FG),
            GetColorSetting(DSK_SETTING_ALERT_FG),
            GetColorSetting(DSK_SETTING_WARN_FG),
            GetColorSetting(DSK_SETTING_ALRM_FG),
            GetColorSetting(DSK_SETTING_EMERG_FG));
        break;
    case color_item::border:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_BORDER_COLOR),
            GetColorSetting(DSK_SETTING_BORDER_COLOR),
            GetColorSetting(DSK_SETTING_BORDER_COLOR),
            GetColorSetting(DSK_SETTING_BORDER_COLOR),
            GetColorSetting(DSK_SETTING_BORDER_COLOR),
            GetColorSetting(DSK_SETTING_BORDER_COLOR));
        break;
    }
    return c;
}

PLUGIN_END_NAMESPACE
