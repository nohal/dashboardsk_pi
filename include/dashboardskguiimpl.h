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

#ifndef _DASHBOARDSKGUIIMPL_H_
#define _DASHBOARDSKGUIIMPL_H_

#include "dashboard.h"
#include "dashboardskgui.h"
#include "instrument.h"
#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

class dashboardsk_pi;
// class Dashboard;

/// Implementation of the GUI functionality for Preferences dialog.
/// To obtain \c MainConfigFrame information use \c wxFormBuilder to open \c
/// dashboardsk.fbp
class MainConfigFrameImpl : public MainConfigFrame {
private:
    /// Pointer to the main plugin object
    dashboardsk_pi* m_dsk_pi;
    /// Pointer to the currently edited dashboard
    Dashboard* m_edited_dashboard;
    /// Pointer to the currently edited instrument
    Instrument* m_edited_instrument;
    /// Copy of the original configuration used to revert all the changes on
    /// Cancel
    wxJSONValue m_orig_config;

    /// Enables or disables form items
    /// \param dashboard_selection Desired state for the dashboard selection
    /// dropdown and buttons \param instr_list Desired state for the list of
    /// instruments and buttons \param dashboard_config Desired state for the
    /// widgets editing configuration of the dashboard \param instr_config
    /// Desired state of the widgets editing configuratio of the instrument
    void EnableItems(bool dashboard_selection, bool instr_list,
        bool dashboard_config, bool instr_config);
    /// Enable/disable items based on the currently selected dashboard
    void EnableItemsForSelectedDashboard();
    /// Enable/disable buttons related to the instrument list
    void EnableInstrumentListButtons();
    /// Fill in the instrument list with items
    void FillInstrumentList();
    /// Fill the widgets with data for currently selected instrument
    void FillInstrumentDetails();
    /// Extract value from a dynamically generated widget in instrument
    /// configuration
    ///
    /// \param ctrl Pointer to the widget
    /// \return String representation of the value obtained from the widget
    const wxString ExtractValue(wxWindow* ctrl);
    /// Update the currently edited instrument with values set in the GUI
    void UpdateEditedInstrument();
    /// Fill the form with prameters of the currently edited dashboard
    void FillDashboardDetails();
    /// Update the currently edited dashboard with values set in the GUI
    void UpdateEditedDashboard();

public:
    /// Constructor
    ///
    /// \param dsk_pi Pointer to the plugin object
    /// \param parent The parent window.
    /// \param id An identifier for the dialog. \c wxID_ANY is taken to mean a
    /// default.
    /// \param title Title for the window
    /// \param pos The dialog
    /// position. The value \c wxDefaultPosition indicates a default position,
    /// chosen by either the windowing system or wxWidgets, depending on
    /// platform.
    /// \param size The panel size. The value \c wxDefaultSize
    /// indicates a default size, chosen by either the windowing system or
    /// wxWidgets, depending on platform.
    /// \param style The window style. See \c wxDialog.
    MainConfigFrameImpl(dashboardsk_pi* dsk_pi, wxWindow* parent,
        wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(640, 500),
        long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~MainConfigFrameImpl() = default;

protected:
    /// Event handler for displaying the SignalK data
    ///
    /// \param event The event object reference
    virtual void m_btnSignalKOnButtonClick(wxCommandEvent& event);
    /// Event handler for dashboard selection in the combobox
    ///
    /// \param event The event object reference
    virtual void m_comboDashboardOnChoice(wxCommandEvent& event);
    /// Event handler for dashboard removal
    ///
    /// \param event The event object reference
    virtual void m_btnRemoveDashboardOnButtonClick(wxCommandEvent& event);
    /// Event handler for creation of new dashboard
    ///
    /// \param event The event object reference
    virtual void m_btnNewDashboardOnButtonClick(wxCommandEvent& event);
    /// Event handler for instrument selection from the list
    ///
    /// \param event The event object reference
    virtual void m_lbInstrumentsOnListBox(wxCommandEvent& event);
    /// Event handler for addition of new instrument
    ///
    /// \param event The event object reference
    virtual void m_bpAddButtonOnButtonClick(wxCommandEvent& event);
    /// Event handler for instrument removal
    ///
    /// \param event The event object reference
    virtual void m_bpRemoveButtonOnButtonClick(wxCommandEvent& event);
    /// Event handler for instrument reordering. Moving instrument up in the
    /// list.
    ///
    /// \param event The event object reference
    virtual void m_bpMoveUpButtonOnButtonClick(wxCommandEvent& event);
    /// Event handler for instrument reordering. Moving instrument down in the
    /// list.
    ///
    /// \param event The event object reference
    virtual void m_bpMoveDownButtonOnButtonClick(wxCommandEvent& event);
    /// Event handler for instrument import.
    /// \todo Implement this functionality
    ///
    /// \param event The event object reference
    virtual void m_bpSaveInstrButtonOnButtonClick(wxCommandEvent& event)
    {
        // TODO
        event.Skip();
    }
    /// Event handler for instrument export.
    /// \todo Implement this functionality
    ///
    /// \param event The event object reference
    virtual void m_bpImportInstrButtonOnButtonClick(wxCommandEvent& event)
    {
        // TODO
        event.Skip();
    }
    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_cbEnabledOnCheckBox(wxCommandEvent& event)
    {
        UpdateEditedDashboard();
    }
    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spCanvasOnSpinCtrl(wxSpinEvent& event)
    {
        UpdateEditedDashboard();
    }
    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_chAnchorOnChoice(wxCommandEvent& event)
    {
        UpdateEditedDashboard();
    }
    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spOffsetXOnSpinCtrl(wxSpinEvent& event)
    {
        UpdateEditedDashboard();
    }
    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spOffsetYOnSpinCtrl(wxSpinEvent& event)
    {
        UpdateEditedDashboard();
    }
    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spSpacingHOnSpinCtrl(wxSpinEvent& event)
    {
        UpdateEditedDashboard();
    }
    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spSpacingVOnSpinCtrl(wxSpinEvent& event)
    {
        UpdateEditedDashboard();
    }
    /// Event handler for main Cancel button click. Rolls back all the changing
    /// by reloading the original configuration stored in #m_orig_config
    ///
    /// \param event The event object reference
    virtual void m_sdbSizerOnCancelButtonClick(wxCommandEvent& event);
    /// Event handler for main OK button click. Triggers saving of the
    /// configuration persistently to disk.
    ///
    /// \param event The event object reference
    virtual void m_sdbSizerOnOKButtonClick(wxCommandEvent& event);
};

/// Implementation of the form displaying JSON documents
/// To obtain \c SKDataTree information use \c wxFormBuilder to open \c
/// dashboardsk.fbp
class SKDataTreeImpl : public SKDataTree {
public:
    /// Constructor
    /// \param parent Pointer to the owner window
    explicit SKDataTreeImpl(wxWindow* parent)
        : SKDataTree(parent)
    {
        m_scintillaCode->StyleClearAll();
        m_scintillaCode->StyleSetForeground(
            wxSTC_STYLE_DEFAULT, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_STYLE_DEFAULT, GetBackgroundColour());
        m_scintillaCode->SetLexer(wxSTC_LEX_JSON);
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_DEFAULT, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_DEFAULT, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_KEYWORD, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_KEYWORD, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_STRING, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_STRING, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_URI, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_URI, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_NUMBER, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_NUMBER, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_OPERATOR, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_OPERATOR, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(wxSTC_JSON_PROPERTYNAME, *wxRED);
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_PROPERTYNAME, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_ERROR, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_ERROR, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_LDKEYWORD, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_LDKEYWORD, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_LINECOMMENT, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_LINECOMMENT, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_BLOCKCOMMENT, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_BLOCKCOMMENT, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_STRINGEOL, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_STRINGEOL, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_COMPACTIRI, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_COMPACTIRI, GetBackgroundColour());
        m_scintillaCode->StyleSetForeground(
            wxSTC_JSON_ESCAPESEQUENCE, GetForegroundColour());
        m_scintillaCode->StyleSetBackground(
            wxSTC_JSON_ESCAPESEQUENCE, GetBackgroundColour());
    };
    /// Destructor
    ~SKDataTreeImpl() = default;
    /// Fill the form with the SignalK data object
    /// \todo Implement this functionality
    ///
    /// \param dsk Pointer to the plugin logic implementation
    void SetCodeSKTree(DashboardSK* dsk);
    /// Fill the form with configuration file
    void SetCodeConfig(const wxString& config)
    {
        m_sdbSizerBtnsCancel->Show();
    }; // TODO: For configuration JSON editing
};

/// Implementation of widget for editing SignalK path in a textbox or selecting
/// it from a tree structure generated from JSON document To obtain \c SKKeyCtrl
/// information use \c wxFormBuilder to open \c dashboardsk.fbp
class SKKeyCtrlImpl : public SKKeyCtrl {
public:
    /// Constructor
    SKKeyCtrlImpl()
        : SKKeyCtrl(NULL) {};

    /// Constructor
    ///
    /// \param parent Parent window pointer
    /// \param id Id of the window
    /// \param pos Position, defaults to \c wxDefaultPosition
    /// \param size Size, defaults to \c wxDefaultSize
    /// \param style Style, defaults to \c wxTAB_TRAVERSAL
    /// \param name Name of the widget
    /// \param value Initial value of the textbox
    SKKeyCtrlImpl(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL,
        const wxString& name = wxEmptyString,
        const wxString& value = wxEmptyString)
        : SKKeyCtrl(parent, id, pos, size, style, name)
    {
        m_tSKKey->SetValue(value);
        m_sk_tree = nullptr;
    };

    /// Destructor
    ~SKKeyCtrlImpl() = default;

    /// Get the value from the textbox
    /// \return String value from the textbox
    wxString GetValue() const { return m_tSKKey->GetValue(); };

    /// Set value of the textbox
    ///
    /// \param value Value to set
    void SetValue(const wxString& value) const { m_tSKKey->SetValue(value); };

    /// Set pointer to the SignalK full data object
    ///
    /// \param sk_tree Pointer to the \c wxJSONValue holding the data
    void SetSKTree(wxJSONValue* sk_tree) { m_sk_tree = sk_tree; }

protected:
    /// Event handler for click on the button invoking the path selection from a
    /// tree of known values
    virtual void m_btnSelectOnButtonClick(wxCommandEvent& event);

    /// Get best size for the widget
    virtual wxSize DoGetBestSize() const
    {
        wxSize s1 = m_tSKKey->GetBestSize();
        wxSize s2 = m_btnSelect->GetBestSize();
        return wxSize(s1.GetWidth() + s2.GetWidth(),
            wxMax(s1.GetHeight(), s2.GetHeight()));
    }

private:
    /// Pointer to the \c wxJSONValue object holding SignalK data
    wxJSONValue* m_sk_tree;

    /// Obligatory for the widget
    wxDECLARE_DYNAMIC_CLASS(SKKeyCtrlImpl);
    // wxDECLARE_EVENT_TABLE();
};

/// Implementation of the form displaying SignalK data tree view allowing
/// selection To obtain \c SKKeyCtrl information use \c wxFormBuilder to open \c
/// dashboardsk.fbp
class SKPathBrowserImpl : public SKPathBrowser {
public:
    /// Constructor
    /// \param parent Pointer to the owner window
    /// \param id An identifier for the dialog. \c wxID_ANY is taken to mean a
    /// default.
    /// \param sk_tree Reference to the wxJSONValue holding the full SignalK
    /// data tree \param title Title of the dialog window \param pos The dialog
    /// position. The value \c wxDefaultPosition indicates a default position,
    /// chosen by either the windowing system or wxWidgets, depending on
    /// platform.
    /// \param size The panel size. The value \c wxDefaultSize
    /// indicates a default size, chosen by either the windowing system or
    /// wxWidgets, depending on platform.
    /// \param style The window style. See \c wxDialog.

    explicit SKPathBrowserImpl(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxJSONValue& sk_tree = wxJSONValue(),
        const wxString& title = _("SignalK Browser"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(500, 300),
        long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
        : SKPathBrowser(parent, id, title, pos, size, style)
    {
        m_sk_tree = sk_tree;
        wxTreeItemId root = m_treePaths->AddRoot("SignalK");
        AddChildren(root, m_sk_tree);
        m_treePaths->ExpandAll();
    };

    /// Destructor
    ~SKPathBrowserImpl() = default;

    /// Get the full path selected from the tree
    ///
    /// \return The full path
    const wxString GetSKPath()
    {
        wxTreeItemId selected = m_treePaths->GetSelection();
        wxString path = wxEmptyString;
        while (selected.IsOk() && selected != m_treePaths->GetRootItem()) {
            if (!path.IsEmpty()) {
                path.Prepend(".");
            }
            path.Prepend(m_treePaths->GetItemText(selected));
            selected = m_treePaths->GetItemParent(selected);
        }
        return path;
    };

    /// Set pointer to the SignalK full data object
    ///
    /// \param sk_tree Pointer to the \c wxJSONValue holding the data
    void SetSKTree(wxJSONValue* sk_tree)
    {
        m_sk_tree = *sk_tree;
        wxTreeItemId root = m_treePaths->AddRoot("SignalK");
        AddChildren(root, m_sk_tree);
        m_treePaths->ExpandAll();
    };

private:
    /// Recursively populate the tree control with known children from the
    /// SignalK data
    ///
    /// \param parent ID of the parent tree node
    /// \param json_node Reference to the SignalK data node to be used for
    /// population
    void AddChildren(wxTreeItemId parent, wxJSONValue& json_node)
    {
        for (auto member : json_node.GetMemberNames()) {
            if (!(member.IsSameAs("value") || member.IsSameAs("source")
                    || member.IsSameAs("timestamp"))) {
                // TODO: Isn't there a "legal" node with some of the above
                // names?
                wxTreeItemId child = m_treePaths->AppendItem(parent, member);
                AddChildren(child, json_node[member]);
            }
        }
    };

    /// Pointer to the object holding the SignalK data
    wxJSONValue m_sk_tree;
};

PLUGIN_END_NAMESPACE

#endif // _DASHBOARDSKGUIIMPL_H_
