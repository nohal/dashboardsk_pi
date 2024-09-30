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
#include "wx/jsonreader.h"
#include "wx/jsonval.h"
#include "wx/jsonwriter.h"
#include <wx/wfstream.h>

#include "dashboard.h"
#include "dashboardsk.h"
#include "instrument.h"
#include "simplenumberinstrument.h"

using namespace DashboardSKPlugin;

TEST_CASE("DashboardSK Creation - properties set to defaults")
{
    DashboardSK d;

    REQUIRE(true); // TODO
}

TEST_CASE("DashboardSK Configuration Storage - if JSON not complete, defaults "
          "have to stay")
{
    DashboardSK d;

    REQUIRE(true); // TODO
}

TEST_CASE("DashboardSK - ID normalization")
{
    DashboardSK d;
    d.SetSelf("vessels.urn:mrn:imo:mmsi:223456789");

    REQUIRE(d.NormalizeID("123456789").IsSameAs("urn:mrn:imo:mmsi:123456789"));
    REQUIRE(d.NormalizeID("c0d79334-4e25-4245-8892-54e8ccc8021d")
            .IsSameAs(
                "urn:mrn:signalk:uuid:c0d79334-4e25-4245-8892-54e8ccc8021d"));
    REQUIRE(d.NormalizeID("http://example.com")
            .IsSameAs("urn:mrn:http://example.com"));
    REQUIRE(d.NormalizeID("https://example.com")
            .IsSameAs("urn:mrn:https://example.com"));
    REQUIRE(d.NormalizeID("mailto:test@example.com")
            .IsSameAs("urn:mrn:mailto:test@example.com"));
    REQUIRE(d.NormalizeID("tel:+420603200300")
            .IsSameAs("urn:mrn:tel:+420603200300"));
    REQUIRE(d.NormalizeID("") == ("urn:mrn:imo:mmsi:223456789"));
}

TEST_CASE("DashboardSK - Meta Delta")
{
    DashboardSK d;
    wxJSONValue v;
    wxJSONReader r;
    wxFileInputStream s("samples/delta/docs-data_model_meta_deltas.json");
    r.Parse(s, &v);
    d.SendSKDelta(v);
    wxJSONWriter w;
    wxString st;
    w.Write(*d.GetSignalKTree(), st);

    REQUIRE((*d.GetSignalKTree())["vessels"]["urn:mrn:imo:mmsi:234567890"]
                                 ["environment"]["wind"]["speedApparent"]
                                     .HasMember("meta"));
    REQUIRE((*d.GetSignalKTree())["vessels"]["urn:mrn:imo:mmsi:234567890"]
                                 ["environment"]["wind"]["speedApparent"]
                                 ["meta"]["zones"]
                                     .IsArray());
}
