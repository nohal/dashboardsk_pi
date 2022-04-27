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

// Setting nams
#define DSK_SNI_SK_KEY "sk_key"
#define DSK_SNI_FORMAT "format"
#define DSK_SNI_TRANSFORMATION "transformation"
#define DSK_SNI_TITLE_FONT "title_font"
#define DSK_SNI_BODY_FONT "body_font"
#define DSK_SNI_TITLE_BG "title_background"
#define DSK_SNI_TITLE_FG "title_color"
#define DSK_SNI_BODY_BG "body_background"
#define DSK_SNI_BODY_FG "body_color"
#define DSK_SNI_ALERT_BG "alert_background"
#define DSK_SNI_ALERT_FG "alert_color"
#define DSK_SNI_WARN_BG "warn_background"
#define DSK_SNI_WARN_FG "warn_color"
#define DSK_SNI_ALRM_BG "alarm_background"
#define DSK_SNI_ALRM_FG "alarm_color"
#define DSK_SNI_EMERG_BG "emergency_background"
#define DSK_SNI_EMERG_FG "emergency_color"
#define DSK_SNI_BORDER_COLOR "border_color"

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_SNI_SETTINGS                                                       \
    X(0, DSK_SNI_SK_KEY, wxString(wxEmptyString), _("SK Key"), SignalKKeyCtrl, \
        wxEmptyString, AsString, GetStringSetting)                             \
    X(1, DSK_SNI_FORMAT, 0, _("Format"), ChoiceCtrl,                           \
        ConcatChoiceStrings(m_supported_formats), AsInt, GetIntSetting)        \
    X(2, DSK_SNI_TRANSFORMATION, 0, _("Transformation"), ChoiceCtrl,           \
        ConcatChoiceStrings(m_supported_transforms), AsInt, GetIntSetting)     \
    X(3, DSK_SNI_ZONES, wxString(wxEmptyString), _("Zones"), SignalKZonesCtrl, \
        wxEmptyString, AsString, GetStringSetting)                             \
    X(4, DSK_SNI_TITLE_FONT, m_title_font.GetPointSize(), _("Title size"),     \
        SpinCtrl, "5;40", AsInt, GetIntSetting)                                \
    X(5, DSK_SNI_BODY_FONT, m_body_font.GetPointSize(), _("Body size"),        \
        SpinCtrl, "5;40", AsInt, GetIntSetting)                                \
    X(6, DSK_SNI_TITLE_BG, DSK_SNI_COLOR_TITLE_BG, _("Title background"),      \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(7, DSK_SNI_TITLE_FG, DSK_SNI_COLOR_TITLE_FG, _("Title color"),           \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(8, DSK_SNI_BODY_BG, DSK_SNI_COLOR_BODY_BG, _("Body background"),         \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(9, DSK_SNI_BODY_FG, DSK_SNI_COLOR_BODY_FG, _("Body color"),              \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(10, DSK_SNI_ALERT_BG, DSK_SNI_COLOR_ALERT_BG, _("Alert background"),     \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(11, DSK_SNI_ALERT_FG, DSK_SNI_COLOR_ALERT_FG, _("Alert color"),          \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(12, DSK_SNI_WARN_BG, DSK_SNI_COLOR_WARN_BG, _("Warning background"),     \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(13, DSK_SNI_WARN_FG, DSK_SNI_COLOR_WARN_FG, _("Warning color"),          \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(14, DSK_SNI_ALRM_BG, DSK_SNI_COLOR_ALRM_BG, _("Alarm background"),       \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(15, DSK_SNI_ALRM_FG, DSK_SNI_COLOR_ALRM_FG, _("Alarm color"),            \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(16, DSK_SNI_EMERG_BG, DSK_SNI_COLOR_EMERG_BG, _("Emergency background"), \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(17, DSK_SNI_EMERG_FG, DSK_SNI_COLOR_EMERG_FG, _("Emergency color"),      \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(18, DSK_SNI_BORDER_COLOR, DSK_SNI_COLOR_BORDER, _("Border color"),       \
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

    /// Returns color corresponding to the provided value adjusted according to
    /// the zone ot which the value falls. In case of overlapping zones the one
    /// with highest severity takes precedence \param val The value to be
    /// displayed \param nominal_color Color corresponding to nominal value (Not
    /// in any zone) \param normal_color Color corresponding to normal value
    /// \param alert_color Color corresponding to an alert
    /// \param warn_color Color corresponding to a warning
    /// \param alert_color Color corresponding to an alarm
    /// \param emergency_color Color corresponding to an emergency
    /// \return Color
    const wxColor AdjustColorForZone(const double& val,
        const wxColor& nominal_color, const wxColor& normal_color,
        const wxColor& alert_color, const wxColor& warn_color,
        const wxColor& alarm_color, const wxColor& emergency_color)
    {
        wxColor c = nominal_color;
        Zone::state high_state = Zone::state::nominal;
        for (auto zone : m_zones) {
            if (high_state < Zone::state::emergency
                && val >= zone.GetLowerLimit() && val <= zone.GetUpperLimit()
                && high_state < zone.GetState()) {
                high_state = zone.GetState();
                switch (high_state) {
                case Zone::state::normal:
                    c = normal_color;
                    break;
                case Zone::state::alert:
                    c = alert_color;
                    break;
                case Zone::state::warn:
                    c = warn_color;
                    break;
                case Zone::state::alarm:
                    c = alarm_color;
                    break;
                case Zone::state::emergency:
                    c = emergency_color;
                    break;
                default:
                    c = nominal_color;
                }
            }
        }
        return c;
    }

    /// Get color for a part of the instrument corresponding to a value to be
    /// displayed
    ///
    /// \param val Duble value to be displayed
    /// \param item Part of the instrument
    /// \return Color to be used
    const wxColor GetColor(const double& val, const color_item item)
    {
        wxColor c;
        switch (item) {
        case color_item::title_bg:
            c = AdjustColorForZone(val, GetColorSetting(DSK_SNI_TITLE_BG),
                GetColorSetting(DSK_SNI_TITLE_BG),
                GetColorSetting(DSK_SNI_TITLE_BG),
                GetColorSetting(DSK_SNI_TITLE_BG),
                GetColorSetting(DSK_SNI_TITLE_BG),
                GetColorSetting(DSK_SNI_TITLE_BG));
            break;
        case color_item::title_fg:
            c = AdjustColorForZone(val, GetColorSetting(DSK_SNI_TITLE_FG),
                GetColorSetting(DSK_SNI_TITLE_FG),
                GetColorSetting(DSK_SNI_TITLE_FG),
                GetColorSetting(DSK_SNI_TITLE_FG),
                GetColorSetting(DSK_SNI_TITLE_FG),
                GetColorSetting(DSK_SNI_TITLE_FG));
            break;
        case color_item::body_bg:
            c = AdjustColorForZone(val, GetColorSetting(DSK_SNI_BODY_BG),
                GetColorSetting(DSK_SNI_BODY_BG),
                GetColorSetting(DSK_SNI_ALERT_BG),
                GetColorSetting(DSK_SNI_WARN_BG),
                GetColorSetting(DSK_SNI_ALRM_BG),
                GetColorSetting(DSK_SNI_EMERG_BG));
            break;
        case color_item::body_fg:
            c = AdjustColorForZone(val, GetColorSetting(DSK_SNI_BODY_FG),
                GetColorSetting(DSK_SNI_BODY_FG),
                GetColorSetting(DSK_SNI_ALERT_FG),
                GetColorSetting(DSK_SNI_WARN_FG),
                GetColorSetting(DSK_SNI_ALRM_FG),
                GetColorSetting(DSK_SNI_EMERG_FG));
            break;
        case color_item::border:
            c = AdjustColorForZone(val, GetColorSetting(DSK_SNI_BORDER_COLOR),
                GetColorSetting(DSK_SNI_BORDER_COLOR),
                GetColorSetting(DSK_SNI_BORDER_COLOR),
                GetColorSetting(DSK_SNI_BORDER_COLOR),
                GetColorSetting(DSK_SNI_BORDER_COLOR),
                GetColorSetting(DSK_SNI_BORDER_COLOR));
            break;
        }
        return c;
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
    double Transform(const double& val);
};

PLUGIN_END_NAMESPACE

#endif //_SIMPLENUMBER_H
