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

#ifndef _SIMPLEHISTOGRAM_H
#define _SIMPLEHISTOGRAM_H

#include "instrument.h"
#include "pi_common.h"
#include "wx/jsonval.h"
#include <chrono>
#include <deque>
#include <wx/clrpicker.h>

#define BORDER_SIZE 4 * scale
#define BORDER_LINE_WIDTH 2 * scale

// Palette
#define DSK_SHI_COLOR_TITLE_FG wxColor(83, 46, 28)
#define DSK_SHI_COLOR_BODY_BG wxColor(230, 230, 230)
#define DSK_SHI_COLOR_BODY_FG wxColor(15, 15, 15)
#define DSK_SHI_COLOR_MEAN_FG wxColor(255, 50, 50)
#define DSK_SHI_COLOR_TIME_FG wxColor(128, 128, 128)
#define DSK_SHI_COLOR_BORDER *wxBLACK

#define DSK_SHI_INSTR_MIN_WIDTH 50
#define DSK_SHI_INSTR_MAX_WIDTH 500
#define DSK_SHI_INSTR_MIN_HEIGHT 50
#define DSK_SHI_INSTR_MAX_HEIGHT 500

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_SHI_SETTINGS                                                       \
    X(0, DSK_SETTING_SK_KEY, wxString(wxEmptyString), _("SK Key"),             \
        SignalKKeyCtrl, wxEmptyString, AsString, GetStringSetting)             \
    X(1, DSK_SETTING_FORMAT, 0, _("Format"), ChoiceCtrl,                       \
        ConcatChoiceStrings(m_supported_formats), AsInt, GetIntSetting)        \
    X(2, DSK_SETTING_TRANSFORMATION, 0, _("Transformation"), ChoiceCtrl,       \
        ConcatChoiceStrings(m_supported_transforms), AsInt, GetIntSetting)     \
    X(3, DSK_SETTING_ORDER, 0, _("Order"), ChoiceCtrl,                         \
        ConcatChoiceStrings(m_supported_orders), AsInt, GetIntSetting)         \
    X(4, DSK_SETTING_HISTORY, 0, _("History"), ChoiceCtrl,                     \
        ConcatChoiceStrings(m_supported_histories), AsInt, GetIntSetting)      \
    X(5, DSK_SETTING_INSTR_WIDTH, m_instrument_width, _("Instrument width"),   \
        SpinCtrl,                                                              \
        STRINGIFY(DSK_SHI_INSTR_MIN_WIDTH) ";" STRINGIFY(                      \
            DSK_SHI_INSTR_MAX_WIDTH),                                          \
        AsInt, GetIntSetting)                                                  \
    X(6, DSK_SETTING_INSTR_HEIGHT, m_instrument_height,                        \
        _("Instrument height"), SpinCtrl,                                      \
        STRINGIFY(DSK_SHI_INSTR_MIN_WIDTH) ";" STRINGIFY(                      \
            DSK_SHI_INSTR_MAX_WIDTH),                                          \
        AsInt, GetIntSetting)                                                  \
    X(7, DSK_SETTING_TITLE_FG, DSK_SHI_COLOR_TITLE_FG, _("Title color"),       \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(8, DSK_SETTING_BODY_BG, DSK_SHI_COLOR_BODY_BG, _("Background"),          \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(9, DSK_SETTING_BODY_FG, DSK_SHI_COLOR_BODY_FG, _("Graph color"),         \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(10, DSK_SETTING_MEAN_FG, DSK_SHI_COLOR_MEAN_FG, _("Mean color"),         \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(11, DSK_SETTING_TIME_FG, DSK_SHI_COLOR_TIME_FG, _("Time color"),         \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)           \
    X(12, DSK_SETTING_BORDER_COLOR, DSK_SHI_COLOR_BORDER, _("Border color"),   \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)

PLUGIN_BEGIN_NAMESPACE

struct HistoryValue {
    std::chrono::time_point<std::chrono::system_clock> ts;
    size_t values;
    double sum;

    HistoryValue()
        : ts(std::chrono::system_clock::now())
        , values(0)
        , sum(0.0) { };
    explicit HistoryValue(const double& val)
        : ts(std::chrono::system_clock::now())
        , values(1)
        , sum(val) { };
    void Add(const double& val)
    {
        ++values;
        sum += val;
    };
    double GetMean() { return values > 0 ? sum / values : 0.0; };
    bool OlderThan(std::chrono::duration<int64_t> duration)
    {
        return ts + duration < std::chrono::system_clock::now();
    };
    bool NewerThan(std::chrono::duration<int64_t> duration)
    {
        return ts + duration > std::chrono::system_clock::now();
    };
    bool OlderThan(const HistoryValue& other) { return ts > other.ts; };
    bool NewerThan(const HistoryValue& other) { return ts < other.ts; };

    bool operator!=(const HistoryValue& x) const { return x.ts != ts; }
};

class History {
    friend class SimpleHistogramInstrument;

protected:
    /// @brief Buffer for the last minute with 1s granularity (60 values)
    std::deque<HistoryValue> m_last_minute;
    /// @brief Buffer for the last 1 hour with 10 second granularity (360
    /// values)
    std::deque<HistoryValue> m_last_hour;
    /// @brief Buffer for the last 3 days with 5 minute granularity (864 values)
    std::deque<HistoryValue> m_last_3days;

public:
    void Add(const double& value);
};

/// Simple instrument displaying a single value from one SignalK path
class SimpleHistogramInstrument : public Instrument {

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
        border,
        /// Mean value line and label
        mean_fg,
        /// Time labels
        time_fg
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
    /// Array of names of supported data data orders
    wxArrayString m_supported_orders;
    /// Value order
    Instrument::value_order m_value_order;
    /// Array of supported history length labels
    wxArrayString m_supported_histories;
    /// Length of the history displayed
    Instrument::history_length m_history_length;
    /// Bitmap representation of the instrument
    wxBitmap m_bmp;
    /// Instrument in timed out state flag. True if the instrument is not
    /// receiving data. for more than #m_allowed_age_sec seconds.
    bool m_timed_out;
    /// Previous value displayed by the instrument
    double m_old_value;
    /// @brief Historical values
    History m_history;
    /// @brief Width  of the instrument
    wxCoord m_instrument_width;
    /// @brief Height of the instrument
    wxCoord m_instrument_height;

    /// Constructor
    SimpleHistogramInstrument() { Init(); };

    /// Initialize the default parameters of the instrument
    void Init();

    /// Get color for a part of the instrument
    ///
    /// \param item Part of the instrument
    /// \return Color to be used
    const wxColor GetColor(const color_item item);

    /// @brief Format numerical value
    /// @param value Numerical value
    /// @return String representation of the numerical value using the format
    /// set for the instrument
    const wxString FormatValue(const double& value)
    {
        wxString str
            = wxString::Format(m_format_strings[m_format_index], abs(value));
        if (value < 0
            && !m_supported_formats[m_format_index].StartsWith("ABS")) {
            str.Prepend("-");
        }
        return str;
    }

    /// @brief Formats the interval between historical value and current time
    ///
    /// @param val Timestamp in the past
    /// @return String representation of the difference bewenn current time and
    /// the provided timestamp
    const wxString FormatTime(const std::chrono::system_clock::time_point& val)
    {
        auto dur = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - val);
        wxString s;
        if (dur.count() < 120) {
            s = wxString::Format("-%ds", (int)dur.count());
        } else if (dur.count() < 600) {
            s = wxString::Format("-%.1fm", (double)dur.count() / 60);
        } else if (dur.count() < 2 * 3600) {
            s = wxString::Format("-%.0fm", round((double)dur.count() / 60));
        } else if (dur.count() < 6 * 3600) {
            s = wxString::Format("-%.1fh", round((double)dur.count() / 3600));
        } else {
            s = wxString::Format("-%.0fh", round((double)dur.count() / 3600));
        }
        return s;
    }

public:
    /// Constructor
    ///
    /// \param parent Pointer to the dashboard containing the instrument
    explicit SimpleHistogramInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    };

    wxString GetClass() const override
    {
        return SimpleHistogramInstrument::Class();
    };

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "SimpleHistogramInstrument"; };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    wxString GetDisplayType() const override
    {
        return SimpleHistogramInstrument::DisplayType();
    };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    static wxString DisplayType() { return _("Simple histogram"); };

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

    /// Only process the SK data without drawing anything
    void ProcessData() override;
};

PLUGIN_END_NAMESPACE

#endif //_SIMPLEHISTOGRAM_H
