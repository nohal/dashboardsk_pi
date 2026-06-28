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

#ifndef _CONFIGVALIDATOR_H_
#define _CONFIGVALIDATOR_H_

#include <string>

#include <wx/string.h>

#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

/// Validate JSON text against a definition in the configuration schema.
///
/// The schema is loaded from \c schema_path (the
/// \c dashboardsk.config.schema.json file shipped in the plugin data
/// directory) and the document is validated against the sub-schema referenced
/// by \c definition_ref (e.g. \c "#/definitions/DashboardSK").
///
/// \param json_text JSON document to validate
/// \param schema_path Path to the JSON schema file
/// \param definition_ref JSON pointer to the schema definition to validate
///   against, e.g. \c "#/definitions/Instrument"
/// \param errors_out On failure, receives a human readable list of the schema
///   violations (one per line); left untouched on success
/// \return true if the document is valid (or the schema could not be loaded, in
///   which case validation is skipped); false if validation found problems
bool ValidateConfigJSON(const wxString& json_text, const wxString& schema_path,
    const std::string& definition_ref, wxString& errors_out);

PLUGIN_END_NAMESPACE

#endif // _CONFIGVALIDATOR_H_
