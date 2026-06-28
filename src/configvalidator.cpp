/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  DashboardSK Plugin
 * Author:   Pavel Kalian
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

#include "configvalidator.h"

#include <fstream>
#include <map>
#include <memory>

#include <nlohmann/json-schema.hpp>

using nlohmann::json;
using nlohmann::json_schema::json_validator;

PLUGIN_BEGIN_NAMESPACE

namespace {

/// Collects all schema validation errors into a single wxString.
class CollectingErrorHandler : public nlohmann::json_schema::error_handler {
public:
    /// Accumulated errors, one per line
    wxString errors;

    /// Record a single validation error
    ///
    /// \param ptr Location of the offending value in the document
    /// \param instance Offending value (unused)
    /// \param message Human readable description of the violation
    void error(const json::json_pointer& ptr, const json& /*instance*/,
        const std::string& message) override
    {
        errors += wxString::FromUTF8(ptr.to_string().c_str()) + ": "
            + wxString::FromUTF8(message.c_str()) + "\n";
    }
};

/// Load and parse the schema file once, caching by path.
///
/// \param schema_path Path to the JSON schema file
/// \return Pointer to the cached parsed schema, or nullptr if it could not be
///   loaded or parsed
const json* GetSchema(const wxString& schema_path)
{
    // ponytail: one schema per session is plenty; cache by path string
    static std::map<wxString, std::unique_ptr<json>> cache;
    auto it = cache.find(schema_path);
    if (it != cache.end()) {
        return it->second.get();
    }
    std::ifstream f(DSK_TO_STDSTRING_UTF8(schema_path));
    if (!f.is_open()) {
        cache[schema_path] = nullptr;
        return nullptr;
    }
    auto schema = std::make_unique<json>();
    try {
        f >> *schema;
    } catch (const std::exception&) {
        cache[schema_path] = nullptr;
        return nullptr;
    }
    const json* result = schema.get();
    cache[schema_path] = std::move(schema);
    return result;
}

/// Build (and cache) a validator targeting a single schema definition.
///
/// The validator wraps the requested definition in a tiny root schema so that
/// \c $ref and the shared \c definitions resolve correctly.
///
/// \param schema_path Path to the JSON schema file
/// \param definition_ref JSON pointer to the definition, e.g.
///   \c "#/definitions/Instrument"
/// \return Pointer to the cached validator, or nullptr on failure
const json_validator* GetValidator(
    const wxString& schema_path, const std::string& definition_ref)
{
    static std::map<wxString, std::unique_ptr<json_validator>> cache;
    const wxString key
        = schema_path + "|" + wxString::FromUTF8(definition_ref.c_str());
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second.get();
    }
    const json* schema = GetSchema(schema_path);
    if (!schema) {
        cache[key] = nullptr;
        return nullptr;
    }
    json root;
    root["$ref"] = definition_ref;
    if (schema->contains("definitions")) {
        root["definitions"] = (*schema)["definitions"];
    }
    auto validator = std::make_unique<json_validator>();
    try {
        validator->set_root_schema(root);
    } catch (const std::exception&) {
        cache[key] = nullptr;
        return nullptr;
    }
    const json_validator* result = validator.get();
    cache[key] = std::move(validator);
    return result;
}

} // namespace

bool ValidateConfigJSON(const wxString& json_text, const wxString& schema_path,
    const std::string& definition_ref, wxString& errors_out)
{
    const json_validator* validator = GetValidator(schema_path, definition_ref);
    if (!validator) {
        // No usable schema - skip validation rather than blocking the user
        return true;
    }
    json doc;
    try {
        doc = json::parse(DSK_TO_STDSTRING_UTF8(json_text));
    } catch (const std::exception& e) {
        errors_out = wxString::FromUTF8(e.what());
        return false;
    }
    CollectingErrorHandler handler;
    validator->validate(doc, handler);
    if (!handler.errors.empty()) {
        errors_out = handler.errors;
        return false;
    }
    return true;
}

PLUGIN_END_NAMESPACE
