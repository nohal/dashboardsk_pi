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

#include "simplepositioninstrument.h"
#include "dashboard.h"
#include "instrument.h"
#include <limits>

PLUGIN_BEGIN_NAMESPACE

void SimplePositionInstrument::Init()
{
#define X(a, b) m_supported_formats.Add(b);
    DSK_POSITION_FORMATS
#undef X
    // Basic settings inherited from Instrument class
    m_title = DUMMY_TITLE;
    m_name = _("New Simple Position");
    m_sk_key = wxEmptyString;
    // SimplePositionInstrument's own settings
    m_timed_out = false;
    m_needs_redraw = true;
    m_title_font = wxFont(
        10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_body_font
        = wxFont(15, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_SPI_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_SPI_SETTINGS
#undef X
}

void SimplePositionInstrument::SetSetting(
    const wxString& key, const wxString& value)
{
    Instrument::SetSetting(key, value);
    if (key == DSK_SETTING_SK_KEY && !m_sk_key.IsSameAs(value)) {
        m_sk_key = wxString(value);
        if (m_sk_key.EndsWith("latitude") || m_sk_key.EndsWith("longitude")) {
            m_sk_key = m_sk_key.BeforeLast('.');
        }
        if (m_parent_dashboard) {
            m_parent_dashboard->Unsubscribe(this);
            m_parent_dashboard->Subscribe(m_sk_key, this);
        }
    } else if (key.IsSameAs(DSK_SETTING_BODY_FONT)
        || key.IsSameAs(DSK_SETTING_TITLE_FONT)
        || key.IsSameAs(DSK_SETTING_FORMAT)) {
        // TODO: The above manually maintained list should be replaced with
        // something using the information from the DSK_SPI_SETTINGS macro
        int i;
#if (wxCHECK_VERSION(3, 1, 6))
        if (!value.ToInt(&i)) {
            i = 0;
        }
#else
        i = wxAtoi(value);
#endif
        SetSetting(key, i);
    }
}

void SimplePositionInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_TITLE_FONT)) {
        m_title_font.SetPointSize(value);
    } else if (key.IsSameAs(DSK_SETTING_BODY_FONT)) {
        m_body_font.SetPointSize(value);
    } else if (key.IsSameAs(DSK_SETTING_FORMAT)) {
        m_format = static_cast<position_format>(value);
    }
}

wxBitmap SimplePositionInstrument::Render(double scale)
{
    wxString value = "----, ----";
    if (!m_new_data) {
        if (!m_timed_out
            && (m_allowed_age_sec > 0
                && std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now() - m_last_change)
                        .count()
                    > m_allowed_age_sec)) {
            m_needs_redraw = true;
            m_timed_out = true;
        }
    } else {
        m_new_data = false;
        m_needs_redraw = true;
        m_last_change = std::chrono::system_clock::now();
        m_timed_out = false;
        const wxJSONValue* val = m_parent_dashboard->GetSKData(m_sk_key);
        if (val) {
            wxJSONValue v = *val;
            if (v.HasMember("latitude") && v.HasMember("longitude")) {
                m_last_change = std::chrono::system_clock::now();
                double lat = v["latitude"]["value"].AsDouble();
                double lon = v["longitude"]["value"].AsDouble();
                switch (m_format) {
                case Instrument::position_format::deg_decimal_min: {
                    value = wxString::Format(
                        "%.0f\u00B0%06.03f'%s, %.0f\u00B0%06.03f'%s",
                        floor(abs(lat)), (abs(lat) - floor(abs(lat))) * 60,
                        lat >= 0 ? "N" : "S", floor(abs(lon)),
                        (abs(lon) - floor(abs(lon))) * 60,
                        lon >= 0 ? "E" : "W");
                    break;
                }
                case Instrument::position_format::deg_min_sec: {
                    value = wxString::Format(
                        "%.0f\u00B0%.0f'%.0f\"%s, %.0f\u00B0%.0f'%.0f\"%s",
                        floor(abs(lat)), (abs(lat) - floor(abs(lat))) * 60,
                        ((abs(lat) - floor(abs(lat))) * 60
                            - (abs(lat) - floor(abs(lat))) * 60)
                            * 60,
                        lat >= 0 ? "N" : "S", floor(abs(lon)),
                        (abs(lon) - floor(abs(lon))) * 60,
                        ((abs(lon) - floor(abs(lon))) * 60
                            - (abs(lon) - floor(abs(lon))) * 60)
                            * 60,
                        lon >= 0 ? "E" : "W");
                    break;
                }
                case Instrument::position_format::decimal_deg_hem: {
                    value = wxString::Format("%08.5f %s, %09.5f %s", abs(lat),
                        lat >= 0 ? "N" : "S", abs(lon), lon >= 0 ? "E" : "W");
                    break;
                }
                case Instrument::position_format::hem_decimal_deg: {
                    value = wxString::Format("%s %08.5f, %s %09.5f",
                        lat >= 0 ? "N" : "S", abs(lat), lon >= 0 ? "E" : "W",
                        abs(lon));
                    break;
                }
                case Instrument::position_format::hem_deg_decimal_min: {
                    value = wxString::Format(
                        "%s %.0f\u00B0%06.03f', %s %.0f\u00B0%06.03f'",
                        lat >= 0 ? "N" : "S", floor(abs(lat)),
                        (abs(lat) - floor(abs(lat))) * 60, lon >= 0 ? "E" : "W",
                        floor(abs(lon)), (abs(lon) - floor(abs(lon))) * 60);
                    break;
                }
                case Instrument::position_format::hem_deg_min_sec: {
                    value = wxString::Format(
                        "%s %.0f\u00B0%.0f'%.0f\", %s %i\u00B0%.0f'%.0f\"",
                        lat >= 0 ? "N" : "S", floor(abs(lat)),
                        (abs(lat) - floor(abs(lat))) * 60,
                        ((abs(lat) - floor(abs(lat))) * 60
                            - (abs(lat) - floor(abs(lat))) * 60)
                            * 60,
                        lon >= 0 ? "E" : "W", floor(abs(lon)),
                        (abs(lon) - floor(abs(lon))) * 60,
                        ((abs(lon) - floor(abs(lon))) * 60
                            - (abs(lon) - floor(abs(lon))) * 60)
                            * 60);
                    break;
                }
                default: {
                    value = wxString::Format("%08.5f, %09.5f", lat, lon);
                }
                }
            }
        }
    }

    if (!m_needs_redraw) {
        return m_bmp;
    }
    m_needs_redraw = false;

    wxColor ctb = GetDimedColor(GetColorSetting(DSK_SETTING_TITLE_BG));
    wxColor ctf = GetDimedColor(GetColorSetting(DSK_SETTING_TITLE_FG));
    wxColor cbb = GetDimedColor(GetColorSetting(DSK_SETTING_BODY_BG));
    wxColor cbf = GetDimedColor(GetColorSetting(DSK_SETTING_BODY_FG));
    wxColor cb = GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR));
    wxCoord title_x, title_y;
    wxCoord body_x, body_y;
    wxCoord size_x, size_y;

    wxMemoryDC mdc;

    wxFont tf = m_title_font;
    tf.SetPointSize(m_title_font.GetPointSize() * scale);
    mdc.SetFont(tf);
    mdc.GetTextExtent(m_title, &title_x, &title_y);
    wxFont bf = m_body_font;
    bf.SetPointSize(m_body_font.GetPointSize() * scale);
    mdc.SetFont(bf);
    mdc.GetTextExtent(value, &body_x, &body_y);
    size_x = (wxMax(title_x + 3 * BORDER_SIZE, body_x) + 4 * BORDER_SIZE);
    size_y = (title_y + body_y + 3 * BORDER_SIZE);
#ifndef __WXGTK__
    m_bmp = wxBitmap(size_x, size_y);
    m_bmp.UseAlpha();
#else
    m_bmp = wxBitmap(size_x, size_y, 32);
#endif
    mdc.SelectObject(m_bmp);
    wxGCDC dc(mdc);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    // Draw stuff
    dc.SetPen(wxPen(cb, BORDER_LINE_WIDTH));
    dc.SetBrush(wxBrush(ctb));
    dc.DrawRoundedRectangle(title_y, BORDER_LINE_WIDTH / 2,
        title_x + 2 * BORDER_SIZE, title_y * 2, -0.5);
    dc.SetBrush(wxBrush(cbb));
    dc.DrawRoundedRectangle(BORDER_LINE_WIDTH / 2,
        title_y + BORDER_SIZE + BORDER_LINE_WIDTH / 2,
        body_x + 4 * BORDER_SIZE - BORDER_LINE_WIDTH,
        body_y + 2 * BORDER_SIZE - BORDER_LINE_WIDTH, -0.5);
    dc.SetFont(tf);
    dc.SetTextForeground(ctf);
    dc.DrawText(m_title, title_y + BORDER_SIZE, BORDER_SIZE);
    dc.SetFont(bf);
    dc.SetTextForeground(cbf);
    dc.DrawText(value, 2 * BORDER_SIZE, title_y + 2 * BORDER_SIZE);
    // Done drawing
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
}

void SimplePositionInstrument::ReadConfig(wxJSONValue& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.HasMember(b)) {                                                 \
        SetSetting(b, config[b].g());                                          \
    }
    DSK_SPI_SETTINGS
#undef X
}

wxJSONValue SimplePositionInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    wxJSONValue v = Instrument::GenerateJSONConfig();
    // my own parameters
#define X(a, b, c, d, e, f, g, h) v[b] = h(b);
    DSK_SPI_SETTINGS
#undef X
    return v;
}

const wxColor SimplePositionInstrument::GetColor(
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
            GetColorSetting(DSK_SETTING_BODY_BG),
            GetColorSetting(DSK_SETTING_BODY_BG),
            GetColorSetting(DSK_SETTING_BODY_BG),
            GetColorSetting(DSK_SETTING_BODY_BG));
        break;
    case color_item::body_fg:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_BODY_FG),
            GetColorSetting(DSK_SETTING_BODY_FG),
            GetColorSetting(DSK_SETTING_BODY_FG),
            GetColorSetting(DSK_SETTING_BODY_FG),
            GetColorSetting(DSK_SETTING_BODY_FG),
            GetColorSetting(DSK_SETTING_BODY_FG));
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
