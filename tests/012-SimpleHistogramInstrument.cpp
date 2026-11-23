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

#include "instrument.h"
#include "simplehistograminstrument.h"

using namespace DashboardSKPlugin;

TEST_CASE("SimpleHistogramInstrument new values side defaults to left and "
          "survives configuration round trip")
{
    SimpleHistogramInstrument i(nullptr);
    Json::Value v = i.GenerateJSONConfig();
    REQUIRE(v[DSK_SETTING_NEW_VALUES_SIDE].asInt()
        == static_cast<int>(Instrument::new_values_side::left));

    ParseJSON("{ \"new_values_side\": 1 }", v);
    i.ReadConfig(v);
    v = i.GenerateJSONConfig();
    REQUIRE(v[DSK_SETTING_NEW_VALUES_SIDE].asInt()
        == static_cast<int>(Instrument::new_values_side::right));
}
