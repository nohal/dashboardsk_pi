/******************************************************************************
 * DashboardSK composite wind instrument
 * Copyright (C) 2026 Pavel Kalian
 * License: GPLv3+
 *****************************************************************************/

#include "compositewindinstrument.h"
#include "dashboard.h"

#include <cmath>

PLUGIN_BEGIN_NAMESPACE

namespace {
constexpr double pi = 3.14159265358979323846;
/// Fraction of the dial diameter the laylines extend beyond the rim. The
/// rendered bitmap is enlarged by this margin on every side to leave room.
constexpr double layline_protrusion = 0.08;

wxPoint Point(double cx, double cy, double radius, double bearing)
{
    const double angle = (bearing - 90.0) * pi / 180.0;
    return wxPoint(
        cx + radius * std::cos(angle), cy + radius * std::sin(angle));
}

void DrawTriangle(wxDC& dc, double cx, double cy, double bearing,
    double tip_radius, double base_radius, double half_width,
    const wxColor& color)
{
    const wxPoint tip = Point(cx, cy, tip_radius, bearing);
    const wxPoint base = Point(cx, cy, base_radius, bearing);
    const double angle = bearing * pi / 180.0;
    wxPoint points[] { tip,
        wxPoint(base.x - half_width * std::cos(angle),
            base.y - half_width * std::sin(angle)),
        wxPoint(base.x + half_width * std::cos(angle),
            base.y + half_width * std::sin(angle)) };
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(color));
    dc.DrawPolygon(3, points);
}

void DrawCenteredText(wxDC& dc, const wxString& text, wxCoord x, wxCoord y)
{
    const wxSize size = dc.GetTextExtent(text);
    dc.DrawText(text, x - size.x / 2, y - size.y / 2);
}

/// Draw a rounded, semi-transparent backing rectangle sized to cover the two
/// stacked, horizontally centered label lines of one readout column, so the
/// text drawn on top stays legible over a busy chart.
/// @param dc Device context to draw on (its current font must match the text)
/// @param top Upper label line, used to measure the panel width and top edge
/// @param bottom Lower label line, used to measure width and bottom edge
/// @param cx Horizontal center shared by both label lines
/// @param y_top Vertical center of the upper label line
/// @param y_bottom Vertical center of the lower label line
/// @param bg Fill color of the panel (alpha channel honored under wxGCDC)
void DrawLabelPanel(wxDC& dc, const wxString& top, const wxString& bottom,
    wxCoord cx, wxCoord y_top, wxCoord y_bottom, const wxColor& bg)
{
    const wxSize ts = dc.GetTextExtent(top);
    const wxSize bs = dc.GetTextExtent(bottom);
    const wxCoord w = wxMax(ts.x, bs.x);
    const wxCoord pad = ts.y / 3; // padding scales with text height
    const wxCoord left = cx - w / 2 - pad;
    const wxCoord top_y = y_top - ts.y / 2 - pad;
    const wxCoord height = (y_bottom + bs.y / 2 + pad) - top_y;
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(bg));
    dc.DrawRoundedRectangle(left, top_y, w + 2 * pad, height, pad);
}

}

void CompositeWindInstrument::Init()
{
    m_name = _("New Composite Wind");
    m_title = DUMMY_TITLE;
    m_orientation = orientation::north_up;
    m_beat_angle = 45;
    m_gybe_angle = 150;
    m_show_laylines = true;
    m_instrument_size = 200;
    const auto now = std::chrono::system_clock::now();
    for (auto& datum : m_data) {
        datum.changed = now;
    }
#define X(key, value, label, control, parameters, json, getter)                \
    SetSetting(key, value);
    DSK_CWI_SETTINGS
#undef X
#define X(key, value, label, control, parameters, json, getter)                \
    m_config_controls.emplace_back(                                            \
        config_control({ key, label, dskConfigCtrl::control, parameters }));
    DSK_CWI_SETTINGS
#undef X
}

wxString CompositeWindInstrument::BasePath(const wxString& path)
{
    const int source = path.Find(".SRC:");
    return source == wxNOT_FOUND ? path : path.Left(source);
}

double CompositeWindInstrument::Normalize(double degrees)
{
    degrees = std::fmod(degrees, 360.0);
    return degrees < 0.0 ? degrees + 360.0 : degrees;
}

double CompositeWindInstrument::GroundTrack(
    double water_bearing, double stw, double set_bearing, double drift)
{
    const double x = stw * std::sin(deg2rad(water_bearing))
        + drift * std::sin(deg2rad(set_bearing));
    const double y = stw * std::cos(deg2rad(water_bearing))
        + drift * std::cos(deg2rad(set_bearing));
    return Normalize(rad2deg(std::atan2(x, y)));
}

void CompositeWindInstrument::SubscribeAll()
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

void CompositeWindInstrument::SetSetting(
    const wxString& key, const wxString& value)
{
    Instrument::SetSetting(key, value);
    // Order must match the input enum so names[i] maps to m_keys[i].
    const wxString names[] { DSK_CWI_AWA_KEY, DSK_CWI_AWS_KEY, DSK_CWI_TWA_KEY,
        DSK_CWI_TWS_KEY, DSK_CWI_HEADING_KEY, DSK_CWI_COG_KEY, DSK_CWI_BEAT_KEY,
        DSK_CWI_GYBE_KEY, DSK_CWI_STW_KEY, DSK_CWI_CURRENT_SET_KEY,
        DSK_CWI_CURRENT_DRIFT_KEY };
    for (size_t i = 0; i < m_keys.size(); ++i) {
        if (key.IsSameAs(names[i])) {
            m_keys[i] = value;
            SubscribeAll();
            return;
        }
    }
    if (key.IsSameAs(DSK_CWI_ORIENTATION)
        || key.IsSameAs(DSK_SETTING_INSTR_SIZE)
        || key.IsSameAs(DSK_CWI_SHOW_LAYLINES)
        || key.IsSameAs(DSK_CWI_BEAT_ANGLE) || key.IsSameAs(DSK_CWI_GYBE_ANGLE)
        || key.IsSameAs(DSK_CWI_RING_OPACITY)
        || key.IsSameAs(DSK_CWI_LAYLINE_REF)) {
        SetSetting(key, IntFromString(value));
    }
}

void CompositeWindInstrument::SetSetting(const wxString& key, const int& value)
{
    Instrument::SetSetting(key, value);
    if (key.IsSameAs(DSK_CWI_ORIENTATION)) {
        m_orientation = static_cast<orientation>(value);
    } else if (key.IsSameAs(DSK_SETTING_INSTR_SIZE)) {
        m_instrument_size = value;
    } else if (key.IsSameAs(DSK_CWI_SHOW_LAYLINES)) {
        m_show_laylines = value != 0;
    } else if (key.IsSameAs(DSK_CWI_BEAT_ANGLE)) {
        m_beat_angle = value;
    } else if (key.IsSameAs(DSK_CWI_GYBE_ANGLE)) {
        m_gybe_angle = value;
    }
    m_needs_redraw = true;
}

void CompositeWindInstrument::NotifyNewData(const wxString& fullpath)
{
    const auto now = std::chrono::system_clock::now();
    for (size_t i = 0; i < m_keys.size(); ++i) {
        if (BasePath(m_keys[i]).IsSameAs(fullpath)) {
            m_data[i].changed = now;
            m_data[i].received = true;
        }
    }
    m_new_data = true;
    m_needs_redraw = true;
}

void CompositeWindInstrument::RefreshData()
{
    for (size_t i = 0; i < m_keys.size(); ++i) {
        m_data[i].value.reset();
        const Json::Value* value = GetSKDataResolved(m_keys[i]);
        if (value && value->isMember("value")
            && (*value)["value"].isNumeric()) {
            m_data[i].value = (*value)["value"].asDouble();
            if (!m_data[i].received) {
                m_data[i].changed = std::chrono::system_clock::now();
                m_data[i].received = true;
            }
        }
    }
}

std::optional<double> CompositeWindInstrument::Current(input item) const
{
    const datum& datum = m_data[static_cast<size_t>(item)];
    if (datum.received && datum.value) {
        const auto age = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - datum.changed);
        if (m_allowed_age_sec <= 0 || age.count() <= m_allowed_age_sec) {
            return datum.value;
        }
    }
    // Fall back to the values OpenCPN core fuses for own ship when the Signal K
    // key is unset or its data is stale, so we stay consistent with the chart.
    if (m_parent_dashboard) {
        if (item == input::cog) {
            return m_parent_dashboard->GetOwnShipCOG();
        }
        if (item == input::heading) {
            return m_parent_dashboard->GetOwnShipHeadingMagnetic();
        }
    }
    return std::nullopt;
}

void CompositeWindInstrument::ProcessData()
{
    if (m_new_data) {
        RefreshData();
        m_new_data = false;
        m_needs_redraw = true;
    }
    for (size_t i = 0; i < m_data.size(); ++i) {
        if (m_data[i].value && !Current(static_cast<input>(i))) {
            m_needs_redraw = true;
        }
    }
}

wxBitmap CompositeWindInstrument::Render(double scale)
{
    ProcessData();
    if (!m_needs_redraw && m_bmp.IsOk()) {
        return m_bmp;
    }
    m_needs_redraw = false;

    const wxCoord size = m_instrument_size * scale;
    // The dial keeps its size; the bitmap grows so laylines can protrude.
    const double margin = size * layline_protrusion;
    const wxCoord canvas = static_cast<wxCoord>(std::lround(size + 2 * margin));
    const double center = canvas / 2.0;
    const double radius = size * 0.475;
    const double ring_mid = size * 0.425;
    const double heading = Current(input::heading)
        ? Normalize(rad2deg(*Current(input::heading))
              + (m_parent_dashboard ? m_parent_dashboard->GetMagneticVariation()
                                    : 0.0))
        : 0.0;
    const bool has_heading = Current(input::heading).has_value();

#if defined(__WXGTK__) || defined(__WXQT__)
    m_bmp = wxBitmap(canvas, canvas, 32);
#else
    m_bmp = wxBitmap(canvas, canvas);
    m_bmp.UseAlpha();
#endif
    wxMemoryDC memory(m_bmp);
#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC dc(memory);
#else
    wxMemoryDC& dc(memory);
#endif
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();

    // The ring band can be made semi-transparent (255 = opaque, the default)
    // so the chart shows through it; the hollow center always stays empty.
    const int ring_opacity
        = wxClip(GetIntSetting(DSK_CWI_RING_OPACITY), 0, 255);
    const wxColor ring = GetDimedColor(GetColorSetting(DSK_CWI_RING_COLOR));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(
        wxPen(wxColor(ring.Red(), ring.Green(), ring.Blue(), ring_opacity),
            size * 0.10));
    dc.DrawCircle(center, center, ring_mid);
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_SETTING_BORDER_COLOR)),
        wxMax(1, size / 100)));
    dc.DrawCircle(center, center, radius);
    dc.DrawCircle(center, center, size * 0.375);

    const double bow = m_orientation == orientation::north_up ? heading : 0.0;
    for (int i = -30; i < 30; ++i) {
        const wxColor color = i < 0 ? GetColorSetting(DSK_CWI_PORT_COLOR)
                                    : GetColorSetting(DSK_CWI_STARBOARD_COLOR);
        dc.SetPen(wxPen(GetDimedColor(color), wxMax(2, size / 50)));
        dc.DrawLine(Point(center, center, radius, bow + i),
            Point(center, center, radius, bow + i + 1));
    }

    const double dial_rotation
        = m_orientation == orientation::heading_up ? -heading : 0.0;
    dc.SetPen(wxPen(GetDimedColor(GetColorSetting(DSK_CWI_TICK_COLOR)),
        wxMax(1, size / 200)));
    for (int angle = 0; angle < 360; angle += 10) {
        const double length = angle % 30 == 0 ? size * 0.03 : size * 0.02;
        dc.DrawLine(
            Point(center, center, radius - size * 0.01, angle + dial_rotation),
            Point(center, center, radius - size * 0.01 - length,
                angle + dial_rotation));
    }

    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_CWI_TEXT_COLOR)));
    dc.SetFont(wxFont(size / 22 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    const wxString cardinals[] { "N", "E", "S", "W" };
    for (int i = 0; i < 4; ++i) {
        const wxPoint point
            = Point(center, center, size * 0.40, i * 90 + dial_rotation);
        DrawCenteredText(dc, cardinals[i], point.x, point.y);
    }

    const auto awa = Current(input::awa);
    const auto twa = Current(input::twa);
    const auto cog = Current(input::cog);
    const double wind_offset
        = m_orientation == orientation::north_up ? heading : 0.0;
    if (m_show_laylines && twa
        && (has_heading || m_orientation == orientation::heading_up)) {
        // Pick beat (close-hauled) or gybe (running) laylines from the current
        // point of sail: TWA below 90 deg off the bow is upwind, above is down.
        double twa_off_bow = Normalize(rad2deg(*twa));
        if (twa_off_bow > 180.0) {
            twa_off_bow = 360.0 - twa_off_bow;
        }
        const bool upwind = twa_off_bow < 90.0;
        // The configured angle is overridden by its Signal K key (radians).
        double angle = upwind ? m_beat_angle : m_gybe_angle;
        if (const auto sk_angle = Current(upwind ? input::beat : input::gybe)) {
            angle = rad2deg(*sk_angle);
        }
        const double tw = Normalize(wind_offset + rad2deg(*twa));
        // Over-ground reference bends each layline by the current vector, so
        // the line shows the track made good rather than the heading through
        // water. Falls back to the through-water heading when current/STW is
        // missing.
        const auto stw = Current(input::stw);
        const auto set = Current(input::current_set);
        const auto drift = Current(input::current_drift);
        const bool correct = GetIntSetting(DSK_CWI_LAYLINE_REF) == 1 && stw
            && *stw > 0.05 && set && drift;
        const double set_dial = correct
            ? Normalize(rad2deg(*set)
                  - (m_orientation == orientation::heading_up ? heading : 0.0))
            : 0.0;
        const wxColor port = GetColorSetting(DSK_CWI_PORT_COLOR);
        const wxColor starboard = GetColorSetting(DSK_CWI_STARBOARD_COLOR);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        for (const double side : { -1.0, 1.0 }) {
            // tw - angle heads left of the wind, so it is the starboard tack.
            double bearing = Normalize(tw + side * angle);
            if (correct) {
                bearing = GroundTrack(bearing, *stw, set_dial, *drift);
            }
            dc.SetPen(wxPen(GetDimedColor(side < 0 ? starboard : port),
                wxMax(1, size / 100)));
            dc.DrawLine(Point(center, center, size * 0.10, bearing),
                Point(center, center, radius + margin, bearing));
        }
    }
    if (awa && (has_heading || m_orientation == orientation::heading_up)) {
        const double bearing = Normalize(wind_offset + rad2deg(*awa));
        DrawTriangle(dc, center, center, bearing, size * 0.3375, size * 0.445,
            size * 0.045, GetDimedColor(GetColorSetting(DSK_CWI_AWA_COLOR)));
        const wxPoint label = Point(center, center, size * 0.405, bearing);
        dc.SetTextForeground(*wxWHITE);
        DrawCenteredText(dc, "A", label.x, label.y);
    }
    if (twa && (has_heading || m_orientation == orientation::heading_up)) {
        const double bearing = Normalize(wind_offset + rad2deg(*twa));
        DrawTriangle(dc, center, center, bearing, size * 0.3375, size * 0.445,
            size * 0.0425, GetDimedColor(GetColorSetting(DSK_CWI_TWA_COLOR)));
        const wxPoint label = Point(center, center, size * 0.405, bearing);
        dc.SetTextForeground(*wxWHITE);
        DrawCenteredText(dc, "T", label.x, label.y);
    }
    if (cog && (has_heading || m_orientation == orientation::north_up)) {
        const double bearing = Normalize(rad2deg(*cog)
            - (m_orientation == orientation::heading_up ? heading : 0.0));
        DrawTriangle(dc, center, center, bearing, radius, size * 0.39,
            size * 0.025, GetDimedColor(GetColorSetting(DSK_CWI_COG_COLOR)));
    }
    if (has_heading) {
        DrawTriangle(dc, center, center, bow, radius, size * 0.39, size * 0.025,
            GetDimedColor(GetColorSetting(DSK_CWI_HEADING_COLOR)));
    }

    const auto angleText = [](const wxString& prefix,
                               const std::optional<double>& value) {
        // Keep the degree sign out of the printf format string: a non-ASCII
        // char there crashes wxString::Format when the locale is uninitialized.
        const wxString degree(wxUniChar(0x00B0));
        if (!value) {
            return prefix + " ---" + degree;
        }
        double degrees = CompositeWindInstrument::Normalize(rad2deg(*value));
        if (degrees > 180.0) {
            degrees = 360.0 - degrees;
        }
        return wxString::Format("%s %03.0f", prefix.c_str(), degrees) + degree;
    };
    const auto speedText = [](const wxString& prefix,
                               const std::optional<double>& value) {
        return value
            ? wxString::Format("%s %.1f kn", prefix.c_str(), *value * 1.943844)
            : prefix + " --.- kn";
    };
    dc.SetFont(wxFont(size / 28 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    // Label positions are relative to the dial, which sits at the margin offset
    // inside the enlarged bitmap.
    const wxString awa_s = angleText("AWA", awa);
    const wxString aws_s = speedText("AWS", Current(input::aws));
    const wxString twa_s = angleText("TWA", twa);
    const wxString tws_s = speedText("TWS", Current(input::tws));
    // Semi-transparent backing panels keep the labels readable over the chart;
    // tint follows the dial's ring color (dimmed) so it tracks the color
    // scheme.
    const wxColor panel_bg(ring.Red(), ring.Green(), ring.Blue(), 180);
    DrawLabelPanel(dc, awa_s, aws_s, margin + size * 0.39, margin + size * 0.73,
        margin + size * 0.785, panel_bg);
    DrawLabelPanel(dc, twa_s, tws_s, margin + size * 0.61, margin + size * 0.73,
        margin + size * 0.785, panel_bg);
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_CWI_AWA_COLOR)));
    DrawCenteredText(dc, awa_s, margin + size * 0.39, margin + size * 0.73);
    DrawCenteredText(dc, aws_s, margin + size * 0.39, margin + size * 0.785);
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_CWI_TWA_COLOR)));
    DrawCenteredText(dc, twa_s, margin + size * 0.61, margin + size * 0.73);
    DrawCenteredText(dc, tws_s, margin + size * 0.61, margin + size * 0.785);

    memory.SelectObject(wxNullBitmap);
    return m_bmp;
}

wxSize CompositeWindInstrument::ContentSize(const wxBitmap& bmp) const
{
    // The bitmap is enlarged by the layline margin on every side; the dial it
    // contains is the footprint neighbouring instruments lay out against.
    if (!bmp.IsOk()) {
        return wxSize(0, 0);
    }
    const int dial = static_cast<int>(
        std::lround(bmp.GetWidth() / (1.0 + 2.0 * layline_protrusion)));
    return wxSize(dial, dial);
}

bool CompositeWindInstrument::IsClicked(wxCoord x, wxCoord y) const
{
    // m_width is the dial footprint (see ContentSize), so the protruding
    // laylines and the transparent bitmap margin are not part of the hit test.
    const double radius = wxMin(m_width, m_height) / 2.0;
    const double dx = x - (m_x + m_width / 2.0);
    const double dy = y - (m_y + m_height / 2.0);
    const double distance_squared = dx * dx + dy * dy;
    return distance_squared >= radius * radius * 0.75 * 0.75
        && distance_squared <= radius * radius;
}

void CompositeWindInstrument::ReadConfig(Json::Value& config)
{
    Instrument::ReadConfig(config);
#define X(key, value, label, control, parameters, json, getter)                \
    if (config.isMember(key)) {                                                \
        SetSetting(key, fromJsonVal(config[key].json()));                      \
    }
    DSK_CWI_SETTINGS
#undef X
}

Json::Value CompositeWindInstrument::GenerateJSONConfig()
{
    Json::Value config = Instrument::GenerateJSONConfig();
#define X(key, value, label, control, parameters, json, getter)                \
    config[key] = toJson(getter(key));
    DSK_CWI_SETTINGS
#undef X
    return config;
}

wxString CompositeWindInstrument::GetPrimarySKKey() const
{
    return m_keys[static_cast<size_t>(input::awa)];
}

PLUGIN_END_NAMESPACE
