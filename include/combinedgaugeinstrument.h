/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  DashboardSK Plugin
 * Author:   Pavel Kalian
 *
 ******************************************************************************
 * This file is part of the DashboardSK plugin
 * (https://github.com/nohal/dashboardsk_pi).
 *   Copyright (C) 2026 by Pavel Kalian
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

#ifndef _COMBINEDGAUGE_H
#define _COMBINEDGAUGE_H

#include "simplegaugeinstrument.h"

// Setting names
#define DSK_CGI_CENTER_KEY "center_sk_key"
#define DSK_CGI_CENTER_TRANSFORM "center_transformation"

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, Json::Value conversion function, getter function.
// The center value is always what is shown in the center of the gauge; the
// inherited "Format" and "Value suffix" controls are applied to it (the primary
// value only drives the needle and is never displayed as text).
#define DSK_CGI_SETTINGS                                                       \
    X(0, DSK_CGI_CENTER_KEY, wxString(wxEmptyString),                          \
        _("Center value SK Key"), SignalKKeyCtrl, wxEmptyString, asString,     \
        GetStringSetting)                                                      \
    X(1, DSK_CGI_CENTER_TRANSFORM, 0, _("Center value transformation"),        \
        ChoiceCtrl, ConcatChoiceStrings(m_supported_transforms), asInt,        \
        GetIntSetting)

PLUGIN_BEGIN_NAMESPACE

/// Gauge instrument showing an angle/direction with the needle while displaying
/// a numerical value from a second SignalK path in the center. Suitable for
/// wind angle/speed combinations (e.g. AWA needle, AWS in the center).
class CombinedGaugeInstrument : public SimpleGaugeInstrument {
public:
    using SimpleGaugeInstrument::SetSetting;

    /// Construct an instrument attached to a dashboard.
    ///
    /// \param parent Pointer to the dashboard containing the instrument
    explicit CombinedGaugeInstrument(Dashboard* parent)
        : SimpleGaugeInstrument(parent)
    {
        InitExtra();
    }

    /// Class name, must be unique and not localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "CombinedGaugeInstrument"; }
    wxString GetClass() const override { return Class(); }

    /// Human readable name presented in the configuration dialog.
    ///
    /// \return Name of the instrument type
    static wxString DisplayType() { return _("Combined gauge"); }
    wxString GetDisplayType() const override { return DisplayType(); }

    void ReadConfig(Json::Value& config) override;
    Json::Value GenerateJSONConfig() override;
    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;
    void ProcessData() override;

protected:
    wxString FormatCenterValue() override;

private:
    /// Initialize the center value settings and append their config controls.
    /// Called from the constructor in addition to the inherited base
    /// initialization (the base constructor already ran the base Init()).
    void InitExtra();
    /// (Re)subscribe to both the primary and the center value SignalK paths.
    void SubscribeAll();

    /// SignalK path whose value is displayed in the center of the gauge.
    wxString m_center_sk_key;
    /// Last value resolved for the center value path.
    double m_center_value;
    /// Active data transformation for the center value.
    Instrument::transformation m_center_transformation;
};

PLUGIN_END_NAMESPACE

#endif //_COMBINEDGAUGE_H
