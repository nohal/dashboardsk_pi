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

#include "pager.h"
#include "dashboardsk.h"
#include <wx/filename.h>
#include <wx/menu.h>

PLUGIN_BEGIN_NAMESPACE

Pager::Pager(DashboardSK* parent)
    : m_parent(parent)
    , m_current_page(1)
    , m_x_pos(0)
    , m_y_pos(0)
{
}

void Pager::Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex)
{
    if (m_pages.size() < 2) {
        return;
    }
    auto bmp = Render(1.0);
    m_x_pos = PAGER_LEFT_OFFSET;
    m_y_pos = vp->pix_height - PAGER_BOTTOM_OFFSET;
    dc->DrawBitmap(bmp, m_x_pos, m_y_pos,
        bmp.HasAlpha()); // TODO: make the position configurable
}

const size_t Pager::GetNextPage()
{
    auto it = m_pages.find(m_current_page);
    it++;
    if (it == m_pages.end()) {
        it = m_pages.begin();
    }
    return *it;
}

wxBitmap Pager::Render(double scale)
{
    // TODO: Cache the bitmaps
    if (m_pages.find(m_current_page) == m_pages.end()) {
        m_current_page = *m_pages.begin();
    }

    auto bmp = GetBitmapFromSVGFile(m_parent->GetDataDir()
            + wxFileName::GetPathSeparator() + "p"
            + std::to_string(m_current_page) + ".svg",
        PAGER_ICON_SIZE, PAGER_ICON_SIZE);
    return m_parent->ApplyBitmapBrightness(bmp);
}

bool Pager::IsClicked(int& x, int& y)
{
    if (m_pages.size() >= 2 && x >= m_x_pos && x <= m_x_pos + PAGER_ICON_SIZE
        && y >= m_y_pos && y <= m_y_pos + PAGER_ICON_SIZE) {
        return true;
    }
    return false;
}

#define ID_PREFERENCES 2001
#define ID_VISIBILITY 2002

void Pager::OnPopupClick(wxCommandEvent& evt)
{
    void* data = static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    switch (evt.GetId()) {
    case ID_PREFERENCES:
        m_parent->ShowPreferencesDialog();
        break;
    case ID_VISIBILITY:
        m_parent->ToggleVisibility();
        break;
    }
}

bool Pager::ProcessMouseEvent(wxMouseEvent& event)
{
    if (event.LeftIsDown()) {
        int x = event.GetX(); // Convert to per-canvas coordinates
        int y = event.GetY(); // Convert to per-canvas coordinates
        if (IsClicked(x, y)) {
            SetCurrentPage(GetNextPage());
            return true;
        }
    } else if (event.RightIsDown()) {
        int x = event.GetX(); // Convert to per-canvas coordinates
        int y = event.GetY(); // Convert to per-canvas coordinates
        if (!IsClicked(x, y)) {
            return false;
        }
        wxMenu mnu;
        mnu.Append(ID_PREFERENCES, _("Preferences..."));
        mnu.Append(ID_VISIBILITY, _("Toggle visibility"));
        mnu.Connect(wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(Pager::OnPopupClick), NULL, this);
        m_parent->GetParentWindow()->PopupMenu(&mnu);
        return true;
    }
    return false;
}

void Pager::SetCurrentPage(size_t new_page)
{
    if (m_pages.find(new_page) != m_pages.end()) {
        m_current_page = new_page;
    } else {
        if (!m_pages.empty()) {
            m_current_page = *m_pages.begin();
        } else {
            m_current_page = 1;
        }
    }
}

PLUGIN_END_NAMESPACE
