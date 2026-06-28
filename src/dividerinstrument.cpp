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

#include "dividerinstrument.h"
#include "dashboard.h"
#include "instrument.h"

PLUGIN_BEGIN_NAMESPACE

void DividerInstrument::Init()
{
    // Define the orientation and line style labels to be shared between
    // settings GUI and instrument logic (We need it to be consistent)
#define X(a, b) m_supported_orientations.Add(b);
    DSK_DIV_ORIENTATIONS
#undef X
#define X(a, b) m_supported_styles.Add(b);
    DSK_DIV_LINE_STYLES
#undef X
#define X(a, b) m_style_values.push_back(a);
    DSK_DIV_LINE_STYLES
#undef X

    // Basic settings inherited from Instrument class
    m_title = wxEmptyString;
    m_name = _("New Divider");
    // DividerInstrument's own settings
    m_needs_redraw = true;
    m_orientation = orientation::automatic;
    m_length = 100;
    m_line_width = 2;
    m_line_style = 0;
    m_rendered_vertical = false;

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_DIV_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_DIV_SETTINGS
#undef X
}

void DividerInstrument::SetSetting(const wxString& key, const wxString& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_ORIENTATION)
        || key.IsSameAs(DSK_SETTING_LENGTH)
        || key.IsSameAs(DSK_SETTING_LINE_WIDTH)
        || key.IsSameAs(DSK_SETTING_LINE_STYLE)) {
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

void DividerInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_ORIENTATION)) {
        m_orientation = static_cast<orientation>(value);
        m_needs_redraw = true;
    } else if (key.IsSameAs(DSK_SETTING_LENGTH)) {
        m_length = value;
        m_needs_redraw = true;
    } else if (key.IsSameAs(DSK_SETTING_LINE_WIDTH)) {
        m_line_width = value;
        m_needs_redraw = true;
    } else if (key.IsSameAs(DSK_SETTING_LINE_STYLE)) {
        m_line_style = value;
        m_needs_redraw = true;
    }
}

bool DividerInstrument::IsVertical() const
{
    switch (m_orientation) {
    case orientation::horizontal:
        return false;
    case orientation::vertical:
        return true;
    default:
        // Automatic: the divider is drawn perpendicular to the flow of the
        // instruments in the dashboard. Dashboards anchored to the top or
        // bottom edge of the canvas lay out their instruments horizontally.
        if (m_parent_dashboard) {
            Dashboard::anchor_edge edge = m_parent_dashboard->GetAnchorEdge();
            return (edge == Dashboard::anchor_edge::top
                || edge == Dashboard::anchor_edge::bottom);
        }
        return false;
    }
}

wxPenStyle DividerInstrument::GetPenStyle() const
{
    if (m_line_style >= 0
        && m_line_style < static_cast<int>(m_style_values.size())) {
        return m_style_values[m_line_style];
    }
    return wxPENSTYLE_SOLID;
}

wxBitmap DividerInstrument::Render(double scale)
{
    bool vertical = IsVertical();
    if (!m_needs_redraw && vertical == m_rendered_vertical) {
        return m_bmp;
    }
    m_needs_redraw = false;
    m_rendered_vertical = vertical;

    wxCoord length = wxMax(1, m_length * scale);
    wxCoord thickness = wxMax(1, m_line_width * scale);
    wxCoord size_x = vertical ? thickness : length;
    wxCoord size_y = vertical ? length : thickness;
#if defined(__WXGTK__) || defined(__WXQT__)
    m_bmp = wxBitmap(size_x, size_y, 32);
#else
    m_bmp = wxBitmap(size_x, size_y);
    m_bmp.UseAlpha();
#endif
    wxMemoryDC mdc;
    mdc.SelectObject(m_bmp);
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC dc(mdc);
#else
    wxMemoryDC& dc(mdc);
#endif
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    // Draw stuff
    wxColor c = GetDimedColor(GetColorSetting(DSK_SETTING_LINE_COLOR));
    dc.SetPen(wxPen(c, thickness, GetPenStyle()));
    if (vertical) {
        dc.DrawLine(size_x / 2, 0, size_x / 2, size_y);
    } else {
        dc.DrawLine(0, size_y / 2, size_x, size_y / 2);
    }
    // Done drawing
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
}

void DividerInstrument::ReadConfig(Json::Value& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.isMember(b)) {                                                  \
        SetSetting(b, fromJsonVal(config[b].g()));                             \
    }
    DSK_DIV_SETTINGS
#undef X
}

Json::Value DividerInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    Json::Value v = Instrument::GenerateJSONConfig();
    // my own parameters
#define X(a, b, c, d, e, f, g, h) v[b] = toJson(h(b));
    DSK_DIV_SETTINGS
#undef X
    return v;
}

PLUGIN_END_NAMESPACE
