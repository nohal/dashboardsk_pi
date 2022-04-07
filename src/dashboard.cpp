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

#include "dashboard.h"
#include "dashboardsk.h"

PLUGIN_BEGIN_NAMESPACE

vector<wxString> Dashboard::AnchorEdgeLabels { _("Bottom"), _("Top"), _("Left"),
    _("Right") };

Dashboard::Dashboard()
    : m_name(wxEmptyString)
    , m_canvas_nr(0)
    , m_anchor(anchor_edge::bottom)
    , m_offset_x(DEFAULT_OFFSET_X)
    , m_offset_y(DEFAULT_OFFSET_Y)
    , m_spacing_h(DEFAULT_SPACING_H)
    , m_spacing_v(DEFAULT_SPACING_V)
    , m_enabled(true)
    , m_parent(nullptr)
{
}

void Dashboard::Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex)
{
    if (!m_enabled || m_canvas_nr != canvasIndex) {
        return;
    }
    wxCoord canvas_width = vp->pix_width, canvas_height = vp->pix_height, x = 0,
            y = 0, current_row_size = 0, row_offset = 0;
    wxCoord start_pos;
    int dir = 1;
    size_t row_nr = 0;
    switch (m_anchor) {
    case anchor_edge::bottom:
        dir = -1;
        row_offset = m_offset_y;
        x = m_offset_x;
        start_pos = canvas_height;
        row_nr = 1;
        break;
    case anchor_edge::top:
        dir = 1;
        row_offset = m_offset_y;
        x = m_offset_x;
        start_pos = 0;
        break;
    case anchor_edge::left:
        dir = 1;
        y = m_offset_y;
        start_pos = m_offset_x;
        break;
    case anchor_edge::right:
        dir = -1;
        start_pos = canvas_width - m_offset_x;
        row_nr = 1;
        y = m_offset_y;
        break;
    default:
        break;
    }

    double pixel_scale = dc->GetContentScaleFactor();

    for (auto& m_instrument : m_instruments) {
        const wxBitmap bmp(m_instrument->Render(pixel_scale));
        wxCoord width = bmp.GetWidth() / pixel_scale;
        wxCoord height = bmp.GetHeight() / pixel_scale;
        if (bmp.IsOk()) {
            if (m_anchor == anchor_edge::bottom
                || m_anchor == anchor_edge::top) {
                if (x + width + m_offset_x > canvas_width) {
                    // We don't fit, next row
                    row_offset += current_row_size + m_spacing_v;
                    current_row_size = 0;
                    x = m_offset_x;
                }
                y = start_pos + dir * row_offset + dir * row_nr * height;
                current_row_size = wxMax(current_row_size, height);
                dc->DrawBitmap(bmp, x, y, false);
                m_instrument->SetPlacement(x, y, width, height);
                x += width + m_spacing_h;
            } else if (m_anchor == anchor_edge::left
                || m_anchor == anchor_edge::right) {
                if (y + height + m_offset_y > canvas_height) {
                    y = m_offset_y;
                    if (m_anchor == anchor_edge::left) {
                        start_pos += dir * current_row_size + dir * m_spacing_h;
                    } else {
                        row_offset += current_row_size + m_spacing_h;
                    }
                    current_row_size = 0;
                }
                x = start_pos - row_offset - row_nr * width;
                current_row_size = wxMax(current_row_size, width);
                dc->DrawBitmap(bmp, x, y, false);
                m_instrument->SetPlacement(x, y, width, height);
                y += height + m_spacing_v;
            }
        }
    }
}

void Dashboard::SetColorScheme(int cs)
{
    m_color_scheme = cs;
    for (auto& m_instrument : m_instruments) {
        m_instrument->SetColorScheme(cs);
    }
}

void Dashboard::ReadConfig(wxJSONValue& config)
{
    LOG_VERBOSE("DashboardSK_pi: Reading dashboard config");
    if (config.HasMember("name"))
        m_name = config["name"].AsString();
    if (config.HasMember("canvas"))
        m_canvas_nr = config["canvas"].AsInt();
    if (config.HasMember("anchor"))
        m_anchor = (anchor_edge)config["anchor"].AsInt();
    if (config.HasMember("offset_h"))
        m_offset_x = config["offset_h"].AsInt();
    if (config.HasMember("offset_v"))
        m_offset_y = config["offset_v"].AsInt();
    if (config.HasMember("spacing_h"))
        m_spacing_h = config["spacing_h"].AsInt();
    if (config.HasMember("spacing_v"))
        m_spacing_v = config["spacing_v"].AsInt();
    if (config.HasMember("enabled"))
        m_enabled = config["enabled"].AsBool();
    if (config.HasMember("instruments")) {
        LOG_VERBOSE("DashboardSK_pi: Dashboard has instruments");
        if (config["instruments"].IsArray()) {
            LOG_VERBOSE("DashboardSK_pi: instruments are an array");
            Instrument* instr;
            for (int i = 0; i < config["instruments"].Size(); i++) {
                instr = DashboardSK::CreateInstrumentInstance(
                    DashboardSK::GetClassIndex(
                        config["instruments"][i]["config"]["class"].AsString()),
                    this);
                if (!instr) {
                    LOG_VERBOSE(
                        "DashboardSK_pi: Problem loading instrument with class "
                        + config["instruments"][i]["config"]["class"]
                              .AsString());
                    continue;
                }
                instr->ReadConfig(config["instruments"][i]["config"]);
                m_instruments.push_back(instr);
            }
        }
    } else {
        LOG_VERBOSE("DashboardSK_pi: Dashboard config does not contain any "
                    "instruments");
    }
}

const wxJSONValue Dashboard::GenerateJSONConfig()
{
    wxJSONValue v;
    v["name"] = m_name;
    v["canvas"] = m_canvas_nr;
    v["anchor"] = (int)m_anchor;
    v["offset_h"] = m_offset_x;
    v["offset_v"] = m_offset_y;
    v["spacing_h"] = m_spacing_h;
    v["spacing_v"] = m_spacing_v;
    v["enabled"] = m_enabled;
    for (auto& m_instrument : m_instruments) {
        wxJSONValue instr;
        // instr["class"] = m_instrument->GetClass();
        instr["config"] = m_instrument->GenerateJSONConfig();
        v["instruments"].Append(instr);
    }
    return v;
}

void Dashboard::Subscribe(const wxString& path, Instrument* instrument)
{
    if (!m_parent) {
        return;
    }
    m_parent->Subscribe(path, instrument);
}

void Dashboard::Unsubscribe(Instrument* instrument)
{
    if (!m_parent) {
        return;
    }
    m_parent->Unsubscribe(instrument);
}

wxArrayString Dashboard::GetInstrumentNames()
{
    wxArrayString as;
    for (auto instr : m_instruments) {
        as.Add(instr->GetName());
    }
    return as;
}

const wxJSONValue* Dashboard::GetSKData(wxString path)
{
    return m_parent->GetSKData(path);
}

PLUGIN_END_NAMESPACE
