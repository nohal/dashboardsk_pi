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
#include "dividerinstrument.h"
#include "instrument.h"

using namespace DashboardSKPlugin;

TEST_CASE("DividerInstrument Creation - properties set to defaults")
{
    DividerInstrument i(nullptr);
    REQUIRE(i.Class() != "Instrument");
    REQUIRE(i.DisplayType().IsSameAs("Divider"));
    REQUIRE(i.ConfigControls().size() == 5);
    REQUIRE(i.Class().IsSameAs("DividerInstrument"));
}

TEST_CASE("DividerInstrument Configuration Storage - if JSON not "
          "complete, defaults have to stay")
{
    DividerInstrument i(nullptr);
    wxJSONValue v;
    wxJSONReader r;

    r.Parse("{ \"length\": 250 }", &v);
    i.ReadConfig(v);
    v = i.GenerateJSONConfig();

    REQUIRE(v[DSK_SETTING_LENGTH].AsInt() == 250);
    REQUIRE(v[DSK_SETTING_ORIENTATION].AsInt() == 0);
    REQUIRE(v[DSK_SETTING_LINE_WIDTH].AsInt() == 2);
    REQUIRE(v[DSK_SETTING_LINE_STYLE].AsInt() == 0);
    REQUIRE(v[DSK_SETTING_LINE_COLOR].AsString().StartsWith("#"));
}

TEST_CASE("DividerInstrument Rendering - bitmap dimensions follow "
          "orientation, length and line width")
{
    DividerInstrument i(nullptr);
    wxJSONValue v;
    wxJSONReader r;

    // Automatic orientation without a parent dashboard falls back to a
    // horizontal line
    r.Parse("{ \"length\": 120, \"line_width\": 4 }", &v);
    i.ReadConfig(v);
    wxBitmap bmp = i.Render(1.0);
    REQUIRE(bmp.IsOk());
    REQUIRE(bmp.GetWidth() == 120);
    REQUIRE(bmp.GetHeight() == 4);

    // Explicitly vertical
    r.Parse("{ \"orientation\": 2 }", &v);
    i.ReadConfig(v);
    bmp = i.Render(1.0);
    REQUIRE(bmp.IsOk());
    REQUIRE(bmp.GetWidth() == 4);
    REQUIRE(bmp.GetHeight() == 120);

    // Explicitly horizontal, scaled
    r.Parse("{ \"orientation\": 1 }", &v);
    i.ReadConfig(v);
    bmp = i.Render(2.0);
    REQUIRE(bmp.IsOk());
    REQUIRE(bmp.GetWidth() == 240);
    REQUIRE(bmp.GetHeight() == 8);
}
