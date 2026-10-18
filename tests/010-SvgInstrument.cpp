/******************************************************************************
 * DashboardSK SVG skin instrument tests
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include "dashboard.h"
#include "dashboardsk.h"
#include "svginstrument.h"

#include <wx/ffile.h>
#include <wx/filename.h>

using namespace DashboardSKPlugin;

namespace {
/// Send one numeric value for path "test.<path>" through the delta pipeline.
void Send(DashboardSK& dsk, const char* path, double value)
{
    Json::Value update;
    update["context"] = "test";
    update["updates"][0]["values"][0]["path"] = path;
    update["updates"][0]["values"][0]["value"] = value;
    dsk.SendSKDelta(update);
}
}

TEST_CASE("SVG skin defaults and configuration round trip")
{
    SvgInstrument instrument(nullptr);
    REQUIRE(instrument.Class().IsSameAs("SvgInstrument"));
    REQUIRE(instrument.ConfigControls().size() == 23);

    Json::Value config;
    config[DSK_SVG_KEY_2] = "vessels.self.navigation.speedOverGround";
    config[DSK_SETTING_INSTR_SIZE] = 120;
    instrument.ReadConfig(config);
    const Json::Value stored = instrument.GenerateJSONConfig();
    REQUIRE(fromJsonVal(stored[DSK_SVG_KEY_2].asString())
            .IsSameAs("vessels.self.navigation.speedOverGround"));
    REQUIRE(stored[DSK_SETTING_INSTR_SIZE].asInt() == 120);
    REQUIRE(fromJsonVal(stored[DSK_SVG_FILE].asString())
            .IsSameAs("wind_angle.svg"));
}

TEST_CASE("SVG skin with a missing file renders a placeholder")
{
    SvgInstrument instrument(nullptr);
    instrument.SetSetting(
        wxString(DSK_SVG_FILE), wxString("/nonexistent/skin.svg"));
    REQUIRE_FALSE(instrument.GetError().IsEmpty());
    REQUIRE(instrument.PrepareSvg().empty());
    REQUIRE(instrument.Render(1.0).IsOk());
}

TEST_CASE("SVG skin binds values to rotation and text")
{
    DashboardSK dsk(DSK_DATA_DIR);
    Dashboard* dashboard = dsk.AddDashboard();
    SvgInstrument instrument(dashboard);
    // Relative file name resolves into <data dir>/svg/
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("wind_angle.svg"));
    REQUIRE(instrument.GetError().IsEmpty());
    instrument.SetSetting(wxString(DSK_SVG_KEY_0), wxString("test.awa"));
    instrument.SetSetting(wxString(DSK_SVG_KEY_1), wxString("test.aws"));

    // Nothing received yet: needle at rest, values dashed
    std::string svg = instrument.PrepareSvg();
    REQUIRE(svg.find("rotate(") == std::string::npos);
    REQUIRE(svg.find(">---<") != std::string::npos);

    Send(dsk, "awa", 1.5707963267948966);
    Send(dsk, "aws", 5.0);
    instrument.ProcessData();
    svg = instrument.PrepareSvg();
    REQUIRE(svg.find("rotate(90.00 100 100)") != std::string::npos);
    REQUIRE(svg.find(">90\xC2\xB0<") != std::string::npos);
    REQUIRE(svg.find(">9.7 kn<") != std::string::npos);

    const wxBitmap bmp = instrument.Render(1.0);
    REQUIRE(bmp.GetWidth() == 200);
    REQUIRE(bmp.GetHeight() == 200);
    // The blue needle points east (90 deg apparent)
    const wxImage image = bmp.ConvertToImage();
    REQUIRE(image.GetBlue(160, 100) > 200);
    REQUIRE(image.GetRed(160, 100) < 120);
}

TEST_CASE("SVG skin rejects unsafe number formats from the file")
{
    const wxString path = wxFileName::CreateTempFileName("dsksvg");
    wxFFile f(path, "w");
    f.Write("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 10 10\">"
            "<text x=\"0\" y=\"5\" data-dsk-text=\"v0\" data-dsk-fmt=\"%s%n\">"
            "</text></svg>");
    f.Close();
    DashboardSK dsk("");
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), path);
    instrument.SetSetting(wxString(DSK_SVG_KEY_0), wxString("test.v"));
    Send(dsk, "v", 3.4);
    instrument.ProcessData();
    REQUIRE(instrument.PrepareSvg().find(">3<") != std::string::npos);
    wxRemoveFile(path);
}

TEST_CASE("SVG skin colors follow zones and the color scheme")
{
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_KEY_1), wxString("test.aws"));
    instrument.SetSetting(wxString(DSK_SETTING_ZONES), wxString("4,10,alarm"));
    Send(dsk, "aws", 5.0);
    instrument.ProcessData();

    // Day: authored colors untouched, the AWS text in the alarm color
    std::string svg = instrument.PrepareSvg();
    REQUIRE(svg.find("#4DA3FF") != std::string::npos);
    REQUIRE(svg.find("#F98B60") != std::string::npos);

    // Night: everything, zone colors included, has its luminosity reduced
    instrument.SetColorScheme(3);
    svg = instrument.PrepareSvg();
    REQUIRE(svg.find("#4DA3FF") == std::string::npos);
    REQUIRE(svg.find("#F98B60") == std::string::npos);
    REQUIRE(svg.find("#0F2033") != std::string::npos);
    const wxImage image = instrument.Render(1.0).ConvertToImage();
    REQUIRE(image.GetBlue(100, 40) < 80); // dimmed needle tip at rest
}

TEST_CASE("SVG skin catalog merges bundled and imported skins")
{
    // A temporary user directory with an override and a new skin
    const wxString tmp = wxFileName::CreateTempFileName("dskuser");
    wxRemoveFile(tmp);
    const wxString user = tmp + wxFileName::GetPathSeparator();
    const wxString dir = SvgInstrument::SkinDir(user);
    REQUIRE(wxFileName::Mkdir(dir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL));
    REQUIRE(wxCopyFile(SvgInstrument::SkinDir(DSK_DATA_DIR) + "wind_angle.svg",
        dir + "wind_angle.svg"));
    REQUIRE(wxCopyFile(SvgInstrument::SkinDir(DSK_DATA_DIR) + "wind_angle.svg",
        dir + "a.svg"));

    const wxArrayString skins = SvgInstrument::ListSkins(DSK_DATA_DIR, user);
    // Bundled + one new, the override is listed once, sorted
    REQUIRE(
        skins.size() == SvgInstrument::ListSkins(DSK_DATA_DIR, "").size() + 1);
    REQUIRE(skins[0] == "a.svg");
    for (size_t i = 1; i < skins.size(); ++i) {
        REQUIRE(skins[i - 1] < skins[i]);
    }
    REQUIRE(SvgInstrument::ResolveSkin("wind_angle.svg", DSK_DATA_DIR, user)
        == dir + "wind_angle.svg");
    REQUIRE(SvgInstrument::ResolveSkin("wind_angle.svg", DSK_DATA_DIR, "")
        == SvgInstrument::SkinDir(DSK_DATA_DIR) + "wind_angle.svg");
    REQUIRE(SvgInstrument::ValidateSkin(dir + "a.svg").IsEmpty());

    // Imported skins are found by instruments of a configured DashboardSK
    DashboardSK dsk(DSK_DATA_DIR);
    dsk.SetUserDir(user);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("a.svg"));
    REQUIRE(instrument.GetError().IsEmpty());

    wxFileName::Rmdir(user, wxPATH_RMDIR_RECURSIVE);
}

TEST_CASE("SVG skin validation rejects unusable files")
{
    const wxString path = wxFileName::CreateTempFileName("dsksvg");
    auto check = [&path](const char* content) {
        wxFFile f(path, "w");
        f.Write(content);
        f.Close();
        return SvgInstrument::ValidateSkin(path);
    };
    REQUIRE_FALSE(check("not xml at all <").IsEmpty());
    REQUIRE_FALSE(check("<html><body/></html>").IsEmpty());
    REQUIRE_FALSE(
        check("<svg xmlns=\"http://www.w3.org/2000/svg\"/>").IsEmpty());
    REQUIRE(check("<svg xmlns=\"http://www.w3.org/2000/svg\" "
                  "viewBox=\"0 0 1 1\"/>")
            .IsEmpty());
    wxRemoveFile(path);
}

TEST_CASE("SVG skin offers only the settings the skin uses")
{
    auto keys = [](const SvgInstrument& i) {
        wxArrayString out;
        for (const auto& c : i.ConfigControls()) {
            out.Add(c.key);
        }
        return out;
    };
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_KEY_3), wxString("test.unused"));

    // The sample uses v0, v1 (with their readout formats) and zones, and
    // names its slots
    wxArrayString k = keys(instrument);
    REQUIRE(k.size() == 9);
    REQUIRE(k.Index(DSK_SVG_KEY_0) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_KEY_1) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_KEY_2) == wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SETTING_ZONES) != wxNOT_FOUND);
    REQUIRE(instrument.ConfigControls()[k.Index(DSK_SVG_KEY_0)].description
        == "Apparent wind angle");

    // A skin with one rotation and no zones
    const wxString path = wxFileName::CreateTempFileName("dsksvg");
    wxFFile f(path, "w");
    f.Write("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 10 10\">"
            "<path d=\"M5 0v5\" stroke=\"red\" data-dsk-rotate=\"v2\"/></svg>");
    f.Close();
    instrument.SetSetting(wxString(DSK_SVG_FILE), path);
    k = keys(instrument);
    REQUIRE(k.size() == 3);
    REQUIRE(k.Index(DSK_SVG_KEY_2) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SETTING_ZONES) == wxNOT_FOUND);
    REQUIRE(instrument.ConfigControls()[k.Index(DSK_SVG_KEY_2)].description
        == "Value v2 key");

    // Hidden slots keep their keys, a broken skin shows everything
    REQUIRE(
        fromJsonVal(instrument.GenerateJSONConfig()[DSK_SVG_KEY_3].asString())
            .IsSameAs("test.unused"));
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("/nonexistent.svg"));
    REQUIRE(keys(instrument).size() == 23);
    wxRemoveFile(path);
}

TEST_CASE("All bundled SVG skins are valid and render")
{
    const wxArrayString skins = SvgInstrument::ListSkins(DSK_DATA_DIR, "");
    REQUIRE(skins.size() >= 8);
    DashboardSK dsk(DSK_DATA_DIR);
    for (const wxString& skin : skins) {
        INFO(skin.ToStdString());
        REQUIRE(SvgInstrument::ValidateSkin(
            SvgInstrument::SkinDir(DSK_DATA_DIR) + skin)
                .IsEmpty());
        SvgInstrument instrument(dsk.AddDashboard());
        instrument.SetSetting(wxString(DSK_SVG_FILE), skin);
        REQUIRE(instrument.GetError().IsEmpty());
        REQUIRE(instrument.Render(1.0).IsOk());
    }
}

TEST_CASE("Tachometer skin maps revolutions to the dial")
{
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("tachometer.svg"));
    instrument.SetSetting(wxString(DSK_SVG_KEY_0), wxString("test.rev"));
    Send(dsk, "rev", 50.0); // Hz
    instrument.ProcessData();
    const std::string svg = instrument.PrepareSvg();
    REQUIRE(svg.find("rotate(67.50 100 100)") != std::string::npos);
    REQUIRE(svg.find(">3000<") != std::string::npos);
}

TEST_CASE("Rudder skin clamps the angle and keeps its aspect ratio")
{
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("rudder.svg"));
    instrument.SetSetting(wxString(DSK_SVG_KEY_0), wxString("test.rudder"));
    Send(dsk, "rudder", 0.17453292519943295);
    instrument.ProcessData();
    std::string svg = instrument.PrepareSvg();
    REQUIRE(svg.find("rotate(10.00 100 115)") != std::string::npos);
    REQUIRE(svg.find(">+10\xC2\xB0<") != std::string::npos);

    Send(dsk, "rudder", 1.0);
    instrument.ProcessData();
    svg = instrument.PrepareSvg();
    REQUIRE(svg.find("rotate(35.00 100 115)") != std::string::npos);
    const wxBitmap bmp = instrument.Render(1.0);
    REQUIRE(bmp.GetWidth() == 200);
    REQUIRE(bmp.GetHeight() == 120);
}

TEST_CASE("SVG skins suggest Signal K paths for empty slots")
{
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    auto key = [&instrument](
                   const char* k) { return instrument.GetStringSetting(k); };

    // A fresh instrument takes the paths from the default wind skin
    REQUIRE(key(DSK_SVG_KEY_0) == "environment.wind.angleApparent");
    REQUIRE(key(DSK_SVG_KEY_1) == "environment.wind.speedApparent");
    REQUIRE(key(DSK_SVG_KEY_2).IsEmpty());

    // Switching skins keeps paths already set...
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("rudder.svg"));
    REQUIRE(key(DSK_SVG_KEY_0) == "environment.wind.angleApparent");
    // ...but a cleared slot gets the new skin's suggestion
    instrument.SetSetting(wxString(DSK_SVG_KEY_0), wxString());
    REQUIRE(key(DSK_SVG_KEY_0) == "steering.rudderAngle");

    // Empty slots stored in an older config are filled on load too
    Json::Value config;
    config[DSK_SVG_FILE] = "tachometer.svg";
    config[DSK_SVG_KEY_0] = "";
    SvgInstrument loaded(dsk.AddDashboard());
    loaded.ReadConfig(config);
    REQUIRE(loaded.GetStringSetting(DSK_SVG_KEY_0)
        == "propulsion.main.revolutions");
    REQUIRE(fromJsonVal(loaded.GenerateJSONConfig()[DSK_SVG_KEY_0].asString())
            .IsSameAs("propulsion.main.revolutions"));
}

TEST_CASE("Tachometer scale and zones are set in RPM")
{
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("tachometer.svg"));
    instrument.SetSetting(wxString(DSK_SVG_KEY_0), wxString("test.rev"));

    // Skin defaults: 0-4000 rpm scale, red band from 3500 rpm
    REQUIRE(instrument.GetDoubleSetting(DSK_SVG_SCALE_MAX) == 4000);
    REQUIRE(instrument.GetStringSetting(DSK_SETTING_ZONES).StartsWith("3500"));
    wxArrayString keys;
    for (const auto& c : instrument.ConfigControls()) {
        keys.Add(c.key);
    }
    REQUIRE(keys.Index(DSK_SVG_SCALE_MIN) != wxNOT_FOUND);

    Send(dsk, "rev", 60.0); // 3600 rpm, inside the red band
    instrument.ProcessData();
    std::string svg = instrument.PrepareSvg();
    REQUIRE(svg.find(">40<") != std::string::npos); // last tick label
    // One generated arc in the alarm color, the needle turns alarm colored
    size_t arcs = 0;
    for (size_t pos = 0;
        (pos = svg.find("stroke=\"#F98B60\"", pos)) != std::string::npos;
        ++pos) {
        ++arcs;
    }
    REQUIRE(arcs == 1);
    REQUIRE(svg.find("fill=\"#F98B60\"") != std::string::npos);

    // A bigger engine: 0-6000 rpm moves the needle and relabels the dial
    instrument.SetSetting(wxString(DSK_SVG_SCALE_MAX), wxString("6000"));
    instrument.SetSetting(
        wxString(DSK_SETTING_ZONES), wxString("5000,6000,alarm"));
    svg = instrument.PrepareSvg();
    REQUIRE(svg.find("rotate(27.00 100 100)") != std::string::npos);
    REQUIRE(svg.find(">60<") != std::string::npos);
    REQUIRE(svg.find("fill=\"#F98B60\"") == std::string::npos); // 3600 is ok
    // Regenerating doesn't pile up arcs
    svg = instrument.PrepareSvg();
    REQUIRE(svg.find("stroke=\"#F98B60\"") == svg.rfind("stroke=\"#F98B60\""));
}

TEST_CASE("SVG skin scale settings take decimals")
{
    auto step = [](const SvgInstrument& i) {
        for (const auto& c : i.ConfigControls()) {
            if (c.key.IsSameAs(DSK_SVG_SCALE_MAX)) {
                return c.control_settings.AfterLast(';');
            }
        }
        return wxString();
    };
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("tachometer.svg"));
    REQUIRE(step(instrument) == "10"); // 1/100 of 0-4000, a power of ten

    // A small range gets a fine step; decimals survive a config round trip
    const wxString path = wxFileName::CreateTempFileName("dsksvg");
    wxFFile f(path, "w");
    f.Write("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 10 10\" "
            "data-dsk-scale=\"v0\" data-dsk-scale-range=\"0 2.5\">"
            "<path d=\"M5 0v5\" stroke=\"red\" data-dsk-rotate=\"v0\" "
            "data-dsk-sweep=\"0 90\"/></svg>");
    f.Close();
    instrument.SetSetting(wxString(DSK_SVG_FILE), path);
    REQUIRE(step(instrument) == "0.01");
    instrument.SetSetting(wxString(DSK_SVG_SCALE_MAX), wxString("1.75"));
    Json::Value config = instrument.GenerateJSONConfig();
    SvgInstrument loaded(dsk.AddDashboard());
    loaded.ReadConfig(config);
    REQUIRE(loaded.GetDoubleSetting(DSK_SVG_SCALE_MAX) == 1.75);
    wxRemoveFile(path);
}

TEST_CASE("Number skins show user labels, units, title and formats")
{
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("number_2h.svg"));
    REQUIRE(instrument.GetError().IsEmpty());
    instrument.SetSetting(wxString(DSK_SVG_KEY_0), wxString("test.sog"));
    Send(dsk, "sog", 2.5);
    instrument.ProcessData();

    // Nothing set: the skin's format, no title or labels
    std::string svg = instrument.PrepareSvg();
    REQUIRE(svg.find(">2.5<") != std::string::npos);
    REQUIRE(svg.find("???") == std::string::npos);
    REQUIRE(svg.find(">Label<") == std::string::npos);
    REQUIRE(svg.find(">Title<") == std::string::npos);

    instrument.SetSetting(wxString("title"), wxString("Speed"));
    instrument.SetSetting(wxString(DSK_SVG_LABEL_0), wxString("SOG"));
    instrument.SetSetting(wxString(DSK_SVG_UNIT_0), wxString("kn"));
    instrument.SetSetting(wxString(DSK_SVG_TRANSFORM_0),
        static_cast<int>(Instrument::transformation::ms2kn));
    // "9.99", the second format after "Skin default"
    instrument.SetSetting(wxString(DSK_SVG_FORMAT_0), 2);
    svg = instrument.PrepareSvg();
    REQUIRE(svg.find(">Speed<") != std::string::npos);
    REQUIRE(svg.find(">SOG<") != std::string::npos);
    REQUIRE(svg.find(">kn<") != std::string::npos);
    REQUIRE(svg.find(">4.86<") != std::string::npos);

    // "Skin default" uses the skin's format, labels can be cleared
    instrument.SetSetting(wxString(DSK_SVG_FORMAT_0), 0);
    instrument.SetSetting(wxString(DSK_SVG_LABEL_0), wxString());
    svg = instrument.PrepareSvg();
    REQUIRE(svg.find(">4.9<") != std::string::npos);
    REQUIRE(svg.find(">SOG<") == std::string::npos);

    // The settings survive a configuration round trip
    const Json::Value config = instrument.GenerateJSONConfig();
    REQUIRE(fromJsonVal(config[DSK_SVG_UNIT_0].asString()).IsSameAs("kn"));
    REQUIRE(config[DSK_SVG_TRANSFORM_0].asInt()
        == static_cast<int>(Instrument::transformation::ms2kn));

    // Without a transformation the skin's multiplier applies again
    instrument.SetSetting(wxString(DSK_SVG_TRANSFORM_0), 0);
    REQUIRE(instrument.PrepareSvg().find(">2.5<") != std::string::npos);

    // Negative values keep the sign in front of the padding, ABS drops it
    Send(dsk, "sog", -2.5);
    instrument.ProcessData();
    instrument.SetSetting(wxString(DSK_SVG_FORMAT_0), 6); // "009.9"
    REQUIRE(instrument.PrepareSvg().find(">-002.5<") != std::string::npos);
    instrument.SetSetting(wxString(DSK_SVG_FORMAT_0), 15); // "ABS(009.9)"
    REQUIRE(instrument.PrepareSvg().find(">002.5<") != std::string::npos);
}

TEST_CASE("Text settings are offered only for slots the skin shows")
{
    auto keys = [](const SvgInstrument& i) {
        wxArrayString out;
        for (const auto& c : i.ConfigControls()) {
            out.Add(c.key);
        }
        return out;
    };
    DashboardSK dsk(DSK_DATA_DIR);
    SvgInstrument instrument(dsk.AddDashboard());
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("number_2h.svg"));
    wxArrayString k = keys(instrument);
    REQUIRE(k.Index(DSK_SVG_LABEL_1) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_UNIT_1) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_TRANSFORM_1) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_FORMAT_1) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_LABEL_2) == wxNOT_FOUND);
    REQUIRE(instrument.ConfigControls()[k.Index(DSK_SVG_LABEL_0)].description
        == "Left value label");

    // The rudder has a readout but no label or unit to fill in
    instrument.SetSetting(wxString(DSK_SVG_FILE), wxString("rudder.svg"));
    k = keys(instrument);
    REQUIRE(k.Index(DSK_SVG_TRANSFORM_0) != wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_LABEL_0) == wxNOT_FOUND);
    REQUIRE(k.Index(DSK_SVG_UNIT_0) == wxNOT_FOUND);
}
