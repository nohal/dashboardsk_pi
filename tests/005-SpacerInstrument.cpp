/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  DashboardSK Plugin
 * Author:   Pavel Kalian
 *
 ******************************************************************************
 * This file is part of the DashboardSK plugin
 * (https://github.com/nohal/dashboardsk_pi).
 *   Copyright (C) 2026 by Pavel Kalian
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

#include <catch2/catch_test_macros.hpp>

#include "pi_common.h"
#include "wx/jsonreader.h"
#include "wx/jsonval.h"
#include "wx/jsonwriter.h"

#include "dashboard.h"
#include "dashboardsk.h"
#include "instrument.h"
#include "spacerinstrument.h"

using namespace DashboardSKPlugin;

TEST_CASE("SpacerInstrument Creation - properties set to defaults")
{
    SpacerInstrument i(nullptr);
    REQUIRE(i.Class() != "Instrument");
    REQUIRE(i.DisplayType().IsSameAs("Spacer"));
    REQUIRE(i.ConfigControls().size() == 2);
    REQUIRE(i.Class().IsSameAs("SpacerInstrument"));
}

TEST_CASE("SpacerInstrument Configuration Storage - if JSON not "
          "complete, defaults have to stay")
{
    SpacerInstrument i(nullptr);
    wxJSONValue v;
    wxJSONReader r;

    r.Parse("{ \"instrument_width\": 50 }", &v);
    i.ReadConfig(v);
    v = i.GenerateJSONConfig();

    REQUIRE(v[DSK_SETTING_INSTR_WIDTH].AsInt() == 50);
    REQUIRE(v[DSK_SETTING_INSTR_HEIGHT].AsInt() == 20);
}

TEST_CASE("SpacerInstrument Rendering - bitmap dimensions follow the "
          "configured size")
{
    SpacerInstrument i(nullptr);
    wxJSONValue v;
    wxJSONReader r;

    r.Parse("{ \"instrument_width\": 50, \"instrument_height\": 30 }", &v);
    i.ReadConfig(v);
    wxBitmap bmp = i.Render(1.0);
    REQUIRE(bmp.IsOk());
    REQUIRE(bmp.GetWidth() == 50);
    REQUIRE(bmp.GetHeight() == 30);

    // Scaled rendering
    r.Parse("{ \"instrument_height\": 40 }", &v);
    i.ReadConfig(v);
    bmp = i.Render(2.0);
    REQUIRE(bmp.IsOk());
    REQUIRE(bmp.GetWidth() == 100);
    REQUIRE(bmp.GetHeight() == 80);
}
