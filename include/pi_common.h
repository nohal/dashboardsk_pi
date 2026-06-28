/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  DashboardSK Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 *           Pavel Kalian
 ******************************************************************************
 * This file is part of the DashboardSK plugin
 * (https://github.com/nohal/dashboardsk_pi).
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com
 *   Copyright (C) 2012-2013 by Dave Cowell
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net
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

/*
 * This includes all OS and wxWidget includes
 */

#ifndef _PI_COMMON_H_
#define _PI_COMMON_H_

#define PLUGIN_NAMESPACE DashboardSKPlugin
#define PLUGIN_BEGIN_NAMESPACE namespace PLUGIN_NAMESPACE {
#define PLUGIN_END_NAMESPACE }

// For OpenGL
/*
extern "C" {

#ifdef __WXGTK__
#include "GL/gl.h"
#include "GL/glext.h"
#include "GL/glu.h"
#elif __WXMAC__
#include <OpenGL/gl3.h> // from ..../Frameworks/OpenGL.framework/Headers/gl.h
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#elif __WXQT__
// TODO
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "Ws2_32.lib")
#endif
#include <windows.h>
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#include <GL/glu.h>
//#include <opengl/GL/glext.h>
#include <GL/glext.h>
#endif

} // end "extern C"
*/
#include <wx/wxprec.h>
#ifdef __WXOSX__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpotentially-evaluated-expression"
#endif
#include <wx/aui/aui.h>
#include <wx/aui/framemanager.h>
#include <wx/wx.h>
#ifdef __WXOSX__
#pragma clang diagnostic pop
#endif

#include <stdint.h>
#include <wx/translation.h>

#include <fstream>
#include <memory>

#include <json/json.h>

using namespace std;

/// \name JSON conversion helpers
/// Bridge between wxString and the jsoncpp \c Json::Value tree, doing the
/// UTF-8 conversion at the boundary so non-ASCII payloads survive a round
/// trip. \c toJson() goes wxString/scalar -> JSON, \c fromJsonVal() goes the
/// JSON scalar accessor result back to a wxString/scalar. Overloaded so the
/// instrument settings X-macros work uniformly for string and numeric values.
///@{
// ponytail: Android ships an old wxWidgets whose wxString::ToStdString() takes
// no converter argument. Drop this shim once Android moves to wx >= 3.1.1.
#if wxCHECK_VERSION(3, 1, 1)
#define DSK_TO_STDSTRING_UTF8(s) ((s).ToStdString(wxConvUTF8))
#else
#define DSK_TO_STDSTRING_UTF8(s) (std::string((s).utf8_str()))
#endif

/// Convert a wxString to a JSON string value (UTF-8)
inline Json::Value toJson(const wxString& s)
{
    return Json::Value(DSK_TO_STDSTRING_UTF8(s));
}
/// Convert an integer to a JSON value
inline Json::Value toJson(int v) { return Json::Value(v); }
/// Convert a double to a JSON value
inline Json::Value toJson(double v) { return Json::Value(v); }
/// Convert a bool to a JSON value
inline Json::Value toJson(bool v) { return Json::Value(v); }

/// Convert a JSON string accessor result to a wxString (UTF-8)
inline wxString fromJsonVal(const std::string& s)
{
    return wxString::FromUTF8(s.c_str());
}
/// Pass-through for an integer JSON accessor result
inline int fromJsonVal(int v) { return v; }
/// Pass-through for a bool JSON accessor result
inline bool fromJsonVal(bool v) { return v; }
/// Pass-through for a double JSON accessor result
inline double fromJsonVal(double v) { return v; }

/// Parse JSON text into a \c Json::Value
///
/// \param text JSON document
/// \param out Parsed value (untouched on failure)
/// \return true on success
inline bool ParseJSON(const wxString& text, Json::Value& out)
{
    Json::CharReaderBuilder builder;
    const std::string s = DSK_TO_STDSTRING_UTF8(text);
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;
    return reader->parse(s.data(), s.data() + s.size(), &out, &errors);
}

/// Parse a JSON file into a \c Json::Value
///
/// \param path Path to the JSON file
/// \param out Parsed value (untouched on failure)
/// \return true on success
inline bool ParseJSONFile(const std::string& path, Json::Value& out)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        return false;
    }
    Json::CharReaderBuilder builder;
    std::string errors;
    return Json::parseFromStream(builder, f, &out, &errors);
}

/// Serialize a \c Json::Value to indented JSON text
///
/// \param v Value to serialize
/// \return JSON document as wxString
inline wxString DumpJSON(const Json::Value& v)
{
    Json::StreamWriterBuilder builder;
    return wxString::FromUTF8(Json::writeString(builder, v).c_str());
}
///@}

#ifdef __WXGTK__
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif

#ifdef __WXOSX__
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#ifndef SOCKET
#define SOCKET int
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET)~0)
#endif

#ifdef __WXMSW__
#define SOCKETERRSTR (strerror(WSAGetLastError()))
#else
#include <errno.h>
#define SOCKETERRSTR (strerror(errno))
#define closesocket(fd) close(fd)
#endif

#ifndef __WXMSW__
#ifndef UINT8
#define UINT8 uint8_t
#endif
#ifndef UINT16
#define UINT16 uint16_t
#endif
#ifndef UINT32
#define UINT32 uint32_t
#endif
#define wxTPRId64 wxT("ld")
#else
#define wxTPRId64 wxT("I64d")
#endif

#ifndef INT16_MIN
#define INT16_MIN (-32768)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX (255)
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define CLEAR_STRUCT(x) memset(&x, 0, sizeof(x))

#define MILLISECONDS_PER_SECOND (1000)

#ifndef PI
#define PI (3.1415926535897931160E0)
#endif
#ifndef deg2rad
#define deg2rad(x) ((x) * 2 * PI / 360.0)
#endif
#ifndef rad2deg
#define rad2deg(x) ((x) * 360.0 / (2 * PI))
#endif

#define METERS_PER_NM 1852
#define NM(x)                                                                  \
    (METERS_PER_NM                                                             \
        * x) // Define this such that NM(1/8) works even with integer division

#endif

#define M_LOG_SETTINGS DashboardSK::LogLevel()
#define LOGLEVEL_INFO 0
#define LOGLEVEL_VERBOSE 1
#define LOGLEVEL_DIALOG 2
#define LOGLEVEL_RECEIVE 4
#define LOGLEVEL_RECEIVE_DEBUG 8
#define IF_LOG_AT_LEVEL(x) if ((M_LOG_SETTINGS & (x)) != 0)
#define IF_LOG_AT(x, y)                                                        \
    do {                                                                       \
        IF_LOG_AT_LEVEL(x) { y; }                                              \
    } while (0)
#define LOG_INFO wxLogMessage
#define LOG_VERBOSE IF_LOG_AT_LEVEL(LOGLEVEL_VERBOSE) wxLogMessage
#define LOG_DIALOG IF_LOG_AT_LEVEL(LOGLEVEL_DIALOG) wxLogMessage
#define LOG_RECEIVE IF_LOG_AT_LEVEL(LOGLEVEL_RECEIVE) wxLogMessage
#define LOG_RECEIVE_DEBUG IF_LOG_AT_LEVEL(LOGLEVEL_RECEIVE_DEBUG) wxLogMessage

// In wxWidgets 3.0 wxString is not compatible with std::string and can't
// directly be used as a key in std containers
#if wxCHECK_VERSION(3, 1, 0)
#define UNORDERED_KEY(x) x
#else
#define UNORDERED_KEY(x) x.ToStdString()
#endif
