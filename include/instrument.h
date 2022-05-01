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

#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_

#include "pi_common.h"
#include "zone.h"

#include <wx/bitmap.h>
#include <wx/dcgraph.h>

#include "wx/jsonval.h"
#include <unordered_map>

PLUGIN_BEGIN_NAMESPACE

#define LUMIMOSITY_NIGHT (-0.8)
#define LUMIMOSITY_DUSK (-0.5)

// TODO: Add the other "general" setting names like DSK_SNI_TRANSFORMATION
#define DSK_SETTING_ZONES "zones"
#define DSK_SETTING_SK_KEY "sk_key"
#define DSK_SETTING_FORMAT "format"
#define DSK_SETTING_TRANSFORMATION "transformation"
#define DSK_SETTING_SMOOTHING "smoothing"
#define DSK_SETTING_TITLE_FONT "title_font"
#define DSK_SETTING_BODY_FONT "body_font"
#define DSK_SETTING_TITLE_BG "title_background"
#define DSK_SETTING_TITLE_FG "title_color"
#define DSK_SETTING_BODY_BG "body_background"
#define DSK_SETTING_BODY_FG "body_color"
#define DSK_SETTING_NOMINAL_BG "nominal_background"
#define DSK_SETTING_NOMINAL_FG "nominal_color"
#define DSK_SETTING_NORMAL_BG "normal_background"
#define DSK_SETTING_NORMAL_FG "normal_color"
#define DSK_SETTING_ALERT_BG "alert_background"
#define DSK_SETTING_ALERT_FG "alert_color"
#define DSK_SETTING_WARN_BG "warn_background"
#define DSK_SETTING_WARN_FG "warn_color"
#define DSK_SETTING_ALRM_BG "alarm_background"
#define DSK_SETTING_ALRM_FG "alarm_color"
#define DSK_SETTING_EMERG_BG "emergency_background"
#define DSK_SETTING_EMERG_FG "emergency_color"
#define DSK_SETTING_BORDER_COLOR "border_color"
#define DSK_SETTING_INSTR_SIZE "instrument_size"

// Table of transformations to be shown in the GUI
#define DSK_UNIT_TRANSFORMATIONS                                               \
    X(Instrument::transformation::none, _("None"))                             \
    X(Instrument::transformation::rad2deg, _("RAD->DEG"))                      \
    X(Instrument::transformation::ms2kn, _("m/s -> kn"))                       \
    X(Instrument::transformation::ms2kmh, _("m/s -> km/h"))                    \
    X(Instrument::transformation::ms2mph, _("m/s -> mph"))                     \
    X(Instrument::transformation::m2ft, _("m -> feet"))                        \
    X(Instrument::transformation::m2fm, _("m -> fathoms"))                     \
    X(Instrument::transformation::m2nm, _("m -> NMi"))                         \
    X(Instrument::transformation::degk2degc, _("\u00B0K -> \u00B0C"))          \
    X(Instrument::transformation::degk2degf, _("\u00B0K -> \u00B0F"))          \
    X(Instrument::transformation::ratio2perc, _("ratio -> %"))                 \
    X(Instrument::transformation::pa2hpa, _("Pa -> hPa"))                      \
    X(Instrument::transformation::pa2kpa, _("Pa -> kPa"))                      \
    X(Instrument::transformation::pa2mpa, _("Pa -> MPa"))                      \
    X(Instrument::transformation::pa2atm, _("Pa -> atm"))                      \
    X(Instrument::transformation::pa2atm, _("Pa -> mmHg"))                     \
    X(Instrument::transformation::pa2psi, _("Pa -> psi"))                      \
    X(Instrument::transformation::hz2rpm, _("Hz -> RPM"))

// Table of supported formats and the respective formatting strings
#define DSK_VALUE_FORMATS                                                      \
    X(0, "9.9", "%.1f")                                                        \
    X(1, "9.99", "%.2f")                                                       \
    X(2, "9.999", "%.3f")                                                      \
    X(3, "9", "%.0f")                                                          \
    X(4, "009", "%03.0f")                                                      \
    X(5, "009.9", "%05.1f")                                                    \
    X(6, "09.9", "%04.1f")                                                     \
    X(7, "09.99", "%05.2f")                                                    \
    X(8, "009.99", "%06.2f")

/// Key-value pair unordered map for instrument configuration parameters
#if wxCHECK_VERSION(3, 1, 0)
typedef unordered_map<wxString, wxString> config_map_t;
#else
typedef unordered_map<string, wxString> config_map_t;
#endif

/// Instrument configuration controls type enum
enum class dskConfigCtrl {
    Spacer = 0,
    TextCtrl,
    ColourPickerCtrl,
    SpinCtrl,
    SpinCtrlDouble,
    ChoiceCtrl,
    SignalKKeyCtrl,
    SignalKZonesCtrl
};

/// Structure describing a single configuration control for the instrument
struct config_control {
    /// String key identifying the control. Must not be localizable.
    wxString key;
    /// Localizable string description of the parameter being configured to be
    /// used as label
    wxString description;
    /// The type of the control to create
    dskConfigCtrl control;
    /// String with serialized settings of the control
    wxString control_settings;
};

/// Alarm type
/// See \c definitions.json in SignalK schema
enum class alarmType {
    /// No alarm
    none = 0,
    /// Visual alarm
    visual,
    /// Sound alarm
    sound
};

class Dashboard;

/// Abstract parent class for all the instruments
class Instrument {
public:
    /// Supported basic value transformations
    /// New transformations to be added to DSK_BASIC_TRANSFORMATIONS macro and
    /// implemented in #Instrument::Transform
    enum class transformation {
        none = 0,
        /// Radians to degrees
        rad2deg,
        /// meters/second to knots
        ms2kn,
        /// meters/second to km/h
        ms2kmh,
        /// meters/second to miles per hour
        ms2mph,
        /// meters to feet
        m2ft,
        /// meters to fathoms
        m2fm,
        /// meters to nautical miles
        m2nm,
        /// Kelvin to Celsius
        degk2degc,
        /// Kelvin to Fahrenheit
        degk2degf,
        /// Ratio to percent
        ratio2perc,
        /// Pascal to hectopascal
        pa2hpa,
        /// Pascal to kilopascal
        pa2kpa,
        /// Pascal to megapascal
        pa2mpa,
        /// Pascal to atmosphere
        pa2atm,
        /// Pascal to mmHg
        pa2mmhg,
        /// Pascal to psi
        pa2psi,
        /// Hertz to revolutions per minute
        hz2rpm
    };

protected:
    /// User defined name of the instrument
    wxString m_name;
    /// User defined title for the rendered instrument
    wxString m_title;
    /// Vector of configuration controls to be used in the GUI
    vector<config_control> m_config_controls;
    /// Configuration value map
    config_map_t m_config_vals;
    /// Color scheme
    int m_color_scheme;
    /// Timestamp of last update of the value displayed by the instrument
    /// instance
    wxDateTime m_last_change;
    /// Maximum allowed age of the displayed value before it is considered
    /// unusable
    int m_allowed_age_sec;
    /// Pointer to the dashboard owning the instrument instance
    Dashboard* m_parent_dashboard;
    /// Horizontal position of the top left corner of the rendered instrument on
    /// canvas in DIP
    wxCoord m_x;
    /// Vertical position of the top left corner of the rendered instrument on
    /// canvas in DIP
    wxCoord m_y;
    /// Width of the rendered instrument in DIP
    wxCoord m_width;
    /// Height of the rendered instrument in DIP
    wxCoord m_height;
    /// Indicator there is new data available
    bool m_new_data;
    /// Value zones  definitions
    vector<Zone> m_zones;
    /// Alarm state matrix
    unordered_map<Zone::state, vector<alarmType>> m_alarm_methods;
    /// Needs redraw on next overlay refresh
    bool m_needs_redraw;

    /// Get version of a color adjusted to the current color scheme set for the
    /// instrument
    ///
    /// \return Version of the color adjusted to the current color scheme
    wxColor GetDimedColor(const wxColor& c) const;

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
        const wxColor& alarm_color, const wxColor& emergency_color);

    Instrument()
        : m_name(wxEmptyString)
        , m_title(wxEmptyString)
        , m_color_scheme(0)
        , m_last_change(wxInvalidDateTime)
        , m_allowed_age_sec(3)
        , m_parent_dashboard(nullptr)
        , m_x(0)
        , m_y(0)
        , m_width(0)
        , m_height(0)
        , m_new_data(false)
        , m_needs_redraw(true)
    {
    }

    /// Scale a bitmap
    ///
    /// \param bmpIn Bitmap to be scaled
    /// \param scale Scale (1.0 = 100%)
    /// \param antialiasing Whether antialiasing be used when scale < 0.4
    /// \return Scaled version of the bitmap
    static wxBitmap ScaleBitmap(
        wxBitmap& bmpIn, double scale, bool antialiasing);

    /// Return semicolon separated supported items for the GUI
    ///
    /// \ param arr wxArrayString of values to concatenate
    /// \return All format strings separated by semicolons
    const wxString ConcatChoiceStrings(wxArrayString arr);

public:
    /// Constructor
    ///
    /// \param parent Dashboard to which the instrument belongs
    explicit Instrument(Dashboard* parent)
        : Instrument()
    {
        m_parent_dashboard = parent;
    };

    /// Set the actual area occupied by the rendered instrument on the canvas
    ///
    /// \param x Upper left corner horizontal postion
    /// \param y Upper left corner vertical position
    /// \param w Width
    /// \param h Height
    virtual void SetPlacement(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
    {
        m_x = x;
        m_y = y;
        m_width = w;
        m_height = h;
    };

    /// Destructor
    virtual ~Instrument();

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    virtual wxString GetClass() const { return Instrument::Class(); };

    /// Class name, child instrument classes have to override it with their
    /// respective unique value (should be the same as the actual name of the
    /// class). Must not be localizable.
    ///
    /// \return Instrument class name
    static wxString Class() { return "Instrument"; };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    virtual wxString GetDisplayType() const
    {
        return Instrument::DisplayType();
    };

    /// Human readable name for the instrument to be presented in the
    /// configuration dialog, child instrument classes have to override it with
    /// their respective unique value
    ///
    /// \return Name of the instrument gauge
    static wxString DisplayType() { return wxEmptyString; };

    /// Get name of the instrument
    ///
    /// \return The string containing the name of the instrument
    wxString GetName() { return m_name; };

    /// Get title of the rendered instrument
    ///
    /// \return The string containing the title of the rendered instrument
    wxString GetTitle() { return m_title; };

    /// Get timeout length for the instrument.
    ///
    /// \return The number of seconds since last update after which the value is
    /// considered outdated
    int GetTimeout() { return m_allowed_age_sec; };

    /// Returns a list of controls to configure the appearance of the instrument
    /// to be used in the GUI.
    ///
    /// \return Vector of controls to be used in the instrument configuration
    /// panel
    vector<config_control> ConfigControls() const { return m_config_controls; };

    /// Render the instrument to bitmap image
    ///
    /// \param scale Double variable representing the scale (1.0 = 100%)
    /// \return Scaled bitmap
    virtual wxBitmap Render(double scale);

    /// Set color schema, the instrument should respect the application wide
    /// color schema and dime it's output accordingly
    ///
    /// \param scheme Integer parameter specifying the color scheme (0 - RGB, 1
    /// - DAY, 2 - DUSK, 3 - NIGHT)
    void SetColorScheme(int scheme);

    /// Load configuration from JSON object
    /// Extended in child instrument classes
    ///
    /// \param config Reference wxJSONValue containing the configuration
    /// information
    virtual void ReadConfig(wxJSONValue& config);

    /// Generate JSON object with configuration parameters
    /// Extended in child instrument classes
    ///
    /// \return wxJSONValue containing the configuration information
    virtual wxJSONValue GenerateJSONConfig();

    /// Notify the instrument there is new data available for some of the paths
    /// it is subscribed to
    ///
    /// \param fullpath Full SignalK path that changed value
    virtual void NotifyNewData(const wxString& fullpath) { m_new_data = true; };

    /// Get wxColor from string color in web "#FFFFFF" format aka
    /// wxC2S_HTML_SYNTAX
    ///
    /// \param color String version of the color
    /// \return wxColor created from the string
    static const wxColor ColorFromString(const wxString& color);

    /// Get integer from a string numerical
    /// \param str String numerical
    /// \return Integer conversion of the string (0 if the conversion was
    /// unsuccessful)
    static const int IntFromString(const wxString& str);

    /// Get double from a string numerical
    /// \param str String numerical
    /// \return Double conversion of the string (0.0 if the conversion was
    /// unsuccessful)
    static const double DoubleFromString(const wxString& str);

    /// Get a string representation of a configuration parameter of the
    /// instrument
    ///
    /// \param key Identification key of the parameter
    /// \return Value of the parameter as string
    virtual wxString GetStringSetting(const wxString& key);

    /// Get an integer representation of a configuration parameter of the
    /// instrument
    ///
    /// \param key Identification key of the parameter
    /// \return Value of the parameter as integer
    virtual int GetIntSetting(const wxString& key);

    /// Get an double representation of a configuration parameter of the
    /// instrument
    ///
    /// \param key Identification key of the parameter
    /// \return Value of the parameter as double
    virtual int GetDoubleSetting(const wxString& key);

    /// Get a string representation of a configuration parameter of the
    /// instrument
    ///
    /// \param key Identification key of the parameter
    /// \return Value of the parameter as string
    virtual wxColor GetColorSetting(const wxString& key);

    /// Set a string representation of a configuration parameter of the
    /// instrument
    ///
    /// \param key Identification key of the parameter
    /// \param value Value of the parameter as string
    virtual void SetSetting(const wxString& key, const wxString& value);

    /// Set a string representation of a color parameter of the instrument
    ///
    /// \param key Identification key of the parameter
    /// \param value Color value of the parameter
    virtual void SetSetting(const wxString& key, const wxColor& value);

    /// Set a string representation of an integer parameter of the instrument
    ///
    /// \param key Identification key of the parameter
    /// \param value Integer value of the parameter
    virtual void SetSetting(const wxString& key, const int& value);

    /// Force redraw of the instrument on the next overlay refresh
    void ForceRedraw() { m_needs_redraw = true; };

    /// Transform the value using function implemented for the value of
    /// #m_transformation. Every transformation defined in
    /// #Instrument::transformation and DSK_UNIT_TRANSFORMATIONS
    /// macro has to be covered here.
    ///
    /// \param val Double value to be transformed
    /// \param formula The transformation formula to be used
    /// \return Transformed value
    static double Transform(const double& val, const transformation& formula);
};

PLUGIN_END_NAMESPACE

#endif //_INSTRUMENT_H_
