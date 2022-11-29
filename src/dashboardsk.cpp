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
#include "wx/jsonwriter.h"
#include <wx/tokenzr.h>

PLUGIN_BEGIN_NAMESPACE

DashboardSK::DashboardSK()
    : m_self(wxEmptyString)
    , m_self_ptr(nullptr)
    , m_color_scheme(0)
{
    m_sk_data["vessels"].AddComment("Root of the vessel tree");
}

void DashboardSK::Draw(dskDC* dc, PlugIn_ViewPort* vp, int canvasIndex)
{
    Dashboard::ClearOffsets();
    for (auto& dashboard : m_dashboards) {
        dashboard.Draw(dc, vp, canvasIndex);
    }
}

void DashboardSK::ReadConfig(wxJSONValue& config)
{
    LOG_VERBOSE("DashboardSK_pi: Reading DashboardSK config");
    m_dashboards.clear();
    if (config["signalk"].HasMember("self")) {
        SetSelf(config["signalk"]["self"].AsString());
    }
    if (!config.HasMember("dashboards")) {
        LOG_VERBOSE("DashboardSK_pi: No dashboards node in JSON");
    }
    if (config["dashboards"].IsArray()) {
        for (int i = 0; i < config["dashboards"].Size(); i++) {
            auto* d = new Dashboard(this);
            d->ReadConfig(config["dashboards"][i]);
            d->SetColorScheme(m_color_scheme);
            m_dashboards.emplace_back(*d);
        }
    } else {
        LOG_VERBOSE("DashboardSK_pi: No dashboards array");
    }
}

wxJSONValue DashboardSK::GenerateJSONConfig()
{
    wxJSONValue v;
    v["signalk"]["self"] = m_self;
    for (auto& dashboard : m_dashboards) {
        v["dashboards"].Append(dashboard.GenerateJSONConfig());
    }
    return v;
}

void DashboardSK::SetColorScheme(int cs)
{
    m_color_scheme = cs;
    for (auto& dashboard : m_dashboards) {
        dashboard.SetColorScheme(cs);
    }
}

const int DashboardSK::GetColorScheme() { return m_color_scheme; }

const wxJSONValue* DashboardSK::GetSKData(const wxString& path)
{
    wxStringTokenizer tokenizer(path, ".");
    wxJSONValue* ptr = &m_sk_data;
    wxString token;
    while (tokenizer.HasMoreTokens()) {
        token = tokenizer.GetNextToken();
        if (ptr->HasMember(token)) {
            ptr = &(*ptr)[token];
        } else {
            return nullptr; // Not found
        }
    }
    return ptr;
}

void DashboardSK::SendSKDelta(wxJSONValue& message)
{
    LOG_RECEIVE("Received SK message: " + message.AsString());
    if (m_self.IsEmpty() && message.HasMember("self")) {
        // If we still don't have Self ID set, we accept it from the core
        // TODO: We perhaps might allow this until the user ever modifies it
        // manually in the prefs
        LOG_RECEIVE_DEBUG(
            "Message contains self indentifier " + message["self"].AsString());
        SetSelf(message["self"].AsString());
    }
    if (!message.HasMember("context")) {
        LOG_RECEIVE(
            "Message does not contain context " + message.GetMemberNames()[0]);
        if (!message.HasMember("updates") || !message["updates"].IsArray()) {
            LOG_RECEIVE("Message does not look OK");
            return; // Invalid SK delta
        }
        return; // TODO: If no context in message, can we asume self?
    }
    LOG_RECEIVE_DEBUG("Message seems OK");
    wxString fullKey = message["context"].AsString();

    int skip_tokens = 0;
    wxJSONValue* ptr;
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
    wxStringTokenizer ctx_tokenizer(message["context"].AsString(), ".");
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

            if (!ptr->HasMember(token)) {
                LOG_RECEIVE_DEBUG(ptr->GetInfo() + " does NOT have member "
                    + token + ", adding it");
                (*ptr)[token] = wxJSONValue();
                ptr = &(*ptr)[token];
            } else {
                LOG_RECEIVE_DEBUG(ptr->GetInfo() + " does have member " + token
                    + ", reusing it");
                ptr = &(*ptr)[token];
            }
        } else {
            skip_tokens--;
        }
    }
    LOG_RECEIVE_DEBUG("Full key after parsing: " + fullKey);
    wxDateTime ts;
    for (int i = 0; i < message["updates"].Size(); i++) {
        LOG_RECEIVE_DEBUG("processing update #%i", i);
        if (message["updates"][i].HasMember("timestamp")) {
            if (!ts.ParseISOCombined(
                    message["updates"][i]["timestamp"].AsString())) {
                ts = wxDateTime::Now();
            }
        } else {
            ts = wxDateTime::Now();
        }
        // TODO: Some deltas may contain timestamp also as a value (ex.
        // position.timestamp), we could sometimes use or maybe even prefer them
        wxString fullKeyWithPath;
        wxString source = message["updates"][i]["$source"].AsString();
        wxString utoken;
        if (message["updates"][i].HasMember("values")) {
            for (int j = 0; j < message["updates"][i]["values"].Size(); j++) {
                wxJSONValue* val_ptr = ptr;
                fullKeyWithPath = fullKey;
                LOG_RECEIVE_DEBUG("processing value #%i (%s) under %s", j,
                    message["updates"][i]["values"][j]["path"].AsCString(),
                    fullKeyWithPath.c_str());
                wxStringTokenizer path_tokenizer(
                    message["updates"][i]["values"][j]["path"].AsString(), ".");
                while (path_tokenizer.HasMoreTokens()) {
                    utoken = path_tokenizer.GetNextToken();
                    fullKeyWithPath.Append(".").Append(utoken);
                    if (!val_ptr->HasMember(utoken)
                        && path_tokenizer.HasMoreTokens()) {
                        // If we are not done parsing the path yet, we add
                        // another branch if needed
                        LOG_RECEIVE_DEBUG(fullKeyWithPath
                            + " not yet in the tree, adding " + utoken);
                        (*val_ptr)[utoken] = wxJSONValue();
                        val_ptr = &(*val_ptr)[utoken];
                    } else {
                        LOG_RECEIVE_DEBUG(
                            fullKeyWithPath + " already exists in the tree");
                        val_ptr = &(*val_ptr)[utoken];
                    }
                }
                // TODO: What if we get same stuff from multiple sources (Like
                // two GPS or depth sensors on board)? For now we just overwrite
                // the value with newest, unless they use different
                // path/context, but could probably save all of them and let the
                // user select what to show
                if (!message["updates"][i]["values"][j]["value"].IsNull()) {
                    // We ignore NULL values received from SignalK
                    // TODO: Are some NULLs in SignalK data actually good for
                    // something? (If they are, we want to ignore them later
                    // selectively when the instrument processes it's data)
                    (*val_ptr)["value"]
                        = message["updates"][i]["values"][j]["value"];
                    (*val_ptr)["timestamp"] = ts.FormatISOCombined();
                    (*val_ptr)["source"] = source;

                    LOG_RECEIVE_DEBUG(
                        "Notifying update to path " + fullKeyWithPath);
                    for (auto instr :
                        m_path_subscriptions[UNORDERED_KEY(fullKeyWithPath)]) {
                        instr->NotifyNewData(fullKeyWithPath);
                    }
                }
            }
        } else if (message["updates"][i].HasMember("meta")) {
            for (int j = 0; j < message["updates"][i]["meta"].Size(); j++) {
                wxJSONValue* val_ptr = ptr;
                fullKeyWithPath = fullKey;
                LOG_RECEIVE_DEBUG("processing value #%i (%s) under %s", j,
                    message["updates"][i]["meta"][j]["path"].AsCString(),
                    fullKeyWithPath.c_str());
                wxStringTokenizer path_tokenizer(
                    message["updates"][i]["meta"][j]["path"].AsString(), ".");
                while (path_tokenizer.HasMoreTokens()) {
                    utoken = path_tokenizer.GetNextToken();
                    fullKeyWithPath.Append(".").Append(utoken);
                    if (!val_ptr->HasMember(utoken)
                        && path_tokenizer.HasMoreTokens()) {
                        // If we are not done parsing the path yet, we add
                        // another branch if needed
                        LOG_RECEIVE_DEBUG(fullKeyWithPath
                            + " not yet in the tree, adding " + utoken);
                        (*val_ptr)[utoken] = wxJSONValue();
                        val_ptr = &(*val_ptr)[utoken];
                    } else {
                        LOG_RECEIVE_DEBUG(
                            fullKeyWithPath + " already exists in the tree");
                        val_ptr = &(*val_ptr)[utoken];
                    }
                }
                if (!message["updates"][i]["meta"][j].IsNull()) {
                    (*val_ptr)["meta"]
                        = message["updates"][i]["meta"][j]["value"];
                }
            }
        }
    }
}

wxString DashboardSK::GetSignalKTreeText()
{
    wxJSONWriter w;
    wxString s;
    w.Write(m_sk_data, s);
    return s;
}

wxJSONValue* DashboardSK::GetSignalKTree() { return &m_sk_data; }

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

PLUGIN_END_NAMESPACE
