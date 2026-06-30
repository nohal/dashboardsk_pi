/******************************************************************************
 * DashboardSK composite wind instrument
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#ifndef _COMPOSITEWINDINSTRUMENT_H
#define _COMPOSITEWINDINSTRUMENT_H

#include "instrument.h"
#include "pi_common.h"

#include <array>
#include <optional>

#define DSK_CWI_AWA_KEY "awa_sk_key"
#define DSK_CWI_AWS_KEY "aws_sk_key"
#define DSK_CWI_TWA_KEY "twa_sk_key"
#define DSK_CWI_TWS_KEY "tws_sk_key"
#define DSK_CWI_HEADING_KEY "heading_sk_key"
#define DSK_CWI_COG_KEY "cog_sk_key"
#define DSK_CWI_BEAT_KEY "beat_sk_key"
#define DSK_CWI_GYBE_KEY "gybe_sk_key"
#define DSK_CWI_STW_KEY "stw_sk_key"
#define DSK_CWI_CURRENT_SET_KEY "current_set_sk_key"
#define DSK_CWI_CURRENT_DRIFT_KEY "current_drift_sk_key"
#define DSK_CWI_SHOW_LAYLINES "show_laylines"
#define DSK_CWI_LAYLINE_REF "layline_ref"
#define DSK_CWI_BEAT_ANGLE "beat_angle"
#define DSK_CWI_GYBE_ANGLE "gybe_angle"
#define DSK_CWI_ORIENTATION "orientation"
#define DSK_CWI_RING_OPACITY "ring_opacity"
#define DSK_CWI_RING_COLOR "ring_color"
#define DSK_CWI_TICK_COLOR "tick_color"
#define DSK_CWI_TEXT_COLOR "text_color"
#define DSK_CWI_AWA_COLOR "awa_color"
#define DSK_CWI_TWA_COLOR "twa_color"
#define DSK_CWI_HEADING_COLOR "heading_color"
#define DSK_CWI_COG_COLOR "cog_color"
#define DSK_CWI_PORT_COLOR "port_color"
#define DSK_CWI_STARBOARD_COLOR "starboard_color"

#define DSK_CWI_SETTINGS                                                       \
    X(DSK_CWI_AWA_KEY, wxString(), _("AWA key"), SignalKKeyCtrl,               \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_AWS_KEY, wxString(), _("AWS key"), SignalKKeyCtrl,               \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_TWA_KEY, wxString(), _("TWA key"), SignalKKeyCtrl,               \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_TWS_KEY, wxString(), _("TWS key"), SignalKKeyCtrl,               \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_HEADING_KEY, wxString(), _("Heading key"), SignalKKeyCtrl,       \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_COG_KEY, wxString(), _("COG key"), SignalKKeyCtrl,               \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_BEAT_KEY, wxString(), _("Beat angle key"), SignalKKeyCtrl,       \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_GYBE_KEY, wxString(), _("Gybe angle key"), SignalKKeyCtrl,       \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_STW_KEY, wxString("navigation.speedThroughWater"),               \
        _("Speed through water key"), SignalKKeyCtrl, wxEmptyString, asString, \
        GetStringSetting)                                                      \
    X(DSK_CWI_CURRENT_SET_KEY, wxString("environment.current.setTrue"),        \
        _("Current set key"), SignalKKeyCtrl, wxEmptyString, asString,         \
        GetStringSetting)                                                      \
    X(DSK_CWI_CURRENT_DRIFT_KEY, wxString("environment.current.drift"),        \
        _("Current drift key"), SignalKKeyCtrl, wxEmptyString, asString,       \
        GetStringSetting)                                                      \
    X(DSK_CWI_SHOW_LAYLINES, 1, _("Show laylines"), ChoiceCtrl, _("Off;On"),   \
        asInt, GetIntSetting)                                                  \
    X(DSK_CWI_LAYLINE_REF, 0, _("Layline reference"), ChoiceCtrl,              \
        _("Through water (HDG/STW);Over ground (COG/SOG)"), asInt,             \
        GetIntSetting)                                                         \
    X(DSK_CWI_BEAT_ANGLE, 45, _("Beat angle"), SpinCtrl, "20;90", asInt,       \
        GetIntSetting)                                                         \
    X(DSK_CWI_GYBE_ANGLE, 150, _("Gybe angle"), SpinCtrl, "90;180", asInt,     \
        GetIntSetting)                                                         \
    X(DSK_CWI_ORIENTATION, 0, _("Orientation"), ChoiceCtrl,                    \
        _("North up;Heading up"), asInt, GetIntSetting)                        \
    X(DSK_SETTING_INSTR_SIZE, 200, _("Instrument size"), SpinCtrl, "50;500",   \
        asInt, GetIntSetting)                                                  \
    X(DSK_CWI_RING_OPACITY, 128, _("Ring opacity"), SpinCtrl, "0;255", asInt,  \
        GetIntSetting)                                                         \
    X(DSK_CWI_RING_COLOR, wxColor(32, 37, 43), _("Ring color"),                \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_CWI_TICK_COLOR, wxColor(230, 230, 230), _("Tick color"),             \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_CWI_TEXT_COLOR, wxColor(242, 242, 242), _("Text color"),             \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_CWI_AWA_COLOR, wxColor(77, 163, 255), _("AWA color"),                \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_CWI_TWA_COLOR, wxColor(255, 79, 195), _("TWA color"),                \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_CWI_HEADING_COLOR, *wxWHITE, _("Heading color"), ColourPickerCtrl,   \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_CWI_COG_COLOR, wxColor(138, 143, 152), _("COG color"),               \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_CWI_PORT_COLOR, wxColor(229, 72, 77), _("Port color"),               \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_CWI_STARBOARD_COLOR, wxColor(53, 199, 89), _("Starboard color"),     \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)           \
    X(DSK_SETTING_BORDER_COLOR, wxColor(5, 6, 7), _("Border color"),           \
        ColourPickerCtrl, wxEmptyString, asString, GetStringSetting)

PLUGIN_BEGIN_NAMESPACE

/// Composite display for apparent/true wind, heading and course over ground.
class CompositeWindInstrument : public Instrument {
public:
    using Instrument::SetSetting;

    /// Display orientation.
    enum class orientation { north_up = 0, heading_up };

    /// Construct an instrument attached to a dashboard.
    explicit CompositeWindInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    }

    /// Return the stable class identifier.
    static wxString Class() { return "CompositeWindInstrument"; }
    wxString GetClass() const override { return Class(); }

    /// Return the localized display name.
    static wxString DisplayType() { return _("Composite wind"); }
    wxString GetDisplayType() const override { return DisplayType(); }

    /// Combine a through-water velocity with a current to get the resulting
    /// course over ground. All bearings share one frame (0 = up, growing
    /// clockwise) and the two speeds share units.
    ///
    /// \param water_bearing Direction the boat moves through the water
    /// \param stw Speed through the water
    /// \param set_bearing Direction the current flows towards
    /// \param drift Speed of the current
    /// \return Bearing of the water + current vector sum, in [0, 360)
    static double GroundTrack(
        double water_bearing, double stw, double set_bearing, double drift);

    wxBitmap Render(double scale) override;
    wxSize ContentSize(const wxBitmap& bmp) const override;
    bool IsClicked(wxCoord x, wxCoord y) const override;
    void ProcessData() override;
    void NotifyNewData(const wxString& fullpath) override;
    void ReadConfig(Json::Value& config) override;
    Json::Value GenerateJSONConfig() override;
    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;
    wxString GetPrimarySKKey() const override;
    void SetChartRotation(double degrees) override;

private:
    /// Identifiers for the subscribed Signal K inputs.
    enum class input {
        awa,
        aws,
        twa,
        tws,
        heading,
        cog,
        beat,
        gybe,
        stw,
        current_set,
        current_drift,
        count
    };
    /// Cached value and freshness state for one input.
    struct datum {
        /// Most recently resolved numeric value.
        std::optional<double> value;
        /// Time of the most recent notification.
        std::chrono::system_clock::time_point changed;
        /// Whether this input has received or resolved a value.
        bool received = false;
    };

    /// Configured Signal K paths, indexed by #input.
    std::array<wxString, static_cast<size_t>(input::count)> m_keys;
    /// Cached input states, indexed by #input.
    std::array<datum, static_cast<size_t>(input::count)> m_data;
    /// Current north-up or heading-up orientation.
    orientation m_orientation;
    /// Chart canvas rotation (deg, chart north clockwise from screen up) added
    /// to every compass bearing so the dial tracks a course-up / head-up chart
    /// when the dashboard is anchored to the own ship; 0 otherwise.
    double m_chart_rotation = 0.0;
    /// Fallback close-hauled beat angle in degrees off the true wind.
    int m_beat_angle;
    /// Fallback running gybe angle in degrees off the true wind.
    int m_gybe_angle;
    /// Whether the laylines are drawn.
    bool m_show_laylines;
    /// Configured diameter in device-independent pixels.
    wxCoord m_instrument_size;
    /// Cached rendered bitmap.
    wxBitmap m_bmp;

    /// Initialize defaults and configuration controls.
    void Init();
    /// Subscribe to every configured input.
    void SubscribeAll();
    /// Refresh cached values from the Signal K tree.
    void RefreshData();
    /// Return a current value, or no value when missing/timed out.
    std::optional<double> Current(input item) const;
    /// Strip a source selector from a configured path.
    static wxString BasePath(const wxString& path);
    /// Normalize an angle to [0, 360).
    static double Normalize(double degrees);
};

PLUGIN_END_NAMESPACE

#endif
