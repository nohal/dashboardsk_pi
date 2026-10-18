/******************************************************************************
 * DashboardSK manual screenshot generator
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

// Not a test: renders the SVG skin images of the manual straight into
// manual/modules/ROOT/images. Hidden, so it only runs when asked for:
//
//   ./build/tests/tests "[screenshots]"
//   pre-commit run oxipng --all-files

#include <catch2/catch_test_macros.hpp>

#include "dashboard.h"
#include "dashboardsk.h"
#include "svginstrument.h"

#include <wx/image.h>

using namespace DashboardSKPlugin;

namespace {
using T = Instrument::transformation;

/// One value slot of a screenshot.
struct shot_slot {
    /// Path under "test."
    const char* path;
    /// Value in Signal K units
    double value;
    /// Label setting
    const char* label;
    /// Unit setting
    const char* unit;
    /// Transformation setting
    T transform;
    /// Format setting, index into "Skin default" + DSK_VALUE_FORMATS
    int format;
};

/// One manual image.
struct shot {
    /// Image file name without the extension
    const char* image;
    /// Skin file name
    const char* skin;
    /// Instrument title
    const char* title;
    /// Values, in slot order
    std::vector<shot_slot> slots;
    /// Zones setting, empty for the skin's default
    const char* zones;
};

const char* const key_keys[] { DSK_SVG_KEY_0, DSK_SVG_KEY_1, DSK_SVG_KEY_2,
    DSK_SVG_KEY_3 };
const char* const label_keys[] { DSK_SVG_LABEL_0, DSK_SVG_LABEL_1,
    DSK_SVG_LABEL_2, DSK_SVG_LABEL_3 };
const char* const unit_keys[] { DSK_SVG_UNIT_0, DSK_SVG_UNIT_1, DSK_SVG_UNIT_2,
    DSK_SVG_UNIT_3 };
const char* const transform_keys[] { DSK_SVG_TRANSFORM_0, DSK_SVG_TRANSFORM_1,
    DSK_SVG_TRANSFORM_2, DSK_SVG_TRANSFORM_3 };
const char* const format_keys[] { DSK_SVG_FORMAT_0, DSK_SVG_FORMAT_1,
    DSK_SVG_FORMAT_2, DSK_SVG_FORMAT_3 };

/// Format choices used below
enum { skin_default = 0, f9_9 = 1, f9_99 = 2, f9 = 4 };
}

TEST_CASE("Generate the SVG skin images of the manual", "[.][screenshots]")
{
    const std::vector<shot> shots {
        { "svginstrument_screenshot", "wind_angle.svg", "",
            { { "awa", 1.5707963267948966, "", "", T::none, skin_default },
                { "aws", 5.0, "", "", T::none, skin_default } },
            "" },
        { "svginstrument_tachometer", "tachometer.svg", "",
            { { "rpm", 50.0, "", "", T::none, skin_default } }, "" },
        { "svginstrument_rudder", "rudder.svg", "",
            { { "rudder", 0.17453292519943295, "", "", T::none,
                skin_default } },
            "" },
        { "svginstrument_number_1", "number_1.svg", "",
            { { "sog", 3.29, "SOG", "kn", T::ms2kn, f9_9 } }, "" },
        { "svginstrument_number_2h", "number_2h.svg", "Wind",
            { { "aws", 7.3, "AWS", "kn", T::ms2kn, f9_9 },
                { "awa", 0.733, "AWA", "\xC2\xB0", T::rad2deg, f9 } },
            "" },
        { "svginstrument_number_2v", "number_2v.svg", "Depth",
            { { "dbt", 12.4, "Below transducer", "m", T::none, skin_default },
                { "dbk", 11.9, "Below keel", "m", T::none, skin_default } },
            "" },
        { "svginstrument_number_3v", "number_3v.svg", "Battery",
            { { "volt", 12.62, "Voltage", "V", T::none, f9_99 },
                { "amp", -4.3, "Current", "A", T::none, skin_default },
                { "soc", 0.78, "State of charge", "%", T::ratio2perc, f9 } },
            "" },
        { "svginstrument_number_4", "number_4.svg", "Engine",
            { { "rpm", 38.3, "RPM", "", T::hz2rpm, f9 },
                { "oil", 310000, "Oil", "kPa", T::pa2kpa, f9 },
                { "cool", 369.15, "Coolant",
                    "\xC2\xB0"
                    "C",
                    T::degk2degc, f9 },
                { "fuel", 0.42, "Fuel", "%", T::ratio2perc, f9 } },
            "363.15,400,alarm" },
    };
    if (!wxImage::FindHandler(wxBITMAP_TYPE_PNG)) {
        wxImage::AddHandler(new wxPNGHandler);
    }
    for (const shot& s : shots) {
        INFO(s.image);
        DashboardSK dsk(DSK_DATA_DIR);
        SvgInstrument instrument(dsk.AddDashboard());
        instrument.SetSetting(wxString(DSK_SVG_FILE), wxString(s.skin));
        REQUIRE(instrument.GetError().IsEmpty());
        instrument.SetSetting(wxString("title"), wxString::FromUTF8(s.title));
        for (size_t i = 0; i < s.slots.size(); ++i) {
            const shot_slot& v = s.slots[i];
            Json::Value update;
            update["context"] = "test";
            update["updates"][0]["values"][0]["path"] = v.path;
            update["updates"][0]["values"][0]["value"] = v.value;
            instrument.SetSetting(
                wxString(key_keys[i]), wxString("test.") + v.path);
            instrument.SetSetting(
                wxString(label_keys[i]), wxString::FromUTF8(v.label));
            instrument.SetSetting(
                wxString(unit_keys[i]), wxString::FromUTF8(v.unit));
            instrument.SetSetting(
                wxString(transform_keys[i]), static_cast<int>(v.transform));
            instrument.SetSetting(wxString(format_keys[i]), v.format);
            dsk.SendSKDelta(update);
        }
        if (*s.zones) {
            instrument.SetSetting(
                wxString(DSK_SETTING_ZONES), wxString(s.zones));
        }
        instrument.ProcessData();
        REQUIRE(instrument.Render(1.0).ConvertToImage().SaveFile(
            wxString(DSK_MANUAL_IMAGES_DIR) + s.image + ".png",
            wxBITMAP_TYPE_PNG));
    }
}
