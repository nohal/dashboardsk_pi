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

#ifndef _SIMPLEGAUGE_H
#define _SIMPLEGAUGE_H

#include "instrument.h"
#include "pi_common.h"
#include "wx/jsonval.h"
#include <wx/clrpicker.h>

#define BORDER_SIZE 4 * scale
#define BORDER_LINE_WIDTH 2 * scale

// Setting names
#define DSK_SGI_GAUGE_TYPE "gauge_type"
#define DSK_SGI_NEEDLE_FG "needle_color"
#define DSK_SGI_RIM_NOMINAL "rim_color"
#define DSK_SGI_RIM_PORT "rim_port"
#define DSK_SGI_RIM_STBD "rim_starboard"
#define DSK_SGI_RIM_DEAD "rim_dead"
#define DSK_SGI_DIAL_COLOR "dial_color"
#define DSK_SGI_TICK_FG "tick_color"
#define DSK_SGI_TICK_LEGEND "tick_legend"

#define DSK_SGI_COLOR_NEEDLE *wxBLUE
#define DSK_SGI_COLOR_RIM wxColor(230, 230, 230)
#define DSK_SGI_COLOR_RIM_P *wxRED
#define DSK_SGI_COLOR_RIM_S *wxGREEN
#define DSK_SGI_COLOR_RIM_D wxColor(230, 230, 230)
#define DSK_SGI_COLOR_DIAL wxColor(230, 230, 230)
#define DSK_SGI_COLOR_TITLE wxColor(15, 15, 15)
#define DSK_SGI_COLOR_VALUE wxColor(15, 15, 15)
#define DSK_SGI_COLOR_TICK wxColor(15, 15, 15)
#define DSK_SGI_COLOR_TICK_TXT wxColor(168, 168, 168)
#define DSK_SGI_COLOR_NOMINAL wxColor(230, 230, 230)
#define DSK_SGI_COLOR_NORMAL wxColor(131, 189, 117)
#define DSK_SGI_COLOR_ALERT wxColor(255, 224, 132)
#define DSK_SGI_COLOR_WARN wxColor(255, 192, 87)
#define DSK_SGI_COLOR_ALARM wxColor(249, 139, 96)
#define DSK_SGI_COLOR_EMERG wxColor(234, 89, 89)
#define DSK_SGI_COLOR_BORDER *wxBLACK

#define DSK_SGI_SMOOTHING_MAX 9
#define DSK_SGI_INSTR_MIN_SIZE 50
#define DSK_SGI_INSTR_MAX_SIZE 500

// Table of transformations to be shown in the GUI
// Has to be in same order as gauge_type enum
#define DSK_SGI_GAUGE_TYPES                                                    \
    X(SimpleGaugeInstrument::gauge_type::relative_angle, _("Relative angle"))  \
    X(SimpleGaugeInstrument::gauge_type::direction, _("Direction"))            \
    X(SimpleGaugeInstrument::gauge_type::percent, _("Percentile"))             \
    X(SimpleGaugeInstrument::gauge_type::ranged_adaptive,                      \
        _("Adaptive range indicator"))                                         \
    X(SimpleGaugeInstrument::gauge_type::ranged_fixed,                         \
        _("Fixed range indicator"))

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_SGI_SETTINGS                                                       \
    X(0, DSK_SETTING_SK_KEY, wxString(wxEmptyString), _("SK Key"),             \
        SignalKKeyCtrl, wxEmptyString, AsString, GetStringSetting)             \
    X(1, DSK_SGI_GAUGE_TYPE, 0, _("Gauge type"), ChoiceCtrl,                   \
        ConcatChoiceStrings(m_gauge_types), AsInt, GetIntSetting)              \
    X(2, DSK_SETTING_FORMAT, 0, _("Format"), ChoiceCtrl,                       \
        ConcatChoiceStrings(m_supported_formats), AsInt, GetIntSetting)        \
    X(3, DSK_SETTING_TRANSFORMATION, 0, _("Transformation"), ChoiceCtrl,       \
        ConcatChoiceStrings(m_supported_transforms), AsInt, GetIntSetting)     \
    X(4, DSK_SETTING_ZONES, wxString(wxEmptyString), _("Zones"),               \
        SignalKZonesCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(5, DSK_SETTING_SMOOTHING, m_smoothing, _("Data smoothing"), SpinCtrl,    \
        "0;DSK_SGI_SMOOTHING_MAX", AsInt, GetIntSetting)                       \
    X(6, DSK_SETTING_INSTR_SIZE, m_instrument_size, _("Instrument size"),      \
        SpinCtrl, "DSK_SGI_INSTR_MIN_SIZE;DSK_SGI_INSTR_MAX_SIZE", AsInt,      \
        GetIntSetting)                                                         \
    X(7, DSK_SGI_NEEDLE_FG, DSK_SGI_COLOR_NEEDLE, _("Needle color"),           \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(7, DSK_SGI_RIM_NOMINAL, DSK_SGI_COLOR_RIM, _("Rim color"),               \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(8, DSK_SGI_RIM_PORT, DSK_SGI_COLOR_RIM_P, _("Port color"),               \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(9, DSK_SGI_RIM_STBD, DSK_SGI_COLOR_RIM_S, _("Starboard color"),          \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(10, DSK_SGI_RIM_DEAD, DSK_SGI_COLOR_RIM_D, _("Dead angle color"),        \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(11, DSK_SGI_DIAL_COLOR, DSK_SGI_COLOR_DIAL, _("Dial color"),             \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(12, DSK_SETTING_TITLE_FG, DSK_SGI_COLOR_TITLE, _("Title color"),         \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(13, DSK_SETTING_NOMINAL_FG, DSK_SGI_COLOR_VALUE, _("Value color"),       \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(14, DSK_SGI_TICK_FG, DSK_SGI_COLOR_TICK, _("Tick color"),                \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(15, DSK_SGI_TICK_LEGEND, DSK_SGI_COLOR_TICK_TXT, _("Dial values"),       \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(16, DSK_SETTING_NORMAL_FG, DSK_SGI_COLOR_NORMAL, _("Normal color"),      \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(17, DSK_SETTING_ALERT_FG, DSK_SGI_COLOR_ALERT, _("Alert color"),         \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(18, DSK_SETTING_WARN_FG, DSK_SGI_COLOR_WARN, _("Warn color"),            \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(19, DSK_SETTING_ALRM_FG, DSK_SGI_COLOR_ALARM, _("Alarm color"),          \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(20, DSK_SETTING_EMERG_FG, DSK_SGI_COLOR_EMERG, _("Emergency color"),     \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(21, DSK_SETTING_BORDER_COLOR, DSK_SGI_COLOR_BORDER, _("Border color"),   \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)

PLUGIN_BEGIN_NAMESPACE

/// Simple instrument displaying a single value from one SignalK path using a
/// gauge type instrument
class SimpleGaugeInstrument : public Instrument {

protected:
    /// Type of the gauge
    enum class gauge_type {
        /// Angle relative to the direction, -180..180 degrees
        relative_angle = 0,
        /// Direction, 0..360 degrees
        direction,
        /// Percent, 0..100
        percent,
        /// Ranged, with adaptive from-to
        ranged_adaptive,
        /// Ranged, with fixed from-to
        ranged_fixed // TODO
    };

    /// Enum to identify the part of the instrument graphical representation
    enum class color_item {
        rim = 0,
        rim_stbd,
        rim_port,
        rim_dead,
        dial,
        tick,
        legend,
        title,
        value,
        border
    };

    /// Constructor
    SimpleGaugeInstrument() { Init(); };

    /// Initialize the default parameters of the instrument
    virtual void Init();
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
    /// Array of names of supported gauge types
    wxArrayString m_gauge_types;
    /// Gauge type to be displayed by the instrument
    gauge_type m_gauge_type;
    /// Bitmap representation of the instrument
    wxBitmap m_bmp;
    /// Instrument in timed out state flag. True if the instrument is not
    /// receiving data. for more than #m_allowed_age_sec seconds.
    bool m_timed_out;
    /// Data smoothing ratio (0-DSK_SGI_SMOOTHING_MAX). The value effectively
    /// tells over how many historic values with progressively declining
    /// importance we want to smooth our data. 0 means the newest value is used
    /// as is, while 9 means the newest value has only 10% influence on the
    /// result. a(i+1) = (m_smoothing*a(i) +
    /// (DSK_SGI_SMOOTHING_MAX-m_smoothing+1)*data(i+1)) /
    /// (DSK_SGI_SMOOTHING_MAX + 1)
    size_t m_smoothing;
    /// Previous value displayed by the instrument
    double m_old_value;
    /// Maximum value recorded by the instrument
    double m_max_val;
    /// Minimum value recorded by the instrument
    double m_min_val;
    /// Size of the instrument in device independent pixels
    wxCoord m_instrument_size;

    /// Draw arc sector. The arc is drawn counterclockwise from \c start_angle
    /// to  \c end_angle
    ///
    /// \param dc Canvas to draw on
    /// \param start_angle start angle in degrees
    /// \param end_angle end angle in degrees
    /// \param xc Horizontal coordinate of the center
    /// \param yc Vertical coordinate of the center
    /// \param r Radius
    void DrawArc(wxDC& dc, const int& start_angle, const int& end_angle,
        const wxCoord& xc, const wxCoord& yc, const wxCoord& r);

    /// Draw tick marks on the perimeter of a circle
    ///
    /// \param dc Canvas to draw on
    /// \param start_angle
    /// \param angle_step
    /// \param xc Horizontal coordinate of the center
    /// \param yc Vertical coordinate of the center
    /// \param r Outer radius
    /// \param length Length of the tick towards the center
    /// \param labels Whether labels should be attached to the tick marks
    /// \param except_every Every except_every degree the label will not be
    /// drawn \param relative If true, label -180..180 instead of 0..360 \param
    /// draw_from Angular position of the first tick mark to draw \param draw_to
    /// Angular position of the last tick mark to draw \param labels_from
    /// Initial label value \param labels_step Step between labels
    void DrawTicks(wxDC& dc, const int& start_angle, const int& angle_step,
        const wxCoord& xc, const wxCoord& yc, const wxCoord& r,
        const wxCoord& length, bool labels = false, int except_every = 0,
        bool relative = false, int draw_from = 0, int draw_to = 360,
        int labels_from = 0, int labels_step = 0);

    /// Draw the needle pointing to a point on the rim of the instrument
    /// \param dc Canvas to draw on
    /// \param xc Horizontal coordinate of the center
    /// \param yc Vertical coordinate of the center
    /// \param r Outer radius
    /// \param angle Angle the needle points to
    /// \param perc_length Length of the needle in percent of r
    /// \param perc_width Width of the base of the needle in percent of r
    /// \param start_angle Angle where the instrument scale starts
    void DrawNeedle(wxDC& dc, const wxCoord& xc, const wxCoord& yc,
        const wxCoord& r, const wxCoord& angle, const int& perc_length,
        const int& perc_width = 20, const int& start_angle = 270);

    /// Render an instrument visualizing percentages (= value on the 0..100
    /// scale) into a bitmap
    ///
    /// \param scale scale of the instrument to be rendered (1.0 = natural
    /// scale) \return Instrument rendered into a bitmap with alpha channel
    wxBitmap RenderPercent(double scale);

    /// Render an instrument visualizing numerical value in a gauge with scale
    /// automatically adapting to the values displayed The instrument adjusts
    /// the range dynamically to be able to accomodate all the values
    /// rhistorically received The scale uses nerarest power of 10 not to
    /// fluctuate excessively and be able to label the ticks with integers
    ///
    /// \param scale scale of the instrument to be rendered (1.0 = natural
    /// scale) \param relative True if the displayed angle value is relative to
    /// the vessel (-180..180), false if absolute (0.360) \return Instrument
    /// rendered into a bitmap with alpha channel
    wxBitmap RenderAngle(double scale, bool relative = true);

    /// Render the instrument visualizing percentages (= value on the 0..100
    /// scale) into a bitmap
    ///
    /// \param scale scale of the instrument to be rendered (1.0 = natural
    /// scale) \return Instrument rendered into a bitmap with alpha channel
    wxBitmap RenderAdaptive(double scale);

    /// Get color for a part of the instrument corresponding to a value to be
    /// displayed
    ///
    /// \param val Duble value to be displayed
    /// \param item Part of the instrument
    /// \return Color to be used
    const wxColor GetColor(const double& val, const color_item item);

public:
    // We override some of the implementations of SetSetting to do "special
    // stuff" for some of the settings
    using Instrument::SetSetting;

    /// Constructor
    ///
    /// \param parent Pointer to the dashboard containing the instrument
    explicit SimpleGaugeInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    };

    wxString GetClass() const override
    {
        return SimpleGaugeInstrument::Class();
    };

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "SimpleGaugeInstrument"; };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    wxString GetDisplayType() const override
    {
        return SimpleGaugeInstrument::DisplayType();
    };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    static wxString DisplayType() { return _("Simple gauge"); };

    wxBitmap Render(double scale) override;

    void ReadConfig(wxJSONValue& config) override;

    wxJSONValue GenerateJSONConfig() override;

    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;

    wxString GetPrimarySKKey() const override { return m_sk_key; };
};

PLUGIN_END_NAMESPACE

#endif //_SIMPLEGAUGE_H
