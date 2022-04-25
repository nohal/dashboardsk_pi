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

#include "instrument.h"
#include "dashboard.h"

PLUGIN_BEGIN_NAMESPACE

Instrument::~Instrument()
{
    if (m_parent_dashboard) {
        m_parent_dashboard->Unsubscribe(this);
    }
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
};

void Instrument::ReadConfig(wxJSONValue& config)
{
    m_name = config["name"].AsString();
    m_title = config["title"].AsString();
    m_allowed_age_sec = config["allowed_age"].AsInt();
    m_zones = Zone::ParseZonesFromString(config["zones"].AsString());
};

wxJSONValue Instrument::GenerateJSONConfig()
{
    wxJSONValue v;
    v["name"] = m_name;
    v["title"] = m_title;
    v["class"] = GetClass();
    v["allowed_age"] = m_allowed_age_sec;
    v["zones"] = Zone::ZonesToString(m_zones);
    return v;
};

void Instrument::SetSetting(const wxString& key, const wxString& value)
{
    if (key == "name") {
        m_name = value;
    } else if (key == "title") {
        m_title = value;
    } else if (key == "allowed_age") {
        m_allowed_age_sec = IntFromString(value);
    } else if (key == "zones") {
        m_zones = Zone::ParseZonesFromString(value);
    } else {
        m_config_vals[key] = value;
    }
}

void Instrument::SetSetting(const wxString& key, const wxColor& value)
{
    m_config_vals[key] = value.GetAsString(wxC2S_HTML_SYNTAX);
};

void Instrument::SetSetting(const wxString& key, const int& value)
{
    if (key == "allowed_age") {
        m_allowed_age_sec = value;
    } else {
        m_config_vals[key] = wxString::Format("%i", value);
    }
};

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
        wxGCDC outDC(bmpOut);
        outDC.StretchBlit(wxPoint(0, 0), sizeOut, &inDC, wxPoint(0, 0), sizeIn);
    } else {
        wxMemoryDC outDC(bmpOut);
        outDC.StretchBlit(wxPoint(0, 0), sizeOut, &inDC, wxPoint(0, 0), sizeIn);
        outDC.SelectObject(wxNullBitmap);
    }
    inDC.SelectObject(wxNullBitmap);
    return bmpOut;
}

PLUGIN_END_NAMESPACE
