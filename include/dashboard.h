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

#ifndef _DASHBOARD_H
#define _DASHBOARD_H

#include "dskdc.h"
#include "instrument.h"
#include "ocpn_plugin.h"
#include "pi_common.h"
#include "wx/jsonval.h"
#include <map>
#include <tuple>

#define DEFAULT_OFFSET_X 50
#define DEFAULT_OFFSET_Y 40
#define DEFAULT_SPACING_H 5
#define DEFAULT_SPACING_V 5

PLUGIN_BEGIN_NAMESPACE

/// The edge of the canvas to which the dashboard is attached

class DashboardSK;

/// Class representing a single dashboard holding a set of instruments
class Dashboard {
public:
    /// Canvas edge enum represents the edges of the canvas to which the
    /// dashboard can be anchored
    enum class anchor_edge { bottom = 0, top = 1, left = 2, right = 3 };

    /// Labels for the edges of the canvas to be used in the GUI
    static vector<wxString> AnchorEdgeLabels;

private:
    /// Vector of instruments cointained in the dashboard
    vector<Instrument*> m_instruments;
    /// Name of the dashboard
    wxString m_name;
    /// Id of the canvas on which the dashboard is drawn
    int m_canvas_nr;
    /// Number of "page" on which the dashboard is drawn
    int m_page_nr;
    /// Edge of the canvas to which the dashboard is anchored
    anchor_edge m_anchor;
    /// Horizontal offset of the dashboard from the edge of the canvas
    wxCoord m_offset_x;
    /// Vertical offset of the dashboard from the edge of the canvas
    wxCoord m_offset_y;
    /// Horizontal spacing between instruments
    wxCoord m_spacing_h;
    /// Vertical spacing between instruments
    wxCoord m_spacing_v;
    ///  Should the dashboard be displayed
    bool m_enabled;
    /// DashboardSK object owning the dashboard
    DashboardSK* m_parent;
    /// Color scheme
    int m_color_scheme;

    struct canvas_edge_anchor {
    public:
        int canvas;
        anchor_edge edge;

        canvas_edge_anchor(int c, anchor_edge e)
            : canvas(c)
            , edge(e) { };

        bool operator<(const canvas_edge_anchor& R) const
        {
            return std::tie(canvas, edge) < std::tie(R.canvas, R.edge);
        }
    };

    /// Cumulative offsets from the edges
    /// Increased as each dashboard is drawn
    /// Have to be nulled by DashboardSK  calling \c ClearOffsets before each
    /// round of rendering
    static map<canvas_edge_anchor, wxCoord> m_offsets;

public:
    /// Constructor
    Dashboard();

    /// Constructor
    ///
    /// \param parent Parent plugin object
    explicit Dashboard(DashboardSK* parent);

    ~Dashboard()
    {
        for (auto instr : m_instruments) {
            delete instr;
        }
        m_instruments.clear();
    }

    /// Set user specified name of the dashboard
    ///
    /// \param name The name
    void SetName(const wxString& name) { m_name = name; };

    /// Get name of the dashboard
    ///
    /// \return The string representing the name of the dashboard
    wxString GetName() { return m_name; };

    /// Does the dashboard contain any instruments?
    ///
    /// \return Tru if there is at least one instrument in the dashboard
    bool HasInstruments() { return m_instruments.size() > 0; }

    /// Set the number of the canvas on which the dashboard is displayed
    ///
    /// \param nr The index of the canvas
    void SetCanvasNr(size_t nr) { m_canvas_nr = nr; };

    /// Get the index of the canvas on which the dashboard is displayed
    ///
    /// \return The index of the canvas
    int GetCanvasNr() const { return m_canvas_nr; };

    /// Set the number of the page on which the dashboard is displayed
    ///
    /// \param nr The number of the page (1-9)
    void SetPageNr(size_t nr) { m_page_nr = nr; };

    /// Get the number of the page on which the dashboard is displayed
    ///
    /// \return The number of the page
    size_t GetPageNr() const { return m_page_nr; };

    /// Set the edge of the canvas to which the dashboard is attached
    ///
    /// \param e The edge
    void SetAnchorEdge(anchor_edge e) { m_anchor = e; };

    /// Get the edge of the canvas to which the dashboard is attached
    ///
    /// \return The edge
    anchor_edge GetAnchorEdge() { return m_anchor; };

    /// Set horizontal offset of the dashboard from the edge of the canvas
    ///
    /// \param nr The offset in DIP
    void SetHOffset(wxCoord nr) { m_offset_x = nr; };

    /// Set vertical offset of the dashboard from the edge of the canvas
    ///
    /// \param nr The offset in DIP
    void SetVOffset(wxCoord nr) { m_offset_y = nr; };

    /// Get horizontal offset of the dashboard from the edge of the canvas
    ///
    /// \return The offset in DIP
    wxCoord GetHOffset() const { return m_offset_x; };

    /// Get vertical offset of the dashboard from the edge of the canvas to
    /// which it is attached
    ///
    /// \return The offset in DIP
    wxCoord GetVOffset() const { return m_offset_y; };

    /// Set the horizontal spacing between the instruments in the dashboard
    ///
    /// \param nr Spacing in DIP
    void SetHSpacing(wxCoord nr) { m_spacing_h = nr; };

    /// Get the horizontal spacing between the instruments in the dashboard
    ///
    /// \return Spacing in DIP
    wxCoord GetHSpacing() const { return m_spacing_h; };

    /// Set the vertical spacing between the instruments in the dashboard
    ///
    /// \param nr Spacing in DIP
    void SetVSpacing(wxCoord nr) { m_spacing_v = nr; };

    /// Get the vertical spacing between the instruments in the dashboard
    ///
    /// \return Spacing in DIP
    wxCoord GetVSpacing() const { return m_spacing_v; };

    /// Draw the dashboard
    ///
    /// \param dc The "device context" to draw on
    /// \param vp The plugin viewport
    /// \param canvasIndex The chart canvas index
    void Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex);

    /// Set the color scheme of the dashboard
    ///
    /// \param cs Integer parameter specifying the color scheme (0 - RGB, 1 -
    /// DAY, 2 - DUSK, 3 - NIGHT)
    void SetColorScheme(int cs);

    /// Get color scheme of the dashboard
    ///
    /// \return Color scheme index
    const int GetColorScheme();

    /// Read the config from JSON objects
    ///
    /// \param config Configuration of the dashboard
    void ReadConfig(wxJSONValue& config);

    /// Generate the JSON object with complete configuration of the dashboards
    ///
    /// \return JSON object representing the configuration
    wxJSONValue GenerateJSONConfig();

    /// Subscribe the instrument to notifications about value updates of a path
    ///
    /// \param path SignalK path
    /// \param instrument Pointer to the subscribed instrument
    void Subscribe(const wxString& path, Instrument* instrument);

    /// Unsubscribe instrument from all paths
    ///
    /// \param instrument Pointer to the instrument to unsubscribe
    void Unsubscribe(Instrument* instrument);

    /// Get list of all instruments
    ///
    /// \return Array of all instrument names
    wxArrayString GetInstrumentNames();

    /// Get instrument from list by index
    ///
    /// \param item of the instrument
    /// \return Pointer to the instrument
    Instrument* GetInstrument(int item)
    {
        if (item < 0 || (unsigned)item >= m_instruments.size()) {
            return nullptr;
        }
        return m_instruments[item];
    };

    /// Add new instrument to the dashboard
    ///
    /// \param instrument Pointer to the added instrument
    void AddInstrument(Instrument* instrument)
    {
        instrument->SetColorScheme(m_color_scheme);
        m_instruments.push_back(instrument);
    }

    /// Remove instrument from the dashboard
    ///
    /// \param item Index of the instrument
    void DeleteInstrument(int item)
    {
        if (item < 0 || (unsigned)item >= m_instruments.size()) {
            return;
        }
        delete m_instruments[item];
        m_instruments.erase(m_instruments.begin() + item);
    }

    /// Set whether the dashboard should be displayed or not
    ///
    /// \param enabled Whether the dashboard should be displayed or not
    void Enable(bool enabled) { m_enabled = enabled; }

    /// Whether the dashboard should be displayed or not
    ///
    /// \return Whether the dashboard should be displayed or not
    bool Enabled() { return m_enabled; }

    /// Move instrument in the vector
    ///
    /// \param pos Starting position
    /// \param steps How many steps (negative move towards beginnning, positive
    /// move towards end)
    void MoveInstrument(const int pos, const int steps)
    {
        if (pos < 0 || (unsigned)pos >= m_instruments.size() || pos + steps < 0
            || (unsigned)pos + steps >= m_instruments.size()) {
            return;
        }
        auto old = m_instruments.at(pos);
        m_instruments.at(pos) = m_instruments.at(pos + steps);
        m_instruments.at(pos + steps) = old;
    }

    /// Get pointer to the SignalK object from the data tree
    ///
    /// \param path SignalK fully qualified path
    /// \return Pointer to the data object or NULL if not found
    const wxJSONValue* GetSKData(const wxString& path);

    /// Force redraw of the instrument on the next overlay refresh
    void ForceRedraw()
    {
        for (auto i : m_instruments) {
            i->ForceRedraw();
        }
    };

    /// Clear the cumulative offsets from the canvas edges, needs to be called
    /// every time before the dashboard rendering loop
    static void ClearOffsets() { m_offsets.clear(); }

    /// Process SK data without drawing anything.
    void ProcessData();
};

PLUGIN_END_NAMESPACE

#endif //_DASHBOARD_H
