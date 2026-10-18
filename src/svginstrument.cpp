/******************************************************************************
 * DashboardSK SVG skin instrument
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#include "svginstrument.h"
#include "dashboard.h"
#include "simplegaugeinstrument.h"

#include <cmath>
#include <cstdio>
#include <regex>
#include <sstream>

// wxBitmapBundle (and SVG support) needs wx 3.1.6+, Android is still older
#if wxCHECK_VERSION(3, 1, 6)
#include <wx/bmpbndl.h>
#endif
#include <wx/dir.h>
#include <wx/filename.h>

PLUGIN_BEGIN_NAMESPACE

namespace {

/// Setting keys of the value slots, indexed by slot
const char* const slot_keys[DSK_SVG_SLOTS] { DSK_SVG_KEY_0, DSK_SVG_KEY_1,
    DSK_SVG_KEY_2, DSK_SVG_KEY_3 };
/// Setting keys of the slot labels, indexed by slot
const char* const label_keys[DSK_SVG_SLOTS] { DSK_SVG_LABEL_0, DSK_SVG_LABEL_1,
    DSK_SVG_LABEL_2, DSK_SVG_LABEL_3 };
/// Setting keys of the slot units, indexed by slot
const char* const unit_keys[DSK_SVG_SLOTS] { DSK_SVG_UNIT_0, DSK_SVG_UNIT_1,
    DSK_SVG_UNIT_2, DSK_SVG_UNIT_3 };
/// Setting keys of the slot transformations, indexed by slot
const char* const transform_keys[DSK_SVG_SLOTS] { DSK_SVG_TRANSFORM_0,
    DSK_SVG_TRANSFORM_1, DSK_SVG_TRANSFORM_2, DSK_SVG_TRANSFORM_3 };
/// Setting keys of the slot formats, indexed by slot
const char* const format_keys[DSK_SVG_SLOTS] { DSK_SVG_FORMAT_0,
    DSK_SVG_FORMAT_1, DSK_SVG_FORMAT_2, DSK_SVG_FORMAT_3 };

/// Parse up to @p n whitespace/comma separated numbers from an attribute.
///
/// \param s Attribute text
/// \param n Number of values wanted
/// \return The values, shorter than @p n if the text had fewer
std::vector<double> Numbers(const char* s, size_t n)
{
    std::vector<double> out;
    std::string str(s);
    for (char& c : str) {
        if (c == ',') {
            c = ' ';
        }
    }
    std::istringstream in(str);
    double d;
    while (out.size() < n && in >> d) {
        out.push_back(d);
    }
    return out;
}

/// Parse the slot reference "v0".."v3".
///
/// \param s Attribute text
/// \return Slot index or no value if invalid
std::optional<size_t> Slot(const char* s)
{
    if (s[0] == 'v' && s[1] >= '0' && s[1] < '0' + DSK_SVG_SLOTS && !s[2]) {
        return static_cast<size_t>(s[1] - '0');
    }
    return std::nullopt;
}

/// Check that a printf format coming from the (untrusted) SVG file consumes
/// exactly one double.
///
/// \param fmt The format
/// \return true if safe to use with a double argument
bool IsSafeFormat(const std::string& fmt)
{
    static const std::regex re(
        "^([^%]|%%)*%[-+ 0#]?[0-9]{0,2}(\\.[0-9]{1,2})?[fFeEgG]([^%]|%%)*$");
    return std::regex_match(fmt, re);
}

/// Concatenated character data of a node and its descendants (`<tspan>`).
///
/// \param node The node
/// \return The text
std::string TextOf(const pugi::xml_node& node)
{
    std::string s;
    for (pugi::xml_node n : node.children()) {
        s += n.type() == pugi::node_pcdata ? n.value() : TextOf(n);
    }
    return s;
}

/// Replace the content of an element with plain text.
///
/// \param node The element
/// \param text UTF-8 text
void SetText(pugi::xml_node& node, const char* text)
{
    while (node.first_child()) {
        node.remove_child(node.first_child());
    }
    node.append_child(pugi::node_pcdata).set_value(text);
}

}

void SvgInstrument::Init()
{
#define X(a, b) m_supported_transforms.Add(b);
    DSK_UNIT_TRANSFORMATIONS
#undef X
    m_supported_formats.Add(_("Skin default"));
    m_format_strings.Add(wxEmptyString);
#define X(a, b, c)                                                             \
    m_supported_formats.Add(b);                                                \
    m_format_strings.Add(c);
    DSK_VALUE_FORMATS
#undef X
    m_name = _("New SVG skin");
    m_title = DUMMY_TITLE;
    m_instrument_size = 200;
#define X(key, value, label, control, parameters, json, getter)                \
    SetSetting(key, value);
    DSK_SVG_SETTINGS
#undef X
    BuildControls();
}

void SvgInstrument::BuildControls()
{
    // Without a usable skin we can't tell, so offer everything
    const bool all = !m_doc.child("svg");
    std::array<bool, DSK_SVG_SLOTS> used {};
    std::array<bool, DSK_SVG_SLOTS> texts {};
    std::array<bool, DSK_SVG_SLOTS> labels {};
    std::array<bool, DSK_SVG_SLOTS> units {};
    bool zones = false;
    for (const binding& b : m_bindings) {
        used[b.slot] |= b.operation != op::title;
        zones |= b.operation == op::zone_arc;
        texts[b.slot] |= b.operation == op::text;
        labels[b.slot] |= b.operation == op::caption;
        units[b.slot] |= b.operation == op::unit;
    }
    for (const paint& p : m_paints) {
        if (p.zone_slot) {
            used[*p.zone_slot] = true;
            zones = true;
        }
    }
    const pugi::xml_node root = m_doc.child("svg");
    m_config_controls.clear();
#define X(key, value, label, control, parameters, json, getter)                \
    m_config_controls.emplace_back(                                            \
        config_control({ key, label, dskConfigCtrl::control, parameters }));
    DSK_SVG_SETTINGS
#undef X
    for (auto it = m_config_controls.begin(); it != m_config_controls.end();) {
        bool keep = all || !it->key.IsSameAs(DSK_SETTING_ZONES) || zones;
        if (it->key.IsSameAs(DSK_SVG_SCALE_MIN)
            || it->key.IsSameAs(DSK_SVG_SCALE_MAX)) {
            keep = m_scale_slot.has_value();
            it->control_settings
                = "-1000000;1000000;" + wxString::FromCDouble(m_scale_step);
        }
        for (size_t i = 0; i < DSK_SVG_SLOTS; ++i) {
            const std::string attr = "data-dsk-label-v" + std::to_string(i);
            const wxString name
                = wxString::FromUTF8(root.attribute(attr.c_str()).as_string());
            if (it->key.IsSameAs(slot_keys[i])) {
                keep = all || used[i];
                if (!name.IsEmpty()) {
                    it->description = name;
                }
            } else if (it->key.IsSameAs(label_keys[i])) {
                keep = all || labels[i];
                if (!name.IsEmpty()) {
                    it->description = wxString::Format(_("%s label"), name);
                }
            } else if (it->key.IsSameAs(unit_keys[i])) {
                keep = all || units[i];
                if (!name.IsEmpty()) {
                    it->description = wxString::Format(_("%s unit"), name);
                }
            } else if (it->key.IsSameAs(transform_keys[i])) {
                keep = all || texts[i];
                if (!name.IsEmpty()) {
                    it->description
                        = wxString::Format(_("%s transformation"), name);
                }
            } else if (it->key.IsSameAs(format_keys[i])) {
                keep = all || texts[i];
                if (!name.IsEmpty()) {
                    it->description = wxString::Format(_("%s format"), name);
                }
            }
        }
        it = keep ? it + 1 : m_config_controls.erase(it);
    }
}

void SvgInstrument::SubscribeAll()
{
    if (!m_parent_dashboard) {
        return;
    }
    m_parent_dashboard->Unsubscribe(this);
    for (const auto& key : m_keys) {
        if (!key.IsEmpty()) {
            m_parent_dashboard->Subscribe(key, this);
        }
    }
}

void SvgInstrument::ApplyScaleDefault()
{
    double min = 0, max = 0;
    GetStringSetting(DSK_SVG_SCALE_MIN).ToCDouble(&min);
    GetStringSetting(DSK_SVG_SCALE_MAX).ToCDouble(&max);
    if (m_scale_slot && min == max) {
        Instrument::SetSetting(
            DSK_SVG_SCALE_MIN, wxString::FromCDouble(m_scale_default[0]));
        Instrument::SetSetting(
            DSK_SVG_SCALE_MAX, wxString::FromCDouble(m_scale_default[1]));
    }
    m_needs_redraw = true;
}

std::array<double, 2> SvgInstrument::Scale()
{
    double min = 0, max = 0;
    if (!GetStringSetting(DSK_SVG_SCALE_MIN).ToCDouble(&min)
        || !GetStringSetting(DSK_SVG_SCALE_MAX).ToCDouble(&max) || min >= max) {
        min = m_scale_default[0];
        max = m_scale_default[1];
    }
    return { min / m_scale_mul, max / m_scale_mul };
}

double SvgInstrument::ToAngle(const pugi::xml_node& node, double v)
{
    std::vector<double> r
        = Numbers(node.attribute("data-dsk-range").as_string(), 4);
    const std::vector<double> sweep
        = Numbers(node.attribute("data-dsk-sweep").as_string(), 2);
    if (sweep.size() == 2 && m_scale_slot) {
        const std::array<double, 2> s = Scale();
        r = { s[0], s[1], sweep[0], sweep[1] };
    }
    if (r.size() == 4 && r[1] != r[0]) {
        const double t = wxClip((v - r[0]) / (r[1] - r[0]), 0.0, 1.0);
        return r[2] + t * (r[3] - r[2]);
    }
    // Signal K angles are radians
    return rad2deg(v);
}

std::vector<double> SvgInstrument::Pivot(const pugi::xml_node& node) const
{
    std::vector<double> p
        = Numbers(node.attribute("data-dsk-pivot").as_string(), 2);
    if (p.size() != 2) {
        p = { m_viewbox[0] + m_viewbox[2] / 2,
            m_viewbox[1] + m_viewbox[3] / 2 };
    }
    return p;
}

void SvgInstrument::MakeZoneArcs(const binding& b)
{
    const std::vector<double> p = Pivot(b.node);
    const double r = b.node.attribute("data-dsk-radius")
                         .as_double(wxMin(m_viewbox[2], m_viewbox[3]) / 2);
    const std::string width = b.node.attribute("stroke-width").as_string("1");
    auto point = [&p, r](double deg) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.2f %.2f", p[0] + r * sin(deg2rad(deg)),
            p[1] - r * cos(deg2rad(deg)));
        return std::string(buf);
    };
    pugi::xml_node after = b.node;
    for (Zone z : m_zones) {
        wxColor c;
        switch (z.GetState()) {
        case Zone::state::normal:
            c = DSK_SGI_COLOR_NORMAL;
            break;
        case Zone::state::alert:
            c = DSK_SGI_COLOR_ALERT;
            break;
        case Zone::state::warn:
            c = DSK_SGI_COLOR_WARN;
            break;
        case Zone::state::alarm:
            c = DSK_SGI_COLOR_ALARM;
            break;
        case Zone::state::emergency:
            c = DSK_SGI_COLOR_EMERG;
            break;
        default:
            continue; // Nominal zones aren't highlighted
        }
        // Zone limits are in zone units, angles want Signal K units
        const double k = ZoneUnits(b.slot, 1.0);
        const double a0 = ToAngle(b.node, z.GetLowerLimit() / k);
        const double a1 = ToAngle(b.node, z.GetUpperLimit() / k);
        if (std::fabs(a1 - a0) < 0.01) {
            continue; // Outside the scale
        }
        char flags[32];
        snprintf(flags, sizeof(flags), " 0 %d %d ",
            std::fabs(a1 - a0) > 180 ? 1 : 0, a1 > a0 ? 1 : 0);
        char radius[32];
        snprintf(radius, sizeof(radius), "%g %g", r, r);
        const std::string d
            = "M" + point(a0) + "A" + radius + flags + point(a1);
        pugi::xml_node arc = b.node.parent().insert_child_after("path", after);
        arc.append_attribute("d") = d.c_str();
        arc.append_attribute("fill") = "none";
        arc.append_attribute("stroke") = DSK_TO_STDSTRING_UTF8(
            GetDimedColor(c).GetAsString(wxC2S_HTML_SYNTAX))
                                             .c_str();
        arc.append_attribute("stroke-width") = width.c_str();
        m_generated.push_back(arc);
        after = arc;
    }
}

wxString SvgInstrument::DefaultPath(size_t slot) const
{
    const std::string attr = "data-dsk-path-v" + std::to_string(slot);
    return wxString::FromUTF8(
        m_doc.child("svg").attribute(attr.c_str()).as_string());
}

void SvgInstrument::SetSetting(const wxString& key, const wxString& value)
{
    for (size_t i = 0; i < m_keys.size(); ++i) {
        if (key.IsSameAs(slot_keys[i])) {
            // An empty slot takes the path the skin suggests, if any
            m_keys[i] = value.IsEmpty() ? DefaultPath(i) : value;
            Instrument::SetSetting(key, m_keys[i]);
            SubscribeAll();
            return;
        }
        if (key.IsSameAs(label_keys[i])) {
            m_labels[i] = value;
        } else if (key.IsSameAs(unit_keys[i])) {
            m_units[i] = value;
        } else if (key.IsSameAs(transform_keys[i])
            || key.IsSameAs(format_keys[i])) {
            SetSetting(key, IntFromString(value));
            return;
        } else {
            continue;
        }
        Instrument::SetSetting(key, value);
        return;
    }
    const wxString zones
        = m_doc.child("svg").attribute("data-dsk-zones").as_string();
    if (key.IsSameAs(DSK_SETTING_ZONES) && value.IsEmpty()
        && !zones.IsEmpty()) {
        // No zones configured, use the ones the skin suggests
        Instrument::SetSetting(key, zones);
        return;
    }
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SVG_SCALE_MIN) || key.IsSameAs(DSK_SVG_SCALE_MAX)) {
        ApplyScaleDefault();
    } else if (key.IsSameAs(DSK_SVG_FILE)) {
        LoadSvg(value);
        BuildControls();
        ApplyScaleDefault();
        if (m_zones.empty()) {
            SetSetting(wxString(DSK_SETTING_ZONES), wxString());
        }
        for (size_t i = 0; i < m_keys.size(); ++i) {
            if (m_keys[i].IsEmpty() && !DefaultPath(i).IsEmpty()) {
                SetSetting(slot_keys[i], wxString());
            }
        }
    } else if (key.IsSameAs(DSK_SETTING_INSTR_SIZE)) {
        SetSetting(key, IntFromString(value));
    }
}

void SvgInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_SETTING_INSTR_SIZE)) {
        m_instrument_size = value;
    }
    for (size_t i = 0; i < DSK_SVG_SLOTS; ++i) {
        if (key.IsSameAs(transform_keys[i])) {
            m_transforms[i] = static_cast<transformation>(value);
        } else if (key.IsSameAs(format_keys[i])) {
            m_formats[i] = value;
        }
    }
    m_needs_redraw = true;
}

wxString SvgInstrument::ParseSkin(const wxString& path, pugi::xml_document& doc,
    std::array<double, 4>& viewbox)
{
    const pugi::xml_parse_result res
        = doc.load_file(DSK_TO_STDSTRING_UTF8(path).c_str());
    if (!res) {
        return wxString::Format(_("Can't load %s: %s"), path,
            wxString::FromUTF8(res.description()));
    }
    const pugi::xml_node root = doc.child("svg");
    if (!root) {
        return wxString::Format(_("%s is not an SVG image"), path);
    }
    const std::vector<double> vb
        = Numbers(root.attribute("viewBox").as_string(), 4);
    if (vb.size() != 4 || vb[2] <= 0 || vb[3] <= 0) {
        return wxString::Format(_("%s has no valid viewBox"), path);
    }
    std::copy(vb.begin(), vb.end(), viewbox.begin());
    return wxEmptyString;
}

wxString SvgInstrument::ValidateSkin(const wxString& path)
{
    pugi::xml_document doc;
    std::array<double, 4> viewbox;
    return ParseSkin(path, doc, viewbox);
}

wxString SvgInstrument::ResolveSkin(
    const wxString& name, const wxString& data_dir, const wxString& user_dir)
{
    if (wxFileName(name).IsAbsolute()) {
        return name;
    }
    const wxString user = SkinDir(user_dir) + name;
    return !user_dir.IsEmpty() && wxFileExists(user) ? user
                                                     : SkinDir(data_dir) + name;
}

wxArrayString SvgInstrument::ListSkins(
    const wxString& data_dir, const wxString& user_dir)
{
    wxArrayString names;
    for (const wxString& base : { data_dir, user_dir }) {
        if (base.IsEmpty() || !wxDirExists(SkinDir(base))) {
            continue;
        }
        wxArrayString files;
        wxDir::GetAllFiles(SkinDir(base), &files, "*.svg", wxDIR_FILES);
        for (const wxString& f : files) {
            const wxString name = wxFileName(f).GetFullName();
            if (names.Index(name) == wxNOT_FOUND) {
                names.Add(name);
            }
        }
    }
    names.Sort();
    return names;
}

void SvgInstrument::LoadSvg(const wxString& file)
{
    m_bindings.clear();
    m_paints.clear();
    m_generated.clear();
    m_scale_slot.reset();
    m_doc.reset();
    m_needs_redraw = true;
    const wxString path = m_parent_dashboard
        ? ResolveSkin(file, m_parent_dashboard->GetDataDir(),
              m_parent_dashboard->GetUserDir())
        : file;
    m_error = ParseSkin(path, m_doc, m_viewbox);
    if (!m_error.IsEmpty()) {
        m_doc.reset();
        return;
    }
    const pugi::xml_node root = m_doc.child("svg");
    m_scale_slot = Slot(root.attribute("data-dsk-scale").as_string());
    const std::vector<double> sr
        = Numbers(root.attribute("data-dsk-scale-range").as_string(), 2);
    m_scale_mul = root.attribute("data-dsk-scale-mul").as_double(1.0);
    if (sr.size() != 2 || sr[0] >= sr[1] || m_scale_mul == 0) {
        m_scale_slot.reset(); // An unusable scale is no scale
    } else {
        m_scale_default = { sr[0], sr[1] };
        // Default step: a power of ten around 1/100 of the range
        m_scale_step = root.attribute("data-dsk-scale-step")
                           .as_double(std::pow(10.0,
                               std::floor(std::log10(sr[1] - sr[0])) - 2));
        if (m_scale_step <= 0) {
            m_scale_step = 1.0;
        }
    }
    const std::pair<const char*, op> ops[] {
        { "data-dsk-rotate", op::rotate },
        { "data-dsk-text", op::text },
        { "data-dsk-zone-arc", op::zone_arc },
        { "data-dsk-caption", op::caption },
        { "data-dsk-unit", op::unit },
    };
    // Paint attribute, zone binding attribute adjusting it
    const std::pair<const char*, const char*> paints[] {
        { "fill", "data-dsk-zone-fill" },
        { "stroke", "data-dsk-zone-stroke" },
        { "stop-color", "" },
    };
    for (pugi::xpath_node xn : root.select_nodes("//*")) {
        pugi::xml_node node = xn.node();
        for (const auto& [attr, operation] : ops) {
            const auto slot = Slot(node.attribute(attr).as_string());
            if (!slot) {
                continue;
            }
            m_bindings.push_back({ node, operation, *slot,
                node.attribute("transform").as_string() });
        }
        if (node.attribute("data-dsk-tick") && m_scale_slot) {
            m_bindings.push_back({ node, op::tick, *m_scale_slot, "" });
        }
        if (node.attribute("data-dsk-title")) {
            m_bindings.push_back({ node, op::title, 0, "" });
        }
        for (const auto& [attr, zone] : paints) {
            const auto slot = Slot(node.attribute(zone).as_string());
            if (node.attribute(attr) || slot) {
                // A zone bound element without the attribute defaults to black
                m_paints.push_back({ node, attr,
                    node.attribute(attr).as_string(slot ? "black" : ""),
                    slot });
            }
        }
    }
    m_error.Clear();
}

void SvgInstrument::NotifyNewData(const wxString& fullpath)
{
    const auto now = std::chrono::system_clock::now();
    for (size_t i = 0; i < m_keys.size(); ++i) {
        const int source = m_keys[i].Find(".SRC:");
        const wxString base
            = source == wxNOT_FOUND ? m_keys[i] : m_keys[i].Left(source);
        if (base.IsSameAs(fullpath)) {
            m_data[i].changed = now;
        }
    }
    m_new_data = true;
    m_needs_redraw = true;
}

void SvgInstrument::ProcessData()
{
    if (m_new_data) {
        m_new_data = false;
        m_needs_redraw = true;
        for (size_t i = 0; i < m_keys.size(); ++i) {
            m_data[i].value.reset();
            const Json::Value* value = GetSKDataResolved(m_keys[i]);
            if (value && value->isMember("value")
                && (*value)["value"].isNumeric()) {
                m_data[i].value = (*value)["value"].asDouble();
            }
        }
    }
    for (size_t i = 0; i < m_data.size(); ++i) {
        if (m_data[i].value && !Current(i)) {
            // Just timed out, redraw once showing the stale state
            m_data[i].value.reset();
            m_needs_redraw = true;
        }
    }
}

std::optional<double> SvgInstrument::Current(size_t slot) const
{
    const datum& d = m_data[slot];
    if (d.value
        && (m_allowed_age_sec <= 0
            || std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now() - d.changed)
                    .count()
                <= m_allowed_age_sec)) {
        return d.value;
    }
    return std::nullopt;
}

std::string SvgInstrument::PrepareSvg()
{
    if (!m_error.IsEmpty() || !m_doc.child("svg")) {
        return std::string();
    }
    for (pugi::xml_node n : m_generated) {
        n.parent().remove_child(n);
    }
    m_generated.clear();
    for (binding& b : m_bindings) {
        const std::optional<double> v = Current(b.slot);
        switch (b.operation) {
        case op::zone_arc:
            MakeZoneArcs(b);
            break;
        case op::tick: {
            // Label at a fraction of the scale, in display units
            const std::array<double, 2> s = Scale();
            const double val
                = (s[0]
                      + b.node.attribute("data-dsk-tick").as_double()
                          * (s[1] - s[0]))
                * m_scale_mul * b.node.attribute("data-dsk-mul").as_double(1.0);
            std::string fmt
                = b.node.attribute("data-dsk-fmt").as_string("%.0f");
            if (!IsSafeFormat(fmt)) {
                fmt = "%.0f";
            }
            char buf[64];
            snprintf(buf, sizeof(buf), fmt.c_str(), val);
            SetText(b.node, buf);
            break;
        }
        case op::rotate: {
            if (!v) {
                b.node.remove_attribute("transform");
                if (!b.original.empty()) {
                    b.node.append_attribute("transform") = b.original.c_str();
                }
                break;
            }
            const double deg = ToAngle(b.node, *v);
            const std::vector<double> p = Pivot(b.node);
            char buf[128];
            snprintf(buf, sizeof(buf), "rotate(%.2f %g %g) ", deg, p[0], p[1]);
            b.node.remove_attribute("transform");
            b.node.append_attribute("transform") = (buf + b.original).c_str();
            break;
        }
        case op::text: {
            std::string s = "---";
            if (v) {
                // User settings win over the skin
                const double val = m_transforms[b.slot] == transformation::none
                    ? *v * b.node.attribute("data-dsk-mul").as_double(1.0)
                    : Transform(*v, m_transforms[b.slot]);
                const int f = m_formats[b.slot];
                char buf[64];
                if (f > 0 && f < static_cast<int>(m_format_strings.size())) {
                    // Like SimpleNumberInstrument: the padding applies to the
                    // digits, the sign goes in front unless dropped by ABS
                    snprintf(buf, sizeof(buf),
                        m_format_strings[f].mb_str().data(), std::abs(val));
                    s = val < 0 && !m_supported_formats[f].StartsWith("ABS")
                        ? std::string("-") + buf
                        : std::string(buf);
                } else {
                    std::string fmt
                        = b.node.attribute("data-dsk-fmt").as_string("%.0f");
                    if (!IsSafeFormat(fmt)) {
                        fmt = "%.0f";
                    }
                    snprintf(buf, sizeof(buf), fmt.c_str(), val);
                    s = buf;
                }
            }
            SetText(b.node, s.c_str());
            break;
        }
        // ponytail: an empty text just isn't drawn, the layout doesn't reflow
        case op::caption:
            SetText(b.node, DSK_TO_STDSTRING_UTF8(m_labels[b.slot]).c_str());
            break;
        case op::unit:
            SetText(b.node, DSK_TO_STDSTRING_UTF8(m_units[b.slot]).c_str());
            break;
        case op::title:
            SetText(b.node,
                m_title == DUMMY_TITLE
                    ? ""
                    : DSK_TO_STDSTRING_UTF8(m_title).c_str());
            break;
        }
    }
    // Every color goes through the zones (if bound) and the day/dusk/night
    // luminosity adjustment like in all the other instruments.
    for (paint& p : m_paints) {
        wxColor c(wxString::FromUTF8(p.original.c_str()));
        std::string color = p.original;
        // Unparsable values ("none", gradient references) stay as authored
        if (c.IsOk()) {
            if (p.zone_slot) {
                if (const std::optional<double> v = Current(*p.zone_slot)) {
                    c = AdjustColorForZone(ZoneUnits(*p.zone_slot, *v), c, c,
                        DSK_SGI_COLOR_ALERT, DSK_SGI_COLOR_WARN,
                        DSK_SGI_COLOR_ALARM, DSK_SGI_COLOR_EMERG);
                }
            }
            color = DSK_TO_STDSTRING_UTF8(
                GetDimedColor(c).GetAsString(wxC2S_HTML_SYNTAX));
        }
        p.node.remove_attribute(p.attr);
        p.node.append_attribute(p.attr) = color.c_str();
    }
    std::ostringstream out;
    m_doc.save(out, "", pugi::format_raw);
    return out.str();
}

void SvgInstrument::DrawTexts(wxBitmap& bmp)
{
    const double sx = bmp.GetWidth() / m_viewbox[2];
    const double sy = bmp.GetHeight() / m_viewbox[3];
    wxMemoryDC mdc(bmp);
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC dc(mdc);
#else
    wxMemoryDC& dc(mdc);
#endif
    // ponytail: x/y/font-size/fill/anchor/weight attributes only, no style=,
    // no transforms on ancestors; extend if real skins need more.
    for (pugi::xpath_node xn : m_doc.select_nodes("//*[local-name()='text']")) {
        const pugi::xml_node t = xn.node();
        const wxString text = wxString::FromUTF8(TextOf(t).c_str());
        const double px = t.attribute("font-size").as_double(16) * sy;
        if (text.IsEmpty() || px < 1) {
            continue;
        }
        wxFont font(
            wxFontInfo(wxSize(0, wxRound(px))).Family(wxFONTFAMILY_SWISS));
        if (wxString(t.attribute("font-weight").as_string()).IsSameAs("bold")
            || t.attribute("font-weight").as_int() >= 600) {
            font.SetWeight(wxFONTWEIGHT_BOLD);
        }
        dc.SetFont(font);
        const char* fill = t.attribute("fill").as_string("black");
        // Already dimmed by PrepareSvg
        dc.SetTextForeground(wxColor(wxString::FromUTF8(fill)));
        wxCoord w, h, descent;
        dc.GetTextExtent(text, &w, &h, &descent);
        double x = (Numbers(t.attribute("x").as_string("0"), 1).front()
                       - m_viewbox[0])
            * sx;
        const double y = (Numbers(t.attribute("y").as_string("0"), 1).front()
                             - m_viewbox[1])
            * sy;
        const std::string anchor = t.attribute("text-anchor").as_string();
        if (anchor == "middle") {
            x -= w / 2.0;
        } else if (anchor == "end") {
            x -= w;
        }
        // SVG y is the baseline, wxDC wants the top of the text box
        dc.DrawText(text, wxRound(x), wxRound(y - (h - descent)));
    }
}

wxBitmap SvgInstrument::Render(double scale)
{
    ProcessData();
    if (!m_needs_redraw && m_bmp.IsOk()) {
        return m_bmp;
    }
    m_needs_redraw = false;

    const wxCoord w = m_instrument_size * scale;
#ifdef wxHAS_SVG
    std::string svg = PrepareSvg();
    if (!svg.empty()) {
        const wxSize size(w, wxRound(w * m_viewbox[3] / m_viewbox[2]));
        // The char* overload parses in place, svg is ours to scribble on.
        m_bmp = wxBitmapBundle::FromSVG(svg.data(), size).GetBitmap(size);
        if (m_bmp.IsOk()) {
            DrawTexts(m_bmp);
            return m_bmp;
        }
        m_error = _("The SVG can't be rendered");
    }
#else
    m_error = _("SVG rendering is not supported on this platform");
#endif
    // Placeholder telling the user what's wrong instead of drawing nothing
#if defined(__WXGTK__) || defined(__WXQT__)
    m_bmp = wxBitmap(w, w / 2, 32);
#else
    m_bmp = wxBitmap(w, w / 2);
    m_bmp.UseAlpha();
#endif
    wxMemoryDC dc(m_bmp);
    dc.SetBackground(wxBrush(GetDimedColor(*wxWHITE)));
    dc.Clear();
    dc.SetTextForeground(GetDimedColor(*wxRED));
    dc.DrawLabel(m_error, wxRect(0, 0, w, w / 2),
        wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
    return m_bmp;
}

void SvgInstrument::ReadConfig(Json::Value& config)
{
    Instrument::ReadConfig(config);
#define X(key, value, label, control, parameters, json, getter)                \
    if (config.isMember(key)) {                                                \
        SetSetting(key, fromJsonVal(config[key].json()));                      \
    }
    DSK_SVG_SETTINGS
#undef X
}

Json::Value SvgInstrument::GenerateJSONConfig()
{
    // Zones are stored by the parent
    Json::Value config = Instrument::GenerateJSONConfig();
#define X(key, value, label, control, parameters, json, getter)                \
    if (!wxString(key).IsSameAs(DSK_SETTING_ZONES)) {                          \
        config[key] = toJson(getter(key));                                     \
    }
    DSK_SVG_SETTINGS
#undef X
    return config;
}

PLUGIN_END_NAMESPACE
