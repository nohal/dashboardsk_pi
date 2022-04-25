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

#include "dashboardskguiimpl.h"
#include "dashboardsk.h"
#include "dashboardsk_pi.h"
#include <wx/choicdlg.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>
#include <wx/windowptr.h>

PLUGIN_BEGIN_NAMESPACE

MainConfigFrameImpl::MainConfigFrameImpl(dashboardsk_pi* dsk_pi,
    wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style)
    : MainConfigFrame(parent, id, title, pos, size, style)
    , m_edited_dashboard(nullptr)
    , m_edited_instrument(nullptr)
{
    m_dsk_pi = dsk_pi;
    m_chAnchor->Clear();
    m_chAnchor->Append(Dashboard::AnchorEdgeLabels);
    m_orig_config = m_dsk_pi->GetDSK()->GenerateJSONConfig();
    m_tSelf->SetValue(m_dsk_pi->GetDSK()->Self());
    m_comboDashboard->Append(m_dsk_pi->GetDSK()->GetDashboardNames());

    m_bpAddButton->SetBitmap(wxBitmapBundle::FromSVGFile(
        m_dsk_pi->GetDataDir() + "plus.svg", wxSize(BMP_SZ, BMP_SZ)));
    m_bpRemoveButton->SetBitmap(wxBitmapBundle::FromSVGFile(
        m_dsk_pi->GetDataDir() + "minus.svg", wxSize(BMP_SZ, BMP_SZ)));
    m_bpSaveInstrButton->SetBitmap(wxBitmapBundle::FromSVGFile(
        m_dsk_pi->GetDataDir() + "save.svg", wxSize(BMP_SZ, BMP_SZ)));
    m_bpImportInstrButton->SetBitmap(wxBitmapBundle::FromSVGFile(
        m_dsk_pi->GetDataDir() + "open.svg", wxSize(BMP_SZ, BMP_SZ)));
    m_bpMoveUpButton->SetBitmap(wxBitmapBundle::FromSVGFile(
        m_dsk_pi->GetDataDir() + "up.svg", wxSize(BMP_SZ, BMP_SZ)));
    m_bpMoveDownButton->SetBitmap(wxBitmapBundle::FromSVGFile(
        m_dsk_pi->GetDataDir() + "down.svg", wxSize(BMP_SZ, BMP_SZ)));
    if (m_comboDashboard->GetCount() > 0) {
        m_comboDashboard->SetSelection(0);
        m_edited_dashboard = m_dsk_pi->GetDSK()->GetDashboard(
            m_comboDashboard->GetSelection());
    }
    FillInstrumentList();
    EnableItemsForSelectedDashboard();
}

void MainConfigFrameImpl::EnableItems(bool dashboard_selection, bool instr_list,
    bool dashboard_config, bool instr_config)
{
    if (dashboard_selection) {
        // Dashboard selection
        m_comboDashboard->Enable();
        m_btnRemoveDashboard->Enable(
            m_comboDashboard->GetSelection() != wxNOT_FOUND);
    } else {
        m_comboDashboard->Disable();
        m_btnRemoveDashboard->Disable();
    }
    if (instr_list) {
        // Instrument list
        m_bpAddButton->Enable();
        m_bpRemoveButton->Enable();
        m_bpSaveInstrButton->Enable();
        m_bpImportInstrButton->Enable();
        m_bpMoveUpButton->Enable();
        m_bpMoveDownButton->Enable();
        m_lbInstruments->Enable();
        m_stInstruments->Enable();
    } else {
        m_bpAddButton->Disable();
        m_bpRemoveButton->Disable();
        m_bpSaveInstrButton->Disable();
        m_bpImportInstrButton->Disable();
        m_bpMoveUpButton->Disable();
        m_bpMoveDownButton->Disable();
        m_lbInstruments->Disable();
        m_stInstruments->Disable();
    }
    if (dashboard_config) {
        // Dashboard config
        m_cbEnabled->Enable();
        m_chAnchor->Enable();
        m_spCanvas->Enable();
        m_spOffsetX->Enable();
        m_spOffsetY->Enable();
        m_spSpacingH->Enable();
        m_spSpacingV->Enable();
        m_stAnchor->Enable();
        m_stCanvas->Enable();
        m_stOffsetX->Enable();
        m_stOffsetY->Enable();
        m_stSpacingH->Enable();
        m_stSpacingV->Enable();
    } else {
        m_cbEnabled->Disable();
        m_chAnchor->Disable();
        m_spCanvas->Disable();
        m_spOffsetX->Disable();
        m_spOffsetY->Disable();
        m_spSpacingH->Disable();
        m_spSpacingV->Disable();
        m_stAnchor->Disable();
        m_stCanvas->Disable();
        m_stOffsetX->Disable();
        m_stOffsetY->Disable();
        m_stSpacingH->Disable();
        m_stSpacingV->Disable();
    }
    if (instr_config) {
        // Instrument Config
        m_tName->Enable();
        m_tTitle->Enable();
        m_stInstrument->Enable();
        m_stName->Enable();
        m_stTitle->Enable();
        m_stTimeout->Enable();
        m_spTimeout->Enable();
        m_stSeconds->Enable();
    } else {
        m_tName->Disable();
        m_tTitle->Disable();
        m_stInstrument->Disable();
        m_stName->Disable();
        m_stTitle->Disable();
        m_stTimeout->Disable();
        m_spTimeout->Disable();
        m_stSeconds->Disable();
    }
}

void MainConfigFrameImpl::EnableItemsForSelectedDashboard()
{
    if (m_comboDashboard->GetSelection() != wxNOT_FOUND) {
        bool has_instrs
            = false; // TODO: Does it actually have instruments or not?
        EnableItems(m_comboDashboard->GetCount() > 0, has_instrs, true, false);
        m_bpAddButton
            ->Enable(); // We can always add a new instrument to a dashboard
        m_stInstruments->Enable();
        m_lbInstruments->Enable();
    } else {
        EnableItems(m_comboDashboard->GetCount() > 0, false, false, false);
    }
    FillDashboardDetails();
}

void MainConfigFrameImpl::m_btnNewDashboardOnButtonClick(wxCommandEvent& event)
{
    UpdateEditedInstrument();
    ;

    wxWindowPtr<wxTextEntryDialog> dlg(new wxTextEntryDialog(this,
        _("Enter name for the newly created dashboard"),
        _("Create new dashboard"), _("New dashboard")));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            m_edited_dashboard = m_dsk_pi->GetDSK()->AddDashboard();
            m_edited_instrument = nullptr;
            m_edited_dashboard->SetName(dlg->GetValue());
            m_comboDashboard->Append(m_edited_dashboard->GetName());
            m_comboDashboard->SetSelection(m_comboDashboard->GetCount() - 1);
            FillInstrumentList();
            FillInstrumentDetails();
            EnableItemsForSelectedDashboard();
        }
    });
    event.Skip();
}

void MainConfigFrameImpl::m_btnRemoveDashboardOnButtonClick(
    wxCommandEvent& event)
{
    wxWindowPtr<wxMessageDialog> dlg(new wxMessageDialog(this,
        _("Do you really want to delete dashboard") + " "
            + m_comboDashboard->GetString(m_comboDashboard->GetSelection())
            + "?",
        _("Delete dashboard"), wxYES_NO | wxICON_QUESTION));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_YES) {
            int i = m_comboDashboard->GetSelection();
            m_comboDashboard->Delete(i);
            m_dsk_pi->GetDSK()->DeleteDashboard(i);
            m_edited_instrument = nullptr;
            m_edited_dashboard = nullptr;
            m_comboDashboard->SetSelection(
                wxMin(i, m_comboDashboard->GetCount() - 1));
            FillInstrumentList();
            FillInstrumentDetails();
            EnableItemsForSelectedDashboard();
        }
    });

    event.Skip();
}

void MainConfigFrameImpl::FillInstrumentList()
{
    m_lbInstruments->Clear();
    if (!m_edited_dashboard) {
        return;
    }

    m_lbInstruments->Append(m_edited_dashboard->GetInstrumentNames());
}

void MainConfigFrameImpl::m_comboDashboardOnChoice(wxCommandEvent& event)
{
    UpdateEditedInstrument();
    m_edited_instrument = nullptr;
    m_edited_dashboard
        = m_dsk_pi->GetDSK()->GetDashboard(m_comboDashboard->GetSelection());
    FillInstrumentDetails();
    FillInstrumentList();
    EnableItemsForSelectedDashboard();
    event.Skip();
}

void MainConfigFrameImpl::EnableInstrumentListButtons()
{
    if (m_lbInstruments->GetSelection() != wxNOT_FOUND) {
        m_bpRemoveButton->Enable();
        m_bpSaveInstrButton->Enable();
        if (m_lbInstruments->GetSelection() > 0) {
            m_bpMoveUpButton->Enable();
        } else {
            m_bpMoveUpButton->Disable();
        }
        if (m_lbInstruments->GetSelection() < m_lbInstruments->GetCount() - 1) {
            m_bpMoveDownButton->Enable();
        } else {
            m_bpMoveDownButton->Disable();
        }
    } else {
        m_bpRemoveButton->Disable();
        m_bpSaveInstrButton->Disable();
    }
    m_bpImportInstrButton->Enable();
}

/// Fill the instrument detail form. We must support all the control types
/// defined in #dskConfigCtrl enum here.
void MainConfigFrameImpl::FillInstrumentDetails()
{
    SettingsItemSizer->Clear(true); // Clear the sizer and delete all the
                                    // controls it contains, we always do that
    if (!m_edited_instrument) {
        // Just clear everything
        m_tName->SetValue(wxEmptyString);
        m_tTitle->SetValue(wxEmptyString);
        m_stInstrument->SetLabel(wxEmptyString);
        m_spTimeout->SetValue(0);
        m_swConfig->Layout();
        m_panelConfig->Layout();
        return;
    }
    m_tName->SetValue(m_edited_instrument->GetName());
    m_tTitle->SetValue(m_edited_instrument->GetTitle());
    m_stInstrument->SetLabel(m_edited_instrument->GetDisplayType());
    m_spTimeout->SetValue(m_edited_instrument->GetTimeout());

    for (auto ctrl : m_edited_instrument->ConfigControls()) {
        SettingsItemSizer->Add(
            new wxStaticText(m_swConfig, wxID_ANY, ctrl.description,
                wxDefaultPosition, wxDefaultSize, 0),
            0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        switch (ctrl.control) {
        case dskConfigCtrl::TextCtrl: {
            SettingsItemSizer->Add(
                new wxTextCtrl(m_swConfig, wxID_ANY,
                    m_edited_instrument->GetStringSetting(ctrl.key),
                    wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator,
                    ctrl.key),
                0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        case dskConfigCtrl::ColourPickerCtrl: {
            wxColor clr = m_edited_instrument->GetColorSetting(ctrl.key);
            SettingsItemSizer->Add(
                new wxColourPickerCtrl(m_swConfig, wxID_ANY, clr,
                    wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE,
                    wxDefaultValidator, ctrl.key),
                0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        case dskConfigCtrl::SpinCtrl: {
            int min = -99999;
            int max = 99999;
            if (!ctrl.control_settings.IsEmpty()) {
                int pos = 0;
                wxStringTokenizer tokenizer(ctrl.control_settings, ";");
                while (tokenizer.HasMoreTokens()) {
                    wxString token = tokenizer.GetNextToken();
                    switch (pos) {
                    case 0:
                        if (!token.ToInt(&min)) {
                            min = -99999;
                        }
                        break;
                    case 1:
                        if (!token.ToInt(&max)) {
                            max = 99999;
                        }
                        break;
                    }
                    pos++;
                }
            }
            int i = m_edited_instrument->GetIntSetting(ctrl.key);
            i = wxMin(wxMax(i, min), max); // Adjust to limits

            SettingsItemSizer->Add(
                new wxSpinCtrl(m_swConfig, wxID_ANY, wxEmptyString,
                    wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max,
                    i, ctrl.key),
                0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        case dskConfigCtrl::ChoiceCtrl: {
            int pos = 0;
            int sel = m_edited_instrument->GetIntSetting(ctrl.key);
            wxArrayString as;
            wxStringTokenizer tokenizer(ctrl.control_settings, ";");
            while (tokenizer.HasMoreTokens()) {
                wxString token = tokenizer.GetNextToken();
                as.Add(token);
                if (token == m_edited_instrument->GetStringSetting(ctrl.key)) {
                    sel = pos;
                }
                pos++;
            }
            wxChoice* ch = new wxChoice(m_swConfig, wxID_ANY, wxDefaultPosition,
                wxDefaultSize, 0, NULL, 0, wxDefaultValidator, ctrl.key);
            ch->Append(as);
            ch->SetSelection(sel);
            SettingsItemSizer->Add(
                ch, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        case dskConfigCtrl::SignalKKeyCtrl: {
            SKKeyCtrlImpl* skk = new SKKeyCtrlImpl(m_swConfig);
            skk->SetName(ctrl.key);
            skk->SetValue(m_edited_instrument->GetStringSetting(ctrl.key));
            skk->SetSKTree(m_dsk_pi->GetDSK()->GetSignalKTree());
            SettingsItemSizer->Add(
                skk, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        case dskConfigCtrl::SignalKZonesCtrl: {
            SKZonesCtrlImpl* skz = new SKZonesCtrlImpl(m_swConfig, m_dsk_pi);
            skz->SetName(ctrl.key);
            skz->SetValue(m_edited_instrument->GetStringSetting(ctrl.key));
            SettingsItemSizer->Add(
                skz, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        default:
            // Dunno, add spacer
            SettingsItemSizer->Add(0, 0, 1, wxEXPAND, 5);
        }
    }
    m_swConfig->Layout();
    m_panelConfig->Layout();
}

const wxString MainConfigFrameImpl::ExtractValue(wxWindow* ctrl)
{
    if (wxString(ctrl->GetClassInfo()->GetClassName()) == "wxTextCtrl") {
        return ((wxTextCtrl*)ctrl)->GetValue();
    } else if (wxString(ctrl->GetClassInfo()->GetClassName())
        == "wxColourPickerCtrl") {
        return ((wxColourPickerCtrl*)ctrl)
            ->GetColour()
            .GetAsString(wxC2S_HTML_SYNTAX);
    } else if (wxString(ctrl->GetClassInfo()->GetClassName()) == "wxSpinCtrl") {
        return wxString::Format("%i", ((wxSpinCtrl*)ctrl)->GetValue());
    } else if (wxString(ctrl->GetClassInfo()->GetClassName()) == "wxChoice") {
        return wxString::Format("%i", ((wxChoice*)ctrl)->GetSelection());
    } else if (wxString(ctrl->GetClassInfo()->GetClassName())
        == "SKKeyCtrlImpl") {
        return ((SKKeyCtrlImpl*)ctrl)->GetValue();
    } else if (wxString(ctrl->GetClassInfo()->GetClassName())
        == "SKZonesCtrlImpl") {
        return ((SKZonesCtrlImpl*)ctrl)->GetValue();
    } // TODO: Other supported controls defined in dskConfigCtrl
    LOG_VERBOSE("DashboardSK: Unknown control class %s",
        ctrl->GetClassInfo()->GetClassName());
    return wxEmptyString; // We do not know the control
}

void MainConfigFrameImpl::m_bpAddButtonOnButtonClick(wxCommandEvent& event)
{
    if (!m_edited_dashboard) {
        LOG_VERBOSE("DashboardSK_pi: Trying to add instrument while "
                    "m_edited_dashboard is NULL, we be crazy...");
        return;
    }
    UpdateEditedInstrument();

    wxWindowPtr<wxSingleChoiceDialog> dlg(
        new wxSingleChoiceDialog(this, _("Select type of the new instrument"),
            _("Add new instrument"), m_dsk_pi->GetDSK()->GetInstrumentTypes()));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            m_edited_instrument = m_dsk_pi->GetDSK()->CreateInstrumentInstance(
                dlg->GetSelection(), m_edited_dashboard);
            m_edited_dashboard->AddInstrument(m_edited_instrument);
            m_lbInstruments->Append(m_edited_instrument->GetName());
            m_lbInstruments->Select(m_lbInstruments->GetCount() - 1);
            FillInstrumentDetails();
            EnableItems(true, true, true, true);
            EnableInstrumentListButtons();
        }
    });

    event.Skip();
}

void MainConfigFrameImpl::m_lbInstrumentsOnListBox(wxCommandEvent& event)
{
    UpdateEditedInstrument();
    m_edited_instrument
        = m_edited_dashboard->GetInstrument(m_lbInstruments->GetSelection());
    FillInstrumentDetails();
    EnableItems(true, true, true, true);
    EnableInstrumentListButtons();
    event.Skip();
}

void MainConfigFrameImpl::m_bpRemoveButtonOnButtonClick(wxCommandEvent& event)
{
    int i = m_lbInstruments->GetSelection();
    m_edited_instrument = nullptr;
    // FillInstrumentDetails();
    m_edited_dashboard->DeleteInstrument(i);
    m_lbInstruments->Delete(i);
    i--;
    i = wxMin(i, m_lbInstruments->GetCount() - 1);
    m_lbInstruments->SetSelection(i);
    m_edited_instrument
        = m_edited_dashboard->GetInstrument(m_lbInstruments->GetSelection());
    FillInstrumentDetails();
    EnableItemsForSelectedDashboard();
    EnableInstrumentListButtons();
    event.Skip();
}

void MainConfigFrameImpl::m_bpMoveUpButtonOnButtonClick(wxCommandEvent& event)
{
    UpdateEditedInstrument();
    int pos = m_lbInstruments->GetSelection();
    if (pos == wxNOT_FOUND) {
        event.Skip();
        return;
    }
    wxString val = m_lbInstruments->GetString(pos);
    m_edited_dashboard->MoveInstrument(pos, -1);
    m_lbInstruments->Delete(pos);
    m_lbInstruments->Insert(val, pos - 1);
    m_lbInstruments->SetSelection(pos - 1);
    event.Skip();
}

void MainConfigFrameImpl::m_bpMoveDownButtonOnButtonClick(wxCommandEvent& event)
{
    UpdateEditedInstrument();
    int pos = m_lbInstruments->GetSelection();
    if (pos == wxNOT_FOUND) {
        event.Skip();
        return;
    }
    wxString val = m_lbInstruments->GetString(pos);
    m_edited_dashboard->MoveInstrument(pos, 1);
    m_lbInstruments->Delete(pos);
    m_lbInstruments->Insert(val, pos + 1);
    m_lbInstruments->SetSelection(pos + 1);
    event.Skip();
}

void MainConfigFrameImpl::m_sdbSizerOnOKButtonClick(wxCommandEvent& event)
{
    UpdateEditedDashboard();
    UpdateEditedInstrument();
    m_dsk_pi->GetDSK()->SetSelf(m_tSelf->GetValue());
    m_dsk_pi->SaveConfig();
    event.Skip();
}

void MainConfigFrameImpl::m_sdbSizerOnCancelButtonClick(wxCommandEvent& event)
{
    m_dsk_pi->GetDSK()->ReadConfig(m_orig_config);
    event.Skip();
}

void MainConfigFrameImpl::UpdateEditedInstrument()
{
    if (!m_edited_instrument) {
        return;
    }
    config_map_t map;
    m_edited_instrument->SetSetting("name", m_tName->GetValue());
    m_edited_instrument->SetSetting("title", m_tTitle->GetValue());
    m_edited_instrument->SetSetting("allowed_age", m_spTimeout->GetValue());
    // And now the instrument specific parameters that we generated for the
    // instrument
    int child = 0;
    for (auto ctrl : SettingsItemSizer->GetChildren()) {
        ++child;
        if (child % 2 == 0) { // Every other control is a user input (The one
                              // before is the wxStaticText label)
            m_edited_instrument->SetSetting(
                ctrl->GetWindow()->GetName(), ExtractValue(ctrl->GetWindow()));
        }
    }
}

void MainConfigFrameImpl::FillDashboardDetails()
{
    if (!m_edited_dashboard) {
        return;
    }
    m_spCanvas->SetValue(m_edited_dashboard->GetCanvasNr());
    m_chAnchor->SetSelection((int)m_edited_dashboard->GetAnchorEdge());
    m_spSpacingH->SetValue(m_edited_dashboard->GetHSpacing());
    m_spSpacingV->SetValue(m_edited_dashboard->GetVSpacing());
    m_spOffsetX->SetValue(m_edited_dashboard->GetHOffset());
    m_spOffsetY->SetValue(m_edited_dashboard->GetVOffset());
    m_cbEnabled->SetValue(m_edited_dashboard->Enabled());
}

void MainConfigFrameImpl::UpdateEditedDashboard()
{
    if (!m_edited_dashboard) {
        return;
    }
    m_edited_dashboard->SetCanvasNr((size_t)m_spCanvas->GetValue());
    m_edited_dashboard->SetAnchorEdge(
        (Dashboard::anchor_edge)(m_chAnchor->GetSelection()));
    m_edited_dashboard->SetHSpacing(m_spSpacingH->GetValue());
    m_edited_dashboard->SetVSpacing(m_spSpacingV->GetValue());
    m_edited_dashboard->SetHOffset(m_spOffsetX->GetValue());
    m_edited_dashboard->SetVOffset(m_spOffsetY->GetValue());
    m_edited_dashboard->Enable(m_cbEnabled->GetValue());
}

void MainConfigFrameImpl::m_btnSignalKOnButtonClick(wxCommandEvent& event)
{
    wxWindowPtr<SKDataTreeImpl> dlg(new SKDataTreeImpl(this));
    dlg->SetCodeSKTree(m_dsk_pi->GetDSK());
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            // Nufin, but we perhals could for example look where the cursor was
            // and fill the SK key field from it
        }
    });
}

void SKDataTreeImpl::SetCodeSKTree(DashboardSK* dsk)
{
    m_sdbSizerBtnsCancel->Hide();
    m_scintillaCode->SetReadOnly(false);
    m_scintillaCode->SetText(dsk->GetSignalKTreeText());
    m_scintillaCode->SetReadOnly(true);
}

wxIMPLEMENT_DYNAMIC_CLASS_XTI(SKKeyCtrlImpl, wxPanel, "dashboardsdguiimpl.h");

void SKKeyCtrlImpl::m_btnSelectOnButtonClick(wxCommandEvent& event)
{
    wxWindowPtr<SKPathBrowserImpl> dlg(
        new SKPathBrowserImpl(this, wxID_ANY, m_tSKKey->GetValue()));
    dlg->SetSKTree(m_sk_tree);
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            m_tSKKey->SetValue(dlg->GetSKPath());
        }
    });
    event.Skip();
}

wxIMPLEMENT_DYNAMIC_CLASS_XTI(SKZonesCtrlImpl, wxPanel, "dashboardsdguiimpl.h");

void SKZonesCtrlImpl::m_btnSelectOnButtonClick(wxCommandEvent& event)
{
    wxWindowPtr<ZonesConfigDialogImpl> dlg(new ZonesConfigDialogImpl(
        this, m_dsk_pi, wxID_ANY, m_tZones->GetValue()));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            m_tZones->SetValue(Zone::ZonesToString(dlg->GetZones()));
        }
    });
    event.Skip();
}

ZonesConfigDialogImpl::ZonesConfigDialogImpl(wxWindow* parent,
    dashboardsk_pi* dsk_pi, wxWindowID id, const wxString& value,
    const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : ZonesConfigDialog(parent, id, title, pos, size, style)
{
    m_dsk_pi = dsk_pi;
    if (m_dsk_pi) {
        m_bpAdd->SetBitmap(wxBitmapBundle::FromSVGFile(
            m_dsk_pi->GetDataDir() + "plus.svg", wxSize(BMP_SZ, BMP_SZ)));
        m_bpRemove->SetBitmap(wxBitmapBundle::FromSVGFile(
            m_dsk_pi->GetDataDir() + "minus.svg", wxSize(BMP_SZ, BMP_SZ)));
    }
    m_zones = Zone::ParseZonesFromString(value);
    if (!m_zones.empty()) {
        UpdateList();
        m_lbZones->SetSelection(0);
        m_edited_zone = &m_zones.at(m_lbZones->GetSelection());
    }
    FillZoneControls();
    EnableControls();
};

PLUGIN_END_NAMESPACE