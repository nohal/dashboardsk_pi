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

#ifndef _ZONE_H_
#define _ZONE_H_

#include "pi_common.h"
#include <vector>
#include <wx/tokenzr.h>

PLUGIN_BEGIN_NAMESPACE

/// Class reprezenting the SignalK zone metadata
/// A zone is used to define the display and alarm state when the value is in
/// between bottom and top. See \c definitions.json in SignalK schema
class Zone {
public:
    /// The alarm state when the value is in this zone
    enum class state { nominal = 0, normal, alert, warn, alarm, emergency };

private:
    /// Lower limit of the zone
    double m_lower_limit;
    /// Upper limit of the zone
    double m_upper_limit;
    /// Alarm state assigned to the zone
    state m_state;
    /// Message to be used when value is in the zone
    wxString m_message;

public:
    /// Constructor
    Zone()
    {
        m_lower_limit = 0.0;
        m_upper_limit = 0.0;
        m_state = state::normal;
        m_message = wxEmptyString;
    };

    Zone(double lower, double upper, state st = state::normal,
        const wxString& msg = wxEmptyString)
    {
        m_lower_limit = lower;
        m_upper_limit = upper;
        m_state = st;
        m_message = msg;
    };

    /// Destructor
    ~Zone() = default;

    /// Set the lower limit
    ///
    /// \param val value of the limit
    void SetLowerLimit(const double val) { m_lower_limit = val; };

    /// Get the lower limit
    ///
    /// \return The lower limit value
    const double GetLowerLimit() { return m_lower_limit; };

    /// Set the upper limit
    ///
    /// \param val value of the limit
    void SetUpperLimit(const double val) { m_upper_limit = val; };

    /// Get the upper limit
    ///
    /// \return The upper limit
    const double GetUpperLimit() { return m_upper_limit; };

    /// Set the message assigned to the zone
    ///
    /// \param val Text of the message
    void SetMessage(const wxString& val) { m_message = val; };

    /// Get the message assigned to the zne
    ///
    /// \return The alarm state of the zone
    const wxString GetMessage() { return m_message; };

    /// Set the alarm state assigned to the zone
    ///
    /// \param val The alarm state of the zone
    void SetState(const state val) { m_state = val; };

    /// Get the alarm state of the zone
    ///
    /// \return The alarm state of the zone
    state GetState() { return m_state; };

    /// Get the localized text representation of the zone parameters for use in
    /// the GUI
    ///
    /// \return String to be presented to the user
    const wxString ToUIString()
    {
        return wxString::Format(_("%.1f to %.1f, %s"), m_lower_limit,
            m_upper_limit, UIStringFromState(m_state).c_str());
    }

    /// Get the fixed format text representation of the zone to be machine
    /// processed
    ///
    /// \return String representation of the zone parameters
    const wxString ToString()
    {
        return wxString::Format(_("%.1f,%.1f,%s"), m_lower_limit, m_upper_limit,
            StringFromState(m_state).c_str());
    }

    /// Get state from the string obtained (usually) from the SignalK metadata
    ///
    /// \param val String representaion of the state
    static const state StateFromString(const wxString& val)
    {
        if (val.IsSameAs("normal")) {
            return state::normal;
        } else if (val.IsSameAs("alert")) {
            return state::alert;
        } else if (val.IsSameAs("warn")) {
            return state::warn;
        } else if (val.IsSameAs("alarm")) {
            return state::alarm;
        } else if (val.IsSameAs("emergency")) {
            return state::emergency;
        }
        return state::nominal;
    };

    /// Get string compliant with the SignalK specifiaction from the zone alarm
    /// state
    ///
    /// \param val Zone alarm state
    /// \return Signalk schema compliant string
    static const wxString StringFromState(state val)
    {
        switch (val) {
        case state::normal:
            return "normal";
        case state::alert:
            return "alert";
        case state::warn:
            return "warn";
        case state::alarm:
            return "alarm";
        case state::emergency:
            return "emergency";
        default:
            return "nominal";
        }
    };

    /// Get localized string presentable to the user from the zone alarm state
    ///
    /// \param val Zone alarm state
    /// \return Localized string representation of the state
    static const wxString UIStringFromState(state val)
    {
        switch (val) {
        case state::normal:
            return _("normal");
        case state::alert:
            return _("alert");
        case state::warn:
            return _("warn");
        case state::alarm:
            return _("alarm");
        case state::emergency:
            return _("emergency");
        default:
            return _("nominal");
        }
    };

    /// Parse string definition of zones into a vector of #Zone objects
    ///
    /// \param zones String containing the definition of the zones in
    /// "<lower_limit>,<upper_limit>,<alarm_state>;" format \return vector of
    /// Zone objects parsed from the string
    static const vector<Zone> ParseZonesFromString(const wxString& zones)
    {
        wxStringTokenizer tokenizer(zones, ";");
        vector<Zone> v;
        while (tokenizer.HasMoreTokens()) {
            wxString zone = tokenizer.GetNextToken();
            wxStringTokenizer ztokenizer(zone, ",");
            Zone z;
            double limit;
            wxString s;
            if (!ztokenizer.GetNextToken().ToDouble(&limit)) {
                limit = 0.0;
            }
            z.SetLowerLimit(limit);
            if (!ztokenizer.GetNextToken().ToDouble(&limit)) {
                limit = 0.0;
            }
            z.SetUpperLimit(limit);
            z.SetState(StateFromString(ztokenizer.GetNextToken()));
            v.emplace_back(z);
        }
        return v;
    };

    /// Get a string representation of a vector of zones
    ///
    /// \param zones vector of zones
    /// \return String representation of the vector as semicolon separated
    /// <low>,<high>,<alarm state> triplets
    static const wxString ZonesToString(const vector<Zone>& zones)
    {
        wxString v = wxEmptyString;
        for (auto z : zones) {
            if (!v.IsEmpty()) {
                v.Append(";");
            }
            v.Append(z.ToString());
        }
        return v;
    }
};

PLUGIN_END_NAMESPACE

#endif //_ZONE_H_
