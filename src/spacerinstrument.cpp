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

#include "spacerinstrument.h"
#include "dashboard.h"
#include "instrument.h"

PLUGIN_BEGIN_NAMESPACE

void SpacerInstrument::Init()
{
    // Basic settings inherited from Instrument class
    m_title = wxEmptyString;
    m_name = _("New Spacer");
    // SpacerInstrument's own settings
    m_needs_redraw = true;
    m_instrument_width = 20;
    m_instrument_height = 20;

#define X(a, b, c, d, e, f, g, h) SetSetting(b, c);
    DSK_SPACER_SETTINGS
#undef X

#define X(a, b, c, d, e, f, g, h)                                              \
    m_config_controls.emplace_back(                                            \
        config_control({ b, d, dskConfigCtrl::e, f }));
    DSK_SPACER_SETTINGS
#undef X
}

void SpacerInstrument::SetSetting(const wxString& key, const wxString& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_INSTR_WIDTH)
        || key.IsSameAs(DSK_SETTING_INSTR_HEIGHT)) {
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

void SpacerInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_INSTR_WIDTH)) {
        m_instrument_width = value;
        m_needs_redraw = true;
    } else if (key.IsSameAs(DSK_SETTING_INSTR_HEIGHT)) {
        m_instrument_height = value;
        m_needs_redraw = true;
    }
}

wxBitmap SpacerInstrument::Render(double scale)
{
    if (!m_needs_redraw) {
        return m_bmp;
    }
    m_needs_redraw = false;

    wxCoord size_x = wxMax(1, m_instrument_width * scale);
    wxCoord size_y = wxMax(1, m_instrument_height * scale);
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
    mdc.SelectObject(wxNullBitmap);
    return m_bmp;
}

void SpacerInstrument::ReadConfig(wxJSONValue& config)
{
    Instrument::ReadConfig(config);
#define X(a, b, c, d, e, f, g, h)                                              \
    if (config.HasMember(b)) {                                                 \
        SetSetting(b, config[b].g());                                          \
    }
    DSK_SPACER_SETTINGS
#undef X
}

wxJSONValue SpacerInstrument::GenerateJSONConfig()
{
    // Shared parameters from the parent
    wxJSONValue v = Instrument::GenerateJSONConfig();
    // my own parameters
#define X(a, b, c, d, e, f, g, h) v[b] = h(b);
    DSK_SPACER_SETTINGS
#undef X
    return v;
}

PLUGIN_END_NAMESPACE
