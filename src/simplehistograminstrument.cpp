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

#include "simplehistograminstrument.h"
#include "dashboard.h"
#include "instrument.h"
#include <limits>

PLUGIN_BEGIN_NAMESPACE

#define HISTORY_1S 60
#define HISTORY_10S 360
#define HISTORY_5M 864

void History::Add(const double& value)
{
    if (m_last_minute.empty() || m_last_minute.back().OlderThan(1s)) {
        HistoryValue h(value);
        m_last_minute.push_back(h);
    } else {
        m_last_minute.back().Add(value);
    }
    if (m_last_minute.size() > HISTORY_1S) {
        m_last_minute.pop_front();
    }
    if (m_last_hour.empty() || m_last_hour.back().OlderThan(10s)) {
        m_last_hour.push_back(HistoryValue(value));
    } else {
        m_last_hour.back().Add(value);
    }
    if (m_last_hour.size() > HISTORY_10S) {
        m_last_hour.pop_front();
    }
    if (m_last_3days.empty() || m_last_3days.back().OlderThan(300s)) {
        m_last_3days.push_back(HistoryValue(value));
    } else {
        m_last_3days.back().Add(value);
    }
    if (m_last_3days.size() > HISTORY_5M) {
        m_last_3days.pop_front();
    }
}

void SimpleHistogramInstrument::Init()
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
#define X(a, b) m_supported_orders.Add(b);
    DSK_VALUE_ORDER
#undef X
#define X(a, b) m_supported_histories.Add(b);
    DSK_HISTORY_LENGTH
#undef X

    // Basic settings inherited from Instrument class
    m_title = DUMMY_TITLE;
    m_name = _("New Simple Histogram");
    m_sk_key = wxEmptyString;
    // SimpleHistogramInstrument's own settings
    m_timed_out = false;
    m_needs_redraw = true;
    m_title_font = wxFont(
        15, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    m_body_font
        = wxFont(30, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    m_old_value = std::numeric_limits<double>::min();
    m_instrument_width = 200;
    m_instrument_height = 100;

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_SHI_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_SHI_SETTINGS
#undef X
}

void SimpleHistogramInstrument::SetSetting(
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
        || key.IsSameAs(DSK_SETTING_ORDER) || key.IsSameAs(DSK_SETTING_HISTORY)
        || key.IsSameAs(DSK_SETTING_INSTR_WIDTH)
        || key.IsSameAs(DSK_SETTING_INSTR_HEIGHT)) {
        // TODO: The above manually maintained list should be replaced with
        // something using the information from the DSK_SHI_SETTINGS macro
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

void SimpleHistogramInstrument::SetSetting(
    const wxString& key, const int& value)
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
    } else if (key.IsSameAs(DSK_SETTING_ORDER)) {
        m_value_order = static_cast<value_order>(value);
    } else if (key.IsSameAs(DSK_SETTING_HISTORY)) {
        m_history_length = static_cast<history_length>(value);
    } else if (key.IsSameAs(DSK_SETTING_INSTR_WIDTH)) {
        m_instrument_width = value;
    } else if (key.IsSameAs(DSK_SETTING_INSTR_HEIGHT)) {
        m_instrument_height = value;
    }
}

void SimpleHistogramInstrument::ProcessData()
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
        } else {
            if (std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now() - m_last_change)
                    .count()
                > 5) {
                // Even timed out we want to redraw from time to time to shift
                // the graph
                m_needs_redraw = true;
            }
        }
    } else {
        m_new_data = false;
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
                                                 : 0.0);
            m_old_value = dval;
            m_history.Add(dval);
        }
    }
}

wxBitmap SimpleHistogramInstrument::Render(double scale)
{
    ProcessData();

    if (!m_needs_redraw) {
        return m_bmp;
    }
    m_needs_redraw = false;
    wxMemoryDC mdc;
    wxCoord width = m_instrument_width * scale;
    wxCoord height = m_instrument_height * scale;

#if defined(__WXGTK__) || defined(__WXQT__)
    m_bmp = wxBitmap(width, height, 32);
#else
    m_bmp = wxBitmap(width, height);
    m_bmp.UseAlpha();
#endif
    mdc.SelectObject(m_bmp);
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC dc(mdc);
#else
    wxMemoryDC& dc(mdc);
#endif
    dc.SetBackground(GetDimedColor(GetColor(color_item::body_bg)));
    dc.Clear();
    // Draw graph
    std::vector<HistoryValue> vals;
    double min = std::numeric_limits<double>::max();
    double max = -std::numeric_limits<double>::max();
    if (m_timed_out) {
        // If we are timed out, we still want to push the graph off the screen
        HistoryValue dummy;
        vals.push_back(dummy);
    }
    for (auto it = m_history.m_last_minute.rbegin();
        it != m_history.m_last_minute.rend(); ++it) {
        if (m_history_length == history_length::len_1min
            && it->OlderThan(60s)) {
            break;
        }
        if (it->GetMean() > max) {
            max = it->GetMean();
        }
        if (it->GetMean() < min) {
            min = it->GetMean();
        }
        vals.push_back(*it);
    }
    if (m_history.m_last_minute.size() == HISTORY_1S
        && m_history_length > history_length::len_1min) {
        for (auto it = m_history.m_last_hour.rbegin();
            it != m_history.m_last_hour.rend(); ++it) {
            if (it->OlderThan(vals.back())) { // Skip the values we have with
                                              // better precision
                continue;
            }
            if (m_history_length == history_length::len_5min
                && it->OlderThan(300s)) {
                break;
            }
            if (m_history_length == history_length::len_15min
                && it->OlderThan(900s)) {
                break;
            }
            if (m_history_length == history_length::len_30min
                && it->OlderThan(1800s)) {
                break;
            }
            if (m_history_length == history_length::len_1hour
                && it->OlderThan(3600s)) {
                break;
            }
            if (it->GetMean() > max) {
                max = it->GetMean();
            }
            if (it->GetMean() < min) {
                min = it->GetMean();
            }
            vals.push_back(*it);
        }
    }
    if (m_history.m_last_hour.size() == HISTORY_10S
        && m_history_length > history_length::len_1hour) {
        for (auto it = m_history.m_last_3days.rbegin();
            it != m_history.m_last_3days.rend(); ++it) {
            if (it->OlderThan(vals.back())) { // Skip the values we have with
                                              // better precision
                continue;
            }
            if (m_history_length == history_length::len_1day
                && it->OlderThan(86400s)) {
                break;
            }
            if (m_history_length == history_length::len_3days
                && it->OlderThan(259200s)) {
                break;
            }
            if (it->GetMean() > max) {
                max = it->GetMean();
            }
            if (it->GetMean() < min) {
                min = it->GetMean();
            }
            vals.push_back(*it);
        }
    }
    if (vals.size() <= 1) {
        min = 0.0;
        max = 0.0;
    }
    double range = max - min;
    double height_coef
        = range != 0.0 ? static_cast<double>(height) / range : 1.0;
    height_coef *= 0.9;
    wxCoord vertical_shift = (height - height * 0.9) / 2;
    HistoryValue lastval;
    double sum = 0.0;
    size_t cnt = 0;
    double pps = 0.0;
    if (vals.size() > 0) {
        auto dur = std::chrono::duration_cast<std::chrono::seconds>(
            vals.front().ts - vals.back().ts);
        pps = static_cast<double>(width) / dur.count();
    }
    dc.SetPen(wxPen(GetDimedColor(GetColor(color_item::body_fg)),
        BORDER_LINE_WIDTH * 2, wxPENSTYLE_SOLID));
    dc.SetTextForeground(GetDimedColor(GetColor(color_item::time_fg)));
    dc.SetFont(wxFont(height / 8 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    int max_labels = width / (dc.GetTextExtent("100s").GetWidth() * 1.5);
    int current_label = 1;
    for (auto& v : vals) {
        if (cnt > 0) {
            // We only draw continuous line if the values are not timed out
            auto age = std::chrono::duration_cast<std::chrono::seconds>(
                lastval.ts - v.ts)
                           .count();
            if (age < 5
                || (m_history_length <= history_length::len_1hour && cnt >= 60
                    && age < 30)
                || (m_history_length > history_length::len_1hour && cnt >= 360
                    && age < 600)) {
                wxCoord x1 = std::chrono::duration_cast<std::chrono::seconds>(
                                 vals.front().ts - lastval.ts)
                                 .count()
                    * pps;
                wxCoord x2 = std::chrono::duration_cast<std::chrono::seconds>(
                                 vals.front().ts - v.ts)
                                 .count()
                    * pps;
                if (m_value_order == value_order::lowest_highest) {
                    dc.DrawLine(x1,
                        vertical_shift
                            + (lastval.GetMean() - min) * height_coef,
                        x2, vertical_shift + (v.GetMean() - min) * height_coef);
                } else {
                    dc.DrawLine(x1,
                        height
                            - (vertical_shift
                                + (lastval.GetMean() - min) * height_coef),
                        x2,
                        height
                            - (vertical_shift
                                + (v.GetMean() - min) * height_coef));
                }
                if (x2 > width * current_label / max_labels) {
                    wxString lbl = FormatTime(v.ts);
                    dc.DrawText(lbl, x2 - dc.GetTextExtent(lbl).GetWidth() / 2,
                        height - dc.GetTextExtent(lbl).GetHeight()
                            - BORDER_LINE_WIDTH);
                    ++current_label;
                }
            }
        }
        lastval = v;
        sum += lastval.GetMean();
        ++cnt;
    }
    if (m_timed_out) {
        dc.SetTextForeground(GetDimedColor(GetColor(color_item::mean_fg)));
        dc.SetFont(wxFont(height / 4 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
            wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        wxString s = _("NO DATA");
        dc.DrawText(s, (width - dc.GetTextExtent(s).GetWidth()) / 2,
            (height - dc.GetTextExtent(s).GetHeight()) / 2);
    }
    // Mean
    if (cnt > 1) {
        dc.SetPen(wxPen(GetDimedColor(GetColor(color_item::mean_fg)),
            BORDER_LINE_WIDTH, wxPENSTYLE_SOLID));
        if (m_value_order == value_order::lowest_highest) {
            dc.DrawLine(0, vertical_shift + (sum / cnt - min) * height_coef,
                width, vertical_shift + (sum / cnt - min) * height_coef);
        } else {
            dc.DrawLine(0,
                height - (vertical_shift + (sum / cnt - min) * height_coef),
                width,
                height - (vertical_shift + (sum / cnt - min) * height_coef));
        }
        dc.SetTextForeground(GetDimedColor(GetColor(color_item::mean_fg)));
        dc.SetFont(wxFont(height / 8 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
            wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        if (m_value_order == value_order::lowest_highest) {
            dc.DrawText(FormatValue(sum / cnt), BORDER_LINE_WIDTH,
                (sum / cnt - min) * height_coef);
        } else {
            dc.DrawText(FormatValue(sum / cnt), BORDER_LINE_WIDTH,
                height - ((sum / cnt - min) * height_coef));
        }
    }

    // Labels for Y-axis
    dc.SetTextForeground(GetDimedColor(GetColor(color_item::body_fg)));
    dc.SetFont(wxFont(height / 8 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    wxString lbl_btm;
    wxString lbl_top;
    if (m_value_order == value_order::lowest_highest) {
        lbl_top = FormatValue(min);
        lbl_btm = FormatValue(max);
    } else {
        lbl_top = FormatValue(max);
        lbl_btm = FormatValue(min);
    }
    dc.DrawText(lbl_top, BORDER_LINE_WIDTH, 0);
    dc.DrawText(
        lbl_btm, BORDER_LINE_WIDTH, height - dc.GetTextExtent("9").GetHeight());

    // Title
    dc.SetTextForeground(GetDimedColor(GetColor(color_item::title_fg)));
    dc.SetFont(wxFont(height / 6 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText(m_title,
        width - dc.GetTextExtent(m_title).GetWidth() - BORDER_LINE_WIDTH,
        BORDER_LINE_WIDTH);

    // Border
    dc.SetPen(wxPen(GetDimedColor(GetColor(color_item::border)),
        BORDER_LINE_WIDTH, wxPENSTYLE_SOLID));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(BORDER_LINE_WIDTH / 2, BORDER_LINE_WIDTH / 2,
        width - BORDER_LINE_WIDTH / 2, height - BORDER_LINE_WIDTH / 2);
    // Done drawing
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
}

void SimpleHistogramInstrument::ReadConfig(wxJSONValue& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.HasMember(b)) {                                                 \
        SetSetting(b, config[b].g());                                          \
    }
    DSK_SHI_SETTINGS
#undef X
}

wxJSONValue SimpleHistogramInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    wxJSONValue v = Instrument::GenerateJSONConfig();
#define X(a, b, c, d, e, f, g, h)                                              \
    if (!wxString(b).IsSameAs(DSK_SETTING_ZONES)) {                            \
        v[b] = h(b);                                                           \
    }
    DSK_SHI_SETTINGS
#undef X
    return v;
}

double SimpleHistogramInstrument::Transform(const double& val)
{
    return Instrument::Transform(val, m_transformation);
}

const wxColor SimpleHistogramInstrument::GetColor(const color_item item)
{
    wxColor c;
    switch (item) {
    case color_item::title_bg:
        c = GetColorSetting(DSK_SETTING_TITLE_BG);
        break;
    case color_item::title_fg:
        c = GetColorSetting(DSK_SETTING_TITLE_FG);
        break;
    case color_item::body_bg:
        c = GetColorSetting(DSK_SETTING_BODY_BG);
        break;
    case color_item::body_fg:
        c = GetColorSetting(DSK_SETTING_BODY_FG);
        break;
    case color_item::border:
        c = GetColorSetting(DSK_SETTING_BORDER_COLOR);
        break;
    case color_item::mean_fg:
        c = GetColorSetting(DSK_SETTING_MEAN_FG);
        break;
    case color_item::time_fg:
        c = GetColorSetting(DSK_SETTING_TIME_FG);
        break;
    }
    return c;
}

PLUGIN_END_NAMESPACE
