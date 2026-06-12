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

#ifndef _DIVIDERINSTRUMENT_H
#define _DIVIDERINSTRUMENT_H

#include "instrument.h"
#include "pi_common.h"
#include "wx/jsonval.h"
#include <wx/pen.h>

// Default line color
#define DSK_DIV_COLOR_LINE wxColor(128, 128, 128)

// Setting names
#define DSK_SETTING_ORIENTATION "orientation"
#define DSK_SETTING_LENGTH "length"
#define DSK_SETTING_LINE_WIDTH "line_width"
#define DSK_SETTING_LINE_STYLE "line_style"
#define DSK_SETTING_LINE_COLOR "line_color"

#define DSK_DIV_MIN_LENGTH 10
#define DSK_DIV_MAX_LENGTH 1000
#define DSK_DIV_MIN_LINE_WIDTH 1
#define DSK_DIV_MAX_LINE_WIDTH 50

// Table of supported divider orientations to be shown in the GUI. The order
// must correspond to the values of DividerInstrument::orientation
#define DSK_DIV_ORIENTATIONS                                                   \
    X(DividerInstrument::orientation::automatic, _("Auto"))                    \
    X(DividerInstrument::orientation::horizontal, _("Horizontal"))             \
    X(DividerInstrument::orientation::vertical, _("Vertical"))

// Table of supported line styles and their respective wxPenStyle values
#define DSK_DIV_LINE_STYLES                                                    \
    X(wxPENSTYLE_SOLID, _("Solid"))                                            \
    X(wxPENSTYLE_SHORT_DASH, _("Dashes"))                                      \
    X(wxPENSTYLE_DOT, _("Dots"))                                               \
    X(wxPENSTYLE_DOT_DASH, _("Dash-Dot"))

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, wxJSONValue conversion function, getter function
#define DSK_DIV_SETTINGS                                                       \
    X(0, DSK_SETTING_ORIENTATION, 0, _("Orientation"), ChoiceCtrl,             \
        ConcatChoiceStrings(m_supported_orientations), AsInt, GetIntSetting)   \
    X(1, DSK_SETTING_LENGTH, m_length, _("Length"), SpinCtrl,                  \
        STRINGIFY(DSK_DIV_MIN_LENGTH) ";" STRINGIFY(DSK_DIV_MAX_LENGTH),       \
        AsInt, GetIntSetting)                                                  \
    X(2, DSK_SETTING_LINE_WIDTH, m_line_width, _("Line width"), SpinCtrl,      \
        STRINGIFY(DSK_DIV_MIN_LINE_WIDTH) ";" STRINGIFY(                       \
            DSK_DIV_MAX_LINE_WIDTH),                                           \
        AsInt, GetIntSetting)                                                  \
    X(3, DSK_SETTING_LINE_STYLE, 0, _("Line style"), ChoiceCtrl,               \
        ConcatChoiceStrings(m_supported_styles), AsInt, GetIntSetting)         \
    X(4, DSK_SETTING_LINE_COLOR, DSK_DIV_COLOR_LINE, _("Line color"),          \
        ColourPickerCtrl, wxEmptyString, AsString, GetStringSetting)

PLUGIN_BEGIN_NAMESPACE

/// "Dummy" instrument displaying a divider line of user defined length,
/// thickness, style and color, used to allow visual grouping of the
/// instruments in the dashboard. Does not consume any SignalK data.
class DividerInstrument : public Instrument {

public:
    // We override some of the implementations of SetSetting to do "special
    // stuff" for some of the settings
    using Instrument::SetSetting;

    /// Orientation of the divider line
    enum class orientation {
        /// Perpendicular to the flow of the instruments in the dashboard,
        /// derived from the anchor edge of the parent dashboard
        automatic = 0,
        /// Horizontal line
        horizontal,
        /// Vertical line
        vertical
    };

protected:
    /// Orientation of the divider line
    orientation m_orientation;
    /// Length of the line in DIP
    wxCoord m_length;
    /// Thickness of the line in DIP
    wxCoord m_line_width;
    /// Index of the line style in the DSK_DIV_LINE_STYLES table
    int m_line_style;
    /// Localized labels of the supported orientations
    wxArrayString m_supported_orientations;
    /// Localized labels of the supported line styles
    wxArrayString m_supported_styles;
    /// wxPenStyle values corresponding to #m_supported_styles
    vector<wxPenStyle> m_style_values;
    /// Orientation in which the cached bitmap was rendered. In automatic
    /// orientation mode the effective orientation can change without any
    /// setting being touched (when the anchor edge of the parent dashboard
    /// changes), invalidating the cached bitmap.
    bool m_rendered_vertical;
    /// Bitmap representation of the instrument
    wxBitmap m_bmp;

    /// Constructor
    DividerInstrument() { Init(); };

    /// Initialize the default parameters of the instrument
    void Init();

    /// Resolve the effective orientation of the divider line
    ///
    /// \return True if the line is to be drawn vertically
    bool IsVertical() const;

    /// Get the pen style corresponding to the configured line style
    ///
    /// \return wxPenStyle to be used for drawing the line
    wxPenStyle GetPenStyle() const;

public:
    /// Constructor
    ///
    /// \param parent Pointer to the dashboard containing the instrument
    explicit DividerInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    };

    wxString GetClass() const override { return DividerInstrument::Class(); };

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "DividerInstrument"; };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    wxString GetDisplayType() const override
    {
        return DividerInstrument::DisplayType();
    };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    static wxString DisplayType() { return _("Divider"); };

    wxBitmap Render(double scale) override;

    void ReadConfig(wxJSONValue& config) override;

    wxJSONValue GenerateJSONConfig() override;

    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;
};

PLUGIN_END_NAMESPACE

#endif //_DIVIDERINSTRUMENT_H
