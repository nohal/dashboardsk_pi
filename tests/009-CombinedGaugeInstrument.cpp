/******************************************************************************
 * DashboardSK combined gauge instrument tests
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include "combinedgaugeinstrument.h"
#include "dashboard.h"
#include "dashboardsk.h"

using namespace DashboardSKPlugin;

TEST_CASE("Combined gauge defaults and configuration round trip")
{
    CombinedGaugeInstrument instrument(nullptr);
    REQUIRE(instrument.Class().IsSameAs("CombinedGaugeInstrument"));
    REQUIRE(instrument.DisplayType().IsSameAs("Combined gauge"));
    // Inherited simple gauge settings plus the two center value settings
    // (center key and center transformation); format/suffix are inherited.
    REQUIRE(instrument.ConfigControls().size() == 26);

    Json::Value config;
    config[DSK_SETTING_SK_KEY] = "vessels.self.environment.wind.angleApparent";
    config[DSK_CGI_CENTER_KEY] = "vessels.self.environment.wind.speedApparent";
    // Inherited format/suffix apply to the center value.
    config[DSK_SETTING_VALUE_SUFFIX] = "kn";
    config[DSK_SETTING_FORMAT] = 1;
    instrument.ReadConfig(config);

    // Primary key (driving the needle) is unchanged from the simple gauge.
    REQUIRE(instrument.GetPrimarySKKey().IsSameAs(
        "vessels.self.environment.wind.angleApparent"));

    const Json::Value stored = instrument.GenerateJSONConfig();
    REQUIRE(fromJsonVal(stored[DSK_CGI_CENTER_KEY].asString())
            .IsSameAs("vessels.self.environment.wind.speedApparent"));
    REQUIRE(fromJsonVal(stored[DSK_SETTING_VALUE_SUFFIX].asString())
            .IsSameAs("kn"));
    REQUIRE(stored[DSK_SETTING_FORMAT].asInt() == 1);
}

TEST_CASE("Combined gauge subscribes to both the primary and the center keys")
{
    DashboardSK dsk("");
    Dashboard* dashboard = dsk.AddDashboard();
    CombinedGaugeInstrument instrument(dashboard);
    instrument.SetSetting(wxString(DSK_SETTING_SK_KEY), wxString("test.awa"));
    instrument.SetSetting(wxString(DSK_CGI_CENTER_KEY), wxString("test.aws"));

    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["values"][0]["path"] = "awa";
    update["updates"][0]["values"][0]["value"] = 0.5;
    update["updates"][0]["values"][1]["path"] = "aws";
    update["updates"][0]["values"][1]["value"] = 7.5;
    dsk.SendSKDelta(update);

    // Both subscribed paths produce a valid render without throwing.
    wxBitmap bitmap = instrument.Render(1.0);
    REQUIRE(bitmap.IsOk());
}
