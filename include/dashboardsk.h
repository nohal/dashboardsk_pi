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

#ifndef _DASHBOARDSK_H_
#define _DASHBOARDSK_H_

#include "dashboard.h"
#include "dskdc.h"
#include "ocpn_plugin.h"
#include "pager.h"
#include "pi_common.h"
#include "wx/jsonval.h"
#include <unordered_map>

// All the instrument class headers must be included here
#include "simplegaugeinstrument.h"
#include "simplehistograminstrument.h"
#include "simplenumberinstrument.h"
#include "simplepositioninstrument.h"
#include "simpletextinstrument.h"
// #include "andanotherinstrument.h"
//  All the instrument classes must be defined here
#define INSTRUMENTS                                                            \
    X(0, SimpleNumberInstrument)                                               \
    X(1, SimpleGaugeInstrument)                                                \
    X(2, SimpleTextInstrument)                                                 \
    X(3, SimplePositionInstrument)                                             \
    X(4, SimpleHistogramInstrument)                                            \
    // X(4, AndAnotherInstrument)

#define SRC_MAGIC_STRING "SRC:"

PLUGIN_BEGIN_NAMESPACE

class dskDC;
class dashboardsk_pi;

/// Toplevel class implementing the functionality of the plugin and hosting the
/// SignalK data tree
class DashboardSK {
private:
    /// Parent window pointer
    wxWindow* m_parent_window;
    /// Plugin
    dashboardsk_pi* m_parent_plugin;
    /// Vector of dashboards
    vector<Dashboard*> m_dashboards;
    /// Storage object for SignalK full data dynamically updated from the
    /// received deltas
    wxJSONValue m_sk_data;
    /// SignalK self context (aka the key to which vessels.self translates)
    wxString m_self;
    /// Pointer to the point of the JSON document tree in #m_sk_data where the
    /// self context is
    wxJSONValue* m_self_ptr;
    /// Updates to the dashboard are not performed if true
    bool m_frozen;
    /// Map of dashboard pages displayed on canvases
    std::unordered_map<int, Pager*> m_displayed_pages;
    /// Map of instrument subscription to the data paths. Only instruments
    /// interested in changed data are notified and poll them on next update
#if wxCHECK_VERSION(3, 1, 0)
    std::unordered_map<wxString, vector<Instrument*>> m_path_subscriptions;
#else
    std::unordered_map<string, vector<Instrument*>> m_path_subscriptions;
#endif
    /// Color scheme to be used when rendering the dashboards on screen
    int m_color_scheme;

    /// Path to the directory with data
    wxString m_data_dir;

    /// Process the SK value and if it is an object, extend the data structure
    /// to make the actual values leaves
    ///
    /// \param parent Pointer to the branch of the tree where the value belongs
    /// \param value Value to be processed
    /// \param ts Timestamp
    /// \param source Data source name
    void ProcessComplexValue(wxJSONValue* parent, const wxJSONValue& value,
        const wxDateTime& ts, const wxString& source);

public:
    /// Get current log level
    ///
    /// \return log level
    static uint8_t LogLevel() { return 0; }; // TODO: Set from config

    /// Constructor
    explicit DashboardSK(const wxString& data_path);

    /// Constructor
    ~DashboardSK()
    {
        for (auto db : m_dashboards) {
            delete db;
        }
        m_dashboards.clear();
    }

    /// Draw all the active dashboards on a canvas
    ///
    /// \param dc The "device context" to draw on
    /// \param vp The plugin viewport
    /// \param canvasIndex The chart canvas index
    void Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex);

    /// Read the config from JSON objects
    ///
    /// \param config DashboardSK_pi configuration data
    void ReadConfig(wxJSONValue& config);

    /// Generate the JSON object with complete configuration of the dashboards
    ///
    /// \return JSON object representing the configuration
    wxJSONValue GenerateJSONConfig();

    /// Set the color scheme of all the dashboards
    ///
    /// \param cs Integer parameter specifying the color scheme (0 - RGB, 1 -
    /// DAY, 2 - DUSK, 3 - NIGHT)
    void SetColorScheme(int cs);

    /// Get color scheme of the dashboards
    ///
    /// \return Color scheme index
    const int GetColorScheme();

    /// Get pointer to the SignalK object from the data tree
    ///
    ///\param path SignalK fully qualified path
    ///\return Pointer to the data object or NULL if not found
    const wxJSONValue* GetSKData(const wxString& path);

    /// Process a JSON object representing SignalK delta message
    ///
    /// \param message JSON object representing SignalK delta message
    void SendSKDelta(wxJSONValue& message);

    /// Return the SignalK context representing the generic "vessels.self"
    const wxString Self() { return m_self; };

    /// Set the SignalK context representing the generic "vessels.self".
    /// Performs some automatic translations for the well-known cases from value
    /// to the fully qualified representation (MMSI, UUID, URL)
    ///
    /// \param self The string representing the own vessel
    void SetSelf(const wxString& self)
    {
        m_self = NormalizeID(self);
        if (!m_sk_data["vessels"].HasMember(m_self)) {
            m_sk_data["vessels"][Self()].AddComment("Own vessel data");
        }
        m_self_ptr = &m_sk_data["vessels"][Self()];
    };

    /// Normalize the vessel id (MMSI, UUID, URL...)
    ///
    /// \param id The non-normal id without the qualifiers
    /// \return The ID in normalized form conformant to
    /// (^urn:mrn:(imo:mmsi:[2-7][0-9]{8}$|signalk:uuid:[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-4[0-9A-Fa-f]{3}-[89ABab][0-9A-Fa-f]{3}-[0-9A-Fa-f]{12}$))|^(http(s?):.*|mailto:.*|tel:(\\+?)[0-9]{4,})$
    /// or the original ID in case we have no idea what to do with it
    const wxString NormalizeID(const wxString& id)
    {
        if (id.IsEmpty()) {
            // return Self().AfterFirst('.');
            return Self();
        } else if (id.StartsWith(
                       "vessels")) { // TODO: Other types (aircraft, aton, sar)
            return NormalizeID(id.AfterFirst('.'));
        } else if (id.StartsWith("urn:")) {
            return id;
        } else if (id.IsNumber()) {
            return "urn:mrn:imo:mmsi:" + id;
        } else if (id.Matches("*-*-*-*-*")) {
            return "urn:mrn:signalk:uuid:" + id;
        } else if (id.StartsWith("http") || id.StartsWith("mailto")
            || id.StartsWith("tel")) {
            return "urn:mrn:" + id;
        } else {
            return id; // We don't know what it is, let's try to return it as it
                       // is anyway
        }
    }

    /// Subscribe the instrument to notifications about value updates of a path
    ///
    /// \param path SignalK path
    /// \param instrument Pointer to the subscribed instrument
    void Subscribe(const wxString& path, Instrument* instrument)
    {
        wxString ps = path.Left(path.Find(SRC_MAGIC_STRING) - 1);
        m_path_subscriptions[UNORDERED_KEY(ps)].push_back(instrument);
    }

    /// Unsubscribe instrument from all paths
    ///
    /// \param instrument Pointer to the instrument to unsubscribe
    void Unsubscribe(Instrument* instrument)
    {
        for (auto& sub : m_path_subscriptions) {
            std::vector<Instrument*>::iterator it = sub.second.begin();
            while (it != sub.second.end()) {
                if (*it == instrument) {
                    it = sub.second.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    /// Get list of all dashboards
    ///
    /// \return Array of all dashboard names
    wxArrayString GetDashboardNames()
    {
        wxArrayString as;
        for (auto dashboard : m_dashboards) {
            as.Add(dashboard->GetName());
        }
        return as;
    }

    /// Add a new dashboard and return a pointer to it
    ///
    /// \return Pointer to the newly created dashboard
    Dashboard* AddDashboard()
    {
        m_dashboards.emplace_back(new Dashboard(this));
        return m_dashboards.back();
    }

    /// Get pointer to a dashboard with given index
    ///
    /// \param item Index of the dashboard
    Dashboard* GetDashboard(int item)
    {
        if (item < 0 || (unsigned)item >= m_dashboards.size()) {
            return nullptr;
        }
        return *(m_dashboards.begin() + item);
    }

    /// Delete dashboard
    ///
    /// \param item Index of the dashboard
    void DeleteDashboard(int item)
    {
        if (item < 0 || (unsigned)item >= m_dashboards.size()) {
            return;
        }
        delete *(m_dashboards.begin() + item);
        m_dashboards.erase(m_dashboards.begin() + item);
    }

    /// Get names of all available types of instruments.
    ///
    /// \return Array of instrument type names. The order must be the same in
    /// GetInstrumentTypes, GetClassIndex and CreateInstrumentInstance.
    static wxArrayString GetInstrumentTypes()
    {
        // To add new instruments update the INSTRUMENTS macro at the beginning
        // of the file
        wxArrayString as;
#define X(a, b) as.Add(b::DisplayType());
        INSTRUMENTS
#undef X
        return as;
    }

    /// Create new instance of instrument.
    ///
    /// \return Pointer to the instance
    static Instrument* CreateInstrumentInstance(int type, Dashboard* dashboard)
    {
        // To add new instruments update the INSTRUMENTS macro at the beginning
        // of the file
        switch (type) {
#define X(a, b)                                                                \
    case a:                                                                    \
        return new b(dashboard);
            INSTRUMENTS
#undef X
        default:
            return nullptr;
        }
    };

    /// Get instrument class index
    ///
    /// \param cls Class name as returned from Instrument::Class()
    /// \return Index of the class in the list of instrument classes
    static const int GetClassIndex(const wxString& cls)
    {
        // To add new instruments update the INSTRUMENTS macro at the beginning
        // of the file
#define X(a, b)                                                                \
    if (cls.IsSameAs(b::Class())) {                                            \
        return a;                                                              \
    }
        INSTRUMENTS
#undef X
        return wxNOT_FOUND;
    };

    /// Get JSON text of the whole SignalK data structure
    ///
    /// \return JSON text
    wxString GetSignalKTreeText();

    /// Get pointer to the SignalK data
    ///
    /// \return Pointer to the data
    wxJSONValue* GetSignalKTree();

    /// Force redraw of the instrument on the next overlay refresh
    void ForceRedraw()
    {
        for (auto d : m_dashboards) {
            d->ForceRedraw();
        }
    };

    /// Translate own fully qualified ID to "self" in SignalK path
    /// References to fully quallified IDs are usually not transferable to
    /// different environment
    ///
    /// \param path SignalK path
    /// \return SignalK path with fully quallified ID replaced by keyword "self"
    const wxString SelfTranslate(const wxString& path);

    /// Reverse functionality to SeflTranslate
    /// References to fully quallified IDs are usually not transferable to
    /// different environment
    ///
    /// \param path SignalK path with "self"
    /// \return SignalK path with fully quallified ID
    const wxString SelfPopulate(const wxString& path);

    /// Freeze/unfreeze the drawing of the dashboards
    ///
    /// \param state Desired state
    void Freeze(bool state) { m_frozen = state; };

    /// Get data directory path
    ///
    /// @return Path to the data directory
    const wxString& GetDataDir() { return m_data_dir; }

    /// Adds a numbered page to the canvas
    ///
    /// @param canvas Index of the canvas
    /// @param page Page number (1-9)
    void AddPageToCanvas(const int& canvas, const size_t& page);

    /// Process the mouse event
    ///
    /// @param event
    /// @return true if we did process the event, false if it was not
    /// interesting
    bool ProcessMouseEvent(wxMouseEvent& event);

    /// Modify bitmap brightness according to the current color scheme
    ///
    /// @param bitmap Bitmap to be modified
    /// @return Modified bitmap
    wxBitmap ApplyBitmapBrightness(wxBitmap& bitmap);

    /// Modify bitmap brightness
    ///
    /// @param bitmap Bitmap to be modified
    /// @param level Light level, the lower the darker
    /// @return Modified bitmap
    wxBitmap SetBitmapBrightnessAbs(wxBitmap& bitmap, double level);

    /// Reset the pagers to pristine state
    void ResetPagers();

    /// Set pointer to the parent window
    ///
    /// @param parent Parent window pointer
    void SetParentWindow(wxWindow* parent) { m_parent_window = parent; }

    /// Get pointer tot the parent window
    ///
    /// @return Parent window pointer
    wxWindow* GetParentWindow() { return m_parent_window; }

    /// Set pointer to the plugin instance
    ///
    /// @param parent Pointer to the plugin instance
    void SetParentPlugin(dashboardsk_pi* parent);

    /// Show the preferences dialog
    void ShowPreferencesDialog();

    /// Toggle the visibility of the dashboards
    void ToggleVisibility();

    /// Check if the dashboard is visible
    /// @return true if visible
    bool IsVisible();

    /// Process data without drawing anything
    /// Serves primarily to update instruments with history
    void ProcessData();

    /// @brief Converts DIP to physical pixels
    /// @param x Device independent pixels
    /// @return Physical pixels
    int ToPhys(int x);

    /// Get the scale factor of the device context
    ///
    /// \return The scale factor
    double GetContentScaleFactor() const;
};

PLUGIN_END_NAMESPACE

#endif //_DASHBOARDSK_H_
