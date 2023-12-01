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

#include "simplegaugeinstrument.h"
#include "dashboard.h"
#include "instrument.h"
#include <limits>

PLUGIN_BEGIN_NAMESPACE

void SimpleGaugeInstrument::Init()
{
#define X(a, b, c) m_supported_formats.Add(b);
    DSK_VALUE_FORMATS
#undef X
#define X(a, b, c) m_format_strings.Add(c);
    DSK_VALUE_FORMATS
#undef X
#define X(a, b) m_supported_transforms.Add(b);
    DSK_UNIT_TRANSFORMATIONS
#undef X
#define X(a, b) m_gauge_types.Add(b);
    DSK_SGI_GAUGE_TYPES
#undef X
    // Instrument::Init();
    //  Basic settings inherited from Instrument class
    m_title = DUMMY_TITLE;
    m_name = _("New Simple Gauge");
    m_sk_key = wxEmptyString;
    // SimpleGaugeInstrument's own settings
    m_timed_out = false;
    m_needs_redraw = true;
    m_smoothing = 0;
    m_old_value = std::numeric_limits<double>::min();
    m_instrument_size = 200;
    m_gauge_type = gauge_type::relative_angle;
    m_max_val = std::numeric_limits<double>::min();
    m_min_val = std::numeric_limits<double>::max();

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_SGI_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_SGI_SETTINGS
#undef X
}

void SimpleGaugeInstrument::SetSetting(
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
        || key.IsSameAs(DSK_SETTING_INSTR_SIZE)
        || key.IsSameAs(DSK_SGI_GAUGE_TYPE)) {
        // TODO: The above manually maintained list should be replaced with
        // something using the information from the DSK_SNI_SETTINGS macro
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

void SimpleGaugeInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_FORMAT)) {
        m_format_index = value;
    } else if (key.IsSameAs(DSK_SETTING_TRANSFORMATION)) {
        m_transformation = static_cast<transformation>(value);
    } else if (key.IsSameAs(DSK_SETTING_INSTR_SIZE)) {
        m_instrument_size = value;
    } else if (key.IsSameAs(DSK_SGI_GAUGE_TYPE)) {
        m_gauge_type = static_cast<gauge_type>(value);
    } else if (key.IsSameAs(DSK_SETTING_SMOOTHING)) {
        m_smoothing = value;
    }
}

// Counterclockwise
void SimpleGaugeInstrument::DrawArc(wxDC& dc, const int& start_angle,
    const int& end_angle, const wxCoord& xc, const wxCoord& yc,
    const wxCoord& r)
{
    wxCoord xStart = xc + cos(deg2rad(270 + start_angle)) * r;
    wxCoord yStart = yc + sin(deg2rad(270 + start_angle)) * r;
    wxCoord xEnd = xc + cos(deg2rad(270 + end_angle)) * r;
    wxCoord yEnd = yc + sin(deg2rad(270 + end_angle)) * r;

    dc.DrawArc(xStart, yStart, xEnd, yEnd, xc, yc);
}

void SimpleGaugeInstrument::DrawTicks(wxDC& dc, const int& start_angle,
    const int& angle_step, const wxCoord& xc, const wxCoord& yc,
    const wxCoord& r, const wxCoord& length, bool labels, int except_every,
    bool relative, int draw_from, int draw_to, int labels_from, int labels_step)
{
    int angle = start_angle;
    wxCoord xStart;
    wxCoord yStart;
    wxCoord xEnd;
    wxCoord yEnd;
    double c, s;
    int tick = 0;
    while (angle <= 360) {
        if (angle >= draw_from && angle <= draw_to) {
            c = cos(deg2rad(270 + angle));
            s = sin(deg2rad(270 + angle));
            xStart = xc + c * r;
            yStart = yc + s * r;
            xEnd = xc + c * (r - length);
            yEnd = yc + s * (r - length);
            dc.DrawLine(xStart, yStart, xEnd, yEnd);
            xEnd = xc + c * (r - length * 1.15);
            yEnd = yc + s * (r - length * 1.15);
            if (labels && angle < 360
                && (except_every == 0 || angle % except_every != 0)) {
                int lbl_val;
                if (labels_step != 0) { // Custom labeling
                    lbl_val = labels_from + tick * labels_step;
                    tick++;
                } else { // Angle labels
                    lbl_val = (relative && angle > 180) ? 180 - (angle - 180)
                                                        : angle;
                }
                wxString val = wxString::Format("%i", lbl_val);
                wxCoord x = xEnd
                    - cos(deg2rad(angle)) * dc.GetTextExtent(val).GetX() / 2;
                wxCoord y = yEnd
                    - sin(deg2rad(angle)) * dc.GetTextExtent(val).GetX() / 2;
                dc.DrawRotatedText(val, x, y, -angle);
            }
        }
        angle += angle_step;
    }
}

void SimpleGaugeInstrument::DrawNeedle(wxDC& dc, const wxCoord& xc,
    const wxCoord& yc, const wxCoord& r, const wxCoord& angle,
    const int& perc_length, const int& perc_width, const int& start_angle)
{
    wxPoint needle[3] {
        wxPoint(
            r * (100 - perc_length) / 100 * cos(deg2rad(start_angle + angle))
                - cos(deg2rad(angle - 270 + start_angle)) * r * perc_width
                    / 200,
            r * (100 - perc_length) / 100 * sin(deg2rad(start_angle + angle))
                - sin(deg2rad(angle - 270 + start_angle)) * r * perc_width
                    / 200),
        wxPoint(
            r * (100 - perc_length) / 100 * cos(deg2rad(start_angle + angle))
                + cos(deg2rad(angle - 270 + start_angle)) * r * perc_width
                    / 200,
            r * (100 - perc_length) / 100 * sin(deg2rad(start_angle + angle))
                + sin(deg2rad(angle - 270 + start_angle)) * r * perc_width
                    / 200),
        wxPoint(r * cos(deg2rad(start_angle + angle)),
            r * sin(deg2rad(start_angle + angle)))
    };
    dc.DrawPolygon(3, needle, xc, yc);
}

wxBitmap SimpleGaugeInstrument::RenderAngle(double scale, bool relative)
{
    wxString value = "---";

    if (m_new_data) {
        m_new_data = false;
        if (!m_timed_out) {
            value = wxString::Format(
                m_format_strings[m_format_index], abs(m_old_value));
            if (m_old_value < 0
                && !m_supported_formats[m_format_index].StartsWith("ABS")) {
                value.Prepend("-");
            }
        }
    } else {
        if (!m_timed_out && m_bmp.IsOk()) {
            return m_bmp;
        }
    }

    wxMemoryDC mdc;

    wxCoord size_x = m_instrument_size * scale;
    wxCoord size_y = m_instrument_size * scale;
    wxCoord xc = size_x / 2;
    wxCoord yc = size_y / 2;
    wxCoord r = size_y / 2 - size_x / 200 - 1;

#if defined(__WXGTK__) || defined(__WXQT__)
    m_bmp = wxBitmap(size_x, size_y, 32);
#else
    m_bmp = wxBitmap(size_x, size_y);
    m_bmp.UseAlpha();
#endif
    mdc.SelectObject(m_bmp);
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC dc(mdc);
#else
    wxMemoryDC& dc(mdc);
#endif
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();

    // Gauge background
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_RIM_NOMINAL))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR))));
    dc.DrawCircle(xc, yc, r);
    if (relative) {
        // Arcs
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_RIM_DEAD))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_RIM_DEAD))));
        DrawArc(dc, 15, -15, xc, yc, r);
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_RIM_STBD))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_RIM_STBD))));
        DrawArc(dc, 60, 15, xc, yc, r);
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_RIM_PORT))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_RIM_PORT))));
        DrawArc(dc, -15, -60, xc, yc, r);
    }
    // Face
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.DrawCircle(xc, yc, r * 0.85);
    // Ticks
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_SGI_TICK_LEGEND)));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 200));
    DrawTicks(dc, 0, 30, xc, yc, r, r * 0.15, true, 90, relative);
    DrawTicks(dc, 0, 10, xc, yc, r, r * 0.1);
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 100));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    DrawTicks(dc, 0, 90, xc, yc, r, r * 0.2, true, 0, relative);
    // Border
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR)),
        size_x / 100 + 1));
    dc.DrawCircle(xc, yc, r);
    // Needle
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG)), 3));
    DrawNeedle(dc, xc, yc, r * 0.9, m_old_value, 30);
    // Text
    // Label
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::title)));
    dc.SetFont(wxFont(size_x / 8 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText(m_title, xc - dc.GetTextExtent(m_title).GetX() / 2,
        yc - dc.GetTextExtent(m_title).GetY() * 1.5);
    // Data
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::value)));
    dc.SetFont(wxFont(size_x / 3 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.DrawText(value, xc - dc.GetTextExtent(value).GetX() / 2,
        yc
            - (wxCoord)round(
                  dc.GetTextExtent(value).GetY() * AUTO_TEXT_SHIFT_COEF)
                / 4);
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
}

wxBitmap SimpleGaugeInstrument::RenderAdaptive(double scale)
{
#define PERC 30
    wxString value = "----";
    bool has_value = false;

    if (m_new_data) {
        m_new_data = false;
        if (!m_timed_out) {
            has_value = true;
            value = wxString::Format(
                m_format_strings[m_format_index], abs(m_old_value));
            if (m_old_value < 0
                && !m_supported_formats[m_format_index].StartsWith("ABS")) {
                value.Prepend("-");
            }
        }
    } else {
        if (!m_timed_out && m_bmp.IsOk()) {
            return m_bmp;
        }
    }

    wxCoord size_x = m_instrument_size * scale;
    wxCoord size_y = m_instrument_size * scale * (50 + PERC) / 100;
    wxCoord xc = size_x / 2;
    wxCoord yc = m_instrument_size * scale / 2;
    wxCoord r = size_x / 2 - size_x / 200 - 1;

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
    // Gauge background
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_RIM_NOMINAL))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR))));
    dc.DrawCircle(xc, yc, r);

    int magnitude = -3;
    int upper = 0;
    int lower = 0;
    int step = 0;
    if (has_value) {
        double range = m_max_val - m_min_val;
        while (range / pow(10, magnitude) >= 10) {
            magnitude++;
        }

        upper
            = ceil(m_max_val / pow(10, magnitude + 1)) * pow(10, magnitude + 1);
        lower = floor(m_min_val / pow(10, magnitude + 1))
            * pow(10, magnitude + 1);

        step = (upper - lower) / pow(10, magnitude) / 5;
        if (step == 0) {
            ++step;
        }
        upper = lower + 6 * pow(10, magnitude) * step;
    } else {
        magnitude = 0;
    }
    if (upper == lower) {
        upper++;
    }
    // Arcs for zones
    for (auto& zone : m_zones) {
        int angle_from = (wxMax(zone.GetLowerLimit(), lower) - lower) * 240
                / (upper - lower)
            - 120;
        int angle_to = (wxMin(upper, zone.GetUpperLimit()) - lower) * 240
                / (upper - lower)
            - 120;
        wxString zone_color;
        switch (zone.GetState()) {
        case Zone::state::nominal:
            zone_color = DSK_SETTING_NOMINAL_FG;
            break;
        case Zone::state::normal:
            zone_color = DSK_SETTING_NORMAL_FG;
            break;
        case Zone::state::alert:
            zone_color = DSK_SETTING_ALERT_FG;
            break;
        case Zone::state::warn:
            zone_color = DSK_SETTING_WARN_FG;
            break;
        case Zone::state::alarm:
            zone_color = DSK_SETTING_ALRM_FG;
            break;
        case Zone::state::emergency:
            zone_color = DSK_SETTING_EMERG_FG;
            break;
        }
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(zone_color))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(zone_color))));
        DrawArc(dc, angle_to, angle_from, xc, yc, r);
    }
    // Face
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.DrawCircle(xc, yc, r * 0.85);
    // Ticks
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_SGI_TICK_LEGEND)));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 200));
    DrawTicks(dc, 0, 20, xc, yc, r, r * 0.15, false, 90, false, 0, 120);
    DrawTicks(dc, 0, 10, xc, yc, r, r * 0.1, false, 90, false, 0, 120);
    DrawTicks(dc, 0, 20, xc, yc, r, r * 0.15, false, 90, false, 240, 360);
    DrawTicks(dc, 0, 10, xc, yc, r, r * 0.1, false, 90, false, 240, 360);
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 100));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    if (has_value) {
        DrawTicks(dc, 0, 40, xc, yc, r, r * 0.2, true, 0, false, 0, 120,
            lower / pow(10, magnitude) + 3 * step,
            step); // TODO: Labels have to be adapted to the value scale
        DrawTicks(dc, 0, 40, xc, yc, r, r * 0.2, true, 0, false, 240, 360,
            lower / pow(10, magnitude),
            step); // TODO: Labels may have to be further adapted to the value
                   // scale
    }
    // Border
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    int border_width = size_x / 100 + 1;
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR)),
        border_width));
    dc.DrawCircle(xc, yc, r);
    int shift = r - sqrt(r * r - r * 2 * PERC * r * 2 * PERC / 10000);
    dc.DrawLine(shift + border_width, size_y - border_width / 2,
        size_x - shift - border_width, size_y - border_width / 2);
    // Needle
    if (has_value) {
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG)), 3));
        DrawNeedle(dc, xc, yc, r * 0.9,
            (m_old_value - lower) * 240 / (upper - lower) - 90, 30, 20, 240);
    }
    // Text
    // Scale
    wxString sscale;
    if (magnitude >= 0) {
        sscale = wxString::Format("x%.0f", powf(10, magnitude));
    } else {
        sscale = wxString::Format("/%.0f", powf(10, -magnitude));
    }
    dc.DrawText(sscale, xc - dc.GetTextExtent(sscale).GetX() / 2, r * 0.5);
    // Label
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::title)));
    dc.SetFont(wxFont(size_x / 8 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText(m_title, xc - dc.GetTextExtent(m_title).GetX() / 2,
        yc - dc.GetTextExtent(m_title).GetY() * 1.1);
    // Data
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::value)));
    dc.SetFont(wxFont(size_x / 3 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.DrawText(value, xc - dc.GetTextExtent(value).GetX() / 2,
        yc / AUTO_TEXT_SHIFT_COEF);
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
#undef PERC
}

wxBitmap SimpleGaugeInstrument::RenderFixed(double scale)
{
#define PERC 30
    wxString value = "----";
    bool has_value = false;

    if (m_new_data) {
        m_new_data = false;
        if (!m_timed_out) {
            has_value = true;
            value = wxString::Format(
                m_format_strings[m_format_index], abs(m_old_value));
            if (m_old_value < 0
                && !m_supported_formats[m_format_index].StartsWith("ABS")) {
                value.Prepend("-");
            }
        }
    } else {
        if (!m_timed_out && m_bmp.IsOk()) {
            return m_bmp;
        }
    }

    wxCoord size_x = m_instrument_size * scale;
    wxCoord size_y = m_instrument_size * scale * (50 + PERC) / 100;
    wxCoord xc = size_x / 2;
    wxCoord yc = m_instrument_size * scale / 2;
    wxCoord r = size_x / 2 - size_x / 200 - 1;

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
    // Gauge background
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_RIM_NOMINAL))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR))));
    dc.DrawCircle(xc, yc, r);

    int magnitude = 0;
    int upper = 1;
    int lower = 0;
    int step = 0;

    double zone_lowest = 9999.0;
    double zone_highest = -9999.0;
    for (auto& zone : m_zones) {
        if (zone.GetLowerLimit() < zone_lowest) {
            zone_lowest = zone.GetLowerLimit();
        }
        if (zone.GetUpperLimit() > zone_highest) {
            zone_highest = zone.GetUpperLimit();
        }
    }
    double range = abs(zone_highest - zone_lowest);
    while (range / pow(10, magnitude) >= 10) {
        magnitude++;
    }
    magnitude--;
    upper = ceil(zone_highest);

    lower = floor(zone_lowest);
    step = ceil((double)(upper - lower) / pow(10, magnitude) / 6);
    if (step == 0) {
        ++step;
    }
    upper = lower + 6 * step * pow(10, magnitude);
    // Arcs for zones
    for (auto& zone : m_zones) {
        int angle_from = (wxMax(zone.GetLowerLimit(), lower) - lower) * 240
                / (upper - lower)
            - 120;
        int angle_to = (wxMin(upper, zone.GetUpperLimit()) - lower) * 240
                / (upper - lower)
            - 120;
        wxString zone_color;
        switch (zone.GetState()) {
        case Zone::state::nominal:
            zone_color = DSK_SETTING_NOMINAL_FG;
            break;
        case Zone::state::normal:
            zone_color = DSK_SETTING_NORMAL_FG;
            break;
        case Zone::state::alert:
            zone_color = DSK_SETTING_ALERT_FG;
            break;
        case Zone::state::warn:
            zone_color = DSK_SETTING_WARN_FG;
            break;
        case Zone::state::alarm:
            zone_color = DSK_SETTING_ALRM_FG;
            break;
        case Zone::state::emergency:
            zone_color = DSK_SETTING_EMERG_FG;
            break;
        }
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(zone_color))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(zone_color))));
        DrawArc(dc, angle_to, angle_from, xc, yc, r);
    }
    // Face
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.DrawCircle(xc, yc, r * 0.85);
    // Ticks
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_SGI_TICK_LEGEND)));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 200));
    DrawTicks(dc, 0, 20, xc, yc, r, r * 0.15, false, 90, false, 0, 120);
    DrawTicks(dc, 0, 10, xc, yc, r, r * 0.1, false, 90, false, 0, 120);
    DrawTicks(dc, 0, 20, xc, yc, r, r * 0.15, false, 90, false, 240, 360);
    DrawTicks(dc, 0, 10, xc, yc, r, r * 0.1, false, 90, false, 240, 360);
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 100));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    DrawTicks(dc, 0, 40, xc, yc, r, r * 0.2, true, 0, false, 0, 120,
        lower / pow(10, magnitude) + 3 * step,
        step); // TODO: Labels have to be adapted to the value scale
    DrawTicks(dc, 0, 40, xc, yc, r, r * 0.2, true, 0, false, 240, 360,
        lower / pow(10, magnitude),
        step); // TODO: Labels may have to be further adapted to the value
               // scale
    // Border
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    int border_width = size_x / 100 + 1;
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR)),
        border_width));
    dc.DrawCircle(xc, yc, r);
    int shift = r - sqrt(r * r - r * 2 * PERC * r * 2 * PERC / 10000);
    dc.DrawLine(shift + border_width, size_y - border_width / 2,
        size_x - shift - border_width, size_y - border_width / 2);
    // Needle
    if (has_value && m_old_value >= lower && m_old_value <= upper) {
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG)), 3));
        DrawNeedle(dc, xc, yc, r * 0.9,
            (m_old_value - lower) * 240 / (upper - lower) - 90, 30, 20, 240);
    }
    // Text
    // Scale
    wxString sscale;
    if (magnitude >= 0) {
        sscale = wxString::Format("x%.0f", powf(10, magnitude));
    } else {
        sscale = wxString::Format("/%.0f", powf(10, -magnitude));
    }
    dc.DrawText(sscale, xc - dc.GetTextExtent(sscale).GetX() / 2, r * 0.5);
    // Label
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::title)));
    dc.SetFont(wxFont(size_x / 8 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText(m_title, xc - dc.GetTextExtent(m_title).GetX() / 2,
        yc - dc.GetTextExtent(m_title).GetY() * 1.1);
    // Data
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::value)));
    dc.SetFont(wxFont(size_x / 3 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.DrawText(value, xc - dc.GetTextExtent(value).GetX() / 2,
        yc / AUTO_TEXT_SHIFT_COEF);
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
#undef PERC
}

wxBitmap SimpleGaugeInstrument::RenderPercent(double scale)
{
#define PERC 10
    wxString value = "---";

    if (m_new_data) {
        m_new_data = false;
        if (!m_timed_out) {
            value = wxString::Format(
                m_format_strings[m_format_index], abs(m_old_value));
            if (m_old_value < 0
                && !m_supported_formats[m_format_index].StartsWith("ABS")) {
                value.Prepend("-");
            }
        }
    } else {
        if (!m_timed_out && m_bmp.IsOk()) {
            return m_bmp;
        }
    }

    wxMemoryDC mdc;

    wxCoord size_x = m_instrument_size * scale;
    wxCoord size_y = m_instrument_size * scale * (50 + PERC) / 100;
    wxCoord xc = size_x / 2;
    wxCoord yc = m_instrument_size * scale / 2;
    wxCoord r = size_x / 2 - size_x / 200 - 1;

#if defined(__WXGTK__) || defined(__WXQT__)
    m_bmp = wxBitmap(size_x, size_y, 32);
#else
    m_bmp = wxBitmap(size_x, size_y);
    m_bmp.UseAlpha();
#endif
    mdc.SelectObject(m_bmp);
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC dc(mdc);
#else
    wxMemoryDC& dc(mdc);
#endif
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    // Gauge background
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_RIM_NOMINAL))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR))));
    dc.DrawCircle(xc, yc, r);
    // Arcs for zones
    for (auto& zone : m_zones) {
        int angle_from = zone.GetLowerLimit() * 1.8 - 90;
        int angle_to = zone.GetUpperLimit() * 1.8 - 90;
        wxString zone_color;
        switch (zone.GetState()) {
        case Zone::state::nominal:
            zone_color = DSK_SETTING_NOMINAL_FG;
            break;
        case Zone::state::normal:
            zone_color = DSK_SETTING_NORMAL_FG;
            break;
        case Zone::state::alert:
            zone_color = DSK_SETTING_ALERT_FG;
            break;
        case Zone::state::warn:
            zone_color = DSK_SETTING_WARN_FG;
            break;
        case Zone::state::alarm:
            zone_color = DSK_SETTING_ALRM_FG;
            break;
        case Zone::state::emergency:
            zone_color = DSK_SETTING_EMERG_FG;
            break;
        }
        dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(zone_color))));
        dc.SetPen(wxPen(GetDimedColor(GetColorSetting(zone_color))));
        DrawArc(dc, angle_to, angle_from, xc, yc, r);
    }
    // Face
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_DIAL_COLOR))));
    dc.DrawCircle(xc, yc, r * 0.85);
    // Ticks
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_SGI_TICK_LEGEND)));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 200));
    DrawTicks(dc, 0, 18, xc, yc, r, r * 0.15, false, 90, false, 0, 90);
    DrawTicks(dc, 0, 9, xc, yc, r, r * 0.1, false, 90, false, 0, 90);
    DrawTicks(dc, 0, 18, xc, yc, r, r * 0.15, false, 90, false, 270, 360);
    DrawTicks(dc, 0, 9, xc, yc, r, r * 0.1, false, 90, false, 270, 360);
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 100));
    dc.SetFont(wxFont(size_x / 12 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    DrawTicks(dc, 0, 90, xc, yc, r, r * 0.2, false, 0, false, 0, 90);
    DrawTicks(dc, 0, 90, xc, yc, r, r * 0.2, false, 0, false, 270, 360);
    // Border
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    int border_width = size_x / 100 + 1;
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR)),
        border_width));
    dc.DrawCircle(xc, yc, r);
    int shift = r - sqrt(r * r - r * 2 * PERC * r * 2 * PERC / 100 / 100);
    dc.DrawLine(shift + border_width, size_y - border_width / 2,
        size_x - shift - border_width, size_y - border_width / 2);
    // Needle
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG)), 3));
    DrawNeedle(dc, xc, yc, r * 0.9, m_old_value * 1.8 - 90, 30);
    // Text
    // Label
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::title)));
    dc.SetFont(wxFont(size_x / 8 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText(m_title, xc - dc.GetTextExtent(m_title).GetX() / 2,
        yc - dc.GetTextExtent(m_title).GetY() * 2.2);
    // Data
    dc.SetTextForeground(
        GetDimedColor(GetColor(m_old_value, color_item::value)));
    dc.SetFont(wxFont(size_x / 3 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.DrawText(value, xc - dc.GetTextExtent(value).GetX() / 2,
        yc - dc.GetTextExtent(value).GetY() / 1.8 * AUTO_TEXT_SHIFT_COEF);
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
#undef PERC
}

void SimpleGaugeInstrument::ProcessData()
{
    if (!m_new_data) {
        if (!m_timed_out
            && (m_allowed_age_sec > 0
                && std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now() - m_last_change)
                        .count()
                    > m_allowed_age_sec)) {
            m_needs_redraw = true;
            m_timed_out = true;
            m_old_value = std::numeric_limits<double>::min();
        }
    } else {
        m_needs_redraw = true;
        m_last_change = std::chrono::system_clock::now();
        m_timed_out = false;
        const wxJSONValue* val = m_parent_dashboard->GetSKData(m_sk_key);
        if (val) {
            wxJSONValue v = *val;
            if (val->IsObject()) {
                v = v["value"];
            }

            double dval = Transform(v.IsDouble() ? v.AsDouble()
                    : v.IsLong()                 ? v.AsLong()
                                                 : 0.0,
                m_transformation);
            if (m_old_value > std::numeric_limits<double>::min()) {
                dval = (m_smoothing * m_old_value
                           + (DSK_SGI_SMOOTHING_MAX - m_smoothing + 1) * dval)
                    / (DSK_SGI_SMOOTHING_MAX + 1);
            }
            m_old_value = dval;
            m_min_val = wxMin(dval, m_min_val);
            m_max_val = wxMax(dval, m_max_val);
        }
    }
}

wxBitmap SimpleGaugeInstrument::Render(double scale)
{
    ProcessData();

    if (!m_needs_redraw) {
        return m_bmp;
    }
    switch (m_gauge_type) {
    case gauge_type::relative_angle:
        return RenderAngle(scale, true);
    case gauge_type::direction:
        return RenderAngle(scale, false);
    case gauge_type::percent:
        return RenderPercent(scale);
    case gauge_type::ranged_adaptive:
        return RenderAdaptive(scale);
    case gauge_type::ranged_fixed:
        return RenderFixed(scale);
    default:
        return wxNullBitmap;
    }
}

void SimpleGaugeInstrument::ReadConfig(wxJSONValue& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.HasMember(b)) {                                                 \
        SetSetting(b, config[b].g());                                          \
    }
    DSK_SGI_SETTINGS
#undef X
}

wxJSONValue SimpleGaugeInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    wxJSONValue v = Instrument::GenerateJSONConfig();
    // my own parameters
#define X(a, b, c, d, e, f, g, h)                                              \
    if (!wxString(b).IsSameAs(DSK_SETTING_ZONES)) {                            \
        v[b] = h(b);                                                           \
    }
    DSK_SGI_SETTINGS
#undef X
    return v;
}

const wxColor SimpleGaugeInstrument::GetColor(
    const double& val, const color_item item)
{
    wxColor c;
    switch (item) {
    case color_item::title:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG),
            GetColorSetting(DSK_SETTING_TITLE_FG));
        break;
    case color_item::value:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SETTING_NOMINAL_FG),
            GetColorSetting(DSK_SETTING_NOMINAL_FG),
            GetColorSetting(DSK_SETTING_NOMINAL_FG),
            GetColorSetting(DSK_SETTING_NOMINAL_FG),
            GetColorSetting(DSK_SETTING_NOMINAL_FG),
            GetColorSetting(DSK_SETTING_NOMINAL_FG));
        break;
    case color_item::dial:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SGI_DIAL_COLOR),
            GetColorSetting(DSK_SGI_DIAL_COLOR),
            GetColorSetting(DSK_SGI_DIAL_COLOR),
            GetColorSetting(DSK_SGI_DIAL_COLOR),
            GetColorSetting(DSK_SGI_DIAL_COLOR),
            GetColorSetting(DSK_SGI_DIAL_COLOR));
        break;
    case color_item::tick:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SGI_TICK_FG),
            GetColorSetting(DSK_SGI_TICK_FG), GetColorSetting(DSK_SGI_TICK_FG),
            GetColorSetting(DSK_SGI_TICK_FG), GetColorSetting(DSK_SGI_TICK_FG),
            GetColorSetting(DSK_SGI_TICK_FG));
        break;
    case color_item::legend:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SGI_TICK_LEGEND),
            GetColorSetting(DSK_SGI_TICK_LEGEND),
            GetColorSetting(DSK_SGI_TICK_LEGEND),
            GetColorSetting(DSK_SGI_TICK_LEGEND),
            GetColorSetting(DSK_SGI_TICK_LEGEND),
            GetColorSetting(DSK_SGI_TICK_LEGEND));
        break;
    case color_item::rim:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SGI_RIM_NOMINAL),
            GetColorSetting(DSK_SGI_RIM_NOMINAL),
            GetColorSetting(DSK_SGI_RIM_NOMINAL),
            GetColorSetting(DSK_SGI_RIM_NOMINAL),
            GetColorSetting(DSK_SGI_RIM_NOMINAL),
            GetColorSetting(DSK_SGI_RIM_NOMINAL));
        break;
    case color_item::rim_dead:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SGI_RIM_DEAD),
            GetColorSetting(DSK_SGI_RIM_DEAD),
            GetColorSetting(DSK_SGI_RIM_DEAD),
            GetColorSetting(DSK_SGI_RIM_DEAD),
            GetColorSetting(DSK_SGI_RIM_DEAD),
            GetColorSetting(DSK_SGI_RIM_DEAD));
        break;
    case color_item::rim_stbd:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SGI_RIM_STBD),
            GetColorSetting(DSK_SGI_RIM_STBD),
            GetColorSetting(DSK_SGI_RIM_STBD),
            GetColorSetting(DSK_SGI_RIM_STBD),
            GetColorSetting(DSK_SGI_RIM_STBD),
            GetColorSetting(DSK_SGI_RIM_STBD));
        break;
    case color_item::rim_port:
        c = AdjustColorForZone(val, GetColorSetting(DSK_SGI_RIM_PORT),
            GetColorSetting(DSK_SGI_RIM_PORT),
            GetColorSetting(DSK_SGI_RIM_PORT),
            GetColorSetting(DSK_SGI_RIM_PORT),
            GetColorSetting(DSK_SGI_RIM_PORT),
            GetColorSetting(DSK_SGI_RIM_PORT));
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
