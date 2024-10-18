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

#ifndef _DASHBOARDSKPI_H_
#define _DASHBOARDSKPI_H_

#include "config.h"
#include "dashboardsk.h"
#include "dskdc.h"
#include "pi_common.h"
#include "wx/jsonreader.h"

constexpr int MY_API_VERSION_MAJOR = 1;
constexpr int MY_API_VERSION_MINOR = 18;

constexpr int DASHBOARDSK_TOOL_POSITION
    = -1; // Request default positioning of toolbar tool

PLUGIN_BEGIN_NAMESPACE

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------
/// Class representing the plugin for OpenCPN Plugin API
class dashboardsk_pi final : public opencpn_plugin_118 {
private:
    /// Parent window pointer
    wxWindow* m_parent_window;
    /// Id of the tool installed to the OpenCPN application toolbar
    int m_leftclick_tool_id;
    /// Color scheme used by the plugin
    int m_color_scheme;
    /// Bitmap representation of the logo of the plugin
    wxBitmap m_logo;
    /// Visibility state of the plugin
    bool m_shown;
    /// Pointer to the Implementation of the plugin functionality
    DashboardSK* m_dsk;
    /// Pointer to the "device context" to draw on
    dskDC* m_oDC;
    /// Path to the configuration file
    wxString m_config_file;
    /// Reader used for the JSON data parsing
    wxJSONReader m_json_reader;

    /// Load the configuration from disk
    void LoadConfig();

public:
    /// Constructor
    ///
    /// \param ppimgr Pointer to the plugin manager
    explicit dashboardsk_pi(void* ppimgr);

    /// Destructor
    ~dashboardsk_pi() override;

    //    The required PlugIn Methods
    /// Initialize the plugin
    ///
    /// \return
    int Init() override;

    /// Deinitialize the plugin
    ///
    /// \return
    bool DeInit() override;

    /// Save the configuration to disk
    void SaveConfig();

    /// Get major version of the plugin API the plugin requires
    ///
    /// \return Major version of the API
    int GetAPIVersionMajor() override;

    /// Get minor version of the plugin API the plugin requires
    ///
    /// \return Minor version of the API
    int GetAPIVersionMinor() override;

    /// Get major version of the plugin
    ///
    /// \return MAjor version of the plugin
    int GetPlugInVersionMajor() override;

    /// Get minor version of the plugin
    ///
    /// \return Minor version of the plugin
    int GetPlugInVersionMinor() override;

    /// Forms a semantic version together with GetPlugInVersionMajor() and
    /// GetPlugInVersionMinor(), see https://semver.org/
    int GetPlugInVersionPatch() override;

    /// Post-release version part, extends the semver spec.
    int GetPlugInVersionPost() override;

    /// Pre-release tag version part, see GetPlugInVersionPatch()
    const char* GetPlugInVersionPre() override;

    /// Build version part  see GetPlugInVersionPatch()
    const char* GetPlugInVersionBuild() override;

    /// Get bitmap icon of the plugin logo
    ///
    /// \return pointer to the bitmap containing the logo
    wxBitmap* GetPlugInBitmap() override;

    /// Get the name of the plugin
    ///
    /// \return Name of the plugin
    wxString GetCommonName() override;

    /// Get short description of the plugin
    /// The description should be a short single line text that fits the list
    /// view in the OpenCPN plugin manager tab of the Toolbox
    ///
    /// \return Short description of the plugin
    wxString GetShortDescription() override;

    /// Get long description of the plugin
    ///
    /// \return Longer text describing the plugin displayed on the plugin detail
    /// tile
    ///         in the OpenCPN plugin manager tab of the Toolbox once the plugin
    ///         is selected.
    wxString GetLongDescription() override;

    //    The override PlugIn Methods
    /// Render the overlay on the chart canvas in non-OpenGL mode
    ///
    /// \param dc
    /// \param vp
    /// \param canvasIndex
    /// \param priority
    /// \return
    bool RenderOverlayMultiCanvas(
        wxDC& dc, PlugIn_ViewPort* vp, int canvasIndex, int priority) override;

    /// Render the overlay on the chart canvas in OpenGL mode
    ///
    /// \param pcontext
    /// \param vp
    /// \param canvasIndex
    /// \param priority
    /// \return
    bool RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
        int canvasIndex, int priority) override;

    /// Get the number of toolbar icons the plugin provides
    int GetToolbarToolCount() override;

    /// Show preferences dialog for the plugin when the respective button is
    /// clicked on the plugin detail tile in the OpenCPN plugin manager tab of
    /// the Toolbox once the plugin is selected.
    ///
    /// \param parent Parent window owning the preferences dialog
    void ShowPreferencesDialog(wxWindow* parent) override;

    /// Callback to perform any actions bound to the click on a toolbar icon
    /// provided by the plugin
    ///
    /// \param id Id of the toolbar tool clicked
    void OnToolbarToolCallback(int id) override;

    /// Set color scheme the plugin should use
    /// Invoked when the core application color scheme is changed
    ///
    /// \param cs Color scheme
    void SetColorScheme(PI_ColorScheme cs) override;

    /// Callback delivering JSON messages from the core application
    ///
    /// \param message_id id of the message (We are interested at least in
    /// OCPN_CORE_SIGNAL, but there might be more to come) \param message_body
    /// The actual JSON message
    void SetPluginMessage(
        wxString& message_id, wxString& message_body) override;

    /// Get pointer to the actual dashboard logic
    ///
    /// \return Pointer to the DashboardSK instance
    DashboardSK* GetDSK() { return m_dsk; };

    /// Get Path to the plugin data
    ///
    /// \return Path to the plugin data including the trailing separator
    wxString GetDataDir() const;

    /// Get Path to the plugin configuration data
    ///
    /// \return Path to the plugin configuration data including the trailing
    /// separator
    wxString GetConfigDir() const;

    /// Get bitmap from SVG file
    ///
    /// \param filename Path to the SVG file
    /// \param w Width of the requested bitmap
    /// \param h Height of the requested bitmal
    /// \return Generated bitmap
    wxBitmap GetBitmapFromSVG(
        const wxString& filename, const wxCoord w, const wxCoord h);

    /// Hook for the mouse events relayed from the core application
    ///
    /// @param event
    /// @return True if we processed the event ourselves, false if we do not
    /// care about it.
    bool MouseEventHook(wxMouseEvent& event) override;

    /// Check if the dashboard is visible
    /// @return true if visible
    bool IsVisible();

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

#endif //_DASHBOARDSKPI_H_
