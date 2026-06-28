/******************************************************************************
 * DashboardSK composite wind instrument tests
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include "compositewindinstrument.h"
#include "dashboard.h"
#include "dashboardsk.h"

using namespace DashboardSKPlugin;

TEST_CASE("Composite wind defaults and configuration round trip")
{
    CompositeWindInstrument instrument(nullptr);
    REQUIRE(instrument.Class().IsSameAs("CompositeWindInstrument"));
    REQUIRE(instrument.DisplayType().IsSameAs("Composite wind"));
    REQUIRE(instrument.ConfigControls().size() == 18);

    Json::Value config;
    config[DSK_CWI_AWA_KEY] = "vessels.self.environment.wind.angleApparent";
    config[DSK_CWI_ORIENTATION] = 1;
    config[DSK_SETTING_INSTR_SIZE] = 240;
    instrument.ReadConfig(config);

    const Json::Value stored = instrument.GenerateJSONConfig();
    REQUIRE(fromJsonVal(stored[DSK_CWI_AWA_KEY].asString())
            .IsSameAs("vessels.self.environment.wind.angleApparent"));
    REQUIRE(stored[DSK_CWI_ORIENTATION].asInt() == 1);
    REQUIRE(stored[DSK_SETTING_INSTR_SIZE].asInt() == 240);
    REQUIRE(fromJsonVal(stored[DSK_CWI_COG_COLOR].asString()).StartsWith("#"));
}

TEST_CASE("Composite wind renders a square bitmap with transparent center")
{
    CompositeWindInstrument instrument(nullptr);
    wxBitmap bitmap = instrument.Render(1.0);
    REQUIRE(bitmap.IsOk());
    REQUIRE(bitmap.GetWidth() == 200);
    REQUIRE(bitmap.GetHeight() == 200);

    wxImage image = bitmap.ConvertToImage();
    REQUIRE(image.HasAlpha());
    REQUIRE(image.GetAlpha(100, 100) == 0);
    REQUIRE(image.GetAlpha(100, 0) == 0);
    REQUIRE(image.GetAlpha(0, 100) == 0);
    REQUIRE(image.GetAlpha(199, 100) == 0);
    REQUIRE(image.GetAlpha(100, 199) == 0);
}

TEST_CASE("Composite wind hit testing excludes its hollow center")
{
    CompositeWindInstrument instrument(nullptr);
    instrument.SetPlacement(20, 30, 200, 200);

    REQUIRE_FALSE(instrument.IsClicked(120, 130));
    REQUIRE(instrument.IsClicked(120, 40));
    REQUIRE_FALSE(instrument.IsClicked(20, 30));
}

TEST_CASE("Composite wind plots apparent wind relative to heading")
{
    DashboardSK dsk("");
    Dashboard* dashboard = dsk.AddDashboard();
    CompositeWindInstrument instrument(dashboard);
    instrument.SetSetting(wxString(DSK_CWI_AWA_KEY), wxString("test.awa"));
    instrument.SetSetting(
        wxString(DSK_CWI_HEADING_KEY), wxString("test.heading"));

    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["values"][0]["path"] = "awa";
    update["updates"][0]["values"][0]["value"] = 1.5707963267948966;
    update["updates"][0]["values"][1]["path"] = "heading";
    update["updates"][0]["values"][1]["value"] = 0.0;
    dsk.SendSKDelta(update);

    const wxImage image = instrument.Render(1.0).ConvertToImage();
    bool found_blue = false;
    for (int x = 165; x < 190; ++x) {
        for (int y = 85; y < 115; ++y) {
            found_blue = found_blue
                || (image.GetRed(x, y) < 100 && image.GetGreen(x, y) > 120
                    && image.GetBlue(x, y) > 200);
        }
    }
    REQUIRE(found_blue);
}

TEST_CASE("Composite wind applies OpenCPN magnetic variation to heading")
{
    DashboardSK dsk("");
    Dashboard* dashboard = dsk.AddDashboard();
    CompositeWindInstrument instrument(dashboard);
    instrument.SetSetting(
        wxString(DSK_CWI_HEADING_KEY), wxString("test.heading"));
    dsk.SetMagneticVariation(90.0);

    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["values"][0]["path"] = "heading";
    update["updates"][0]["values"][0]["value"] = 0.0;
    dsk.SendSKDelta(update);

    const wxImage image = instrument.Render(1.0).ConvertToImage();
    bool found_heading = false;
    for (int x = 175; x < 198; ++x) {
        for (int y = 90; y < 110; ++y) {
            found_heading = found_heading
                || (image.GetRed(x, y) > 200 && image.GetGreen(x, y) > 200
                    && image.GetBlue(x, y) > 200);
        }
    }
    REQUIRE(found_heading);
}
