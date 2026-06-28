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

#include <catch2/catch_test_macros.hpp>

#include "pi_common.h"

#include "dashboard.h"
#include "dashboardsk.h"
#include "instrument.h"
#include "simplenumberinstrument.h"

using namespace DashboardSKPlugin;

TEST_CASE("Dashboard Creation - properties set to defaults")
{
    Dashboard d(nullptr);

    Json::Value config = d.GenerateJSONConfig();
    REQUIRE(fromJsonVal(config["name"].asString()).IsSameAs(wxEmptyString));
    REQUIRE(config["canvas"].asInt() == 0);
    REQUIRE((Dashboard::anchor_edge)config["anchor"].asInt()
        == Dashboard::anchor_edge::bottom);
    REQUIRE(config["offset_h"].asInt() == DEFAULT_OFFSET_X);
    REQUIRE(config["offset_v"].asInt() == DEFAULT_OFFSET_Y);
    REQUIRE(config["spacing_h"].asInt() == DEFAULT_SPACING_H);
    REQUIRE(config["spacing_v"].asInt() == DEFAULT_SPACING_V);
    REQUIRE(config["enabled"].asBool() == true);
}

TEST_CASE("Dashboard Configuration Storage - if JSON not complete, defaults "
          "have to stay")
{
    Dashboard d(nullptr);
    Json::Value config;

    ParseJSON("{ \"name\": \"My Dashboard\" }", config);
    d.ReadConfig(config);
    config = d.GenerateJSONConfig();

    REQUIRE(fromJsonVal(config["name"].asString()).IsSameAs("My Dashboard"));
    REQUIRE(config["canvas"].asInt() == 0);
    REQUIRE((Dashboard::anchor_edge)config["anchor"].asInt()
        == Dashboard::anchor_edge::bottom);
    REQUIRE(config["offset_h"].asInt() == DEFAULT_OFFSET_X);
    REQUIRE(config["offset_v"].asInt() == DEFAULT_OFFSET_Y);
    REQUIRE(config["spacing_h"].asInt() == DEFAULT_SPACING_H);
    REQUIRE(config["spacing_v"].asInt() == DEFAULT_SPACING_V);
    REQUIRE(config["enabled"].asBool() == true);
}

TEST_CASE("Dashboard own-ship anchor survives configuration round trip")
{
    Dashboard d(nullptr);
    Json::Value config;
    ParseJSON("{ \"anchor\": 4 }", config);

    d.ReadConfig(config);

    REQUIRE(d.GetAnchorEdge() == Dashboard::anchor_edge::own_ship);
    REQUIRE(d.GenerateJSONConfig()["anchor"].asInt() == 4);
}
