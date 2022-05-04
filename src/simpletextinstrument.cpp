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

#include "simpletextinstrument.h"
#include "dashboard.h"
#include "instrument.h"
#include <limits>

PLUGIN_BEGIN_NAMESPACE

void SimpleTextInstrument::Init()
{
    // Basic settings inherited from Instrument class
    m_title = "???";
    m_name = _("New Simple Text");
    m_sk_key = wxEmptyString;
    // SimpleTextInstrument's own settings
    m_timed_out = false;
    m_needs_redraw = true;
    m_title_font = wxFont(
        10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_body_font
        = wxFont(15, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_STI_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_STI_SETTINGS
#undef X
}

void SimpleTextInstrument::SetSetting(
    const wxString& key, const wxString& value)
{
    Instrument::SetSetting(key, value);
    if (key == DSK_SETTING_SK_KEY && !m_sk_key.IsSameAs(value)) {
        m_sk_key = wxString(value);
        if (m_parent_dashboard) {
            m_parent_dashboard->Unsubscribe(this);
            m_parent_dashboard->Subscribe(m_sk_key, this);
        }
    } else if (key.IsSameAs(DSK_SETTING_BODY_FONT)
        || key.IsSameAs(DSK_SETTING_TITLE_FONT)) {
        // TODO: The above manually maintained list should be replaced with
        // something using the information from the DSK_STI_SETTINGS macro
        int i = 0;
#if (wxCHECK_VERSION(3, 1, 6))
        value.ToInt(&i);
#else
        i = wxAtoi(value);
#endif
        SetSetting(key, i);
    }
}

void SimpleTextInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_TITLE_FONT)) {
        m_title_font.SetPointSize(value);
    } else if (key.IsSameAs(DSK_SETTING_BODY_FONT)) {
        m_body_font.SetPointSize(value);
    }
}

wxBitmap SimpleTextInstrument::Render(double scale)
{
    wxString value = "----";
    const wxJSONValue* val = m_parent_dashboard->GetSKData(m_sk_key);
    if (val) {
        wxJSONValue v = val->Get("value", wxJSONValue("----"));
        value = v.AsString();
    }

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
    size_x = (wxMax(title_x + 2 * BORDER_SIZE, body_x) + 4 * BORDER_SIZE);
    size_y = (title_y + body_y + 3 * BORDER_SIZE);
    m_bmp = wxBitmap(size_x, size_y);
#ifndef __WXGTK__
    m_bmp.UseAlpha();
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

void SimpleTextInstrument::ReadConfig(wxJSONValue& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.HasMember(b)) {                                                 \
        SetSetting(b, config[b].g());                                          \
    }
    DSK_STI_SETTINGS
#undef X
}

wxJSONValue SimpleTextInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    wxJSONValue v = Instrument::GenerateJSONConfig();
    // my own parameters
#define X(a, b, c, d, e, f, g, h) v[b] = h(b);
    DSK_STI_SETTINGS
#undef X
    return v;
}

const wxColor SimpleTextInstrument::GetColor(
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
