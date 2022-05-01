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
    m_title = "???";
    m_name = _("New Simple Gauge");
    m_sk_key = wxEmptyString;
    // SimpleGaugeInstrument's own settings
    m_timed_out = false;
    m_needs_redraw = true;
    m_smoothing = 0;
    m_old_value = std::numeric_limits<double>::lowest();
    m_instrument_size = 200;
    m_gauge_type = gauge_type::relative_angle;

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
        int i = 0;
#if (wxCHECK_VERSION(3, 1, 6))
        value.ToInt(&i);
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
    bool relative)
{
    int angle = start_angle;
    wxCoord xStart;
    wxCoord yStart;
    wxCoord xEnd;
    wxCoord yEnd;
    double c, s;
    while (angle <= 360) {
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
            wxString val = wxString::Format("%i", angle);
            if (relative && angle > 180) {
                val = wxString::Format("%i", 180 - (angle - 180));
            }
            wxCoord x
                = xEnd - cos(deg2rad(angle)) * dc.GetTextExtent(val).GetX() / 2;
            wxCoord y
                = yEnd - sin(deg2rad(angle)) * dc.GetTextExtent(val).GetX() / 2;
            dc.DrawRotatedText(val, x, y, -angle);
        }
        angle += angle_step;
    }
}

void SimpleGaugeInstrument::DrawNeedle(wxDC& dc, const wxCoord& xc,
    const wxCoord& yc, const wxCoord& r, const wxCoord& angle,
    const int& perc_length, const int& perc_width)
{
    wxPoint needle[3] {
        wxPoint(r * (100 - perc_length) / 100 * cos(deg2rad(270 + angle))
                - cos(deg2rad(angle)) * r * perc_width / 200,
            r * (100 - perc_length) / 100 * sin(deg2rad(270 + angle))
                - sin(deg2rad(angle)) * r * perc_width / 200),
        wxPoint(r * (100 - perc_length) / 100 * cos(deg2rad(270 + angle))
                + cos(deg2rad(angle)) * r * perc_width / 200,
            r * (100 - perc_length) / 100 * sin(deg2rad(270 + angle))
                + sin(deg2rad(angle)) * r * perc_width / 200),
        wxPoint(r * cos(deg2rad(270 + angle)), r * sin(deg2rad(270 + angle)))
    };
    dc.DrawPolygon(3, needle, xc, yc);
}

wxBitmap SimpleGaugeInstrument::RenderAngle(double scale, bool relative)
{
    double dval = 0.0;
    wxString value = wxEmptyString;

    const wxJSONValue* val = m_parent_dashboard->GetSKData(m_sk_key);
    if (val) {
        wxJSONValue v = val->Get("value", *val);
        dval = Transform(v.AsDouble(), m_transformation);
        m_old_value = dval;
        value = wxString::Format(m_format_strings[m_format_index], abs(dval));
        if (dval < 0) {
            value.Prepend("-");
        }
    }

    wxColor mask_color
        = GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR));
    mask_color.SetRGB(mask_color.GetRGB() > 2 ? mask_color.GetRGB() - 1
                                              : mask_color.GetRGB() + 1);

    wxMemoryDC dc;

    wxCoord size_x = m_instrument_size;
    wxCoord size_y = m_instrument_size;
    wxCoord xc = size_x / 2;
    wxCoord yc = size_y / 2;
    wxCoord r = size_y / 2 - size_x / 200;

    m_bmp = wxBitmap(size_x, size_y);
    dc.SelectObject(m_bmp);
    dc.SetBackground(wxBrush(mask_color));
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
    dc.SetFont(wxFont(size_x / 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 200));
    DrawTicks(dc, 0, 30, xc, yc, r, r * 0.15, true, 90, relative);
    DrawTicks(dc, 0, 10, xc, yc, r, r * 0.1);
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_SGI_TICK_FG)), size_x / 100));
    dc.SetFont(wxFont(size_x / 12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD));
    DrawTicks(dc, 0, 90, xc, yc, r, r * 0.2, true, 0, relative);
    // Border
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR)),
        size_x / 100 + 1));
    dc.DrawCircle(xc, yc, r);
    // Needle
    dc.SetBrush(wxBrush(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG))));
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SGI_NEEDLE_FG)), 3));
    DrawNeedle(dc, xc, yc, r * 0.9, dval, 30);
    // Text
    // Label
    dc.SetTextForeground(GetDimedColor(GetColor(dval, color_item::title)));
    dc.SetFont(wxFont(size_x / 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL));
    dc.DrawText(m_title, xc - dc.GetTextExtent(m_title).GetX() / 2,
        yc - dc.GetTextExtent(m_title).GetY() * 1.5);
    // Data
    dc.SetTextForeground(GetDimedColor(GetColor(dval, color_item::value)));
    dc.SetFont(wxFont(
        size_x / 3, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.DrawText(value, xc - dc.GetTextExtent(value).GetX() / 2,
        yc - dc.GetTextExtent(value).GetY() / 4);
    dc.SelectObject(wxNullBitmap);
    m_bmp.SetMask(new wxMask(m_bmp, mask_color));
    return m_bmp;
}

wxBitmap SimpleGaugeInstrument::Render(double scale)
{
    switch (m_gauge_type) {
    case gauge_type::relative_angle:
        return RenderAngle(scale, true);
        break;
    case gauge_type::direction:
        RenderAngle(scale, true);
        break;
    // TODO: Percentile and ranged gauge
    default:
        return wxNullBitmap;
    }
    return RenderAngle(scale, false);
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
