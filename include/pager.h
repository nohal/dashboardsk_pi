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

#ifndef _PAGER_H
#define _PAGER_H

#define PAGER_ICON_SIZE 48
#define PAGER_LEFT_OFFSET 5
#define PAGER_BOTTOM_OFFSET 100

#include "dskdc.h"
#include "ocpn_plugin.h"
#include "pi_common.h"
#include <set>
#include <wx/event.h>

PLUGIN_BEGIN_NAMESPACE

/// The edge of the canvas to which the dashboard is attached

class DashboardSK;

/// Class representing the dashboard page control
class Pager : public wxEvtHandler {
public:
    /// Constructor
    Pager();

    /// Constructor
    ///
    /// \param parent Parent plugin object
    explicit Pager(DashboardSK* parent);

    ~Pager() { }

    /// Draw the pager
    ///
    /// \param dc The "device context" to draw on
    /// \param vp The plugin viewport
    /// \param canvasIndex The chart canvas index
    void Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex);

    wxBitmap Render(double scale);

    /// Set the color scheme of the dashboard
    ///
    /// \param cs Integer parameter specifying the color scheme (0 - RGB, 1 -
    /// DAY, 2 - DUSK, 3 - NIGHT)
    void SetColorScheme(int cs);

    /// Get color scheme of the dashboard
    ///
    /// \return Color scheme index
    const int GetColorScheme();

    /// Set currently displayed page
    /// @param new_page number of currently displayed
    void SetCurrentPage(size_t new_page);

    /// Get currently displayed page number
    /// @return page number
    const size_t GetCurrentPage() { return m_current_page; }

    /// Adds a page
    /// @param pg Page number
    void AddPage(const size_t pg) { m_pages.insert(pg); }

    /// Get next page with rollover to the beginning of the list
    /// @return Page number
    const size_t GetNextPage();

    /// @brief Return true if the are covered by the pager was clicked
    /// @param x
    /// @param y
    /// @return
    bool IsClicked(int& x, int& y);

    /// Process the mouse event
    /// @param event
    /// @return true if we did process the event, false if it was not
    /// interesting
    bool ProcessMouseEvent(wxMouseEvent& event);

    /// Event handler for the popup menu click
    /// @param evt
    void OnPopupClick(wxCommandEvent& evt);

    /// Reset the pager
    void Reset() { m_pages.clear(); }

private:
    DashboardSK* m_parent;
    size_t m_current_page;
    std::set<size_t> m_pages;
    wxCoord m_x_pos;
    wxCoord m_y_pos;
};

PLUGIN_END_NAMESPACE

#endif //_DASHBOARD_H
