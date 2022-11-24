///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 3.10.1-254-gc2ef7767)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>

#include "dashboardskgui.h"

///////////////////////////////////////////////////////////////////////////
using namespace DashboardSKPlugin;

MainConfigFrame::MainConfigFrame(wxWindow* parent, wxWindowID id,
    const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxSize(500, 300), wxDefaultSize);

    wxBoxSizer* MainSizer;
    MainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* ConfigSizer;
    ConfigSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* fgSizerDashboards;
    fgSizerDashboards = new wxFlexGridSizer(0, 4, 0, 0);
    fgSizerDashboards->AddGrowableCol(1);
    fgSizerDashboards->SetFlexibleDirection(wxBOTH);
    fgSizerDashboards->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    m_stSelf = new wxStaticText(
        this, wxID_ANY, _("Own context"), wxDefaultPosition, wxDefaultSize, 0);
    m_stSelf->Wrap(-1);
    fgSizerDashboards->Add(m_stSelf, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_tSelf = new wxTextCtrl(
        this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    fgSizerDashboards->Add(
        m_tSelf, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

    m_btnCfgEdit = new wxButton(
        this, wxID_ANY, _("Cfg Edit"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizerDashboards->Add(m_btnCfgEdit, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_btnSignalK = new wxButton(
        this, wxID_ANY, _("SignalK"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizerDashboards->Add(m_btnSignalK, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_stDashboard = new wxStaticText(
        this, wxID_ANY, _("Dashboard"), wxDefaultPosition, wxDefaultSize, 0);
    m_stDashboard->Wrap(-1);
    fgSizerDashboards->Add(
        m_stDashboard, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxArrayString m_comboDashboardChoices;
    m_comboDashboard = new wxChoice(this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, m_comboDashboardChoices, 0);
    m_comboDashboard->SetSelection(0);
    fgSizerDashboards->Add(
        m_comboDashboard, 0, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5);

    m_btnRemoveDashboard = new wxButton(
        this, wxID_ANY, _("Remove"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizerDashboards->Add(
        m_btnRemoveDashboard, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_btnNewDashboard = new wxButton(
        this, wxID_ANY, _("New"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizerDashboards->Add(
        m_btnNewDashboard, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    ConfigSizer->Add(fgSizerDashboards, 0, wxEXPAND, 5);

    wxBoxSizer* PanelSizer;
    PanelSizer = new wxBoxSizer(wxHORIZONTAL);

    m_panelList = new wxPanel(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* ListMainSizer;
    ListMainSizer = new wxBoxSizer(wxHORIZONTAL);

    m_scrolledWindowInstrumentList = new wxScrolledWindow(m_panelList, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    m_scrolledWindowInstrumentList->SetScrollRate(5, 5);
    wxBoxSizer* InstrumentListSizer;
    InstrumentListSizer = new wxBoxSizer(wxVERTICAL);

    m_stInstruments = new wxStaticText(m_scrolledWindowInstrumentList, wxID_ANY,
        _("Instruments"), wxDefaultPosition, wxDefaultSize, 0);
    m_stInstruments->Wrap(-1);
    InstrumentListSizer->Add(m_stInstruments, 0, wxALL, 5);

    wxBoxSizer* InstrumentListControlsSizer;
    InstrumentListControlsSizer = new wxBoxSizer(wxHORIZONTAL);

    m_lbInstruments = new wxListBox(m_scrolledWindowInstrumentList, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE);
    InstrumentListControlsSizer->Add(m_lbInstruments, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* InstrumentListButtonsSizer;
    InstrumentListButtonsSizer = new wxBoxSizer(wxVERTICAL);

    m_bpAddButton = new wxBitmapButton(m_scrolledWindowInstrumentList, wxID_ANY,
        wxNullBitmap, wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    InstrumentListButtonsSizer->Add(m_bpAddButton, 0, wxALL, 5);

    m_bpRemoveButton
        = new wxBitmapButton(m_scrolledWindowInstrumentList, wxID_ANY,
            wxNullBitmap, wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    InstrumentListButtonsSizer->Add(m_bpRemoveButton, 0, wxALL, 5);

    m_bpMoveUpButton
        = new wxBitmapButton(m_scrolledWindowInstrumentList, wxID_ANY,
            wxNullBitmap, wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    InstrumentListButtonsSizer->Add(m_bpMoveUpButton, 0, wxALL, 5);

    m_bpMoveDownButton
        = new wxBitmapButton(m_scrolledWindowInstrumentList, wxID_ANY,
            wxNullBitmap, wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    InstrumentListButtonsSizer->Add(m_bpMoveDownButton, 0, wxALL, 5);

    m_bpSaveInstrButton
        = new wxBitmapButton(m_scrolledWindowInstrumentList, wxID_ANY,
            wxNullBitmap, wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    InstrumentListButtonsSizer->Add(m_bpSaveInstrButton, 0, wxALL, 5);

    m_bpImportInstrButton
        = new wxBitmapButton(m_scrolledWindowInstrumentList, wxID_ANY,
            wxNullBitmap, wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    InstrumentListButtonsSizer->Add(m_bpImportInstrButton, 0, wxALL, 5);

    InstrumentListControlsSizer->Add(
        InstrumentListButtonsSizer, 0, wxEXPAND, 5);

    InstrumentListSizer->Add(InstrumentListControlsSizer, 1, wxEXPAND, 5);

    m_scrolledWindowInstrumentList->SetSizer(InstrumentListSizer);
    m_scrolledWindowInstrumentList->Layout();
    InstrumentListSizer->Fit(m_scrolledWindowInstrumentList);
    ListMainSizer->Add(m_scrolledWindowInstrumentList, 1, wxEXPAND | wxALL, 5);

    m_panelList->SetSizer(ListMainSizer);
    m_panelList->Layout();
    ListMainSizer->Fit(m_panelList);
    PanelSizer->Add(m_panelList, 3, wxEXPAND | wxALL, 5);

    m_panelConfig = new wxPanel(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* ConfigMainSizer;
    ConfigMainSizer = new wxBoxSizer(wxVERTICAL);

    m_swConfig = new wxScrolledWindow(m_panelConfig, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    m_swConfig->SetScrollRate(5, 5);
    wxBoxSizer* bScrolledSizer;
    bScrolledSizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* BasicSettingSizer;
    BasicSettingSizer = new wxFlexGridSizer(0, 2, 0, 0);
    BasicSettingSizer->AddGrowableCol(1);
    BasicSettingSizer->SetFlexibleDirection(wxBOTH);
    BasicSettingSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    m_stName = new wxStaticText(
        m_swConfig, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0);
    m_stName->Wrap(-1);
    BasicSettingSizer->Add(m_stName, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_tName = new wxTextCtrl(m_swConfig, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, 0);
    BasicSettingSizer->Add(m_tName, 0, wxALL | wxEXPAND, 5);

    m_stTitle = new wxStaticText(
        m_swConfig, wxID_ANY, _("Title"), wxDefaultPosition, wxDefaultSize, 0);
    m_stTitle->Wrap(-1);
    BasicSettingSizer->Add(m_stTitle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_tTitle = new wxTextCtrl(m_swConfig, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, 0);
    BasicSettingSizer->Add(m_tTitle, 0, wxALL | wxEXPAND, 5);

    bScrolledSizer->Add(BasicSettingSizer, 0, wxEXPAND, 5);

    wxBoxSizer* bSizerTimeout;
    bSizerTimeout = new wxBoxSizer(wxHORIZONTAL);

    m_stInstrument = new wxStaticText(m_swConfig, wxID_ANY, _("Instrument"),
        wxDefaultPosition, wxDefaultSize, 0);
    m_stInstrument->Wrap(-1);
    bSizerTimeout->Add(m_stInstrument, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    bSizerTimeout->Add(0, 0, 1, wxEXPAND, 5);

    m_stTimeout = new wxStaticText(m_swConfig, wxID_ANY, _("Timeout"),
        wxDefaultPosition, wxDefaultSize, 0);
    m_stTimeout->Wrap(-1);
    bSizerTimeout->Add(m_stTimeout, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_spTimeout = new wxSpinCtrl(m_swConfig, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 10);
    bSizerTimeout->Add(m_spTimeout, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_stSeconds = new wxStaticText(
        m_swConfig, wxID_ANY, _("sec"), wxDefaultPosition, wxDefaultSize, 0);
    m_stSeconds->Wrap(-1);
    bSizerTimeout->Add(m_stSeconds, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    bScrolledSizer->Add(bSizerTimeout, 0, wxEXPAND, 0);

    wxBoxSizer* SettingsSizer;
    SettingsSizer = new wxBoxSizer(wxHORIZONTAL);

    SettingsItemSizer = new wxFlexGridSizer(0, 2, 0, 0);
    SettingsItemSizer->AddGrowableCol(1);
    SettingsItemSizer->SetFlexibleDirection(wxBOTH);
    SettingsItemSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    SettingsItemSizer->Add(0, 0, 1, wxEXPAND, 5);

    SettingsItemSizer->Add(0, 0, 1, wxEXPAND, 5);

    SettingsSizer->Add(SettingsItemSizer, 1, wxEXPAND, 5);

    bScrolledSizer->Add(SettingsSizer, 0, wxEXPAND, 5);

    m_swConfig->SetSizer(bScrolledSizer);
    m_swConfig->Layout();
    bScrolledSizer->Fit(m_swConfig);
    ConfigMainSizer->Add(m_swConfig, 1, wxEXPAND | wxALL, 5);

    m_panelConfig->SetSizer(ConfigMainSizer);
    m_panelConfig->Layout();
    ConfigMainSizer->Fit(m_panelConfig);
    PanelSizer->Add(m_panelConfig, 5, wxEXPAND | wxALL, 5);

    ConfigSizer->Add(PanelSizer, 1, wxEXPAND, 5);

    wxFlexGridSizer* fgSizerLayout;
    fgSizerLayout = new wxFlexGridSizer(0, 8, 0, 0);
    fgSizerLayout->SetFlexibleDirection(wxBOTH);
    fgSizerLayout->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    m_cbEnabled = new wxCheckBox(
        this, wxID_ANY, _("Enabled"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizerLayout->Add(m_cbEnabled, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    fgSizerLayout->Add(0, 0, 1, wxEXPAND, 5);

    m_stCanvas = new wxStaticText(
        this, wxID_ANY, _("Canvas"), wxDefaultPosition, wxDefaultSize, 0);
    m_stCanvas->Wrap(-1);
    fgSizerLayout->Add(m_stCanvas, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_spCanvas = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 1, 0);
    fgSizerLayout->Add(m_spCanvas, 0, wxALL, 5);

    m_stAnchor = new wxStaticText(
        this, wxID_ANY, _("Anchor"), wxDefaultPosition, wxDefaultSize, 0);
    m_stAnchor->Wrap(-1);
    fgSizerLayout->Add(m_stAnchor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxArrayString m_chAnchorChoices;
    m_chAnchor = new wxChoice(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chAnchorChoices, 0);
    m_chAnchor->SetSelection(0);
    fgSizerLayout->Add(m_chAnchor, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_stOffsetX = new wxStaticText(
        this, wxID_ANY, _("Offset h"), wxDefaultPosition, wxDefaultSize, 0);
    m_stOffsetX->Wrap(-1);
    fgSizerLayout->Add(m_stOffsetX, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_spOffsetX = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
    fgSizerLayout->Add(m_spOffsetX, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_stOffsetY = new wxStaticText(
        this, wxID_ANY, _("Offset v"), wxDefaultPosition, wxDefaultSize, 0);
    m_stOffsetY->Wrap(-1);
    fgSizerLayout->Add(m_stOffsetY, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_spOffsetY = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
    fgSizerLayout->Add(m_spOffsetY, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_stSpacingH = new wxStaticText(
        this, wxID_ANY, _("Spacing h"), wxDefaultPosition, wxDefaultSize, 0);
    m_stSpacingH->Wrap(-1);
    fgSizerLayout->Add(m_stSpacingH, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_spSpacingH = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
    fgSizerLayout->Add(m_spSpacingH, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_stSpacingV = new wxStaticText(
        this, wxID_ANY, _("Spacing v"), wxDefaultPosition, wxDefaultSize, 0);
    m_stSpacingV->Wrap(-1);
    fgSizerLayout->Add(m_stSpacingV, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_spSpacingV = new wxSpinCtrl(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0);
    fgSizerLayout->Add(m_spSpacingV, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    ConfigSizer->Add(fgSizerLayout, 0, wxEXPAND, 5);

    MainSizer->Add(ConfigSizer, 1, wxEXPAND, 5);

    m_sdbSizer = new wxStdDialogButtonSizer();
    m_sdbSizerOK = new wxButton(this, wxID_OK);
    m_sdbSizer->AddButton(m_sdbSizerOK);
    m_sdbSizerCancel = new wxButton(this, wxID_CANCEL);
    m_sdbSizer->AddButton(m_sdbSizerCancel);
    m_sdbSizer->Realize();

    MainSizer->Add(m_sdbSizer, 0, wxALL | wxEXPAND, 5);

    this->SetSizer(MainSizer);
    this->Layout();

    this->Centre(wxBOTH);

    // Connect Events
    m_btnCfgEdit->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_btnCfgEditOnButtonClick), NULL,
        this);
    m_btnSignalK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_btnSignalKOnButtonClick), NULL,
        this);
    m_comboDashboard->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
        wxCommandEventHandler(MainConfigFrame::m_comboDashboardOnChoice), NULL,
        this);
    m_btnRemoveDashboard->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            MainConfigFrame::m_btnRemoveDashboardOnButtonClick),
        NULL, this);
    m_btnNewDashboard->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_btnNewDashboardOnButtonClick),
        NULL, this);
    m_lbInstruments->Connect(wxEVT_COMMAND_LISTBOX_SELECTED,
        wxCommandEventHandler(MainConfigFrame::m_lbInstrumentsOnListBox), NULL,
        this);
    m_bpAddButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpAddButtonOnButtonClick),
        NULL, this);
    m_bpRemoveButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpRemoveButtonOnButtonClick),
        NULL, this);
    m_bpMoveUpButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpMoveUpButtonOnButtonClick),
        NULL, this);
    m_bpMoveDownButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpMoveDownButtonOnButtonClick),
        NULL, this);
    m_bpSaveInstrButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            MainConfigFrame::m_bpSaveInstrButtonOnButtonClick),
        NULL, this);
    m_bpImportInstrButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            MainConfigFrame::m_bpImportInstrButtonOnButtonClick),
        NULL, this);
    m_cbEnabled->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_cbEnabledOnCheckBox), NULL,
        this);
    m_spCanvas->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spCanvasOnSpinCtrl), NULL, this);
    m_chAnchor->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
        wxCommandEventHandler(MainConfigFrame::m_chAnchorOnChoice), NULL, this);
    m_spOffsetX->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spOffsetXOnSpinCtrl), NULL, this);
    m_spOffsetY->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spOffsetYOnSpinCtrl), NULL, this);
    m_spSpacingH->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spSpacingHOnSpinCtrl), NULL,
        this);
    m_spSpacingV->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spSpacingVOnSpinCtrl), NULL,
        this);
    m_sdbSizerCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_sdbSizerOnCancelButtonClick),
        NULL, this);
    m_sdbSizerOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_sdbSizerOnOKButtonClick), NULL,
        this);
}

MainConfigFrame::~MainConfigFrame()
{
    // Disconnect Events
    m_btnCfgEdit->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_btnCfgEditOnButtonClick), NULL,
        this);
    m_btnSignalK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_btnSignalKOnButtonClick), NULL,
        this);
    m_comboDashboard->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED,
        wxCommandEventHandler(MainConfigFrame::m_comboDashboardOnChoice), NULL,
        this);
    m_btnRemoveDashboard->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            MainConfigFrame::m_btnRemoveDashboardOnButtonClick),
        NULL, this);
    m_btnNewDashboard->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_btnNewDashboardOnButtonClick),
        NULL, this);
    m_lbInstruments->Disconnect(wxEVT_COMMAND_LISTBOX_SELECTED,
        wxCommandEventHandler(MainConfigFrame::m_lbInstrumentsOnListBox), NULL,
        this);
    m_bpAddButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpAddButtonOnButtonClick),
        NULL, this);
    m_bpRemoveButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpRemoveButtonOnButtonClick),
        NULL, this);
    m_bpMoveUpButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpMoveUpButtonOnButtonClick),
        NULL, this);
    m_bpMoveDownButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_bpMoveDownButtonOnButtonClick),
        NULL, this);
    m_bpSaveInstrButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            MainConfigFrame::m_bpSaveInstrButtonOnButtonClick),
        NULL, this);
    m_bpImportInstrButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            MainConfigFrame::m_bpImportInstrButtonOnButtonClick),
        NULL, this);
    m_cbEnabled->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_cbEnabledOnCheckBox), NULL,
        this);
    m_spCanvas->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spCanvasOnSpinCtrl), NULL, this);
    m_chAnchor->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED,
        wxCommandEventHandler(MainConfigFrame::m_chAnchorOnChoice), NULL, this);
    m_spOffsetX->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spOffsetXOnSpinCtrl), NULL, this);
    m_spOffsetY->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spOffsetYOnSpinCtrl), NULL, this);
    m_spSpacingH->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spSpacingHOnSpinCtrl), NULL,
        this);
    m_spSpacingV->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED,
        wxSpinEventHandler(MainConfigFrame::m_spSpacingVOnSpinCtrl), NULL,
        this);
    m_sdbSizerCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_sdbSizerOnCancelButtonClick),
        NULL, this);
    m_sdbSizerOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(MainConfigFrame::m_sdbSizerOnOKButtonClick), NULL,
        this);
}

SKDataTree::SKDataTree(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxSize(200, 200), wxDefaultSize);

    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer(wxVERTICAL);

    m_scintillaCode = new wxStyledTextCtrl(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, wxEmptyString);
    m_scintillaCode->SetUseTabs(true);
    m_scintillaCode->SetTabWidth(4);
    m_scintillaCode->SetIndent(4);
    m_scintillaCode->SetTabIndents(false);
    m_scintillaCode->SetBackSpaceUnIndents(true);
    m_scintillaCode->SetViewEOL(false);
    m_scintillaCode->SetViewWhiteSpace(false);
    m_scintillaCode->SetMarginWidth(2, 0);
    m_scintillaCode->SetIndentationGuides(true);
    m_scintillaCode->SetReadOnly(false);
    m_scintillaCode->SetMarginWidth(1, 0);
    m_scintillaCode->SetMarginWidth(0, 0);
    m_scintillaCode->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUS);
    m_scintillaCode->MarkerSetBackground(
        wxSTC_MARKNUM_FOLDER, wxColour(wxT("BLACK")));
    m_scintillaCode->MarkerSetForeground(
        wxSTC_MARKNUM_FOLDER, wxColour(wxT("WHITE")));
    m_scintillaCode->MarkerDefine(
        wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUS);
    m_scintillaCode->MarkerSetBackground(
        wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("BLACK")));
    m_scintillaCode->MarkerSetForeground(
        wxSTC_MARKNUM_FOLDEROPEN, wxColour(wxT("WHITE")));
    m_scintillaCode->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
    m_scintillaCode->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_BOXPLUS);
    m_scintillaCode->MarkerSetBackground(
        wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("BLACK")));
    m_scintillaCode->MarkerSetForeground(
        wxSTC_MARKNUM_FOLDEREND, wxColour(wxT("WHITE")));
    m_scintillaCode->MarkerDefine(
        wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_BOXMINUS);
    m_scintillaCode->MarkerSetBackground(
        wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("BLACK")));
    m_scintillaCode->MarkerSetForeground(
        wxSTC_MARKNUM_FOLDEROPENMID, wxColour(wxT("WHITE")));
    m_scintillaCode->MarkerDefine(
        wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
    m_scintillaCode->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
    m_scintillaCode->SetSelBackground(
        true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
    m_scintillaCode->SetSelForeground(
        true, wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
    m_scintillaCode->SetForegroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    m_scintillaCode->SetBackgroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    bSizerMain->Add(m_scintillaCode, 1, wxEXPAND | wxALL, 5);

    m_sdbSizerBtns = new wxStdDialogButtonSizer();
    m_sdbSizerBtnsOK = new wxButton(this, wxID_OK);
    m_sdbSizerBtns->AddButton(m_sdbSizerBtnsOK);
    m_sdbSizerBtnsCancel = new wxButton(this, wxID_CANCEL);
    m_sdbSizerBtns->AddButton(m_sdbSizerBtnsCancel);
    m_sdbSizerBtns->Realize();

    bSizerMain->Add(m_sdbSizerBtns, 0, wxEXPAND, 5);

    this->SetSizer(bSizerMain);
    this->Layout();

    this->Centre(wxBOTH);

    // Connect Events
    m_sdbSizerBtnsCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKDataTree::m_sdbSizerBtnsOnCancelButtonClick),
        NULL, this);
    m_sdbSizerBtnsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKDataTree::m_sdbSizerBtnsOnOKButtonClick), NULL,
        this);
}

SKDataTree::~SKDataTree()
{
    // Disconnect Events
    m_sdbSizerBtnsCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKDataTree::m_sdbSizerBtnsOnCancelButtonClick),
        NULL, this);
    m_sdbSizerBtnsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKDataTree::m_sdbSizerBtnsOnOKButtonClick), NULL,
        this);
}

SKKeyCtrl::SKKeyCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name)
    : wxPanel(parent, id, pos, size, style, name)
{
    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer(wxHORIZONTAL);

    m_tSKKey = new wxTextCtrl(
        this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    bSizerMain->Add(m_tSKKey, 1, wxEXPAND | wxRIGHT, 5);

    m_btnSelect = new wxButton(this, wxID_ANY, _("..."), wxDefaultPosition,
        wxDefaultSize, wxBU_EXACTFIT);
    bSizerMain->Add(m_btnSelect, 0, wxEXPAND | wxLEFT, 5);

    this->SetSizer(bSizerMain);
    this->Layout();

    // Connect Events
    m_btnSelect->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKKeyCtrl::m_btnSelectOnButtonClick), NULL, this);
}

SKKeyCtrl::~SKKeyCtrl()
{
    // Disconnect Events
    m_btnSelect->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKKeyCtrl::m_btnSelectOnButtonClick), NULL, this);
}

SKZonesCtrl::SKZonesCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name)
    : wxPanel(parent, id, pos, size, style, name)
{
    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer(wxHORIZONTAL);

    m_tZones = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxDefaultSize, wxTE_READONLY);
    bSizerMain->Add(m_tZones, 1, wxEXPAND | wxRIGHT, 5);

    m_btnSelect = new wxButton(this, wxID_ANY, _("..."), wxDefaultPosition,
        wxDefaultSize, wxBU_EXACTFIT);
    bSizerMain->Add(m_btnSelect, 0, wxEXPAND | wxLEFT, 5);

    this->SetSizer(bSizerMain);
    this->Layout();

    // Connect Events
    m_btnSelect->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKZonesCtrl::m_btnSelectOnButtonClick), NULL,
        this);
}

SKZonesCtrl::~SKZonesCtrl()
{
    // Disconnect Events
    m_btnSelect->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKZonesCtrl::m_btnSelectOnButtonClick), NULL,
        this);
}

SKPathBrowser::SKPathBrowser(wxWindow* parent, wxWindowID id,
    const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxSize(200, 200), wxDefaultSize);

    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer(wxVERTICAL);

    m_treePaths = new wxTreeCtrl(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE);
    bSizerMain->Add(m_treePaths, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* bSizerButtons;
    bSizerButtons = new wxBoxSizer(wxHORIZONTAL);

    m_btnCollapse = new wxButton(
        this, wxID_ANY, _("Collapse"), wxDefaultPosition, wxDefaultSize, 0);
    bSizerButtons->Add(m_btnCollapse, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_btnSelf = new wxButton(
        this, wxID_ANY, _("Own vessel"), wxDefaultPosition, wxDefaultSize, 0);
    bSizerButtons->Add(m_btnSelf, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    bSizerButtons->Add(0, 0, 1, wxEXPAND, 5);

    m_sdbSizerButtons = new wxStdDialogButtonSizer();
    m_sdbSizerButtonsOK = new wxButton(this, wxID_OK);
    m_sdbSizerButtons->AddButton(m_sdbSizerButtonsOK);
    m_sdbSizerButtonsCancel = new wxButton(this, wxID_CANCEL);
    m_sdbSizerButtons->AddButton(m_sdbSizerButtonsCancel);
    m_sdbSizerButtons->Realize();

    bSizerButtons->Add(m_sdbSizerButtons, 0, wxEXPAND, 5);

    bSizerMain->Add(bSizerButtons, 0, wxEXPAND, 5);

    this->SetSizer(bSizerMain);
    this->Layout();

    this->Centre(wxBOTH);

    // Connect Events
    m_treePaths->Connect(wxEVT_COMMAND_TREE_SEL_CHANGED,
        wxTreeEventHandler(SKPathBrowser::m_treePathOnTreeSelChanged), NULL,
        this);
    m_btnCollapse->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKPathBrowser::m_btnCollapseOnButtonClick), NULL,
        this);
    m_btnSelf->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKPathBrowser::m_btnSelfOnButtonClick), NULL,
        this);
    m_sdbSizerButtonsCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            SKPathBrowser::m_sdbSizerButtonsOnCancelButtonClick),
        NULL, this);
    m_sdbSizerButtonsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKPathBrowser::m_sdbSizerButtonsOnOKButtonClick),
        NULL, this);
}

SKPathBrowser::~SKPathBrowser()
{
    // Disconnect Events
    m_treePaths->Disconnect(wxEVT_COMMAND_TREE_SEL_CHANGED,
        wxTreeEventHandler(SKPathBrowser::m_treePathOnTreeSelChanged), NULL,
        this);
    m_btnCollapse->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKPathBrowser::m_btnCollapseOnButtonClick), NULL,
        this);
    m_btnSelf->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKPathBrowser::m_btnSelfOnButtonClick), NULL,
        this);
    m_sdbSizerButtonsCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            SKPathBrowser::m_sdbSizerButtonsOnCancelButtonClick),
        NULL, this);
    m_sdbSizerButtonsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(SKPathBrowser::m_sdbSizerButtonsOnOKButtonClick),
        NULL, this);
}

ZonesConfigDialog::ZonesConfigDialog(wxWindow* parent, wxWindowID id,
    const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxSize(200, 200), wxDefaultSize);

    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer(wxVERTICAL);

    m_stZones = new wxStaticText(
        this, wxID_ANY, _("Zones"), wxDefaultPosition, wxDefaultSize, 0);
    m_stZones->Wrap(-1);
    bSizerMain->Add(m_stZones, 0, wxALL, 5);

    wxBoxSizer* bSizerData;
    bSizerData = new wxBoxSizer(wxHORIZONTAL);

    m_lbZones = new wxListBox(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);
    bSizerData->Add(m_lbZones, 1, wxALL | wxEXPAND, 5);

    wxBoxSizer* bSizerButtons;
    bSizerButtons = new wxBoxSizer(wxVERTICAL);

    m_bpAdd = new wxBitmapButton(this, wxID_ANY, wxNullBitmap,
        wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    bSizerButtons->Add(m_bpAdd, 0, wxALL, 5);

    m_bpRemove = new wxBitmapButton(this, wxID_ANY, wxNullBitmap,
        wxDefaultPosition, wxSize(32, 32), wxBU_AUTODRAW | 0);
    bSizerButtons->Add(m_bpRemove, 0, wxALL, 5);

    bSizerData->Add(bSizerButtons, 0, wxEXPAND, 5);

    wxFlexGridSizer* fgSizerZone;
    fgSizerZone = new wxFlexGridSizer(0, 2, 0, 0);
    fgSizerZone->SetFlexibleDirection(wxBOTH);
    fgSizerZone->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    m_stLower = new wxStaticText(
        this, wxID_ANY, _("Lower bound"), wxDefaultPosition, wxDefaultSize, 0);
    m_stLower->Wrap(-1);
    fgSizerZone->Add(m_stLower, 0, wxALL, 5);

    m_spLower = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -99999, 99999, 0, 1);
    m_spLower->SetDigits(1);
    fgSizerZone->Add(m_spLower, 0, wxALL, 5);

    m_stUpper = new wxStaticText(
        this, wxID_ANY, _("Upper bound"), wxDefaultPosition, wxDefaultSize, 0);
    m_stUpper->Wrap(-1);
    fgSizerZone->Add(m_stUpper, 0, wxALL, 5);

    m_spUpper = new wxSpinCtrlDouble(this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -99999, 99999, 0, 1);
    m_spUpper->SetDigits(1);
    fgSizerZone->Add(m_spUpper, 0, wxALL, 5);

    m_stState = new wxStaticText(
        this, wxID_ANY, _("State"), wxDefaultPosition, wxDefaultSize, 0);
    m_stState->Wrap(-1);
    fgSizerZone->Add(m_stState, 0, wxALL, 5);

    wxString m_choiceStateChoices[] = { _("Nominal"), _("Normal"), _("Alert"),
        _("Warning"), _("Alarm"), _("Emergency") };
    int m_choiceStateNChoices = sizeof(m_choiceStateChoices) / sizeof(wxString);
    m_choiceState = new wxChoice(this, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, m_choiceStateNChoices, m_choiceStateChoices, 0);
    m_choiceState->SetSelection(0);
    fgSizerZone->Add(m_choiceState, 0, wxALL, 5);

    bSizerData->Add(fgSizerZone, 1, wxEXPAND, 5);

    bSizerMain->Add(bSizerData, 1, wxEXPAND, 5);

    m_stNominal = new wxStaticText(this, wxID_ANY,
        _("All values not covered by the zones are considered nominal."),
        wxDefaultPosition, wxDefaultSize, 0);
    m_stNominal->Wrap(-1);
    bSizerMain->Add(m_stNominal, 0, wxALL, 5);

    m_sdbSizerButtons = new wxStdDialogButtonSizer();
    m_sdbSizerButtonsOK = new wxButton(this, wxID_OK);
    m_sdbSizerButtons->AddButton(m_sdbSizerButtonsOK);
    m_sdbSizerButtonsCancel = new wxButton(this, wxID_CANCEL);
    m_sdbSizerButtons->AddButton(m_sdbSizerButtonsCancel);
    m_sdbSizerButtons->Realize();

    bSizerMain->Add(m_sdbSizerButtons, 0, wxEXPAND, 5);

    this->SetSizer(bSizerMain);
    this->Layout();

    this->Centre(wxBOTH);

    // Connect Events
    m_lbZones->Connect(wxEVT_COMMAND_LISTBOX_SELECTED,
        wxCommandEventHandler(ZonesConfigDialog::m_lbZonesOnListBox), NULL,
        this);
    m_bpAdd->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ZonesConfigDialog::m_bpAddOnButtonClick), NULL,
        this);
    m_bpRemove->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ZonesConfigDialog::m_bpRemoveOnButtonClick), NULL,
        this);
    m_spLower->Connect(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED,
        wxSpinDoubleEventHandler(ZonesConfigDialog::m_spLowerOnSpinCtrlDouble),
        NULL, this);
    m_spUpper->Connect(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED,
        wxSpinDoubleEventHandler(ZonesConfigDialog::m_spUpperOnSpinCtrlDouble),
        NULL, this);
    m_choiceState->Connect(wxEVT_COMMAND_CHOICE_SELECTED,
        wxCommandEventHandler(ZonesConfigDialog::m_choiceStateOnChoice), NULL,
        this);
    m_sdbSizerButtonsCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            ZonesConfigDialog::m_sdbSizerButtonsOnCancelButtonClick),
        NULL, this);
    m_sdbSizerButtonsOK->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            ZonesConfigDialog::m_sdbSizerButtonsOnOKButtonClick),
        NULL, this);
}

ZonesConfigDialog::~ZonesConfigDialog()
{
    // Disconnect Events
    m_lbZones->Disconnect(wxEVT_COMMAND_LISTBOX_SELECTED,
        wxCommandEventHandler(ZonesConfigDialog::m_lbZonesOnListBox), NULL,
        this);
    m_bpAdd->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ZonesConfigDialog::m_bpAddOnButtonClick), NULL,
        this);
    m_bpRemove->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(ZonesConfigDialog::m_bpRemoveOnButtonClick), NULL,
        this);
    m_spLower->Disconnect(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED,
        wxSpinDoubleEventHandler(ZonesConfigDialog::m_spLowerOnSpinCtrlDouble),
        NULL, this);
    m_spUpper->Disconnect(wxEVT_COMMAND_SPINCTRLDOUBLE_UPDATED,
        wxSpinDoubleEventHandler(ZonesConfigDialog::m_spUpperOnSpinCtrlDouble),
        NULL, this);
    m_choiceState->Disconnect(wxEVT_COMMAND_CHOICE_SELECTED,
        wxCommandEventHandler(ZonesConfigDialog::m_choiceStateOnChoice), NULL,
        this);
    m_sdbSizerButtonsCancel->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            ZonesConfigDialog::m_sdbSizerButtonsOnCancelButtonClick),
        NULL, this);
    m_sdbSizerButtonsOK->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(
            ZonesConfigDialog::m_sdbSizerButtonsOnOKButtonClick),
        NULL, this);
}
