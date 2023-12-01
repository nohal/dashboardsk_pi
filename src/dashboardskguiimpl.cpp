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
#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"
#include <wx/choicdlg.h>
#include <wx/dialog.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/tokenzr.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/windowptr.h>

PLUGIN_BEGIN_NAMESPACE

//====================================
// MainConfigFrameImpl
//====================================

MainConfigFrameImpl::MainConfigFrameImpl(dashboardsk_pi* dsk_pi,
    wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos,
    const wxSize& size, long style)
    : MainConfigFrame(parent, id, title, pos, size, style)
    , m_edited_dashboard(nullptr)
    , m_edited_instrument(nullptr)
{
#if (not __WXQT__ and wxCHECK_VERSION(3, 1, 0))
    SetSize(FromDIP(GetSize()));
#endif
    m_dsk_pi = dsk_pi;
    m_chAnchor->Clear();
#if wxCHECK_VERSION(3, 1, 0)
    m_chAnchor->Append(Dashboard::AnchorEdgeLabels);
#else
    for (auto lbl : Dashboard::AnchorEdgeLabels) {
        m_chAnchor->Append(wxString(lbl));
    }
#endif
    m_orig_config = m_dsk_pi->GetDSK()->GenerateJSONConfig();
    m_tSelf->SetValue(m_dsk_pi->GetDSK()->Self());

#if (wxCHECK_VERSION(3, 1, 6))
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
    m_btnSignalK->SetBitmap(wxBitmapBundle::FromSVGFile(
        m_dsk_pi->GetDataDir() + "signalk_button.svg", wxSize(BMP_SZ, BMP_SZ)));
#else
    m_bpAddButton->SetBitmap(GetBitmapFromSVGFile(
        m_dsk_pi->GetDataDir() + "plus.svg", BMP_SZ, BMP_SZ));
    m_bpRemoveButton->SetBitmap(GetBitmapFromSVGFile(
        m_dsk_pi->GetDataDir() + "minus.svg", BMP_SZ, BMP_SZ));
    m_bpSaveInstrButton->SetBitmap(GetBitmapFromSVGFile(
        m_dsk_pi->GetDataDir() + "save.svg", BMP_SZ, BMP_SZ));
    m_bpImportInstrButton->SetBitmap(GetBitmapFromSVGFile(
        m_dsk_pi->GetDataDir() + "open.svg", BMP_SZ, BMP_SZ));
    m_bpMoveUpButton->SetBitmap(GetBitmapFromSVGFile(
        m_dsk_pi->GetDataDir() + "up.svg", BMP_SZ, BMP_SZ));
    m_bpMoveDownButton->SetBitmap(GetBitmapFromSVGFile(
        m_dsk_pi->GetDataDir() + "down.svg", BMP_SZ, BMP_SZ));
#endif
    DimeWindow(this);
    FillForm();
}

void MainConfigFrameImpl::FillForm(bool select_last)
{
    m_comboDashboard->Clear();
    m_comboDashboard->Append(m_dsk_pi->GetDSK()->GetDashboardNames());
    if (m_comboDashboard->GetCount() > 0) {
        if (select_last) {
            m_comboDashboard->SetSelection(m_comboDashboard->GetCount() - 1);
        } else {
            m_comboDashboard->SetSelection(0);
        }
        m_edited_dashboard = m_dsk_pi->GetDSK()->GetDashboard(
            m_comboDashboard->GetSelection());
    }
    FillInstrumentList();
    FillInstrumentDetails();
    EnableItemsForSelectedDashboard();
    EnableInstrumentListButtons();
}

void MainConfigFrameImpl::EnableItems(bool dashboard_selection, bool instr_list,
    bool dashboard_config, bool instr_config)
{
    if (dashboard_selection) {
        // Dashboard selection
        m_comboDashboard->Enable();
        m_btnRemoveDashboard->Enable(
            m_comboDashboard->GetSelection() != wxNOT_FOUND);
        m_btnRenameDashboard->Enable(
            m_comboDashboard->GetSelection() != wxNOT_FOUND);
    } else {
        m_comboDashboard->Disable();
        m_btnRemoveDashboard->Disable();
        m_btnRenameDashboard->Disable();
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
        m_spPage->Enable();
        m_spOffsetX->Enable();
        m_spOffsetY->Enable();
        m_spSpacingH->Enable();
        m_spSpacingV->Enable();
        m_stAnchor->Enable();
        m_stCanvas->Enable();
        m_stPage->Enable();
        m_stOffsetX->Enable();
        m_stOffsetY->Enable();
        m_stSpacingH->Enable();
        m_stSpacingV->Enable();
    } else {
        m_cbEnabled->Disable();
        m_chAnchor->Disable();
        m_spCanvas->Disable();
        m_spPage->Disable();
        m_spOffsetX->Disable();
        m_spOffsetY->Disable();
        m_spSpacingH->Disable();
        m_spSpacingV->Disable();
        m_stAnchor->Disable();
        m_stCanvas->Disable();
        m_stPage->Disable();
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
        m_edited_dashboard = m_dsk_pi->GetDSK()->GetDashboard(
            m_comboDashboard->GetSelection());
        EnableItems(m_comboDashboard->GetCount() > 0,
            m_edited_dashboard->HasInstruments(), true,
            m_edited_dashboard->HasInstruments());
        if (m_edited_dashboard->HasInstruments() && !m_edited_instrument) {
            m_lbInstruments->SetSelection(0);
            m_edited_instrument = m_edited_dashboard->GetInstrument(0);
            FillInstrumentDetails();
        }
        m_bpAddButton
            ->Enable(); // We can always add a new instrument to a dashboard
        m_bpImportInstrButton
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

    wxWindowPtr<wxTextEntryDialog> dlg(new wxTextEntryDialog(this,
        _("Enter name for the newly created dashboard"),
        _("Create new dashboard"), _("New dashboard")));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            m_edited_dashboard = m_dsk_pi->GetDSK()->AddDashboard();
            m_edited_instrument = nullptr;
            m_edited_dashboard->SetName(dlg->GetValue());
            FillForm(true);
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
            FillInstrumentList();
            FillInstrumentDetails();
            if (m_comboDashboard->GetCount() > 0) {
                i = wxMin(i, m_comboDashboard->GetCount() - 1);
                m_comboDashboard->SetSelection(i);
                m_edited_dashboard = m_dsk_pi->GetDSK()->GetDashboard(i);
                if (m_edited_dashboard->HasInstruments()) {
                    FillInstrumentList();
                    m_lbInstruments->SetSelection(0);
                    m_edited_instrument = m_edited_dashboard->GetInstrument(0);
                    FillInstrumentDetails();
                }
            }
            EnableItemsForSelectedDashboard();
        }
    });

    event.Skip();
}

void MainConfigFrameImpl::m_btnRenameDashboardOnButtonClick(
    wxCommandEvent& event)
{
    wxWindowPtr<wxTextEntryDialog> dlg(
        new wxTextEntryDialog(this, _("Enter new name for the dashboard"),
            _("Rename dashboard"), m_edited_dashboard->GetName()));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            m_edited_dashboard->SetName(dlg->GetValue());
            m_comboDashboard->SetString(m_comboDashboard->GetSelection(),
                m_edited_dashboard->GetName());
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
        if ((unsigned)m_lbInstruments->GetSelection()
            < m_lbInstruments->GetCount() - 1) {
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

    for (const auto& ctrl : m_edited_instrument->ConfigControls()) {
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
#if (wxCHECK_VERSION(3, 1, 6))
                        if (!token.ToInt(&min)) {
                            min = -99999;
                        }
#else
                        min = wxAtoi(token);
#endif
                        break;
                    case 1:
#if (wxCHECK_VERSION(3, 1, 6))
                        if (!token.ToInt(&max)) {
                            max = 99999;
                        }
#else
                        max = wxAtoi(token);
#endif
                        break;
                    default:
                        LOG_VERBOSE("Uncovered SpinCtrl parameter %i", pos);
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
        case dskConfigCtrl::SpinCtrlDouble: {
            double min = -99999.9;
            double max = 99999.9;
            if (!ctrl.control_settings.IsEmpty()) {
                int pos = 0;
                wxStringTokenizer tokenizer(ctrl.control_settings, ";");
                while (tokenizer.HasMoreTokens()) {
                    wxString token = tokenizer.GetNextToken();
                    switch (pos) {
                    case 0:
#if (wxCHECK_VERSION(3, 1, 6))
                        if (!token.ToDouble(&min)) {
                            min = -99999.9;
                        }
#else
                        min = wxAtof(token);
#endif
                        break;
                    case 1:
#if (wxCHECK_VERSION(3, 1, 6))
                        if (!token.ToDouble(&max)) {
                            max = 99999.9;
                        }
#else
                        max = wxAtof(token);
#endif
                        break;
                    default:
                        LOG_VERBOSE(
                            "Uncovered SpinCtrlDouble parameter %i", pos);
                    }
                    pos++;
                }
            }
            double i = m_edited_instrument->GetDoubleSetting(ctrl.key);
            i = wxMin(wxMax(i, min), max); // Adjust to limits

            SettingsItemSizer->Add(
                new wxSpinCtrlDouble(m_swConfig, wxID_ANY, wxEmptyString,
                    wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max,
                    i, 1, ctrl.key),
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
            auto* ch = new wxChoice(m_swConfig, wxID_ANY, wxDefaultPosition,
                wxDefaultSize, 0, nullptr, 0, wxDefaultValidator, ctrl.key);
            ch->Append(as);
            ch->SetSelection(sel);
            SettingsItemSizer->Add(
                ch, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        case dskConfigCtrl::SignalKKeyCtrl: {
            auto* skk = new SKKeyCtrlImpl(m_swConfig);
            skk->SetName(ctrl.key);
            skk->SetValue(m_edited_instrument->GetStringSetting(ctrl.key));
            skk->SetSelf(m_dsk_pi->GetDSK()->Self());
            skk->SetSKTree(m_dsk_pi->GetDSK()->GetSignalKTree());
            SettingsItemSizer->Add(
                skk, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);
            break;
        }
        case dskConfigCtrl::SignalKZonesCtrl: {
            auto* skz = new SKZonesCtrlImpl(m_swConfig, m_dsk_pi);
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

wxString MainConfigFrameImpl::ExtractValue(wxWindow* ctrl)
{
    if (wxString(ctrl->GetClassInfo()->GetClassName()) == "wxTextCtrl") {
        return static_cast<wxTextCtrl*>(ctrl)->GetValue();
    } else if (wxString(ctrl->GetClassInfo()->GetClassName())
        == "wxColourPickerCtrl") {
        return static_cast<wxColourPickerCtrl*>(ctrl)->GetColour().GetAsString(
            wxC2S_HTML_SYNTAX);
    } else if (wxString(ctrl->GetClassInfo()->GetClassName()) == "wxSpinCtrl") {
        return wxString::Format(
            "%i", static_cast<wxSpinCtrl*>(ctrl)->GetValue());
    } else if (wxString(ctrl->GetClassInfo()->GetClassName()) == "wxChoice") {
        return wxString::Format(
            "%i", static_cast<wxChoice*>(ctrl)->GetSelection());
    } else if (wxString(ctrl->GetClassInfo()->GetClassName())
        == "SKKeyCtrlImpl") {
        return static_cast<SKKeyCtrlImpl*>(ctrl)->GetValue();
    } else if (wxString(ctrl->GetClassInfo()->GetClassName())
        == "SKZonesCtrlImpl") {
        return static_cast<SKZonesCtrlImpl*>(ctrl)->GetValue();
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
            FillInstrumentList();
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
    int i = m_lbInstruments->GetSelection();
    FillInstrumentList();
    m_lbInstruments->SetSelection(i);
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
    FillInstrumentList();
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
    EnableInstrumentListButtons();
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
    EnableInstrumentListButtons();
    event.Skip();
}

void MainConfigFrameImpl::m_sdbSizerOnOKButtonClick(wxCommandEvent& event)
{
    UpdateEditedDashboard();
    UpdateEditedInstrument();
    m_dsk_pi->GetDSK()->ResetPagers();
    m_dsk_pi->GetDSK()->SetSelf(m_tSelf->GetValue());
    m_dsk_pi->GetDSK()->ForceRedraw();
    m_dsk_pi->SaveConfig();
    event.Skip();
}

void MainConfigFrameImpl::m_sdbSizerOnCancelButtonClick(wxCommandEvent& event)
{
    m_dsk_pi->GetDSK()->ReadConfig(m_orig_config);
    event.Skip();
}

void MainConfigFrameImpl::m_btnCfgEditOnButtonClick(wxCommandEvent& event)
{
    m_dsk_pi->GetDSK()->Freeze(true);
    wxWindowPtr<SKDataTreeImpl> dlg(new SKDataTreeImpl(this));
    dlg->SetTitle(_("Configuration data (Edit carefully!)"));
    wxString s;
    wxJSONWriter w;
    w.Write(m_dsk_pi->GetDSK()->GenerateJSONConfig(), s);
    dlg->SetCodeConfig(s);
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            wxJSONReader r(wxJSONREADER_STRICT);
            wxJSONValue v;
            int ret = r.Parse(dlg->GetValue(), &v);
            if (0 == ret && v.HasMember("signalk")) {
                m_edited_instrument = nullptr;
                m_edited_dashboard = nullptr;
                m_dsk_pi->GetDSK()->ReadConfig(v);
                FillForm();
                // TODO: The above is VERY fragile, we should add as many checks
                // as possible both here and to the editing form before screwing
                // up the configuration (Although on Cancel click we load the
                // old one, which is not broken, so we are not a complete
                // disaster)
            } else {
                wxMessageBox(_("The configuration data were not a valid JSON "
                               "document and could not be used!"),
                    _("JSON not valid"), wxOK | wxCENTRE | wxICON_ERROR);
            }
        }
    });
    m_dsk_pi->GetDSK()->Freeze(false);
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
    for (auto& ctrl : SettingsItemSizer->GetChildren()) {
        ++child;
        if (child % 2 == 0) { // Every other control is a user input (The one
                              // before is the wxStaticText label)
            m_edited_instrument->SetSetting(
                ctrl->GetWindow()->GetName(), ExtractValue(ctrl->GetWindow()));
        }
    }
    m_edited_instrument->ConfigureFromKey(
        m_edited_instrument->GetPrimarySKKey());
}

void MainConfigFrameImpl::FillDashboardDetails()
{
    if (!m_edited_dashboard) {
        return;
    }
    m_spCanvas->SetValue(m_edited_dashboard->GetCanvasNr());
    m_spPage->SetValue(m_edited_dashboard->GetPageNr());
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
    m_edited_dashboard->SetPageNr((size_t)m_spPage->GetValue());
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

void MainConfigFrameImpl::m_cbEnabledOnCheckBox(wxCommandEvent& event)
{
    UpdateEditedDashboard();
}

void MainConfigFrameImpl::m_spCanvasOnSpinCtrl(wxSpinEvent& event)
{
    UpdateEditedDashboard();
}

void MainConfigFrameImpl::m_spPageOnSpinCtrl(wxSpinEvent& event)
{
    UpdateEditedDashboard();
}

void MainConfigFrameImpl::m_chAnchorOnChoice(wxCommandEvent& event)
{
    UpdateEditedDashboard();
}

void MainConfigFrameImpl::m_spOffsetXOnSpinCtrl(wxSpinEvent& event)
{
    UpdateEditedDashboard();
}

void MainConfigFrameImpl::m_spOffsetYOnSpinCtrl(wxSpinEvent& event)
{
    UpdateEditedDashboard();
}

void MainConfigFrameImpl::m_spSpacingHOnSpinCtrl(wxSpinEvent& event)
{
    UpdateEditedDashboard();
}

void MainConfigFrameImpl::m_spSpacingVOnSpinCtrl(wxSpinEvent& event)
{
    UpdateEditedDashboard();
}

wxString LoadStringFromFile(wxFileInputStream& fis)
{
    wxString s;
    wxTextInputStream tis(fis);
    while (!fis.Eof()) {
        s = s.Append(tis.ReadLine()).Append('\n');
    }
    return s;
}

void MainConfigFrameImpl::m_bpSaveInstrButtonOnButtonClick(
    wxCommandEvent& event)
{
    wxString fn;
    if (m_edited_instrument) {
        fn = m_edited_instrument->GetName();
    }
    wxWindowPtr<wxFileDialog> dlg(
        new wxFileDialog(this, _("Save instrument to file"), "", fn,
            "DashboardSK JSON files (*.json)|*.json",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK && m_edited_instrument) {
            wxFileOutputStream output_stream(dlg->GetPath());
            if (output_stream.IsOk()) {
                wxJSONWriter w;
                wxString s;
                wxTextOutputStream tos(output_stream);
                w.Write(m_edited_instrument->GenerateJSONConfig(), s);
                tos.WriteString(m_dsk_pi->GetDSK()->SelfTranslate(s));
                output_stream.Close();
                LOG_INFO("Cannot save current contents in file '%s'.",
                    dlg->GetPath());
            }
        }
    });
    event.Skip();
}

void MainConfigFrameImpl::m_bpImportInstrButtonOnButtonClick(
    wxCommandEvent& event)
{
    wxWindowPtr<wxFileDialog> dlg(
        new wxFileDialog(this, _("Load instrument(s) from file(s)"), "", "",
            "DashboardSK JSON files (*.json)|*.json",
            wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK && m_edited_dashboard) {
            wxArrayString paths;
            dlg->GetPaths(paths);
            for (auto& p : paths) {
                wxFileInputStream input_stream(p);
                if (input_stream.IsOk() && m_edited_dashboard) {
                    wxJSONValue v;
                    wxJSONReader r;
                    wxString s = LoadStringFromFile(input_stream);
                    r.Parse(m_dsk_pi->GetDSK()->SelfPopulate(s), &v);

                    Instrument* instr = DashboardSK::CreateInstrumentInstance(
                        DashboardSK::GetClassIndex(v["class"].AsString()),
                        m_edited_dashboard);
                    if (!instr) {
                        LOG_VERBOSE("DashboardSK_pi: Problem loading "
                                    "instrument with class "
                            + v["class"].AsString());
                        wxMessageBox(
                            wxString::Format(
                                _("The file %s does not seem to be a "
                                  "DashboardSK instrument JSON definition."),
                                p.c_str()),
                            _("Error during import"), wxICON_EXCLAMATION);
                        continue;
                    }
                    instr->ReadConfig(v);
                    m_edited_dashboard->AddInstrument(instr);
                    m_edited_instrument = instr;
                }
            }
            FillInstrumentList();
            m_lbInstruments->SetSelection(m_lbInstruments->GetCount() - 1);
            FillInstrumentDetails();
            EnableInstrumentListButtons();
        }
    });
    event.Skip();
}

void MainConfigFrameImpl::m_btnExportDashboardOnButtonClick(
    wxCommandEvent& event)
{
    wxString fn;
    if (m_edited_dashboard) {
        fn = m_edited_dashboard->GetName();
    }
    wxWindowPtr<wxFileDialog> dlg(
        new wxFileDialog(this, _("Save dashboard to file"), "", fn,
            "DashboardSK JSON files (*.json)|*.json",
            wxFD_SAVE | wxFD_OVERWRITE_PROMPT));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK && m_edited_instrument) {
            wxFileOutputStream output_stream(dlg->GetPath());
            if (output_stream.IsOk()) {
                wxJSONWriter w;
                wxString s;
                wxTextOutputStream tos(output_stream);
                w.Write(m_edited_dashboard->GenerateJSONConfig(), s);
                tos.WriteString(m_dsk_pi->GetDSK()->SelfTranslate(s));
                output_stream.Close();
                LOG_INFO("Cannot save current contents in file '%s'.",
                    dlg->GetPath());
            }
        }
    });
    event.Skip();
}

void MainConfigFrameImpl::m_btnImportDashboardOnButtonClick(
    wxCommandEvent& event)
{
    wxWindowPtr<wxFileDialog> dlg(
        new wxFileDialog(this, _("Load dashboard(s) from file(s)"), "", "",
            "DashboardSK JSON files (*.json)|*.json",
            wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST));
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            wxArrayString paths;
            dlg->GetPaths(paths);
            for (auto& p : paths) {
                wxFileInputStream input_stream(p);
                if (input_stream.IsOk()) {
                    wxJSONValue v;
                    wxJSONReader r;
                    r.Parse(m_dsk_pi->GetDSK()->SelfPopulate(
                                LoadStringFromFile(input_stream)),
                        &v);
                    if (v.HasMember("instruments")) {
                        m_edited_dashboard = m_dsk_pi->GetDSK()->AddDashboard();
                        m_edited_dashboard->ReadConfig(v);
                        m_edited_instrument = nullptr;
                        FillForm(true);
                    } else {
                        wxMessageBox(
                            wxString::Format(
                                _("The file %s does not seem to be a "
                                  "DashboardSK dashboard JSON definition."),
                                p.c_str()),
                            _("Error during import"), wxICON_EXCLAMATION);
                    }
                }
            }
        }
    });
    event.Skip();
}

//====================================
// SKDataTreeImpl
//====================================

SKDataTreeImpl::SKDataTreeImpl(wxWindow* parent)
    : SKDataTree(parent)
{
#if (not __WXQT__ and wxCHECK_VERSION(3, 1, 0))
    SetSize(FromDIP(GetSize()));
#endif
#if not __WXQT__
    m_scintillaCode->StyleClearAll();
    m_scintillaCode->StyleSetForeground(
        wxSTC_STYLE_DEFAULT, GetForegroundColour());
    m_scintillaCode->StyleSetBackground(
        wxSTC_STYLE_DEFAULT, GetBackgroundColour());
#if wxCHECK_VERSION(3, 1, 0)
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
    m_scintillaCode->StyleSetForeground(wxSTC_JSON_URI, GetForegroundColour());
    m_scintillaCode->StyleSetBackground(wxSTC_JSON_URI, GetBackgroundColour());
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
    m_scintillaCode->SetCaretForeground(GetForegroundColour());
#endif
#endif
    DimeWindow(this);
}

void SKDataTreeImpl::SetCodeSKTree(DashboardSK* dsk)
{
    m_sdbSizerBtnsCancel->Hide();
#if not __WXQT__
    m_scintillaCode->SetReadOnly(false);
    m_scintillaCode->SetText(dsk->GetSignalKTreeText());
    m_scintillaCode->SetReadOnly(true);
#else
    m_scintillaCode->SetValue(dsk->GetSignalKTreeText());
#endif
}

//====================================
// SKPathBrowserImpl
//====================================

SKPathBrowserImpl::SKPathBrowserImpl(wxWindow* parent, wxWindowID id,
    const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : SKPathBrowser(parent, id, title, pos, size, style)
{
#if (not __WXQT__ and wxCHECK_VERSION(3, 1, 0))
    SetSize(FromDIP(GetSize()));
#endif
    DimeWindow(this);
    m_self = wxEmptyString;
    m_self_item_id = m_treePaths->GetRootItem();
}

wxString SKPathBrowserImpl::GetSKPath()
{
    wxTreeItemId selected = m_treePaths->GetSelection();
    wxString path = wxEmptyString;
    auto orig_selected = selected;
    while (selected.IsOk() && selected != m_treePaths->GetRootItem()) {
        if (!path.IsEmpty()) {
            path.Prepend(".");
        }
        path.Prepend(m_treePaths->GetItemText(selected));
        selected = m_treePaths->GetItemParent(selected);
    }
    if (!path.Contains(SRC_MAGIC_STRING)) {
        while (
            orig_selected.IsOk() && m_treePaths->HasChildren(orig_selected)) {
            wxTreeItemIdValue cookie;
            orig_selected = m_treePaths->GetFirstChild(orig_selected, cookie);
            if (orig_selected.IsOk()) {
                wxString component = m_treePaths->GetItemText(orig_selected);
                path.Append(".").Append(component);
                if (component.StartsWith(SRC_MAGIC_STRING)) {
                    break;
                }
            }
        }
    }
    return path;
}

/// Set pointer to the SignalK full data object
///
/// \param sk_tree Pointer to the \c wxJSONValue holding the data
void SKPathBrowserImpl::SetSKTree(wxJSONValue* sk_tree)
{
    m_sk_tree = *sk_tree;
    wxTreeItemId root = m_treePaths->GetRootItem();
    if (!root.IsOk()) {
        root = m_treePaths->AddRoot("SignalK");
    }
    m_self_item_id = root;
    AddChildren(root, m_sk_tree);
    m_treePaths->ExpandAll();
    m_btnCollapse->SetLabel(_("Collapse"));
}

void SKPathBrowserImpl::AddChildren(wxTreeItemId parent, wxJSONValue& json_node)
{
    if (!json_node.IsNull()) {
        for (const auto& member : json_node.GetMemberNames()) {
            if (!(member.IsSameAs("value") || member.IsSameAs("source")
                    || member.IsSameAs("timestamp"))) {
                // TODO: Isn't there a "legal" node with some of the above
                // names?
                wxTreeItemId child = m_treePaths->AppendItem(parent, member);
                if (member.IsSameAs(m_self)) {
                    m_self_item_id = child;
                }
                AddChildren(child, json_node[member]);
            }
        }
    }
}

void SKPathBrowserImpl::m_btnCollapseOnButtonClick(wxCommandEvent& event)
{
    if (m_treePaths->IsExpanded(m_treePaths->GetRootItem())) {
        m_treePaths->CollapseAll();
        m_btnCollapse->SetLabel(_("Expand"));
    } else {
        m_treePaths->ExpandAll();
        m_btnCollapse->SetLabel(_("Collapse"));
    }
    event.Skip();
}

void SKPathBrowserImpl::m_btnSelfOnButtonClick(wxCommandEvent& event)
{
    m_treePaths->SetFocusedItem(m_self_item_id);
}

void SKPathBrowserImpl::SetSelf(const wxString& self) { m_self = self; }

//====================================
// SKKeyCtrlImpl
//====================================

wxIMPLEMENT_DYNAMIC_CLASS_XTI(SKKeyCtrlImpl, wxPanel, "dashboardsdguiimpl.h")

    SKKeyCtrlImpl::SKKeyCtrlImpl(wxWindow* parent, wxWindowID id,
        const wxPoint& pos, const wxSize& size, long style,
        const wxString& name, const wxString& value)
    : SKKeyCtrl(parent, id, pos, size, style, name)
{
    m_tSKKey->SetValue(value);
    m_sk_tree = nullptr;
    DimeWindow(this);
}

wxString SKKeyCtrlImpl::GetValue() const { return m_tSKKey->GetValue(); }

void SKKeyCtrlImpl::SetValue(const wxString& value) const
{
    m_tSKKey->SetValue(value);
}

void SKKeyCtrlImpl::SetSKTree(wxJSONValue* sk_tree) { m_sk_tree = sk_tree; }

void SKKeyCtrlImpl::SetSelf(const wxString& self) { m_self = self; }

wxSize SKKeyCtrlImpl::DoGetBestSize() const
{
    wxSize s1 = m_tSKKey->GetBestSize();
    wxSize s2 = m_btnSelect->GetBestSize();
    return { s1.GetWidth() + s2.GetWidth(),
        wxMax(s1.GetHeight(), s2.GetHeight()) };
}

void SKKeyCtrlImpl::m_btnSelectOnButtonClick(wxCommandEvent& event)
{
    wxWindowPtr<SKPathBrowserImpl> dlg(
        new SKPathBrowserImpl(this, wxID_ANY, m_tSKKey->GetValue()));
    dlg->SetSelf(m_self);
    dlg->SetSKTree(m_sk_tree);
    dlg->ShowWindowModalThenDo([this, dlg](int retcode) {
        if (retcode == wxID_OK) {
            m_tSKKey->SetValue(dlg->GetSKPath());
        }
    });
    event.Skip();
}

//====================================
// SKZonesCtrlImpl
//====================================

wxIMPLEMENT_DYNAMIC_CLASS_XTI(SKZonesCtrlImpl, wxPanel, "dashboardsdguiimpl.h")

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

SKZonesCtrlImpl::SKZonesCtrlImpl(wxWindow* parent, dashboardsk_pi* dsk_pi,
    wxWindowID id, const wxPoint& pos, const wxSize& size, long style,
    const wxString& name, const wxString& value)
    : SKZonesCtrl(parent, id, pos, size, style, name)
{
    m_dsk_pi = dsk_pi;
    m_tZones->SetValue(value);
}

wxString SKZonesCtrlImpl::GetValue() const { return m_tZones->GetValue(); }

void SKZonesCtrlImpl::SetValue(const wxString& value) const
{
    m_tZones->SetValue(value);
}

wxSize SKZonesCtrlImpl::DoGetBestSize() const
{
    wxSize s1 = m_tZones->GetBestSize();
    wxSize s2 = m_btnSelect->GetBestSize();
    return { s1.GetWidth() + s2.GetWidth(),
        wxMax(s1.GetHeight(), s2.GetHeight()) };
}

//====================================
// ZonesConfigDialogImpl
//====================================

ZonesConfigDialogImpl::ZonesConfigDialogImpl(wxWindow* parent,
    dashboardsk_pi* dsk_pi, wxWindowID id, const wxString& value,
    const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : ZonesConfigDialog(parent, id, title, pos, size, style)
    , m_edited_zone(nullptr)
{
#if (not __WXQT__ and wxCHECK_VERSION(3, 1, 0))
    SetSize(FromDIP(GetSize()));
#endif
    m_dsk_pi = dsk_pi;
    if (m_dsk_pi) {
#if (wxCHECK_VERSION(3, 1, 6))
        m_bpAdd->SetBitmap(wxBitmapBundle::FromSVGFile(
            m_dsk_pi->GetDataDir() + "plus.svg", wxSize(BMP_SZ, BMP_SZ)));
        m_bpRemove->SetBitmap(wxBitmapBundle::FromSVGFile(
            m_dsk_pi->GetDataDir() + "minus.svg", wxSize(BMP_SZ, BMP_SZ)));
#else
        m_bpAdd->SetBitmap(GetBitmapFromSVGFile(
            m_dsk_pi->GetDataDir() + "plus.svg", BMP_SZ, BMP_SZ));
        m_bpRemove->SetBitmap(GetBitmapFromSVGFile(
            m_dsk_pi->GetDataDir() + "minus.svg", BMP_SZ, BMP_SZ));
#endif
    }
    m_zones = Zone::ParseZonesFromString(value);
    if (!m_zones.empty()) {
        UpdateList();
        m_lbZones->SetSelection(0);
        m_edited_zone = &m_zones.at(m_lbZones->GetSelection());
    }
    FillZoneControls();
    EnableControls();
    DimeWindow(this);
}

vector<Zone> ZonesConfigDialogImpl::GetZones() { return m_zones; }

void ZonesConfigDialogImpl::EnableControls()
{
    bool enable;
    if (m_lbZones->GetSelection() != wxNOT_FOUND) {
        enable = true;
    } else {
        // Enable only the Add button
        enable = false;
        m_bpAdd->Enable();
    }
    m_bpRemove->Enable(enable);
    m_stLower->Enable(enable);
    m_spLower->Enable(enable);
    m_stUpper->Enable(enable);
    m_spUpper->Enable(enable);
    m_stState->Enable(enable);
    m_choiceState->Enable(enable);
}

void ZonesConfigDialogImpl::FillZoneControls()
{
    if (m_edited_zone) {
        m_spLower->SetRange(-99999, m_edited_zone->GetUpperLimit());
        m_spLower->SetValue(m_edited_zone->GetLowerLimit());
        m_spUpper->SetRange(m_edited_zone->GetLowerLimit(), 99999);
        m_spUpper->SetValue(m_edited_zone->GetUpperLimit());
        m_choiceState->SetSelection(
            static_cast<int>(m_edited_zone->GetState()));
    } else {
        m_spLower->SetValue(0);
        m_spUpper->SetValue(0);
        m_choiceState->SetSelection(static_cast<int>(Zone::state::nominal));
    }
}

void ZonesConfigDialogImpl::UpdateList()
{
    m_edited_zone = nullptr;
    int sel = m_lbZones->GetSelection();
    m_lbZones->Clear();
    for (auto& zone : m_zones) {
        m_lbZones->Append(zone.ToUIString());
    }
    m_lbZones->SetSelection(sel);
    if (sel >= 0 && (unsigned)sel < m_zones.size()) {
        m_edited_zone = &m_zones.at(sel);
    }
}

void ZonesConfigDialogImpl::m_lbZonesOnListBox(wxCommandEvent& event)
{
    m_edited_zone = &m_zones.at(m_lbZones->GetSelection());
    FillZoneControls();
    event.Skip();
}

void ZonesConfigDialogImpl::m_bpAddOnButtonClick(wxCommandEvent& event)
{
    Zone z;
    m_zones.emplace_back(z);
    m_edited_zone = &m_zones.back();
    m_lbZones->Append(z.ToUIString());
    m_lbZones->SetSelection(m_lbZones->GetCount() - 1);
    FillZoneControls();
    EnableControls();
    event.Skip();
}

void ZonesConfigDialogImpl::m_bpRemoveOnButtonClick(wxCommandEvent& event)
{
    int sel = m_lbZones->GetSelection();
    m_edited_zone = nullptr;
    m_zones.erase(m_zones.begin() + sel);
    m_lbZones->Delete(sel);
    sel = wxMin(sel, m_lbZones->GetCount() - 1);
    if (sel >= 0 && (unsigned)sel < m_zones.size()) {
        m_lbZones->SetSelection(sel);
        m_edited_zone = &m_zones.at(sel);
    }
    FillZoneControls();
    EnableControls();
    event.Skip();
}

void ZonesConfigDialogImpl::m_spLowerOnSpinCtrlDouble(wxSpinDoubleEvent& event)
{
    m_spUpper->SetRange(m_spLower->GetValue(), 99999);
    m_edited_zone->SetLowerLimit(m_spLower->GetValue());
    UpdateList();
    event.Skip();
}

void ZonesConfigDialogImpl::m_spUpperOnSpinCtrlDouble(wxSpinDoubleEvent& event)
{
    m_spLower->SetRange(-99999, m_spUpper->GetValue());
    m_edited_zone->SetUpperLimit(m_spUpper->GetValue());
    UpdateList();
    event.Skip();
}

void ZonesConfigDialogImpl::m_choiceStateOnChoice(wxCommandEvent& event)
{
    m_edited_zone->SetState(
        static_cast<Zone::state>(m_choiceState->GetSelection()));
    UpdateList();
    event.Skip();
}

void ZonesConfigDialogImpl::m_sdbSizerButtonsOnCancelButtonClick(
    wxCommandEvent& event)
{
    m_zones = m_original_zones;
    event.Skip();
}

void ZonesConfigDialogImpl::m_sdbSizerButtonsOnOKButtonClick(
    wxCommandEvent& event)
{
    // We just let it close, m_zones contains all the info
    event.Skip();
}

PLUGIN_END_NAMESPACE
