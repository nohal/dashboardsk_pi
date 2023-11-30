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

#ifndef _SIMPLENUMBER_H
#define _SIMPLENUMBER_H

#include "instrument.h"
#include "pi_common.h"
#include "wx/jsonval.h"
#include <wx/clrpicker.h>

#define BORDER_SIZE 4 * scale
#define BORDER_LINE_WIDTH 2 * scale

// Palette from
// Normal: https://colorhunt.co/palette/e6e6e6c5a880532e1c0f0f0f
// Backgrounds: https://colorhunt.co/palette/ea5959f98b60ffc057ffe084
#define DSK_SNI_COLOR_TITLE_BG wxColor(83, 46, 28)
#define DSK_SNI_COLOR_TITLE_FG wxColor(197, 168, 128)
#define DSK_SNI_COLOR_BODY_BG wxColor(230, 230, 230)
#define DSK_SNI_COLOR_BODY_FG wxColor(15, 15, 15)
#define DSK_SNI_COLOR_ALERT_BG wxColor(255, 224, 132)
#define DSK_SNI_COLOR_ALERT_FG wxColor(15, 15, 15)
#define DSK_SNI_COLOR_WARN_BG wxColor(255, 192, 87)
#define DSK_SNI_COLOR_WARN_FG wxColor(15, 15, 15)
#define DSK_SNI_COLOR_ALRM_BG wxColor(249, 139, 96)
#define DSK_SNI_COLOR_ALRM_FG wxColor(15, 15, 15)
#define DSK_SNI_COLOR_EMERG_BG wxColor(234, 89, 89)
#define DSK_SNI_COLOR_EMERG_FG wxColor(255, 255, 255)
#define DSK_SNI_COLOR_BORDER *wxBLACK

#define DSK_SNI_SMOOTHING_MAX 9

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_SNI_SETTINGS                                                       \
    X(0, DSK_SETTING_SK_KEY, wxString(wxEmptyString), _("SK Key"),             \
        SignalKKeyCtrl, wxEmptyString, AsString, GetStringSetting)             \
    X(1, DSK_SETTING_FORMAT, 0, _("Format"), ChoiceCtrl,                       \
        ConcatChoiceStrings(m_supported_formats), AsInt, GetIntSetting)        \
    X(2, DSK_SETTING_TRANSFORMATION, 0, _("Transformation"), ChoiceCtrl,       \
        ConcatChoiceStrings(m_supported_transforms), AsInt, GetIntSetting)     \
    X(3, DSK_SETTING_ZONES, wxString(wxEmptyString), _("Zones"),               \
        SignalKZonesCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(4, DSK_SETTING_SMOOTHING, m_smoothing, _("Data smoothing"), SpinCtrl,    \
        "0;" STRINGIFY(DSK_SNI_SMOOTHING_MAX), AsInt, GetIntSetting)           \
    X(5, DSK_SETTING_TITLE_FONT, m_title_font.GetPointSize(), _("Title size"), \
        SpinCtrl, "5;40", AsInt, GetIntSetting)                                \
    X(6, DSK_SETTING_BODY_FONT, m_body_font.GetPointSize(), _("Body size"),    \
        SpinCtrl, "5;40", AsInt, GetIntSetting)                                \
    X(7, DSK_SETTING_TITLE_BG, DSK_SNI_COLOR_TITLE_BG, _("Title background"),  \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(8, DSK_SETTING_TITLE_FG, DSK_SNI_COLOR_TITLE_FG, _("Title color"),       \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(9, DSK_SETTING_BODY_BG, DSK_SNI_COLOR_BODY_BG, _("Body background"),     \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(10, DSK_SETTING_BODY_FG, DSK_SNI_COLOR_BODY_FG, _("Body color"),         \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(11, DSK_SETTING_ALERT_BG, DSK_SNI_COLOR_ALERT_BG, _("Alert background"), \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(12, DSK_SETTING_ALERT_FG, DSK_SNI_COLOR_ALERT_FG, _("Alert color"),      \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(13, DSK_SETTING_WARN_BG, DSK_SNI_COLOR_WARN_BG, _("Warning background"), \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(14, DSK_SETTING_WARN_FG, DSK_SNI_COLOR_WARN_FG, _("Warning color"),      \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(15, DSK_SETTING_ALRM_BG, DSK_SNI_COLOR_ALRM_BG, _("Alarm background"),   \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(16, DSK_SETTING_ALRM_FG, DSK_SNI_COLOR_ALRM_FG, _("Alarm color"),        \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(17, DSK_SETTING_EMERG_BG, DSK_SNI_COLOR_EMERG_BG,                        \
        _("Emergency background"), ColourPickerCtrl, wxEmptyString, AsString,  \
        GetStringSetting)                                                      \
    X(18, DSK_SETTING_EMERG_FG, DSK_SNI_COLOR_EMERG_FG, _("Emergency color"),  \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(19, DSK_SETTING_BORDER_COLOR, DSK_SNI_COLOR_BORDER, _("Border color"),   \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)

PLUGIN_BEGIN_NAMESPACE

/// Simple instrument displaying a single value from one SignalK path
class SimpleNumberInstrument : public Instrument {

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
    /// Array of names of supported formats
    wxArrayString m_supported_formats;
    /// Array of formatting strings implementing the supported formats
    wxArrayString m_format_strings;
    /// Active format
    int m_format_index;
    /// Array of names of supported data transformations
    wxArrayString m_supported_transforms;
    /// Active data transformation
    Instrument::transformation m_transformation;
    /// Bitmap representation of the instrument
    wxBitmap m_bmp;
    /// Instrument in timed out state flag. True if the instrument is not
    /// receiving data. for more than #m_allowed_age_sec seconds.
    bool m_timed_out;
    /// Data smoothing ratio (0-DSK_SNI_SMOOTHING_MAX). The value effectively
    /// tells over how many historic values with progressively declining
    /// importance we want to smooth our data. 0 means the newest value is used
    /// as is, while 9 means the newest value has only 10% influence on the
    /// result. a(i+1) = (m_smoothing*a(i) +
    /// (DSK_SNI_SMOOTHING_MAX-m_smoothing+1)*data(i+1)) /
    /// (DSK_SNI_SMOOTHING_MAX + 1)
    size_t m_smoothing;
    /// Previous value displayed by the instrument
    double m_old_value;

    /// Constructor
    SimpleNumberInstrument() { Init(); };

    /// Initialize the default parameters of the instrument
    void Init();

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
    explicit SimpleNumberInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    };

    wxString GetClass() const override
    {
        return SimpleNumberInstrument::Class();
    };

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "SimpleNumberInstrument"; };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    wxString GetDisplayType() const override
    {
        return SimpleNumberInstrument::DisplayType();
    };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    static wxString DisplayType() { return _("Simple number"); };

    wxBitmap Render(double scale) override;

    void ReadConfig(wxJSONValue& config) override;

    wxJSONValue GenerateJSONConfig() override;

    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;

    /// Transform the value using function implemented for the value of
    /// #m_transformation.
    ///
    /// \param val Double value to be transformed
    /// \return Transformed value
    double Transform(const double& val);

    wxString GetPrimarySKKey() const override { return m_sk_key; };

    void ProcessData() override;
};

PLUGIN_END_NAMESPACE

#endif //_SIMPLENUMBER_H
