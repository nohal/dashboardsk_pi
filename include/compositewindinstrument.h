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
#define DSK_CWI_ORIENTATION "orientation"
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
    X(DSK_CWI_ORIENTATION, 0, _("Orientation"), ChoiceCtrl,                    \
        _("North up;Heading up"), asInt, GetIntSetting)                        \
    X(DSK_SETTING_INSTR_SIZE, 200, _("Instrument size"), SpinCtrl, "50;500",   \
        asInt, GetIntSetting)                                                  \
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

    wxBitmap Render(double scale) override;
    bool IsClicked(wxCoord x, wxCoord y) const override;
    void ProcessData() override;
    void NotifyNewData(const wxString& fullpath) override;
    void ReadConfig(Json::Value& config) override;
    Json::Value GenerateJSONConfig() override;
    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;
    wxString GetPrimarySKKey() const override;

private:
    /// Identifiers for the six subscribed Signal K inputs.
    enum class input { awa, aws, twa, tws, heading, cog, count };
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
