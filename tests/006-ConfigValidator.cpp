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

#include <fstream>
#include <sstream>

#include "pi_common.h"

#include "configvalidator.h"
#include "dashboard.h"
#include "dashboardsk.h"
#include "instrument.h"

using namespace DashboardSKPlugin;

static const wxString kSchema = wxString::FromUTF8(DSK_SCHEMA_PATH);

/// Read a whole file into a wxString (UTF-8)
static wxString ReadFile(const std::string& path)
{
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return wxString::FromUTF8(ss.str().c_str());
}

TEST_CASE("Shipped sample config validates against the schema")
{
    wxString text = ReadFile(DSK_SAMPLE_PATH);
    REQUIRE_FALSE(text.IsEmpty());
    wxString errors;
    bool ok = ValidateConfigJSON(
        text, kSchema, "#/definitions/DashboardSK", errors);
    INFO(errors.ToStdString());
    REQUIRE(ok);
}

TEST_CASE("A config saved by the plugin stays in line with the schema")
{
    // Build a config containing one instance of every registered instrument
    // type, exactly as the plugin would save it, and make sure the schema
    // still accepts it. This fails whenever an instrument starts emitting a
    // key the schema rejects.
    DashboardSK dsk(wxEmptyString);
    Dashboard* db = dsk.AddDashboard();
    REQUIRE(db != nullptr);
    const int type_count = (int)DashboardSK::GetInstrumentTypes().GetCount();
    REQUIRE(type_count > 0);
    for (int i = 0; i < type_count; i++) {
        Instrument* instr = DashboardSK::CreateInstrumentInstance(i, db);
        REQUIRE(instr != nullptr);
        db->AddInstrument(instr);
    }

    Json::Value config;
    config["shown"] = true;
    config["dashboardsk"] = dsk.GenerateJSONConfig();
    wxString text = DumpJSON(config);

    wxString errors;
    bool ok = ValidateConfigJSON(
        text, kSchema, "#/definitions/DashboardSK", errors);
    INFO(errors.ToStdString());
    REQUIRE(ok);
}

TEST_CASE("A malformed config is rejected")
{
    // Missing the required "class" key in an instrument config
    const wxString bad = R"({
        "shown": true,
        "dashboardsk": {
            "signalk": { "self": "x" },
            "dashboards": [
                { "name": "d", "instruments": [ { "config": { } } ] }
            ]
        }
    })";
    wxString errors;
    bool ok
        = ValidateConfigJSON(bad, kSchema, "#/definitions/DashboardSK", errors);
    INFO(errors.ToStdString());
    REQUIRE_FALSE(ok);
    REQUIRE_FALSE(errors.IsEmpty());
}
