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
#include <wx/menu.h>

PLUGIN_BEGIN_NAMESPACE

#define ID_INSTR_CONFIG 3001
#define ID_INSTR_ACTION_BASE 3100

vector<wxString> Dashboard::AnchorEdgeLabels { _("Bottom"), _("Top"), _("Left"),
    _("Right"), _("Own ship") };

map<Dashboard::canvas_edge_anchor, wxCoord> Dashboard::m_offsets;

Dashboard::Dashboard()
    : m_name(wxEmptyString)
    , m_canvas_nr(0)
    , m_page_nr(1)
    , m_anchor(anchor_edge::bottom)
    , m_offset_x(DEFAULT_OFFSET_X)
    , m_offset_y(DEFAULT_OFFSET_Y)
    , m_spacing_h(DEFAULT_SPACING_H)
    , m_spacing_v(DEFAULT_SPACING_V)
    , m_enabled(true)
    , m_parent(nullptr)
    , m_color_scheme(0)
{
}

Dashboard::Dashboard(DashboardSK* parent)
    : Dashboard()
{
    m_parent = parent;
    if (parent) {
        m_color_scheme = parent->GetColorScheme();
    }
}

void Dashboard::ProcessData()
{
    if (!m_enabled) {
        return;
    }
    for (auto& instrument : m_instruments) {
        instrument->ProcessData();
    }
}

void Dashboard::Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex)
{
    if (!m_enabled || m_canvas_nr != canvasIndex) {
        return;
    }

    if (m_anchor == anchor_edge::own_ship) {
        double lat;
        double lon;
        if (!m_parent->GetOwnShipPosition(lat, lon)) {
            return;
        }
        wxPoint ship;
        GetCanvasPixLL(vp, &ship, lat, lon);
        if (ship.x < 0 || ship.x >= vp->pix_width || ship.y < 0
            || ship.y >= vp->pix_height) {
            return;
        }

        vector<wxBitmap> bitmaps;
        wxCoord width = 0;
        for (auto instrument : m_instruments) {
            bitmaps.emplace_back(
                instrument->Render(m_parent->GetContentScaleFactor()));
            if (!bitmaps.back().IsOk()) {
                continue;
            }
            if (width > 0) {
                width += m_parent->ToPhys(m_spacing_h);
            }
            width += bitmaps.back().GetWidth();
        }

        wxCoord x = ship.x - width / 2;
        for (size_t i = 0; i < bitmaps.size(); ++i) {
            const wxBitmap& bmp = bitmaps[i];
            if (!bmp.IsOk()) {
                continue;
            }
            wxCoord y = ship.y - bmp.GetHeight() / 2;
            dc->DrawBitmap(bmp, x, y, bmp.HasAlpha());
            m_instruments[i]->SetPlacement(
                x, y, bmp.GetWidth(), bmp.GetHeight());
            x += bmp.GetWidth() + m_parent->ToPhys(m_spacing_h);
        }
        return;
    }

    wxCoord canvas_width = vp->pix_width, canvas_height = vp->pix_height, x = 0,
            y = 0, current_row_size = 0, row_offset = 0;
    wxCoord start_pos;
    int dir = 1;
    size_t row_nr = 0;
    wxCoord dashboard_offset = 0;
    if (m_offsets.find(canvas_edge_anchor(canvasIndex, m_anchor))
        != m_offsets.end()) {
        dashboard_offset = m_offsets[canvas_edge_anchor(canvasIndex, m_anchor)];
        m_offsets[canvas_edge_anchor(canvasIndex, m_anchor)] = 0;
    }
    switch (m_anchor) {
    case anchor_edge::bottom:
        dir = -1;
        row_offset = m_parent->ToPhys(m_offset_y);
        x = m_parent->ToPhys(m_offset_x);
        start_pos = canvas_height - m_parent->ToPhys(dashboard_offset);
        row_nr = 1;
        break;
    case anchor_edge::top:
        dir = 1;
        row_offset = m_parent->ToPhys(m_offset_y);
        x = m_parent->ToPhys(m_offset_x);
        start_pos = m_parent->ToPhys(dashboard_offset);
        break;
    case anchor_edge::left:
        dir = 1;
        y = m_parent->ToPhys(m_offset_y);
        start_pos = m_parent->ToPhys(m_offset_x + dashboard_offset);
        break;
    case anchor_edge::right:
        dir = -1;
        start_pos = canvas_width - m_parent->ToPhys(m_offset_x)
            - m_parent->ToPhys(dashboard_offset);
        row_nr = 1;
        y = m_parent->ToPhys(m_offset_y);
        break;
    default:
        break;
    }

    for (auto& instrument : m_instruments) {
        const wxBitmap bmp(
            instrument->Render(m_parent->GetContentScaleFactor()));
        wxCoord width = bmp.GetWidth();
        wxCoord height = bmp.GetHeight();
        if (bmp.IsOk()) {
            if (m_anchor == anchor_edge::bottom
                || m_anchor == anchor_edge::top) {
                if (x + width + m_parent->ToPhys(m_offset_x) > canvas_width) {
                    // We don't fit, next row
                    row_offset
                        += current_row_size + m_parent->ToPhys(m_spacing_v);
                    current_row_size = 0;
                    x = m_parent->ToPhys(m_offset_x);
                }
                y = start_pos + dir * row_offset + dir * row_nr * height;
                current_row_size = wxMax(current_row_size, height);
                dc->DrawBitmap(bmp, x, y, bmp.HasAlpha());
                instrument->SetPlacement(x, y, width, height);
                x += width + m_spacing_h;
            } else if (m_anchor == anchor_edge::left
                || m_anchor == anchor_edge::right) {
                if (y + height + m_parent->ToPhys(m_offset_y) > canvas_height) {
                    y = m_parent->ToPhys(m_offset_y);
                    if (m_anchor == anchor_edge::left) {
                        start_pos += dir * current_row_size
                            + dir * m_parent->ToPhys(m_spacing_h);
                    } else {
                        row_offset
                            += current_row_size + m_parent->ToPhys(m_spacing_h);
                    }
                    current_row_size = 0;
                }
                x = start_pos - row_offset - row_nr * width;
                current_row_size = wxMax(current_row_size, width);
                dc->DrawBitmap(bmp, x, y, bmp.HasAlpha());
                instrument->SetPlacement(x, y, width, height);
                y += height + m_parent->ToPhys(m_spacing_v);
            }
        }
    }
    m_offsets[canvas_edge_anchor(canvasIndex, m_anchor)]
        += row_offset + current_row_size;
}

bool Dashboard::ProcessMouseEvent(wxMouseEvent& event, int dashboard_idx)
{
    if (!m_enabled || !event.RightIsDown()) {
        return false;
    }
    int x = m_parent->ToPhys(event.GetX());
    int y = m_parent->ToPhys(event.GetY());
    for (size_t i = 0; i < m_instruments.size(); ++i) {
        Instrument* instr = m_instruments[i];
        if (!instr->IsClicked(x, y)) {
            continue;
        }
        wxMenu mnu;
        mnu.Append(ID_INSTR_CONFIG,
            wxString::Format(_("Configure %s..."), instr->GetName()));
        wxArrayString actions = instr->GetContextMenuActions();
        for (size_t a = 0; a < actions.GetCount(); ++a) {
            mnu.Append(ID_INSTR_ACTION_BASE + static_cast<int>(a), actions[a]);
        }
        int sel
            = m_parent->GetParentWindow()->GetPopupMenuSelectionFromUser(mnu);
        if (sel == ID_INSTR_CONFIG) {
            m_parent->ShowPreferencesDialog(dashboard_idx, static_cast<int>(i));
        } else if (sel >= ID_INSTR_ACTION_BASE) {
            instr->DoContextMenuAction(sel - ID_INSTR_ACTION_BASE);
        }
        return true;
    }
    return false;
}

void Dashboard::SetColorScheme(int cs)
{
    m_color_scheme = cs;
    for (auto& m_instrument : m_instruments) {
        m_instrument->SetColorScheme(cs);
    }
}

const int Dashboard::GetColorScheme() { return m_color_scheme; }

void Dashboard::ReadConfig(Json::Value& config)
{
    LOG_VERBOSE("DashboardSK_pi: Reading dashboard config");
    if (config.isMember("name"))
        m_name = fromJsonVal(config["name"].asString());
    if (config.isMember("canvas"))
        m_canvas_nr = config["canvas"].asInt();
    if (config.isMember("page")) {
        m_page_nr = config["page"].asInt();
        m_parent->AddPageToCanvas(m_canvas_nr, m_page_nr);
    }
    if (config.isMember("anchor"))
        m_anchor = (anchor_edge)config["anchor"].asInt();
    if (config.isMember("offset_h"))
        m_offset_x = config["offset_h"].asInt();
    if (config.isMember("offset_v"))
        m_offset_y = config["offset_v"].asInt();
    if (config.isMember("spacing_h"))
        m_spacing_h = config["spacing_h"].asInt();
    if (config.isMember("spacing_v"))
        m_spacing_v = config["spacing_v"].asInt();
    if (config.isMember("enabled"))
        m_enabled = config["enabled"].asBool();
    if (config.isMember("instruments")) {
        LOG_VERBOSE("DashboardSK_pi: Dashboard has instruments");
        if (config["instruments"].isArray()) {
            LOG_VERBOSE("DashboardSK_pi: instruments are an array");
            for (int i = 0; i < (int)config["instruments"].size(); i++) {
                auto instr = DashboardSK::CreateInstrumentInstance(
                    DashboardSK::GetClassIndex(
                        fromJsonVal(config["instruments"][i]["config"]["class"]
                                .asString())),
                    this);
                if (!instr) {
                    LOG_VERBOSE(
                        "DashboardSK_pi: Problem loading instrument with class "
                        + fromJsonVal(
                            config["instruments"][i]["config"]["class"]
                                .asString()));
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

Json::Value Dashboard::GenerateJSONConfig()
{
    Json::Value v;
    v["name"] = toJson(m_name);
    v["canvas"] = m_canvas_nr;
    v["page"] = m_page_nr;
    v["anchor"] = (int)m_anchor;
    v["offset_h"] = m_offset_x;
    v["offset_v"] = m_offset_y;
    v["spacing_h"] = m_spacing_h;
    v["spacing_v"] = m_spacing_v;
    v["enabled"] = m_enabled;
    for (auto& m_instrument : m_instruments) {
        Json::Value instr;
        instr["config"] = m_instrument->GenerateJSONConfig();
        v["instruments"].append(instr);
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

const Json::Value* Dashboard::GetSKData(const wxString& path)
{
    return m_parent->GetSKData(path);
}

double Dashboard::GetMagneticVariation() const
{
    return m_parent ? m_parent->GetMagneticVariation() : 0.0;
}

PLUGIN_END_NAMESPACE
