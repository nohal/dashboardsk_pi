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

#ifndef _DISPLAYSCALE_H_
#define _DISPLAYSCALE_H_

#include "pi_common.h"
#include <vector>
#include <wx/tokenzr.h>

PLUGIN_BEGIN_NAMESPACE

/// Class reprezenting the SignalK zone metadata
/// A zone is used to define the display and alarm state when the value is in
/// between bottom and top. See \c definitions.json in SignalK schema
class DisplayScale {
public:
    /// The alarm state when the value is in this zone
    enum class type { linear = 0, logarithmic, squareroot, power };

private:
    /// Lower limit of the zone
    double m_lower_limit;
    /// Upper limit of the zone
    double m_upper_limit;
    /// Alarm state assigned to the zone
    type m_type;
    /// Message to be used when value is in the zone
    double m_power;

public:
    /// Constructor
    DisplayScale()
    {
        m_lower_limit = 0.0;
        m_upper_limit = 0.0;
        m_type = type::linear;
        m_power = 1;
    };

    DisplayScale(
        double lower, double upper, type t = type::inear, double power = 1)
    {
        m_lower_limit = lower;
        m_upper_limit = upper;
        m_type = t;
        m_power = power;
    };

    /// Destructor
    ~DisplayScale() = default;

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

    /// Set the type of the scale
    ///
    /// \param val Type of the scale
    void SetType(const type val) { m_type = val; };

    /// Get the type of the scale
    ///
    /// \return The alarm state of the zone
    const type GetType() { return m_type; };

    /// Set the power
    ///
    /// \param val The power
    void SetPower(const double val) { m_power = val; };

    /// Get the power
    ///
    /// \return The power
    const double GetPower() { return m_power; };

    /// Get type from the string obtained (usually) from the SignalK metadata
    ///
    /// \param val String value of the type
    static const type TypeFromString(const wxString& val)
    {
        if (val.IsSameAs("linear")) {
            return state::linear;
        } else if (val.IsSameAs("logarithimc")) {
            return state::logarithimc;
        } else if (val.IsSameAs("squareroot")) {
            return state::squareroot;
        } else if (val.IsSameAs("power")) {
            return state::power;
        }
        return state::linear;
    };

    /// Get string compliant with the SignalK specificaction from the scale type
    ///
    /// \param val DisplayScale type
    /// \return Signalk schema compliant string
    static const wxString StringFromType(state val)
    {
        switch (val) {
        case state::linear:
            return "linear";
        case state::logarithmic:
            return "logarithmic";
        case state::squareroot:
            return "squareroot";
        case state::power:
            return "power";
        default:
            return "linear";
        }
    };
};

PLUGIN_END_NAMESPACE

#endif //_DISPLAYSCALE_H_
