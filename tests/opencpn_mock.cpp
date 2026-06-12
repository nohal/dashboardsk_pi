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
#include "pi_common.h"

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <wx/app.h>
#include <wx/init.h>

/// Initialize the wxWidgets library (and through it the underlying GUI
/// toolkit) once for the whole test run, the instruments cannot render
/// without it. A full GUI wxApp instance is needed, wxInitialize() alone
/// would only set up a console application object and never initialize the
/// toolkit.
class WxInitListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const&) override
    {
        // Never pop up (and hang on) interactive assert dialogs in the tests
        wxSetAssertHandler(nullptr);
        wxApp::SetInstance(new wxApp());
        int argc = 0;
        wxEntryStart(argc, static_cast<wxChar**>(nullptr));
    }

    void testRunEnded(Catch::TestRunStats const&) override { wxEntryCleanup(); }
};
CATCH_REGISTER_LISTENER(WxInitListener)

// Mocks of the OpenCPN API functions actually accessed from our code
// These functions are declared external DECL_EXP in ocpn_plugin.h and normally
// defined in the OpenCPN core application, which we obviously don't have
// here...

wxString GetLocaleCanonicalName() { return "en_US"; }

int GetCanvasCount() { return 1; }

int GetCanvasIndexUnderMouse() { return 0; }

wxBitmap GetBitmapFromSVGFile(
    wxString filename, unsigned int width, unsigned int height)
{
    return wxBitmap(width, height);
}

// Mocks of the dashboardsk_pi plugin class methods referenced from the code
// under test. The real implementations live in dashboardsk_pi.cpp, which
// needs the actual OpenCPN application to link against.

PLUGIN_BEGIN_NAMESPACE

void dashboardsk_pi::ShowPreferencesDialog(wxWindow* parent) { }

void dashboardsk_pi::OnToolbarToolCallback(int id) { }

bool dashboardsk_pi::IsVisible() { return true; }

int dashboardsk_pi::ToPhys(int x) { return x; }

double dashboardsk_pi::GetContentScaleFactor() const { return 1.0; }

PLUGIN_END_NAMESPACE
