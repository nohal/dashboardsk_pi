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

#ifndef _SPACERINSTRUMENT_H
#define _SPACERINSTRUMENT_H

#include "instrument.h"
#include "pi_common.h"
#include "wx/jsonval.h"

#define DSK_SPACER_MIN_SIZE 1
#define DSK_SPACER_MAX_SIZE 500

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_SPACER_SETTINGS                                                    \
    X(0, DSK_SETTING_INSTR_WIDTH, m_instrument_width, _("Width"), SpinCtrl,    \
        STRINGIFY(DSK_SPACER_MIN_SIZE) ";" STRINGIFY(DSK_SPACER_MAX_SIZE),     \
        AsInt, GetIntSetting)                                                  \
    X(1, DSK_SETTING_INSTR_HEIGHT, m_instrument_height, _("Height"), SpinCtrl, \
        STRINGIFY(DSK_SPACER_MIN_SIZE) ";" STRINGIFY(DSK_SPACER_MAX_SIZE),     \
        AsInt, GetIntSetting)

PLUGIN_BEGIN_NAMESPACE

/// "Dummy" instrument displaying nothing, used to insert a blank space of
/// user defined size between instruments to allow their visual grouping in
/// the dashboard. Does not consume any SignalK data.
class SpacerInstrument : public Instrument {

public:
    // We override some of the implementations of SetSetting to do "special
    // stuff" for some of the settings
    using Instrument::SetSetting;

protected:
    /// Width of the spacer in DIP
    wxCoord m_instrument_width;
    /// Height of the spacer in DIP
    wxCoord m_instrument_height;
    /// Bitmap representation of the instrument
    wxBitmap m_bmp;

    /// Constructor
    SpacerInstrument() { Init(); };

    /// Initialize the default parameters of the instrument
    void Init();

public:
    /// Constructor
    ///
    /// \param parent Pointer to the dashboard containing the instrument
    explicit SpacerInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    };

    wxString GetClass() const override { return SpacerInstrument::Class(); };

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "SpacerInstrument"; };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    wxString GetDisplayType() const override
    {
        return SpacerInstrument::DisplayType();
    };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    static wxString DisplayType() { return _("Spacer"); };

    wxBitmap Render(double scale) override;

    void ReadConfig(wxJSONValue& config) override;

    wxJSONValue GenerateJSONConfig() override;

    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;
};

PLUGIN_END_NAMESPACE

#endif //_SPACERINSTRUMENT_H
