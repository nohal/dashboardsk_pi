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

// Table of supported formats and the respective formatting strings
#define DSK_SNI_FORMATS                                                        \
    X(0, "9.9", "%.1f")                                                        \
    X(1, "9.99", "%.2f")                                                       \
    X(2, "9.999", "%.3f")                                                      \
    X(3, "9", "%.0f")                                                          \
    X(4, "009", "%03.0f")                                                      \
    X(5, "009.9", "%05.1f")                                                    \
    X(6, "09.9", "%04.1f")                                                     \
    X(7, "09.99", "%05.2f")                                                    \
    X(8, "009.99", "%06.2f")

// Table of transformations to be shown in the GUI
#define DSK_SNI_TRANSFORMATIONS                                                \
    X(SimpleNumberInstrument::transformation::none, _("None"))                 \
    X(SimpleNumberInstrument::transformation::rad2deg, _("RAD->DEG"))          \
    X(SimpleNumberInstrument::transformation::ms2kn, _("m/s -> kn"))           \
    X(SimpleNumberInstrument::transformation::ms2kmh, _("m/s -> km/h"))        \
    X(SimpleNumberInstrument::transformation::ms2mih, _("m/s -> mph"))

// Palette from https://colorhunt.co/palette/125b50f8b400faf5e4ff6363
#define DSK_SNI_COLOR_TITLE_BG wxColor(18, 91, 80)
#define DSK_SNI_COLOR_TITLE_FG wxColor(248, 180, 0)
#define DSK_SNI_COLOR_BODY_BG wxColor(250, 245, 228)
#define DSK_SNI_COLOR_BODY_FG wxColor(255, 99, 99)
#define DSK_SNI_COLOR_BORDER *wxBLACK

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_SNI_SETTINGS                                                       \
    X(0, "sk_key", wxString(wxEmptyString), _("SK Key"), SignalKKeyCtrl,       \
        wxEmptyString, AsString, GetStringSetting)                             \
    X(1, "format", 0, _("Format"), ChoiceCtrl,                                 \
        ConcatChoiceStrings(m_supported_formats), AsInt, GetIntSetting)        \
    X(2, "transformation", 0, _("Transformation"), ChoiceCtrl,                 \
        ConcatChoiceStrings(m_supported_transforms), AsInt, GetIntSetting)     \
    X(3, "title_font", m_title_font.GetPointSize(), _("Title size"), SpinCtrl, \
        "5;40", AsInt, GetIntSetting)                                          \
    X(4, "body_font", m_body_font.GetPointSize(), _("Body size"), SpinCtrl,    \
        "5;40", AsInt, GetIntSetting)                                          \
    X(5, "title_background", DSK_SNI_COLOR_TITLE_BG, _("Title background"),    \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(6, "title_color", DSK_SNI_COLOR_TITLE_FG, _("Title color"),              \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(7, "body_background", DSK_SNI_COLOR_BODY_BG, _("Body background"),       \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(8, "body_color", DSK_SNI_COLOR_BODY_FG, _("Body color"),                 \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(9, "border_color", DSK_SNI_COLOR_BORDER, _("Border color"),              \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)

PLUGIN_BEGIN_NAMESPACE

/// Simple instrument displaying a single value from one SignalK path
class SimpleNumberInstrument : public Instrument {
    // We override some of the implementations of SetSetting to do "special
    // stuff" for some of the settings
    using Instrument::SetSetting;

public:
    /// Supported transformations
    /// New transformations to be added to DSK_SNI_TRANSFORMATIONS macro and
    /// implemented in #SimpleNumberInstrument::Transform
    enum class transformation {
        none = 0,
        /// Radians to degrees
        rad2deg,
        /// meters/second to knots
        ms2kn,
        /// meters/second to km/h
        ms2kmh,
        /// meters/second to miles per hour
        ms2mph
    };

private:
    /// Font used for the title of the instrument
    wxFont m_title_font;
    /// Font used for the body of the instrument
    wxFont m_body_font;
    /// SignalK fully quakified path whose value is to be displayed by the
    /// instrument
    wxString m_sk_key;
    /// Array of names of supported formas
    wxArrayString m_supported_formats;
    /// Array of formatting strings implementing the supported formats
    wxArrayString m_format_strings;
    /// Active format
    int m_format_index;
    /// Array of names of supported data transformations
    wxArrayString m_supported_transforms;
    /// Active data transformation
    SimpleNumberInstrument::transformation m_transformation;
    /// Bitmap representation of the instrument
    wxBitmap m_bmp;
    /// Instrument in timed out state flag. True if the instrument is not
    /// receiving data. for more than #m_allowed_age_sec seconds.
    bool m_timed_out;

    /// Constructor
    SimpleNumberInstrument() { Init(); };

    /// Initialize the default parameters of the instrument
    void Init();

    /// Return semicolon separated supported items for the GUI
    ///
    /// \ param arr wxArrayString of values to concatenate
    /// \return All format strings separated by semicolons
    const wxString ConcatChoiceStrings(wxArrayString arr)
    {
        wxString s = wxEmptyString;
        for (auto str : arr) {
            if (!s.IsEmpty()) {
                s.Append(";");
            }
            s.Append(str);
        }
        return s;
    };

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
    /// #m_transformation. Every transformation defined in
    /// #SimpleNumberInstrument::transformation and DSK_SNI_TRANSFORMATIONS
    /// macro has to be covered here.
    ///
    /// \param val Double value to be transformed
    /// \return Transformed value
    const double Transform(const double& val);
};

PLUGIN_END_NAMESPACE

#endif //_SIMPLENUMBER_H
