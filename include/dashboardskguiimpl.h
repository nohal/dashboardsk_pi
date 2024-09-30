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
#if __WXQT__
#include "dashboardskguiandroid.h"
#else
#include "dashboardskgui.h"
#endif
#include "instrument.h"
#include "pi_common.h"

#if not __WXQT__ and (wxCHECK_VERSION(3, 1, 0))
#define BMP_SZ FromDIP(16) // Size of the bitmap for wxBitmapButton
#else
#define BMP_SZ 16
#endif

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

    /// Fill the form with data
    void FillForm(bool select_last = false);

    /// Extract value from a dynamically generated widget in instrument
    /// configuration
    ///
    /// \param ctrl Pointer to the widget
    /// \return String representation of the value obtained from the widget
    wxString ExtractValue(wxWindow* ctrl);

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
        const wxSize& size = wxSize(880, 620),
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

    /// Event handler for dashboard rename
    ///
    /// \param event The event object reference
    virtual void m_btnRenameDashboardOnButtonClick(wxCommandEvent& event);

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
    ///
    /// \param event The event object reference
    virtual void m_bpSaveInstrButtonOnButtonClick(wxCommandEvent& event);

    /// Event handler for instrument export.
    ///
    /// \param event The event object reference
    virtual void m_bpImportInstrButtonOnButtonClick(wxCommandEvent& event);

    /// Event handler for dashboard export.
    ///
    /// \param event The event object reference
    virtual void m_btnExportDashboardOnButtonClick(wxCommandEvent& event);

    /// Event handler for dashboard import.
    ///
    /// \param event The event object reference
    virtual void m_btnImportDashboardOnButtonClick(wxCommandEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_cbEnabledOnCheckBox(wxCommandEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spCanvasOnSpinCtrl(wxSpinEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spPageOnSpinCtrl(wxSpinEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_chAnchorOnChoice(wxCommandEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spOffsetXOnSpinCtrl(wxSpinEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spOffsetYOnSpinCtrl(wxSpinEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spSpacingHOnSpinCtrl(wxSpinEvent& event);

    /// Event handler for change of parameter of edited dashboard
    ///
    /// \param event The event object reference
    virtual void m_spSpacingVOnSpinCtrl(wxSpinEvent& event);

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

    /// Event handler for configurarion JSON editor
    ///
    /// \param event The event object reference
    virtual void m_btnCfgEditOnButtonClick(wxCommandEvent& event);
};

/// Implementation of the form displaying JSON documents
/// To obtain \c SKDataTree information use \c wxFormBuilder to open \c
/// dashboardsk.fbp
class SKDataTreeImpl : public SKDataTree {
public:
    /// Constructor
    /// \param parent Pointer to the owner window
    explicit SKDataTreeImpl(wxWindow* parent);

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
#if not __WXQT__
        m_scintillaCode->SetReadOnly(false);
        m_scintillaCode->SetText(config);
        m_scintillaCode->SetSTCCursor(wxSTC_CURSORNORMAL);
#endif
        m_sdbSizerBtnsCancel->Show();
    };

    /// Get the displayed document as text
    ///
    /// \return (Hopefully) JSON text
    const wxString GetValue()
    {
#if not __WXQT__
        return m_scintillaCode->GetText();
#else
        return m_scintillaCode->GetValue();
#endif
    }
};

/// Implementation of the widget for editing SignalK path in a textbox or
/// selecting it from a tree structure generated from JSON document To obtain \c
/// SKKeyCtrl information use \c wxFormBuilder to open \c dashboardsk.fbp
class SKKeyCtrlImpl : public SKKeyCtrl {
public:
    /// Constructor
    SKKeyCtrlImpl()
        : SKKeyCtrl(NULL)
        , m_sk_tree(nullptr) { };

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
        const wxString& value = wxEmptyString);

    /// Destructor
    ~SKKeyCtrlImpl() = default;

    /// Get the value from the textbox
    /// \return String value from the textbox
    wxString GetValue() const;

    /// Set value of the textbox
    ///
    /// \param value Value to set
    void SetValue(const wxString& value) const;

    /// Set pointer to the SignalK full data object
    ///
    /// \param sk_tree Pointer to the \c wxJSONValue holding the data
    void SetSKTree(wxJSONValue* sk_tree);

    // Set ID of own vessel
    /// Must be called before \c SetSKTree
    ///
    /// \param self String SignalK  ID of the own vessel
    void SetSelf(const wxString& self);

protected:
    /// Event handler for click on the button invoking the path selection from a
    /// tree of known values
    virtual void m_btnSelectOnButtonClick(wxCommandEvent& event);

    /// Get best size for the widget
    virtual wxSize DoGetBestSize() const;

private:
    /// Pointer to the \c wxJSONValue object holding SignalK data
    wxJSONValue* m_sk_tree;

    /// ID of the own vessel
    wxString m_self;

    /// Obligatory for the widget
    wxDECLARE_DYNAMIC_CLASS(SKKeyCtrlImpl);
    // wxDECLARE_EVENT_TABLE();
};

/// Implementation of the form displaying SignalK data tree view allowing
/// selection To obtain \c SKPathBrowser information use \c wxFormBuilder to
/// open \c dashboardsk.fbp
class SKPathBrowserImpl : public SKPathBrowser {
public:
    /// Constructor
    /// \param parent Pointer to the owner window
    /// \param id An identifier for the dialog. \c wxID_ANY is taken to mean a
    /// default.
    /// \param title Title of the dialog window \param pos The dialog
    /// position. The value \c wxDefaultPosition indicates a default position,
    /// chosen by either the windowing system or wxWidgets, depending on
    /// platform.
    /// \param size The panel size. The value \c wxDefaultSize
    /// indicates a default size, chosen by either the windowing system or
    /// wxWidgets, depending on platform.
    /// \param style The window style. See \c wxDialog.

    explicit SKPathBrowserImpl(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& title = _("SignalK Browser"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(500, 500),
        long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

    /// Destructor
    ~SKPathBrowserImpl() = default;

    /// Get the full path selected from the tree
    ///
    /// \return The full path
    wxString GetSKPath();

    /// Set pointer to the SignalK full data object
    ///
    /// \param sk_tree Pointer to the \c wxJSONValue holding the data
    void SetSKTree(wxJSONValue* sk_tree);

    /// Set ID of own vessel
    /// Must be called before \c SetSKTree
    ///
    /// \param self String SignalK  ID of the own vessel
    void SetSelf(const wxString& self);

protected:
    virtual void m_btnCollapseOnButtonClick(wxCommandEvent& event);
    virtual void m_btnSelfOnButtonClick(wxCommandEvent& event);

private:
    /// Recursively populate the tree control with known children from the
    /// SignalK data
    ///
    /// \param parent ID of the parent tree node
    /// \param json_node Reference to the SignalK data node to be used for
    /// population
    void AddChildren(wxTreeItemId parent, wxJSONValue& json_node);

    /// Pointer to the object holding the SignalK data
    wxJSONValue m_sk_tree;

    /// ID of the own vessel
    wxString m_self;

    /// Tree item ID of the own vessel
    wxTreeItemId m_self_item_id;
};

/// Implementation of the widget for editing warning and alarm zones. To obtain
/// \c SKZonesCtrl information use \c wxFormBuilder to open \c dashboardsk.fbp
class SKZonesCtrlImpl : public SKZonesCtrl {
private:
    /// Pointer to the plugin object
    dashboardsk_pi* m_dsk_pi;

public:
    /// Constructor
    SKZonesCtrlImpl()
        : SKZonesCtrl(NULL)
        , m_dsk_pi(nullptr) { };

    /// Constructor
    ///
    /// \param parent Parent window pointer
    /// \param dsk_pi Pointer to the plugin object
    /// \param id Id of the window
    /// \param pos Position, defaults to \c wxDefaultPosition
    /// \param size Size, defaults to \c wxDefaultSize
    /// \param style Style, defaults to \c wxTAB_TRAVERSAL
    /// \param name Name of the widget
    /// \param value Initial value of the textbox
    SKZonesCtrlImpl(wxWindow* parent, dashboardsk_pi* dsk_pi = nullptr,
        wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL,
        const wxString& name = wxEmptyString,
        const wxString& value = wxEmptyString);

    /// Destructor
    ~SKZonesCtrlImpl() = default;

    /// Get the value from the textbox
    /// \return String value from the textbox
    wxString GetValue() const;

    /// Set value of the textbox
    ///
    /// \param value Value to set
    void SetValue(const wxString& value) const;

protected:
    /// Event handler for click on the button invoking the path selection from a
    /// tree of known values
    virtual void m_btnSelectOnButtonClick(wxCommandEvent& event);

    /// Get best size for the widget
    virtual wxSize DoGetBestSize() const;

private:
    /// Obligatory for the widget
    wxDECLARE_DYNAMIC_CLASS(SKZonesCtrlImpl);
    // wxDECLARE_EVENT_TABLE();
};

/// Implementation of the form displaying value zones
/// To obtain \c ZonesConfigDialog information use \c wxFormBuilder to open \c
/// dashboardsk.fbp
class ZonesConfigDialogImpl : public ZonesConfigDialog {
private:
    /// Pointer to the dashboard plugin
    dashboardsk_pi* m_dsk_pi;

public:
    /// Constructor
    /// \param parent Pointer to the owner window
    /// \param dsk_pi Pointer to the plugin object
    /// \param id An identifier for the dialog. \c wxID_ANY is taken to mean a
    /// default.
    /// \param value String representation of the zones
    /// data tree \param title Title of the dialog window \param pos The dialog
    /// position. The value \c wxDefaultPosition indicates a default position,
    /// chosen by either the windowing system or wxWidgets, depending on
    /// platform.
    /// \param size The panel size. The value \c wxDefaultSize
    /// indicates a default size, chosen by either the windowing system or
    /// wxWidgets, depending on platform.
    /// \param style The window style. See \c wxDialog.
    explicit ZonesConfigDialogImpl(wxWindow* parent,
        dashboardsk_pi* dsk_pi = nullptr, wxWindowID id = wxID_ANY,
        const wxString& value = wxEmptyString,
        const wxString& title = _("SignalK Zones Configuration"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxSize(500, 300),
        long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

    /// Destructor
    ~ZonesConfigDialogImpl() = default;

    /// Get the zones defined in the dialog
    ///
    /// \return vector of zones
    vector<Zone> GetZones();

protected:
    /// Original zone definition to revert to on Cancel
    vector<Zone> m_original_zones;
    /// Current zone configuration state
    vector<Zone> m_zones;
    /// Pointer to the currently edited zone
    Zone* m_edited_zone;

    /// Enable/disable controls based on the state of the form
    void EnableControls();

    /// Fill the controls with zone values
    void FillZoneControls();

    /// Update the zone list widget
    void UpdateList();

    /// Event handler for zone list widget
    ///
    /// \param event The event data
    void m_lbZonesOnListBox(wxCommandEvent& event);

    /// Event handler for zone addition button
    ///
    /// \param event The event data
    void m_bpAddOnButtonClick(wxCommandEvent& event);

    /// Event handler for zone removal button
    ///
    /// \param event The event data
    void m_bpRemoveOnButtonClick(wxCommandEvent& event);

    /// Event handler for the zone lower limit spin control
    ///
    /// \param event The event data
    void m_spLowerOnSpinCtrlDouble(wxSpinDoubleEvent& event);

    /// Event handler for the zone upper limit spin control
    ///
    /// \param event The event data
    void m_spUpperOnSpinCtrlDouble(wxSpinDoubleEvent& event);

    /// Event handler for the zone alarm state dropdown
    ///
    /// \param event The event data
    void m_choiceStateOnChoice(wxCommandEvent& event);

    /// Event handler for the cancel button
    ///
    /// \param event The event data
    void m_sdbSizerButtonsOnCancelButtonClick(wxCommandEvent& event);

    /// Event handler for the OK button
    ///
    /// \param event The event data
    void m_sdbSizerButtonsOnOKButtonClick(wxCommandEvent& event);
};

PLUGIN_END_NAMESPACE

#endif // _DASHBOARDSKGUIIMPL_H_
