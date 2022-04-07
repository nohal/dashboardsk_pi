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

#include <catch2/catch.hpp>

#include "pi_common.h"
#include "wx/jsonreader.h"
#include "wx/jsonval.h"

#include "dashboard.h"
#include "dashboardsk.h"
#include "instrument.h"
#include "simplenumberinstrument.h"

using namespace DashboardSKPlugin;

TEST_CASE("SimpleNumberInstrument Creation - properties set to defaults")
{
    SimpleNumberInstrument i(nullptr);
    REQUIRE(i.Class() != "Instrument");
    REQUIRE(i.DisplayType().IsSameAs("Simple number"));
    REQUIRE(i.ConfigControls().size() == 1);
    REQUIRE(i.Class().IsSameAs("SimpleNumberInstrument"));
}

TEST_CASE("SimpleNumberInstrument Configuration Storage - if JSON not "
          "complete, defaults have to stay")
{
    SimpleNumberInstrument i(nullptr);
    wxJSONValue v;
    wxJSONReader r;

    r.Parse("{ \"sk_key\": \"vessels.123456789.navigation.test\" }", &v);
    i.ReadConfig(v);
    v = i.GenerateJSONConfig();

    REQUIRE(
        v["sk_key"].AsString().IsSameAs("vessels.123456789.navigation.test"));

    REQUIRE(v["title_background"].AsString().StartsWith("#"));
    REQUIRE(v["title_color"].AsString().StartsWith("#"));
    REQUIRE(v["body_background"].AsString().StartsWith("#"));
    REQUIRE(v["body_color"].AsString().StartsWith("#"));
    REQUIRE(v["border_color"].AsString().StartsWith("#"));
    REQUIRE(v["body_font"].AsInt() > 1);
    REQUIRE(v["body_font"].AsInt() < 30);
    REQUIRE(v["title_font"].AsInt() > 1);
    REQUIRE(v["title_font"].AsInt() < 20);
}
