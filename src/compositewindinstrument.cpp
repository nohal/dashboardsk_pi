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
}

void CompositeWindInstrument::Init()
{
    m_name = _("New Composite Wind");
    m_title = DUMMY_TITLE;
    m_orientation = orientation::north_up;
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
    const wxString names[] { DSK_CWI_AWA_KEY, DSK_CWI_AWS_KEY, DSK_CWI_TWA_KEY,
        DSK_CWI_TWS_KEY, DSK_CWI_HEADING_KEY, DSK_CWI_COG_KEY };
    for (size_t i = 0; i < m_keys.size(); ++i) {
        if (key.IsSameAs(names[i])) {
            m_keys[i] = value;
            SubscribeAll();
            return;
        }
    }
    if (key.IsSameAs(DSK_CWI_ORIENTATION)
        || key.IsSameAs(DSK_SETTING_INSTR_SIZE)) {
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
    if (!datum.received || !datum.value) {
        return std::nullopt;
    }
    const auto age = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now() - datum.changed);
    return m_allowed_age_sec > 0 && age.count() > m_allowed_age_sec
        ? std::nullopt
        : datum.value;
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
    const double center = size / 2.0;
    const double radius = size * 0.475;
    const double ring_mid = size * 0.425;
    const double heading = Current(input::heading)
        ? Normalize(rad2deg(*Current(input::heading))
              + (m_parent_dashboard ? m_parent_dashboard->GetMagneticVariation()
                                    : 0.0))
        : 0.0;
    const bool has_heading = Current(input::heading).has_value();

#if defined(__WXGTK__) || defined(__WXQT__)
    m_bmp = wxBitmap(size, size, 32);
#else
    m_bmp = wxBitmap(size, size);
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

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(
        wxPen(GetDimedColor(GetColorSetting(DSK_CWI_RING_COLOR)), size * 0.10));
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
        if (!value) {
            return prefix + " ---\u00B0";
        }
        double degrees = CompositeWindInstrument::Normalize(rad2deg(*value));
        if (degrees > 180.0) {
            degrees = 360.0 - degrees;
        }
        return wxString::Format("%s %03.0f\u00B0", prefix.c_str(), degrees);
    };
    const auto speedText = [](const wxString& prefix,
                               const std::optional<double>& value) {
        return value
            ? wxString::Format("%s %.1f kn", prefix.c_str(), *value * 1.943844)
            : prefix + " --.- kn";
    };
    dc.SetFont(wxFont(size / 28 / AUTO_TEXT_SIZE_COEF, wxFONTFAMILY_SWISS,
        wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_CWI_AWA_COLOR)));
    DrawCenteredText(dc, angleText("AWA", awa), size * 0.39, size * 0.73);
    DrawCenteredText(
        dc, speedText("AWS", Current(input::aws)), size * 0.39, size * 0.785);
    dc.SetTextForeground(GetDimedColor(GetColorSetting(DSK_CWI_TWA_COLOR)));
    DrawCenteredText(dc, angleText("TWA", twa), size * 0.61, size * 0.73);
    DrawCenteredText(
        dc, speedText("TWS", Current(input::tws)), size * 0.61, size * 0.785);

    memory.SelectObject(wxNullBitmap);
    return m_bmp;
}

bool CompositeWindInstrument::IsClicked(wxCoord x, wxCoord y) const
{
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
