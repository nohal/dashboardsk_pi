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

#ifndef _SIMPLETEXTINSTRUMENT_H
#define _SIMPLETEXTINSTRUMENT_H

#include "instrument.h"
#include "pi_common.h"
#include "wx/jsonval.h"
#include <wx/clrpicker.h>

#define BORDER_SIZE 4 * scale
#define BORDER_LINE_WIDTH 2 * scale

// Default palette
#define DSK_STI_COLOR_TITLE_BG wxColor(130, 149, 75)
#define DSK_STI_COLOR_TITLE_FG wxColor(255, 239, 130)
#define DSK_STI_COLOR_BODY_BG wxColor(230, 230, 230)
#define DSK_STI_COLOR_BODY_FG wxColor(15, 15, 15)
#define DSK_STI_COLOR_BORDER *wxBLACK

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_STI_SETTINGS                                                       \
    X(0, DSK_SETTING_SK_KEY, wxString(wxEmptyString), _("SK Key"),             \
        SignalKKeyCtrl, wxEmptyString, AsString, GetStringSetting)             \
    X(1, DSK_SETTING_TITLE_FONT, m_title_font.GetPointSize(), _("Title size"), \
        SpinCtrl, "5;40", AsInt, GetIntSetting)                                \
    X(2, DSK_SETTING_BODY_FONT, m_body_font.GetPointSize(), _("Body size"),    \
        SpinCtrl, "5;40", AsInt, GetIntSetting)                                \
    X(3, DSK_SETTING_TITLE_BG, DSK_STI_COLOR_TITLE_BG, _("Title background"),  \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(4, DSK_SETTING_TITLE_FG, DSK_STI_COLOR_TITLE_FG, _("Title color"),       \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(5, DSK_SETTING_BODY_BG, DSK_STI_COLOR_BODY_BG, _("Body background"),     \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(6, DSK_SETTING_BODY_FG, DSK_STI_COLOR_BODY_FG, _("Body color"),          \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(7, DSK_SETTING_BORDER_COLOR, DSK_STI_COLOR_BORDER, _("Border color"),    \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)

PLUGIN_BEGIN_NAMESPACE

/// Simple instrument displaying a single value from one SignalK path
class SimpleTextInstrument : public Instrument {

public:
    // We override some of the implementations of SetSetting to do "special
    // stuff" for some of the settings
    using Instrument::SetSetting;

protected:
    /// Enum to identify the part of the instrument graphical representation
    enum class color_item {
        /// Title background
        title_bg = 0,
        /// Title text
        title_fg,
        /// Body backgorund
        body_bg,
        /// Body text
        body_fg,
        /// Border
        border
    };

    /// Font used for the title of the instrument
    wxFont m_title_font;
    /// Font used for the body of the instrument
    wxFont m_body_font;
    /// SignalK fully quakified path whose value is to be displayed by the
    /// instrument
    wxString m_sk_key;
    /// Bitmap representation of the instrument
    wxBitmap m_bmp;
    /// Instrument in timed out state flag. True if the instrument is not
    /// receiving data. for more than #m_allowed_age_sec seconds.
    bool m_timed_out;

    /// Constructor
    SimpleTextInstrument() { Init(); };

    /// Initialize the default parameters of the instrument
    virtual void Init();

    /// Get color for a part of the instrument corresponding to a value to be
    /// displayed
    ///
    /// \param val Duble value to be displayed
    /// \param item Part of the instrument
    /// \return Color to be used
    const wxColor GetColor(const double& val, const color_item item);

public:
    /// Constructor
    ///
    /// \param parent Pointer to the dashboard containing the instrument
    explicit SimpleTextInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    };

    wxString GetClass() const override
    {
        return SimpleTextInstrument::Class();
    };

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "SimpleTextInstrument"; };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    wxString GetDisplayType() const override
    {
        return SimpleTextInstrument::DisplayType();
    };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    static wxString DisplayType() { return _("Simple text"); };

    wxBitmap Render(double scale) override;

    void ReadConfig(wxJSONValue& config) override;

    wxJSONValue GenerateJSONConfig() override;

    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;

    wxString GetPrimarySKKey() const override { return m_sk_key; };
};

PLUGIN_END_NAMESPACE

#endif //_SIMPLETEXTINSTRUMENT_H
