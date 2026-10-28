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
    auto bmp = Render(m_parent->GetContentScaleFactor());
    m_x_pos = m_parent->ToPhys(PAGER_LEFT_OFFSET);
    m_y_pos = vp->pix_height - m_parent->ToPhys(PAGER_BOTTOM_OFFSET);
    dc->DrawBitmap(bmp, m_x_pos, m_y_pos,
        bmp.HasAlpha()); // TODO: make the position configurable
}

const int Pager::GetNextPage()
{
    auto it = m_pages.find(m_current_page);
    if (it != m_pages.end()) {
        it++;
    }
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
        PAGER_ICON_SIZE * scale, PAGER_ICON_SIZE * scale);
    return m_parent->ApplyBitmapBrightness(bmp);
}

bool Pager::IsClicked(int& x, int& y)
{
    if (m_pages.size() >= 2 && x >= m_x_pos
        && x <= m_x_pos + m_parent->ToPhys(PAGER_ICON_SIZE) && y >= m_y_pos
        && y <= m_y_pos + m_parent->ToPhys(PAGER_ICON_SIZE)) {
        return true;
    }
    return false;
}

#define ID_PREFERENCES 2001
#define ID_VISIBILITY 2002
#define ID_PAGES 2222

bool Pager::ProcessMouseEvent(wxMouseEvent& event)
{
    if (!m_parent->IsVisible()) {
        return false;
    }
    if (event.LeftIsDown()) {
        int x = m_parent->ToPhys(
            event.GetX()); // Convert to per-canvas coordinates
        int y = m_parent->ToPhys(
            event.GetY()); // Convert to per-canvas coordinates
        if (IsClicked(x, y)) {
            SetCurrentPage(GetNextPage());
            return true;
        }
    } else if (event.RightDown()) {
        int x = m_parent->ToPhys(
            event.GetX()); // Convert to per-canvas coordinates
        int y = m_parent->ToPhys(
            event.GetY()); // Convert to per-canvas coordinates
        if (!IsClicked(x, y)) {
            return false;
        }
        // Show the menu only after the canvas mouse handler returned, running
        // the popup menu and modal dialog nested inside it breaks the canvas
        // mouse state on MSW
        m_parent->GetParentWindow()->CallAfter([this]() {
            wxMenu mnu;
            mnu.Append(ID_PREFERENCES, _("Preferences..."));
            mnu.Append(ID_VISIBILITY, _("Toggle visibility"));
            for (auto& page : m_pages) {
                wxString s = wxString::Format(_("Page %d"), page);
                mnu.Append(ID_PAGES + page, s);
            }
            int sel
                = m_parent->GetParentWindow()->GetPopupMenuSelectionFromUser(
                    mnu);
            if (sel == ID_PREFERENCES) {
                m_parent->ShowPreferencesDialog();
            } else if (sel == ID_VISIBILITY) {
                m_parent->ToggleVisibility();
            } else if (sel > ID_PAGES) {
                SetCurrentPage(sel - ID_PAGES);
            }
        });
        return true;
    }
    return false;
}

void Pager::SetCurrentPage(int new_page)
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
