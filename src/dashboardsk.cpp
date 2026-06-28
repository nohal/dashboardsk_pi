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

#include "dashboardsk.h"
#include "dashboardsk_pi.h"
#include <wx/tokenzr.h>

PLUGIN_BEGIN_NAMESPACE

DashboardSK::DashboardSK(const wxString& data_path)
    : m_parent_window(nullptr)
    , m_parent_plugin(nullptr)
    , m_self(wxEmptyString)
    , m_self_ptr(nullptr)
    , m_frozen(false)
    , m_color_scheme(0)
    , m_data_dir(data_path)
{
    for (int i = 0; i < GetCanvasCount(); i++) {
        m_displayed_pages.insert({ i, new Pager(this) });
    }
    m_sk_data["vessels"] = Json::Value(Json::objectValue);
}

void DashboardSK::ProcessData()
{
    for (auto dashboard : m_dashboards) {
        dashboard->ProcessData();
    }
}

int DashboardSK::ToPhys(int x) { return m_parent_plugin->ToPhys(x); }

void DashboardSK::Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex)
{
    if (m_displayed_pages.find(canvasIndex) == m_displayed_pages.end()) {
        m_displayed_pages[canvasIndex] = new Pager(this);
    }
    m_displayed_pages[canvasIndex]->Draw(dc, vp, canvasIndex);
    Dashboard::ClearOffsets();
    bool drawn = false;
    for (auto dashboard : m_dashboards) {
        if (!m_frozen
            && m_displayed_pages[canvasIndex]->GetCurrentPage()
                == dashboard->GetPageNr()) {
            dashboard->Draw(dc, vp, canvasIndex);
            drawn = true;
        } else {
            dashboard->ProcessData();
        }
    }
    if (!drawn) {
        m_displayed_pages[canvasIndex]->ResetCurrentPage();
    }
}

void DashboardSK::ReadConfig(Json::Value& config)
{
    LOG_VERBOSE("DashboardSK_pi: Reading DashboardSK config");
    for (auto db : m_dashboards) {
        delete db;
    }
    m_dashboards.clear();
    if (config["signalk"].isMember("self")) {
        SetSelf(fromJsonVal(config["signalk"]["self"].asString()));
    }
    if (!config.isMember("dashboards")) {
        LOG_VERBOSE("DashboardSK_pi: No dashboards node in JSON");
    }
    if (config["dashboards"].isArray()) {
        for (int i = 0; i < (int)config["dashboards"].size(); i++) {
            auto* d = new Dashboard(this);
            d->ReadConfig(config["dashboards"][i]);
            d->SetColorScheme(m_color_scheme);
            m_dashboards.emplace_back(d);
            if (m_displayed_pages.find(d->GetCanvasNr())
                == m_displayed_pages.end()) {
                m_displayed_pages[d->GetCanvasNr()] = new Pager(this);
            }
            m_displayed_pages[d->GetCanvasNr()]->AddPage(d->GetPageNr());
        }
    } else {
        LOG_VERBOSE("DashboardSK_pi: No dashboards array");
    }
    if (config["canvas"].isArray()) {
        for (int i = 0; i < (int)config["canvas"].size(); i++) {
            if (m_displayed_pages.find(config["canvas"][i]["id"].asInt())
                == m_displayed_pages.end()) {
                m_displayed_pages[config["canvas"][i]["id"].asInt()]
                    = new Pager(this);
            }
            m_displayed_pages[config["canvas"][i]["id"].asInt()]
                ->SetCurrentPage(config["canvas"][i]["page"].asInt());
        }
    }
    for (int i = 0; i < GetCanvasCount(); i++) {
        if (m_displayed_pages.find(i) == m_displayed_pages.end()) {
            m_displayed_pages[i] = new Pager(this);
            m_displayed_pages[i]->SetCurrentPage(1);
        }
    }
}

Json::Value DashboardSK::GenerateJSONConfig()
{
    Json::Value v;
    v["signalk"]["self"] = toJson(m_self);
    for (auto dashboard : m_dashboards) {
        v["dashboards"].append(dashboard->GenerateJSONConfig());
    }
    for (auto page : m_displayed_pages) {
        Json::Value vc;
        vc["id"] = page.first;
        vc["page"] = page.second->GetCurrentPage();
        v["canvas"].append(vc);
    }
    return v;
}

void DashboardSK::SetColorScheme(int cs)
{
    m_color_scheme = cs;
    for (auto dashboard : m_dashboards) {
        dashboard->SetColorScheme(cs);
    }
}

const int DashboardSK::GetColorScheme() { return m_color_scheme; }

const Json::Value* DashboardSK::GetSKData(const wxString& path)
{
    wxStringTokenizer tokenizer(path, ".");
    Json::Value* ptr = &m_sk_data;
    wxString token;
    while (tokenizer.HasMoreTokens()) {
        token = tokenizer.GetNextToken();
        const std::string key = token.ToStdString();
        if (ptr->isMember(key)) {
            ptr = &(*ptr)[key];
        } else {
            return nullptr; // Not found
        }
    }
    return ptr;
}

void DashboardSK::ProcessComplexValue(Json::Value* parent,
    const Json::Value& value, const wxDateTime& ts, const wxString& source)
{
    if (value.isObject()) {
        for (const std::string& val : value.getMemberNames()) {
            (*parent)[val] = Json::Value();
            ProcessComplexValue(
                &(*parent)[val], value.get(val, Json::Value()), ts, source);
        }
    } else {
        (*parent)["value"] = value;
        (*parent)["timestamp"] = toJson(ts.FormatISOCombined());
        (*parent)["source"] = toJson(source);
    }
}

void DashboardSK::SendSKDelta(Json::Value& message)
{
    LOG_RECEIVE("Received SK message: " + DumpJSON(message));
    if (m_self.IsEmpty() && message.isMember("self")) {
        // If we still don't have Self ID set, we accept it from the core
        // TODO: We perhaps might allow this until the user ever modifies it
        // manually in the prefs
        LOG_RECEIVE_DEBUG("Message contains self indentifier "
            + fromJsonVal(message["self"].asString()));
        SetSelf(fromJsonVal(message["self"].asString()));
    }
    wxString fullKey;
    if (!message.isMember("context")) {
        LOG_RECEIVE("Message does not contain context "
            + (message.isObject() && !message.getMemberNames().empty()
                    ? fromJsonVal(message.getMemberNames()[0])
                    : wxString()));
        if (!message.isMember("updates") || !message["updates"].isArray()) {
            LOG_RECEIVE("Message does not look OK");
            return; // Invalid SK delta
        }
        fullKey = "vessels.self";
    } else {
        fullKey = fromJsonVal(message["context"].asString());
    }
    LOG_RECEIVE_DEBUG("Message seems OK");

    int skip_tokens = 0;
    Json::Value* ptr;
    if (fullKey.StartsWith("vessels.self")) {
        // "vessels.self" translated to fully qualified identified ID and we can
        // skip the first two levels directly
        ptr = m_self_ptr;
        fullKey.Replace("vessels.self", "vessels." + Self());
        skip_tokens = 2;
    } else {
        ptr = &m_sk_data;
        fullKey = wxEmptyString;
    }
    LOG_RECEIVE_DEBUG("Full key before parsing: " + fullKey);
    wxStringTokenizer ctx_tokenizer(
        fromJsonVal(message["context"].asString()), ".");
    int token_nr = 0;
    wxString token;
    while (ctx_tokenizer.HasMoreTokens()) {
        ++token_nr;
        token = ctx_tokenizer.GetNextToken();
        if (token_nr > skip_tokens) {
            if (fullKey == wxEmptyString) {
                fullKey = token;
            } else {
                if (token_nr == 2) {
                    fullKey.Append(".").Append(NormalizeID(token));
                } else {
                    fullKey.Append(".").Append(token);
                }
            }

            const std::string tkey = token.ToStdString();
            if (!ptr->isMember(tkey)) {
                LOG_RECEIVE_DEBUG(
                    "Node does NOT have member " + token + ", adding it");
                (*ptr)[tkey] = Json::Value();
                ptr = &(*ptr)[tkey];
            } else {
                LOG_RECEIVE_DEBUG(
                    "Node does have member " + token + ", reusing it");
                ptr = &(*ptr)[tkey];
            }
        } else {
            skip_tokens--;
        }
    }
    LOG_RECEIVE_DEBUG("Full key after parsing: " + fullKey);
    wxDateTime ts;
    for (int i = 0; i < (int)message["updates"].size(); i++) {
        LOG_RECEIVE_DEBUG("processing update #%i", i);
        if (message["updates"][i].isMember("timestamp")) {
            if (!ts.ParseISOCombined(fromJsonVal(
                    message["updates"][i]["timestamp"].asString()))) {
                ts = wxDateTime::Now();
            }
        } else {
            ts = wxDateTime::Now();
        }
        // TODO: Some deltas may contain timestamp also as a value (ex.
        // position.timestamp), we could sometimes use or maybe even prefer them
        wxString fullKeyWithPath;
        wxString source = wxEmptyString;
        if (message["updates"][i].isMember("$source")) {
            source = fromJsonVal(message["updates"][i]["$source"].asString());
        } else if (message["updates"][i].isMember("source")) {
            if (message["updates"][i]["source"]["type"].asString()
                == "NMEA0183") {
                source
                    .Append(message["updates"][i]["source"]["label"].asString())
                    .Append("-")
                    .Append(
                        message["updates"][i]["source"]["talker"].asString())
                    .Append("-")
                    .Append(
                        message["updates"][i]["source"]["sentence"].asString());
            } else if (message["updates"][i]["source"]["type"].asString()
                == "NMEA2000") {
                source
                    .Append(message["updates"][i]["source"]["label"].asString())
                    .Append("-")
                    .Append(message["updates"][i]["source"]["pgn"].asString());
            } else {
                source.Append(
                    message["updates"][i]["source"]["label"].asString());
            }
        }
        wxString utoken;
        if (message["updates"][i].isMember("values")) {
            for (int j = 0; j < (int)message["updates"][i]["values"].size();
                j++) {
                Json::Value* val_ptr = ptr;
                fullKeyWithPath = fullKey;
                LOG_RECEIVE_DEBUG("processing value #%i (%s) under %s", j,
                    fromJsonVal(
                        message["updates"][i]["values"][j]["path"].asString())
                        .c_str(),
                    fullKeyWithPath.c_str());
                wxStringTokenizer path_tokenizer(
                    fromJsonVal(
                        message["updates"][i]["values"][j]["path"].asString()),
                    ".");
                while (path_tokenizer.HasMoreTokens()) {
                    utoken = path_tokenizer.GetNextToken();
                    fullKeyWithPath.Append(".").Append(utoken);
                    const std::string ukey = utoken.ToStdString();
                    if (!val_ptr->isMember(ukey)
                        && path_tokenizer.HasMoreTokens()) {
                        // If we are not done parsing the path yet, we add
                        // another branch if needed
                        LOG_RECEIVE_DEBUG(fullKeyWithPath
                            + " not yet in the tree, adding " + utoken);
                        (*val_ptr)[ukey] = Json::Value();
                        val_ptr = &(*val_ptr)[ukey];
                    } else {
                        LOG_RECEIVE_DEBUG(
                            fullKeyWithPath + " already exists in the tree");
                        val_ptr = &(*val_ptr)[ukey];
                    }
                }
                if (!message["updates"][i]["values"][j]["value"].isNull()) {
                    // We ignore NULL values received from SignalK
                    // TODO: Are some NULLs in SignalK data actually good for
                    // something? (If they are, we want to ignore them later
                    // selectively when the instrument processes it's data)
                    if (!source.IsEmpty()) {
                        wxString src_key = SRC_MAGIC_STRING + source;
                        src_key.Replace(".", "-", true);
                        const std::string sk = src_key.ToStdString();
                        (*val_ptr)[sk] = Json::Value();
                        val_ptr = &(*val_ptr)[sk];
                    }
                    ProcessComplexValue(val_ptr,
                        message["updates"][i]["values"][j]["value"], ts,
                        source);

                    LOG_RECEIVE_DEBUG(
                        "Notifying update to path " + fullKeyWithPath);
                    for (auto instr :
                        m_path_subscriptions[UNORDERED_KEY(fullKeyWithPath)]) {
                        instr->NotifyNewData(fullKeyWithPath);
                    }
                }
            }
        } else if (message["updates"][i].isMember("meta")) {
            for (int j = 0; j < (int)message["updates"][i]["meta"].size();
                j++) {
                Json::Value* val_ptr = ptr;
                fullKeyWithPath = fullKey;
                LOG_RECEIVE_DEBUG("processing value #%i (%s) under %s", j,
                    fromJsonVal(
                        message["updates"][i]["meta"][j]["path"].asString())
                        .c_str(),
                    fullKeyWithPath.c_str());
                wxStringTokenizer path_tokenizer(
                    fromJsonVal(
                        message["updates"][i]["meta"][j]["path"].asString()),
                    ".");
                while (path_tokenizer.HasMoreTokens()) {
                    utoken = path_tokenizer.GetNextToken();
                    fullKeyWithPath.Append(".").Append(utoken);
                    const std::string ukey = utoken.ToStdString();
                    if (!val_ptr->isMember(ukey)
                        && path_tokenizer.HasMoreTokens()) {
                        // If we are not done parsing the path yet, we add
                        // another branch if needed
                        LOG_RECEIVE_DEBUG(fullKeyWithPath
                            + " not yet in the tree, adding " + utoken);
                        (*val_ptr)[ukey] = Json::Value();
                        val_ptr = &(*val_ptr)[ukey];
                    } else {
                        LOG_RECEIVE_DEBUG(
                            fullKeyWithPath + " already exists in the tree");
                        val_ptr = &(*val_ptr)[ukey];
                    }
                }
                if (!message["updates"][i]["meta"][j].isNull()) {
                    (*val_ptr)["meta"]
                        = message["updates"][i]["meta"][j]["value"];
                }
            }
        }
    }
}

wxString DashboardSK::GetSignalKTreeText() { return DumpJSON(m_sk_data); }

Json::Value* DashboardSK::GetSignalKTree() { return &m_sk_data; }

const wxString DashboardSK::SelfTranslate(const wxString& path)
{
    if (Self().IsEmpty()) {
        return path;
    }
    wxString new_path = path;
    new_path.Replace("." + Self(), ".self");
    return new_path;
}

const wxString DashboardSK::SelfPopulate(const wxString& path)
{
    if (Self().IsEmpty()) {
        return path;
    }
    wxString new_path = path;
    new_path.Replace(".self", "." + Self());
    return new_path;
}

void DashboardSK::AddPageToCanvas(const int& canvas, const size_t& page)
{
    if (canvas < GetCanvasCount()) {
        m_displayed_pages[canvas]->AddPage(page);
    }
}

bool DashboardSK::ProcessMouseEvent(wxMouseEvent& event)
{
    // Pagers
    for (auto page : m_displayed_pages) {
        if (GetCanvasIndexUnderMouse() == page.first
            && page.second->ProcessMouseEvent(event)) {
            return true;
        }
    }
    // Dashboards / interactive instruments
    for (int i = 0; i < static_cast<int>(m_dashboards.size()); ++i) {
        Dashboard* d = m_dashboards[i];
        if (d->GetCanvasNr() == GetCanvasIndexUnderMouse()
            && m_displayed_pages[d->GetCanvasNr()]->GetCurrentPage()
                == static_cast<int>(d->GetPageNr())
            && d->ProcessMouseEvent(event, i)) {
            return true;
        }
    }
    return false;
}

wxBitmap DashboardSK::ApplyBitmapBrightness(wxBitmap& bitmap)
{
    double dimLevel;
    switch (m_color_scheme) {
    case PI_GLOBAL_COLOR_SCHEME_DUSK: {
        dimLevel = 0.8;
        break;
    }
    case PI_GLOBAL_COLOR_SCHEME_NIGHT: {
        dimLevel = 0.5;
        break;
    }
    default: {
        return bitmap;
    }
    }

    return SetBitmapBrightnessAbs(bitmap, dimLevel);
}

wxBitmap DashboardSK::SetBitmapBrightnessAbs(wxBitmap& bitmap, double level)
{
    wxImage image = bitmap.ConvertToImage();

    int gimg_width = image.GetWidth();
    int gimg_height = image.GetHeight();

    for (int iy = 0; iy < gimg_height; iy++) {
        for (int ix = 0; ix < gimg_width; ix++) {
            if (!image.IsTransparent(ix, iy, 30)) {
                wxImage::RGBValue rgb(image.GetRed(ix, iy),
                    image.GetGreen(ix, iy), image.GetBlue(ix, iy));
                wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
                hsv.value = hsv.value * level;
                wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
                image.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
            }
        }
    }
    return wxBitmap(image);
}

void DashboardSK::ResetPagers()
{
    for (auto& page : m_displayed_pages) {
        page.second->Reset();
        for (auto& dashboard : m_dashboards) {
            if (page.first == dashboard->GetCanvasNr()) {
                page.second->AddPage(dashboard->GetPageNr());
            }
        }
    }
}

void DashboardSK::SetParentPlugin(dashboardsk_pi* parent)
{
    m_parent_plugin = parent;
}

void DashboardSK::ShowPreferencesDialog()
{
    m_parent_plugin->ShowPreferencesDialog(m_parent_window);
}

void DashboardSK::ShowPreferencesDialog(int dashboard_idx, int instrument_idx)
{
    m_parent_plugin->ShowPreferencesDialog(
        m_parent_window, dashboard_idx, instrument_idx);
}

void DashboardSK::ToggleVisibility()
{
    m_parent_plugin->OnToolbarToolCallback(0);
}

bool DashboardSK::IsVisible() { return m_parent_plugin->IsVisible(); }

double DashboardSK::GetContentScaleFactor() const
{
    return m_parent_plugin->GetContentScaleFactor();
}

PLUGIN_END_NAMESPACE
