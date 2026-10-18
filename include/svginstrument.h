/******************************************************************************
 * DashboardSK SVG skin instrument
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#ifndef _SVGINSTRUMENT_H
#define _SVGINSTRUMENT_H

#include "instrument.h"
#include "pi_common.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

#include <pugixml.hpp>
#include <wx/filename.h>

/// Number of value slots (v0..v3) an SVG skin can bind to
#define DSK_SVG_SLOTS 4

#define DSK_SVG_FILE "svg_file"
#define DSK_SVG_KEY_0 "sk_key_0"
#define DSK_SVG_KEY_1 "sk_key_1"
#define DSK_SVG_KEY_2 "sk_key_2"
#define DSK_SVG_KEY_3 "sk_key_3"
#define DSK_SVG_SCALE_MIN "scale_min"
#define DSK_SVG_SCALE_MAX "scale_max"
#define DSK_SVG_LABEL_0 "label_0"
#define DSK_SVG_LABEL_1 "label_1"
#define DSK_SVG_LABEL_2 "label_2"
#define DSK_SVG_LABEL_3 "label_3"
#define DSK_SVG_UNIT_0 "unit_0"
#define DSK_SVG_UNIT_1 "unit_1"
#define DSK_SVG_UNIT_2 "unit_2"
#define DSK_SVG_UNIT_3 "unit_3"
#define DSK_SVG_TRANSFORM_0 "transform_0"
#define DSK_SVG_TRANSFORM_1 "transform_1"
#define DSK_SVG_TRANSFORM_2 "transform_2"
#define DSK_SVG_TRANSFORM_3 "transform_3"
#define DSK_SVG_FORMAT_0 "format_0"
#define DSK_SVG_FORMAT_1 "format_1"
#define DSK_SVG_FORMAT_2 "format_2"
#define DSK_SVG_FORMAT_3 "format_3"

// Setting name, default value, label, dskConfigCtrl control type, control
// parameters string, Json::Value conversion function, getter function
#define DSK_SVG_SETTINGS                                                       \
    X(DSK_SVG_FILE, wxString("wind_angle.svg"), _("Skin"), SvgSkinCtrl,        \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_KEY_0, wxString(), _("Value v0 key"), SignalKKeyCtrl,            \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_KEY_1, wxString(), _("Value v1 key"), SignalKKeyCtrl,            \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_KEY_2, wxString(), _("Value v2 key"), SignalKKeyCtrl,            \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_KEY_3, wxString(), _("Value v3 key"), SignalKKeyCtrl,            \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_SCALE_MIN, wxString("0"), _("Scale minimum"), SpinCtrlDouble,    \
        "-1000000;1000000", asString, GetStringSetting)                        \
    X(DSK_SVG_SCALE_MAX, wxString("0"), _("Scale maximum"), SpinCtrlDouble,    \
        "-1000000;1000000", asString, GetStringSetting)                        \
    X(DSK_SETTING_ZONES, wxString(), _("Zones"), SignalKZonesCtrl,             \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_LABEL_0, wxString(), _("Value v0 label"), TextCtrl,              \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_LABEL_1, wxString(), _("Value v1 label"), TextCtrl,              \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_LABEL_2, wxString(), _("Value v2 label"), TextCtrl,              \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_LABEL_3, wxString(), _("Value v3 label"), TextCtrl,              \
        wxEmptyString, asString, GetStringSetting)                             \
    X(DSK_SVG_UNIT_0, wxString(), _("Value v0 unit"), TextCtrl, wxEmptyString, \
        asString, GetStringSetting)                                            \
    X(DSK_SVG_UNIT_1, wxString(), _("Value v1 unit"), TextCtrl, wxEmptyString, \
        asString, GetStringSetting)                                            \
    X(DSK_SVG_UNIT_2, wxString(), _("Value v2 unit"), TextCtrl, wxEmptyString, \
        asString, GetStringSetting)                                            \
    X(DSK_SVG_UNIT_3, wxString(), _("Value v3 unit"), TextCtrl, wxEmptyString, \
        asString, GetStringSetting)                                            \
    X(DSK_SVG_TRANSFORM_0, 0, _("Value v0 transformation"), ChoiceCtrl,        \
        ConcatChoiceStrings(m_supported_transforms), asInt, GetIntSetting)     \
    X(DSK_SVG_TRANSFORM_1, 0, _("Value v1 transformation"), ChoiceCtrl,        \
        ConcatChoiceStrings(m_supported_transforms), asInt, GetIntSetting)     \
    X(DSK_SVG_TRANSFORM_2, 0, _("Value v2 transformation"), ChoiceCtrl,        \
        ConcatChoiceStrings(m_supported_transforms), asInt, GetIntSetting)     \
    X(DSK_SVG_TRANSFORM_3, 0, _("Value v3 transformation"), ChoiceCtrl,        \
        ConcatChoiceStrings(m_supported_transforms), asInt, GetIntSetting)     \
    X(DSK_SVG_FORMAT_0, 0, _("Value v0 format"), ChoiceCtrl,                   \
        ConcatChoiceStrings(m_supported_formats), asInt, GetIntSetting)        \
    X(DSK_SVG_FORMAT_1, 0, _("Value v1 format"), ChoiceCtrl,                   \
        ConcatChoiceStrings(m_supported_formats), asInt, GetIntSetting)        \
    X(DSK_SVG_FORMAT_2, 0, _("Value v2 format"), ChoiceCtrl,                   \
        ConcatChoiceStrings(m_supported_formats), asInt, GetIntSetting)        \
    X(DSK_SVG_FORMAT_3, 0, _("Value v3 format"), ChoiceCtrl,                   \
        ConcatChoiceStrings(m_supported_formats), asInt, GetIntSetting)        \
    X(DSK_SETTING_INSTR_SIZE, 200, _("Instrument size"), SpinCtrl, "50;500",   \
        asInt, GetIntSetting)

PLUGIN_BEGIN_NAMESPACE

/// Instrument drawing a user supplied SVG "skin" whose elements are bound to
/// SignalK values through `data-dsk-*` attributes (see the manual for the
/// authoring rules). The SVG is rasterized by wxWidgets' bundled nanosvg,
/// which can't draw text, so `<text>` elements are drawn with wxDC on top.
class SvgInstrument : public Instrument {
public:
    using Instrument::SetSetting;

    /// Construct an instrument attached to a dashboard.
    explicit SvgInstrument(Dashboard* parent)
        : Instrument(parent)
    {
        Init();
    }

    /// Return the stable class identifier.
    static wxString Class() { return "SvgInstrument"; }
    wxString GetClass() const override { return Class(); }

    /// Return the localized display name.
    static wxString DisplayType() { return _("SVG skin"); }
    wxString GetDisplayType() const override { return DisplayType(); }

    wxBitmap Render(double scale) override;
    void ProcessData() override;
    void NotifyNewData(const wxString& fullpath) override;
    void ReadConfig(Json::Value& config) override;
    Json::Value GenerateJSONConfig() override;
    void SetSetting(const wxString& key, const wxString& value) override;
    void SetSetting(const wxString& key, const int& value) override;
    wxString GetPrimarySKKey() const override { return m_keys[0]; }

    /// Apply the current values to the bindings and serialize the SVG as it
    /// will be handed to the rasterizer.
    ///
    /// \return SVG document, empty if no valid skin is loaded
    std::string PrepareSvg();

    /// Folder holding the skins under a data or user directory.
    ///
    /// \param base Directory ending with a path separator
    /// \return The `svg` subfolder, ending with a path separator
    static wxString SkinDir(const wxString& base)
    {
        return base + "svg" + wxFileName::GetPathSeparator();
    }

    /// Names of all the skins available, user ones and bundled ones merged.
    ///
    /// \param data_dir Plugin data directory (bundled skins)
    /// \param user_dir Plugin user directory (imported skins)
    /// \return Sorted, unique file names
    static wxArrayString ListSkins(
        const wxString& data_dir, const wxString& user_dir);

    /// Find the file for a skin name, imported skins win over bundled ones.
    ///
    /// \param name File name or absolute path
    /// \param data_dir Plugin data directory (bundled skins)
    /// \param user_dir Plugin user directory (imported skins)
    /// \return Path to load
    static wxString ResolveSkin(const wxString& name, const wxString& data_dir,
        const wxString& user_dir);

    /// Check that a file is an SVG usable as a skin.
    ///
    /// \param path File to check
    /// \return Error description, empty if the file is fine
    static wxString ValidateSkin(const wxString& path);

    /// Last skin loading error.
    ///
    /// \return Error description, empty if the skin loaded fine
    const wxString& GetError() const { return m_error; }

private:
    /// Kind of manipulation a binding performs.
    enum class op { rotate, text, tick, zone_arc, caption, unit, title };

    /// One `data-dsk-*` binding found in the SVG.
    struct binding {
        /// Element being manipulated
        pugi::xml_node node;
        /// What to do with it
        op operation;
        /// Value slot index (0..DSK_SVG_SLOTS-1)
        size_t slot;
        /// Transform as authored
        std::string original;
    };

    /// One color attribute of the SVG, adjusted for zones and color scheme.
    struct paint {
        /// Element owning the attribute
        pugi::xml_node node;
        /// Attribute name (fill, stroke, stop-color)
        const char* attr;
        /// Color as authored
        std::string original;
        /// Slot whose zones recolor the attribute, if any
        std::optional<size_t> zone_slot;
    };

    /// Cached value and freshness state for one slot.
    struct datum {
        /// Most recently resolved numeric value.
        std::optional<double> value;
        /// Time of the most recent notification.
        std::chrono::system_clock::time_point changed;
    };

    /// Configured Signal K paths, one per slot.
    std::array<wxString, DSK_SVG_SLOTS> m_keys;
    /// Cached slot values.
    std::array<datum, DSK_SVG_SLOTS> m_data;
    /// User labels of the slots (`data-dsk-caption`).
    std::array<wxString, DSK_SVG_SLOTS> m_labels;
    /// User units of the slots (`data-dsk-unit`).
    std::array<wxString, DSK_SVG_SLOTS> m_units;
    /// User unit transformations, anything but none overrides `data-dsk-mul`.
    std::array<transformation, DSK_SVG_SLOTS> m_transforms {};
    /// Names of the supported transformations, for the settings GUI.
    wxArrayString m_supported_transforms;
    /// User formats, indexes to #m_format_strings, 0 keeps `data-dsk-fmt`.
    std::array<int, DSK_SVG_SLOTS> m_formats {};
    /// Names of the formats for the settings GUI, "Skin default" first.
    wxArrayString m_supported_formats;
    /// printf formats matching #m_supported_formats.
    wxArrayString m_format_strings;
    /// The parsed skin.
    pugi::xml_document m_doc;
    /// Bindings collected from #m_doc.
    std::vector<binding> m_bindings;
    /// Color attributes collected from #m_doc.
    std::vector<paint> m_paints;
    /// Zone arcs generated by the last PrepareSvg(), removed by the next one.
    std::vector<pugi::xml_node> m_generated;
    /// Slot whose scale is configurable (`data-dsk-scale`), if any.
    std::optional<size_t> m_scale_slot;
    /// Skin's default scale in display units (`data-dsk-scale-range`).
    std::array<double, 2> m_scale_default { 0, 0 };
    /// Display units per Signal K unit (`data-dsk-scale-mul`).
    double m_scale_mul = 1.0;
    /// Step of the scale settings (`data-dsk-scale-step`).
    double m_scale_step = 1.0;
    /// viewBox of the skin: min x, min y, width, height.
    std::array<double, 4> m_viewbox { 0, 0, 0, 0 };
    /// Skin loading error, empty when the skin is usable.
    wxString m_error;
    /// Configured instrument width in device-independent pixels.
    wxCoord m_instrument_size;
    /// Cached rendered bitmap.
    wxBitmap m_bmp;

    /// Initialize defaults and configuration controls.
    void Init();
    /// Offer only the configuration controls the loaded skin uses, labelled
    /// by its `data-dsk-label-vN` attributes.
    void BuildControls();
    /// Subscribe to every configured slot.
    void SubscribeAll();
    /// Parse a skin file and check the basic requirements.
    ///
    /// \param path File to load
    /// \param doc Document to parse into
    /// \param viewbox Receives the viewBox
    /// \return Error description, empty on success
    static wxString ParseSkin(const wxString& path, pugi::xml_document& doc,
        std::array<double, 4>& viewbox);
    /// Fill the scale settings with the skin's default if they are unset
    /// (minimum equal to maximum).
    void ApplyScaleDefault();
    /// Configured scale, or the skin's default if the configuration is
    /// unusable.
    ///
    /// \return Minimum and maximum in Signal K units
    std::array<double, 2> Scale();
    /// Convert a Signal K value to the units zones and the scale are set in:
    /// display units for the scaled slot, Signal K units otherwise.
    ///
    /// \param slot Slot the value belongs to
    /// \param v Value in Signal K units
    /// \return The value in zone units
    double ZoneUnits(size_t slot, double v) const
    {
        return m_scale_slot == slot ? v * m_scale_mul : v;
    }
    /// Map a value to a rotation angle for an element: `data-dsk-sweep` maps
    /// the instrument scale, `data-dsk-range` a fixed range, otherwise the
    /// value is taken as radians.
    ///
    /// \param node Element with the mapping attributes
    /// \param v Value in Signal K units
    /// \return Angle in degrees, clockwise from up
    double ToAngle(const pugi::xml_node& node, double v);
    /// Rotation center of an element (`data-dsk-pivot`, default viewBox
    /// center).
    ///
    /// \param node The element
    /// \return x and y in viewBox coordinates
    std::vector<double> Pivot(const pugi::xml_node& node) const;
    /// Replace a `data-dsk-zone-arc` template with arcs for the zones.
    ///
    /// \param b The zone arc binding
    void MakeZoneArcs(const binding& b);
    /// Signal K path the skin suggests for a slot (`data-dsk-path-vN`).
    ///
    /// \param slot Slot index
    /// \return The path, empty if the skin doesn't suggest one
    wxString DefaultPath(size_t slot) const;
    /// Load and parse the skin, collecting its bindings.
    ///
    /// \param file Absolute path or a skin name, see ResolveSkin()
    void LoadSvg(const wxString& file);
    /// Return a slot value, or no value when missing/timed out.
    ///
    /// \param slot Slot index
    /// \return The value
    std::optional<double> Current(size_t slot) const;
    /// Draw the `<text>` elements of #m_doc onto a rasterized skin.
    ///
    /// \param bmp Bitmap to draw on
    void DrawTexts(wxBitmap& bmp);
};

PLUGIN_END_NAMESPACE

#endif
