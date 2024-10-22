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

#include "pi_common.h"
#include "pidc.h"

#ifndef _DSKDC_H
#define _DSKDC_H

PLUGIN_BEGIN_NAMESPACE

/// The universal "device context" to draw on
class dskDC : public piDC {
private:
    /// Scale factor of the context
    double m_scale_factor = 1.0;
    bool m_is_gl;

public:
    /// Constructor
    ///
    /// \param canvas OpenGL canvas to draw on
    dskDC(wxGLContext* context)
        : piDC(context)
    {
        m_is_gl = true;
    };

    /// Constructor
    ///
    /// \param pdc Non-OpenGL device context to draw on
    dskDC(wxDC& pdc)
        : piDC(pdc)
    {
        dc = &pdc;
        m_scale_factor = pdc.GetContentScaleFactor();
        m_is_gl = false;
    };

    /// Constructor
    dskDC()
        : piDC()
    {
        m_is_gl = true;
    };

    bool IsGL() const { return m_is_gl; }

    /// Check if our GL context is the same as the one passed
    /// \param context The context to check
    /// \return True if the contexts are the same
    bool CheckContext(wxGLContext* context) const
    {
        return glcontext == context;
    }

    /// Get the scale factor of the device context
    ///
    /// \return The scale factor
    double GetContentScaleFactor() const
    {
        // TODO: For some reason the bellow seems crashing on flatpak
        // if (glcanvas) {
        //     return glcanvas->GetContentScaleFactor();
        // } else if (dc) {
        //     return dc->GetContentScaleFactor();
        // } else
        return m_scale_factor;
    };

    /// Set the scale factor of the device context
    ///
    /// \param f The scale factor
    void SetContentScaleFactor(double f) { m_scale_factor = f; };
};

PLUGIN_END_NAMESPACE

#endif //_DSKDC_H
