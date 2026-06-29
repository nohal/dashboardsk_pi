/******************************************************************************
 * DashboardSK composite wind instrument tests
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include "compositewindinstrument.h"
#include "dashboard.h"
#include "dashboardsk.h"

#include <cmath>

using namespace DashboardSKPlugin;

namespace {
constexpr double test_pi = 3.14159265358979323846;

/// Scan a dial bearing (compass degrees from the bitmap center) for a pixel
/// matching @p pred, between the hollow center and the rim so the result is
/// independent of the transparent margin around the dial.
template <typename Pred>
bool BearingHasColor(const wxImage& image, double bearing, Pred pred)
{
    const double cx = image.GetWidth() / 2.0;
    const double cy = image.GetHeight() / 2.0;
    const double angle = (bearing - 90.0) * test_pi / 180.0;
    const double half = wxMin(cx, cy);
    for (double r = half * 0.32; r <= half * 0.74; r += 0.5) {
        const int x = static_cast<int>(cx + r * std::cos(angle));
        const int y = static_cast<int>(cy + r * std::sin(angle));
        if (image.GetAlpha(x, y) > 0
            && pred(image.GetRed(x, y), image.GetGreen(x, y),
                image.GetBlue(x, y))) {
            return true;
        }
    }
    return false;
}

/// Predicate matching the default red port color.
bool IsPort(int r, int g, int b) { return r > 200 && g < 120 && b < 120; }
/// Predicate matching the default green starboard color.
bool IsStarboard(int r, int g, int b) { return g > 150 && r < 100 && b < 150; }
}

TEST_CASE("Composite wind defaults and configuration round trip")
{
    CompositeWindInstrument instrument(nullptr);
    REQUIRE(instrument.Class().IsSameAs("CompositeWindInstrument"));
    REQUIRE(instrument.DisplayType().IsSameAs("Composite wind"));
    REQUIRE(instrument.ConfigControls().size() == 28);

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
    // The bitmap is enlarged beyond the 200 px dial so laylines can protrude.
    REQUIRE(bitmap.GetWidth() == bitmap.GetHeight());
    REQUIRE(bitmap.GetWidth() > 200);

    wxImage image = bitmap.ConvertToImage();
    const int n = bitmap.GetWidth();
    const int c = n / 2;
    REQUIRE(image.HasAlpha());
    REQUIRE(image.GetAlpha(c, c) == 0); // hollow dial center
    REQUIRE(image.GetAlpha(c, 0) == 0); // transparent top margin
    REQUIRE(image.GetAlpha(0, c) == 0); // transparent left margin
    REQUIRE(image.GetAlpha(n - 1, c) == 0); // transparent right margin
    REQUIRE(image.GetAlpha(c, n - 1) == 0); // transparent bottom margin

    // Layout reserves only the 200 px dial, not the enlarged bitmap, so the
    // protruding laylines and the transparent margin do not push neighbours.
    REQUIRE(instrument.ContentSize(bitmap).GetWidth() == 200);
    REQUIRE(instrument.ContentSize(bitmap).GetHeight() == 200);
    REQUIRE(instrument.ContentSize(wxBitmap()).GetWidth() == 0);
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

    // Apparent wind 90 deg off a north heading points due east on the dial.
    const wxImage image = instrument.Render(1.0).ConvertToImage();
    REQUIRE(BearingHasColor(image, 90.0,
        [](int r, int g, int b) { return r < 100 && g > 120 && b > 200; }));
}

TEST_CASE("Composite wind draws close-hauled laylines from the true wind")
{
    // True wind dead ahead (TWA 0), heading north: the port-tack layline lies
    // 45 deg off the wind at bearing 045 in the red port color.
    auto port_layline_pixel = [](CompositeWindInstrument& instrument) {
        const wxImage image = instrument.Render(1.0).ConvertToImage();
        return BearingHasColor(image, 45.0, IsPort);
    };

    DashboardSK dsk("");
    Dashboard* dashboard = dsk.AddDashboard();
    CompositeWindInstrument instrument(dashboard);
    instrument.SetSetting(wxString(DSK_CWI_TWA_KEY), wxString("test.twa"));
    instrument.SetSetting(
        wxString(DSK_CWI_HEADING_KEY), wxString("test.heading"));

    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["values"][0]["path"] = "twa";
    update["updates"][0]["values"][0]["value"] = 0.0;
    update["updates"][0]["values"][1]["path"] = "heading";
    update["updates"][0]["values"][1]["value"] = 0.0;
    dsk.SendSKDelta(update);

    REQUIRE(port_layline_pixel(instrument));

    instrument.SetSetting(wxString(DSK_CWI_SHOW_LAYLINES), 0);
    REQUIRE_FALSE(port_layline_pixel(instrument));
}

TEST_CASE("Composite wind switches to gybe laylines when running downwind")
{
    // True wind dead astern (TWA 180): the starboard gybe layline lies 150 deg
    // off the wind, i.e. bearing 030, drawn in the green starboard color in the
    // upper-right inner zone where a 45 deg beat layline would never reach.
    DashboardSK dsk("");
    Dashboard* dashboard = dsk.AddDashboard();
    CompositeWindInstrument instrument(dashboard);
    instrument.SetSetting(wxString(DSK_CWI_TWA_KEY), wxString("test.twa"));
    instrument.SetSetting(
        wxString(DSK_CWI_HEADING_KEY), wxString("test.heading"));

    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["values"][0]["path"] = "twa";
    update["updates"][0]["values"][0]["value"] = 3.141592653589793;
    update["updates"][0]["values"][1]["path"] = "heading";
    update["updates"][0]["values"][1]["value"] = 0.0;
    dsk.SendSKDelta(update);

    // TWA 180 selects the gybe laylines; the starboard one lies 150 deg off the
    // wind at bearing 030, a place a 45 deg beat layline would never reach.
    const wxImage image = instrument.Render(1.0).ConvertToImage();
    REQUIRE(BearingHasColor(image, 30.0, IsStarboard));
    REQUIRE_FALSE(BearingHasColor(image, 45.0, IsStarboard));
}

TEST_CASE("Composite wind beat angle key overrides the configured default")
{
    DashboardSK dsk("");
    Dashboard* dashboard = dsk.AddDashboard();
    CompositeWindInstrument instrument(dashboard);
    instrument.SetSetting(wxString(DSK_CWI_BEAT_KEY), wxString("test.beat"));

    Json::Value config;
    config[DSK_CWI_BEAT_KEY] = "test.beat";
    config[DSK_CWI_BEAT_ANGLE] = 50;
    config[DSK_CWI_SHOW_LAYLINES] = 1;
    instrument.ReadConfig(config);

    const Json::Value stored = instrument.GenerateJSONConfig();
    REQUIRE(stored[DSK_CWI_BEAT_ANGLE].asInt() == 50);
    REQUIRE(stored[DSK_CWI_SHOW_LAYLINES].asInt() == 1);
    REQUIRE(
        fromJsonVal(stored[DSK_CWI_BEAT_KEY].asString()).IsSameAs("test.beat"));
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

    // A north heading plus 90 deg variation points the white marker due east.
    const wxImage image = instrument.Render(1.0).ConvertToImage();
    REQUIRE(BearingHasColor(image, 90.0,
        [](int r, int g, int b) { return r > 200 && g > 200 && b > 200; }));
}

TEST_CASE("Composite wind ground track combines water course and current")
{
    // No current leaves the through-water course unchanged.
    REQUIRE(std::abs(CompositeWindInstrument::GroundTrack(45.0, 5.0, 123.0, 0.0)
                - 45.0)
        < 1e-6);
    // Equal northbound water and eastward current vectors bisect to 45 deg.
    REQUIRE(std::abs(CompositeWindInstrument::GroundTrack(0.0, 1.0, 90.0, 1.0)
                - 45.0)
        < 1e-6);
    // A westward current set bends a northbound boat's track to port (315 deg).
    REQUIRE(std::abs(CompositeWindInstrument::GroundTrack(0.0, 3.0, 270.0, 3.0)
                - 315.0)
        < 1e-6);
}

TEST_CASE("Composite wind uses the OpenCPN course only as a fallback")
{
    // The COG pointer's default color (138, 143, 152) leans blue (b > g > r),
    // which distinguishes it from the neutral grey of anti-aliased text. Non-
    // cardinal bearings (45/135) also keep clear of the cardinal letters.
    auto is_cog = [](int r, int g, int b) {
        return r > 110 && r < 170 && g > r && b > g + 3;
    };
    DashboardSK dsk("");
    Dashboard* dashboard = dsk.AddDashboard();
    CompositeWindInstrument instrument(dashboard);

    // No COG key configured: the OpenCPN fix (45 deg true) drives the pointer.
    dsk.SetOwnShipCOG(0.7853981633974483);
    wxImage image = instrument.Render(1.0).ConvertToImage();
    REQUIRE(BearingHasColor(image, 45.0, is_cog));

    // A configured Signal K key (135 deg) overrides the OpenCPN fix.
    instrument.SetSetting(wxString(DSK_CWI_COG_KEY), wxString("test.cog"));
    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["values"][0]["path"] = "cog";
    update["updates"][0]["values"][0]["value"] = 2.356194490192345;
    dsk.SendSKDelta(update);
    image = instrument.Render(1.0).ConvertToImage();
    REQUIRE(BearingHasColor(image, 135.0, is_cog));
    REQUIRE_FALSE(BearingHasColor(image, 45.0, is_cog));
}
