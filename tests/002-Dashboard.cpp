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

TEST_CASE("Dashboard Creation - properties set to defaults")
{
    Dashboard d(nullptr);

    wxJSONValue config = d.GenerateJSONConfig();
    REQUIRE(config["name"].AsString().IsSameAs(wxEmptyString));
    REQUIRE(config["canvas"].AsInt() == 0);
    REQUIRE((Dashboard::anchor_edge)config["anchor"].AsInt()
        == Dashboard::anchor_edge::bottom);
    REQUIRE(config["offset_h"].AsInt() == DEFAULT_OFFSET_X);
    REQUIRE(config["offset_v"].AsInt() == DEFAULT_OFFSET_Y);
    REQUIRE(config["spacing_h"].AsInt() == DEFAULT_SPACING_H);
    REQUIRE(config["spacing_v"].AsInt() == DEFAULT_SPACING_V);
    REQUIRE(config["enabled"].AsBool() == true);
}

TEST_CASE("Dashboard Configuration Storage - if JSON not complete, defaults "
          "have to stay")
{
    Dashboard d(nullptr);
    wxJSONValue config;
    wxJSONReader r;

    r.Parse("{ \"name\": \"My Dashboard\" }", &config);
    d.ReadConfig(config);
    config = d.GenerateJSONConfig();

    REQUIRE(config["name"].AsString().IsSameAs("My Dashboard"));
    REQUIRE(config["canvas"].AsInt() == 0);
    REQUIRE((Dashboard::anchor_edge)config["anchor"].AsInt()
        == Dashboard::anchor_edge::bottom);
    REQUIRE(config["offset_h"].AsInt() == DEFAULT_OFFSET_X);
    REQUIRE(config["offset_v"].AsInt() == DEFAULT_OFFSET_Y);
    REQUIRE(config["spacing_h"].AsInt() == DEFAULT_SPACING_H);
    REQUIRE(config["spacing_v"].AsInt() == DEFAULT_SPACING_V);
    REQUIRE(config["enabled"].AsBool() == true);
}
