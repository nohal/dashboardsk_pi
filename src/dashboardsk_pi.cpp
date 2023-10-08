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

#include "dashboardsk_pi.h"
#include "dashboardskguiimpl.h"

#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"
#include <wx/wfstream.h>

PLUGIN_BEGIN_NAMESPACE
// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void* ppimgr)
{
    return static_cast<opencpn_plugin*>(new dashboardsk_pi(ppimgr));
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) { delete p; }

//---------------------------------------------------------------------------------------------------------
//
//    DashboardSK PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

dashboardsk_pi::dashboardsk_pi(void* ppimgr)
    : opencpn_plugin_118(ppimgr)
    , m_leftclick_tool_id(-1)
    , m_color_scheme(PI_GLOBAL_COLOR_SCHEME_RGB)
    , m_shown(false)
    , m_dsk(nullptr)
    , m_oDC(nullptr)

{
    // Get a pointer to the opencpn display canvas, to use as a parent for the
    // dashboard
    m_parent_window = GetOCPNCanvasWindow();
    m_json_reader = new wxJSONReader();

    if (!wxDirExists(GetDataDir())) {
        wxFileName::Mkdir(GetDataDir(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    m_config_file = GetDataDir() + "config.json";
    m_logo = GetBitmapFromSVGFile(
        GetDataDir() + "dashboardsk_pi_toggled.svg", 32, 32);
}

dashboardsk_pi::~dashboardsk_pi() { delete m_json_reader; }

int dashboardsk_pi::Init()
{
    m_dsk = new DashboardSK(GetDataDir());
    m_dsk->SetParentWindow(m_parent_window);
    m_dsk->SetParentPlugin(this);
    LoadConfig();

    wxString _svg_dashboardsk = GetDataDir() + "dashboardsk_pi.svg";
    wxString _svg_dashboardsk_rollover
        = GetDataDir() + "dashboardsk_pi_rollover.svg";
    wxString _svg_dashboardsk_toggled
        = GetDataDir() + "dashboardsk_pi_toggled.svg";
    AddLocaleCatalog(_T("opencpn-dashboardsk_pi"));

    if (m_shown) {
        m_leftclick_tool_id = InsertPlugInToolSVG(_T( "DashboardSK" ),
            _svg_dashboardsk_toggled, _svg_dashboardsk_rollover,
            _svg_dashboardsk, wxITEM_CHECK, _("DashboardSK"), _T( "" ), nullptr,
            DASHBOARDSK_TOOL_POSITION, 0, this);
    } else {
        m_leftclick_tool_id = InsertPlugInToolSVG(_T( "DashboardSK" ),
            _svg_dashboardsk, _svg_dashboardsk_rollover,
            _svg_dashboardsk_toggled, wxITEM_CHECK, _("DashboardSK"), _T( "" ),
            nullptr, DASHBOARDSK_TOOL_POSITION, 0, this);
    }

    return (WANTS_OVERLAY_CALLBACK | WANTS_OPENGL_OVERLAY_CALLBACK
        | WANTS_DYNAMIC_OPENGL_OVERLAY_CALLBACK | WANTS_TOOLBAR_CALLBACK
        | INSTALLS_TOOLBAR_TOOL | WANTS_PREFERENCES | WANTS_PLUGIN_MESSAGING
        | WANTS_MOUSE_EVENTS);
}

bool dashboardsk_pi::DeInit()
{
    SaveConfig();
    delete m_dsk;
    return true;
}

int dashboardsk_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int dashboardsk_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int dashboardsk_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int dashboardsk_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap* dashboardsk_pi::GetPlugInBitmap() { return &m_logo; }
wxString dashboardsk_pi::GetCommonName() { return _("DashboardSK"); }

wxString dashboardsk_pi::GetShortDescription()
{
    return _("SignalK dashboard PlugIn for OpenCPN");
}

wxString dashboardsk_pi::GetLongDescription()
{
    return _("Universal dashboard plugin for OpenCPN based on the SignalK data "
             "model.\n");
}

int dashboardsk_pi::GetToolbarToolCount() { return 1; }

void dashboardsk_pi::ShowPreferencesDialog(wxWindow* parent)
{
    MainConfigFrameImpl mf(this, parent);
    mf.ShowModal();
}

void dashboardsk_pi::OnToolbarToolCallback(int id)
{
    m_shown = !m_shown;
    if (id == 0)
        SetToolbarItemState(m_leftclick_tool_id, m_shown);
}

void dashboardsk_pi::SetColorScheme(PI_ColorScheme cs)
{
    m_color_scheme = cs;
    if (m_dsk) {
        m_dsk->SetColorScheme(cs);
    }
}

void dashboardsk_pi::LoadConfig()
{
    LOG_VERBOSE("DashboardSK_pi: LoadConfig from %s ", m_config_file.c_str());
    if (!wxFileExists(m_config_file)) {
        wxString sample_config = GetDataDir() + "sample_config.json";
        if (wxFileExists(sample_config)) {
            wxCopyFile(sample_config, m_config_file, false);
        }
    }
    if (wxFileExists(m_config_file)) {
        wxFileInputStream str(m_config_file);
        if (!str.IsOk()) {
            LOG_VERBOSE("DashboardSK_pi: Can't load configuration from %s",
                m_config_file.c_str());
            return;
        }
        wxJSONValue config;
        m_json_reader->Parse(str, &config);
        wxJSONValue defaultFalse(false);
        m_shown = config.Get("shown", defaultFalse).AsBool();
        m_dsk->ReadConfig(config["dashboardsk"]);
    }
}
void dashboardsk_pi::SaveConfig()
{
    wxFileOutputStream str(m_config_file);
    if (!str.IsOk()) {
        LOG_VERBOSE("DashboardSK_pi: Can't save configuration to %s",
            m_config_file.c_str());
        return;
    }
    wxJSONWriter writer;
    wxJSONValue config;
    config["shown"] = m_shown;
    config["dashboardsk"] = m_dsk->GenerateJSONConfig();
    writer.Write(config, str);
}

bool dashboardsk_pi::RenderOverlayMultiCanvas(
    wxDC& dc, PlugIn_ViewPort* vp, int canvasIndex, int priority)
{
    if (priority != OVERLAY_OVER_UI) {
        return false;
    }
    if (GetCanvasCount() > canvasIndex + 1) {
        // return false;
    }

    if (!vp || !m_shown) {
        return false;
    }

    if (m_oDC && m_oDC->IsGL()) {
        delete m_oDC;
        m_oDC = nullptr;
    }
    if (!m_oDC) {
        m_oDC = new dskDC(dc);
    }
    m_oDC->SetDC(&dc);
    m_oDC->SetVP(vp);

    if (m_dsk) {
        m_dsk->Draw(m_oDC, vp, canvasIndex);
    }

    return true;
}

bool dashboardsk_pi::RenderGLOverlayMultiCanvas(
    wxGLContext* pcontext, PlugIn_ViewPort* vp, int canvasIndex, int priority)
{
    if (priority != OVERLAY_OVER_EMBOSS) {
        return false;
    }
    if (GetCanvasCount() > canvasIndex + 1) {
        // return false;
    }

    if (!vp || !m_shown) {
        return false;
    }

    if (m_oDC && !m_oDC->IsGL()) {
        delete m_oDC;
        m_oDC = nullptr;
    }
    if (!m_oDC) {
        m_oDC = new dskDC();
        GLint dims[4] = { 0 };
        glGetIntegerv(GL_VIEWPORT, dims);
        GLint fbWidth = dims[2];
        m_oDC->SetContentScaleFactor((double)fbWidth / vp->pix_width);
        m_oDC->SetVP(vp);
    }
    glEnable(GL_BLEND);

    if (m_dsk) {
        m_dsk->Draw(m_oDC, vp, canvasIndex);
    }

    return true;
}

void dashboardsk_pi::SetPluginMessage(
    wxString& message_id, wxString& message_body)
{
    if (message_id.EndsWith(
            "_SIGNALK")) { // From the core application we receive
                           // "OCPN_CORE_SIGNALK", be prepared for other future
                           // sources following common naming convention
        if (m_dsk) {
            wxJSONValue v;
            m_json_reader->Parse(message_body, &v);
            m_dsk->SendSKDelta(v);
        }
    }
}

wxString dashboardsk_pi::GetDataDir()
{
    return GetPluginDataDir("DashboardSK_pi") + wxFileName::GetPathSeparator()
        + "data" + wxFileName::GetPathSeparator();
}

wxBitmap dashboardsk_pi::GetBitmapFromSVG(
    const wxString& filename, const wxCoord w, const wxCoord h)
{
    return GetBitmapFromSVGFile(GetDataDir() + filename, w, h);
}

bool dashboardsk_pi::MouseEventHook(wxMouseEvent& event)
{
    return m_dsk->ProcessMouseEvent(event);
}

PLUGIN_END_NAMESPACE
