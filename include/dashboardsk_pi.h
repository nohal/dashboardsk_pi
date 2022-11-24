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
#include "dskdc.h"
#include "pi_common.h"

#define MY_API_VERSION_MAJOR 1
#define MY_API_VERSION_MINOR 18

#include "dashboardsk.h"

#define DASHBOARDSK_TOOL_POSITION                                              \
    -1 // Request default positioning of toolbar tool

class wxJSONReader;

PLUGIN_BEGIN_NAMESPACE

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------
/// Class representing the plugin for OpenCPN Plugin API
class dashboardsk_pi : public opencpn_plugin_118 {
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
    /// Pointer to the reader used for the JSON dafa parsing
    wxJSONReader* m_json_reader;

    /// Load the configuration from disk
    void LoadConfig();

public:
    /// Constructor
    ///
    /// \param ppimgr Pointer to the plugin manager
    explicit dashboardsk_pi(void* ppimgr);

    /// Destructor
    ~dashboardsk_pi();

    //    The required PlugIn Methods
    /// Initialize the plugin
    ///
    /// \return
    int Init();

    /// Deinitialize the plugin
    ///
    /// \return
    bool DeInit();

    /// Save the configuration to disk
    void SaveConfig();

    /// Get major version of the plugin API the plugin requires
    ///
    /// \return Major version of the API
    int GetAPIVersionMajor();

    /// Get minor version of the plugin API the plugin requires
    ///
    /// \return Minor version of the API
    int GetAPIVersionMinor();

    /// Get major version of the plugin
    ///
    /// \return MAjor version of the plugin
    int GetPlugInVersionMajor();

    /// Get minor version of the plugin
    ///
    /// \return Minor version of the plugin
    int GetPlugInVersionMinor();

    /// Get bitmap icon of the plugin logo
    ///
    /// \return pointer to the bitmap containing the logo
    wxBitmap* GetPlugInBitmap();

    /// Get the name of the plugin
    ///
    /// \return Name of the plugin
    wxString GetCommonName();

    /// Get short description of the plugin
    /// The description should be a short single line text that fits the list
    /// view in the OpenCPN plugin manager tab of the Toolbox
    ///
    /// \return Short description of the plugin
    wxString GetShortDescription();

    /// Get long description of the plugin
    ///
    /// \return Longer text describing the plugin displayed on the plugin detail
    /// tile
    ///         in the OpenCPN plugin manager tab of the Toolbox once the plugin
    ///         is selected.
    wxString GetLongDescription();

    //    The override PlugIn Methods
    /// Render the overlay on the chart canvas in non-OpenGL mode
    ///
    /// \param dc
    /// \param vp
    /// \param canvasIndex
    /// \param priority
    /// \return
    bool RenderOverlayMultiCanvas(
        wxDC& dc, PlugIn_ViewPort* vp, int canvasIndex, int priority);

    /// Render the overlay on the chart canvas in OpenGL mode
    ///
    /// \param pcontext
    /// \param vp
    /// \param canvasIndex
    /// \param priority
    /// \return
    bool RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
        int canvasIndex, int priority);

    /// Get the number of toolbar icons the plugin provides
    int GetToolbarToolCount();

    /// Show preferences dialog for the plugin when the respective button is
    /// clicked on the plugin detail tile in the OpenCPN plugin manager tab of
    /// the Toolbox once the plugin is selected.
    ///
    /// \param parent Parent window owning the preferences dialog
    void ShowPreferencesDialog(wxWindow* parent);

    /// Callback to perform any actions bound to the click on a toolbar icon
    /// provided by the plugin
    ///
    /// \param id Id of the toolbar tool clicked
    void OnToolbarToolCallback(int id);

    /// Set color scheme the plugin should use
    /// Invoked when the core application color scheme is changed
    ///
    /// \param cs Color scheme
    void SetColorScheme(PI_ColorScheme cs);

    /// Callback delivering JSON messages from the core application
    ///
    /// \param message_id id of the message (We are interested at least in
    /// OCPN_CORE_SIGNAL, but there might be more to come) \param message_body
    /// The actual JSON message
    void SetPluginMessage(wxString& message_id, wxString& message_body);

    /// Get pointer to the actual dashboard logic
    ///
    /// \return Pointer to the DashboardSK instance
    DashboardSK* GetDSK() { return m_dsk; };

    /// Get Path to the plaugin data
    ///
    /// \return Path to the plugin data including the trailing separator
    wxString GetDataDir();

    /// Get bitmap from SVG file
    ///
    /// \param filename Path to the SVG file
    /// \param w Width of the requested bitmap
    /// \param h Height of the requested bitmal
    /// \return Generated bitmap
    wxBitmap GetBitmapFromSVG(
        const wxString& filename, const wxCoord w, const wxCoord h);
};

PLUGIN_END_NAMESPACE

#endif //_DASHBOARDSKPI_H_
