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

TEST_CASE("Magic source SRC:any - accepts first available source")
{
    DashboardSK dsk("");
    dsk.SetSelf("urn:mrn:imo:mmsi:265599691");

    // Set up mock data with multiple sources
    Json::Value update;
    update["context"] = "vessels.urn:mrn:imo:mmsi:265599691";
    update["updates"][0]["$source"] = "VDR_halso-VD";
    update["updates"][0]["timestamp"] = "2024-01-01T00:00:00.000Z";
    update["updates"][0]["values"][0]["path"] = "environment.wind.speedTrue";
    update["updates"][0]["values"][0]["value"] = 12.5;

    dsk.SendSKDelta(update);

    // Try to retrieve with SRC:any
    const Json::Value* val
        = dsk.GetSKData("vessels.urn:mrn:imo:mmsi:265599691.environment.wind."
                        "speedTrue.SRC:any");
    REQUIRE(val != nullptr);
    REQUIRE(val->isMember("value"));
    REQUIRE(val->get("value", Json::Value()).asDouble() == 12.5);

    // Add another source
    Json::Value update2;
    update2["context"] = "vessels.urn:mrn:imo:mmsi:265599691";
    update2["updates"][0]["$source"] = "VDR_halso-VE";
    update2["updates"][0]["timestamp"] = "2024-01-01T00:00:01.000Z";
    update2["updates"][0]["values"][0]["path"] = "environment.wind.speedTrue";
    update2["updates"][0]["values"][0]["value"] = 13.5;

    dsk.SendSKDelta(update2);

    // SRC:any should still work (returns first available)
    val = dsk.GetSKData("vessels.urn:mrn:imo:mmsi:265599691.environment.wind."
                        "speedTrue.SRC:any");
    REQUIRE(val != nullptr);
    REQUIRE(val->isMember("value"));
}

TEST_CASE("Magic source SRC:lockfirst - locks to first available on first "
          "access")
{
    DashboardSK dsk("");
    dsk.SetSelf("urn:mrn:imo:mmsi:265599691");

    Dashboard* db = dsk.AddDashboard();
    db->SetCanvasNr(0);

    SimpleNumberInstrument instr(db);
    instr.SetSetting(wxString(DSK_SETTING_SK_KEY),
        wxString(
            "vessels.urn:mrn:imo:mmsi:265599691.environment.wind.speedTrue."
            "SRC:lockfirst"));

    // Set up data with first source
    Json::Value update;
    update["context"] = "vessels.urn:mrn:imo:mmsi:265599691";
    update["updates"][0]["$source"] = "VDR_halso-VD";
    update["updates"][0]["timestamp"] = "2024-01-01T00:00:00.000Z";
    update["updates"][0]["values"][0]["path"] = "environment.wind.speedTrue";
    update["updates"][0]["values"][0]["value"] = 12.5;

    dsk.SendSKDelta(update);

    // First call should lock to VDR_halso-VD
    const Json::Value* val
        = instr.GetSKDataResolved("vessels.urn:mrn:imo:mmsi:265599691."
                                  "environment.wind.speedTrue.SRC:lockfirst");
    REQUIRE(val != nullptr);
    REQUIRE(instr.GetLockedSource().IsSameAs("VDR_halso-VD"));

    // Add another source
    Json::Value update2;
    update2["context"] = "vessels.urn:mrn:imo:mmsi:265599691";
    update2["updates"][0]["$source"] = "VDR_halso-VE";
    update2["updates"][0]["timestamp"] = "2024-01-01T00:00:01.000Z";
    update2["updates"][0]["values"][0]["path"] = "environment.wind.speedTrue";
    update2["updates"][0]["values"][0]["value"] = 13.5;

    dsk.SendSKDelta(update2);

    // Second call should still return data from locked source (VDR_halso-VD)
    val = instr.GetSKDataResolved("vessels.urn:mrn:imo:mmsi:265599691."
                                  "environment.wind.speedTrue.SRC:lockfirst");
    REQUIRE(val != nullptr);
    REQUIRE(instr.GetLockedSource().IsSameAs("VDR_halso-VD"));
}

TEST_CASE("Magic source SRC:lockpersist - persists lock across sessions")
{
    DashboardSK dsk("");
    dsk.SetSelf("urn:mrn:imo:mmsi:265599691");

    Dashboard* db = dsk.AddDashboard();
    db->SetCanvasNr(0);

    SimpleNumberInstrument instr(db);
    instr.SetSetting(wxString(DSK_SETTING_SK_KEY),
        wxString(
            "vessels.urn:mrn:imo:mmsi:265599691.environment.wind.speedTrue."
            "SRC:lockpersist"));

    // Set up data
    Json::Value update;
    update["context"] = "vessels.urn:mrn:imo:mmsi:265599691";
    update["updates"][0]["$source"] = "VDR_halso-VD";
    update["updates"][0]["timestamp"] = "2024-01-01T00:00:00.000Z";
    update["updates"][0]["values"][0]["path"] = "environment.wind.speedTrue";
    update["updates"][0]["values"][0]["value"] = 12.5;

    dsk.SendSKDelta(update);

    // First access locks to VDR_halso-VD
    const Json::Value* val
        = instr.GetSKDataResolved("vessels.urn:mrn:imo:mmsi:265599691."
                                  "environment.wind.speedTrue.SRC:lockpersist");
    REQUIRE(val != nullptr);
    REQUIRE(instr.GetLockedSource().IsSameAs("VDR_halso-VD"));

    // Simulate save/load cycle
    Json::Value config = instr.GenerateJSONConfig();
    REQUIRE(config.isMember("locked_source"));
    REQUIRE(fromJsonVal(config["locked_source"].asString())
            .IsSameAs("VDR_halso-VD"));

    // Create new instrument and load config
    SimpleNumberInstrument instr2(db);
    instr2.ReadConfig(config);
    REQUIRE(instr2.GetLockedSource().IsSameAs("VDR_halso-VD"));
}

TEST_CASE("Magic source locks obey mode and path")
{
    DashboardSK dsk("");
    Dashboard* db = dsk.AddDashboard();
    SimpleNumberInstrument instr(db);

    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["$source"] = "old";
    update["updates"][0]["values"][0]["path"] = "oldValue";
    update["updates"][0]["values"][0]["value"] = 1;
    dsk.SendSKDelta(update);

    REQUIRE(instr.GetSKDataResolved("test.oldValue.SRC:lockfirst") != nullptr);
    instr.SetSetting(
        wxString(DSK_SETTING_SK_KEY), wxString("test.oldValue.SRC:lockfirst"));
    REQUIRE_FALSE(instr.GenerateJSONConfig().isMember("locked_source"));

    update["updates"][0]["$source"] = "new";
    update["updates"][0]["values"][0]["path"] = "newValue";
    update["updates"][0]["values"][0]["value"] = 2;
    dsk.SendSKDelta(update);

    REQUIRE(instr.GetSKDataResolved("test.newValue.SRC:lockfirst") != nullptr);
    REQUIRE(instr.GetLockedSource().IsSameAs("new"));
}

TEST_CASE("Magic source SRC:lockpersist falls back after grace period")
{
    DashboardSK dsk("");
    Dashboard* db = dsk.AddDashboard();
    SimpleNumberInstrument instr(db);
    instr.SetSetting("allowed_age", 0);
    instr.SetLockedSource("missing");

    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["$source"] = "available";
    update["updates"][0]["values"][0]["path"] = "value";
    update["updates"][0]["values"][0]["value"] = 1;
    dsk.SendSKDelta(update);

    REQUIRE(instr.GetSKDataResolved("test.value.SRC:lockpersist") != nullptr);
    REQUIRE(instr.GetLockedSource().IsSameAs("available"));
}

TEST_CASE("Exact source designation - unchanged behavior")
{
    DashboardSK dsk("");
    dsk.SetSelf("urn:mrn:imo:mmsi:265599691");

    Json::Value update;
    update["context"] = "vessels.urn:mrn:imo:mmsi:265599691";
    update["updates"][0]["$source"] = "VDR_halso-VD";
    update["updates"][0]["timestamp"] = "2024-01-01T00:00:00.000Z";
    update["updates"][0]["values"][0]["path"] = "environment.wind.speedTrue";
    update["updates"][0]["values"][0]["value"] = 12.5;

    dsk.SendSKDelta(update);

    // Exact source should work as before
    const Json::Value* val
        = dsk.GetSKData("vessels.urn:mrn:imo:mmsi:265599691.environment.wind."
                        "speedTrue.SRC:VDR_halso-VD");
    REQUIRE(val != nullptr);
    REQUIRE(val->isMember("value"));
    REQUIRE(val->get("value", Json::Value()).asDouble() == 12.5);

    // Non-existent exact source should return nullptr
    val = dsk.GetSKData("vessels.urn:mrn:imo:mmsi:265599691.environment.wind."
                        "speedTrue.SRC:NonExistent");
    REQUIRE(val == nullptr);
}

TEST_CASE("Path without source designation - unchanged behavior")
{
    DashboardSK dsk("");
    dsk.SetSelf("urn:mrn:imo:mmsi:265599691");

    Json::Value update;
    update["context"] = "vessels.urn:mrn:imo:mmsi:265599691";
    update["updates"][0]["source"]["src"] = "VDR_halso-VD";
    update["updates"][0]["timestamp"] = "2024-01-01T00:00:00.000Z";
    update["updates"][0]["values"][0]["path"] = "environment.wind.speedTrue";
    update["updates"][0]["values"][0]["value"] = 12.5;

    dsk.SendSKDelta(update);

    // Path without source should work as before (navigate to base path)
    const Json::Value* val = dsk.GetSKData(
        "vessels.urn:mrn:imo:mmsi:265599691.environment.wind.speedTrue");
    REQUIRE(val != nullptr);
    REQUIRE(val->isMember("value"));
}
