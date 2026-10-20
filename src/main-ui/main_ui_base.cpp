///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "main_ui_base.h"

///////////////////////////////////////////////////////////////////////////

DialogMainGuiBase::DialogMainGuiBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_panel_stream = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );


	bSizer21->Add( 0, 0, 1, wxEXPAND, 5 );

	m_button_start = new wxButton( m_panel_stream, wxID_ANY, _("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_button_start, 0, wxALL, 5 );

	m_button_stop = new wxButton( m_panel_stream, wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_button_stop, 0, wxALL, 5 );


	bSizer21->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer21, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );


	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText2 = new wxStaticText( m_panel_stream, wxID_ANY, _("AIS stream:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer3->Add( m_staticText2, 0, wxALIGN_CENTER|wxALL, 5 );

	m_staticText_streamState = new wxStaticText( m_panel_stream, wxID_ANY, _("Not running"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_streamState->Wrap( -1 );
	bSizer3->Add( m_staticText_streamState, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer3, 1, wxEXPAND, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( m_panel_stream, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer2->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxHORIZONTAL );


	bSizer61->Add( 0, 0, 1, wxEXPAND, 5 );

	m_checkBox_followBoatMode = new wxCheckBox( m_panel_stream, wxID_ANY, _("Follow boat"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_followBoatMode->SetValue(true);
	bSizer61->Add( m_checkBox_followBoatMode, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer61->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer61, 1, wxEXPAND, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticline2 = new wxStaticLine( m_panel_stream, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer2->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText9 = new wxStaticText( m_panel_stream, wxID_ANY, _("Search position: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	bSizer4->Add( m_staticText9, 0, wxALL, 5 );

	m_staticText4 = new wxStaticText( m_panel_stream, wxID_ANY, _("Lat:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer4->Add( m_staticText4, 0, wxALL, 5 );

	m_staticText_searchLatitude = new wxStaticText( m_panel_stream, wxID_ANY, _("0° N"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_searchLatitude->Wrap( -1 );
	bSizer4->Add( m_staticText_searchLatitude, 0, wxALL, 5 );

	m_staticText10 = new wxStaticText( m_panel_stream, wxID_ANY, _("/"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	bSizer4->Add( m_staticText10, 0, wxALL, 5 );

	m_staticText6 = new wxStaticText( m_panel_stream, wxID_ANY, _("Lon:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	bSizer4->Add( m_staticText6, 0, wxALL, 5 );

	m_staticText_searchLongitude = new wxStaticText( m_panel_stream, wxID_ANY, _("0° E"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_searchLongitude->Wrap( -1 );
	bSizer4->Add( m_staticText_searchLongitude, 0, wxALL, 5 );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer4, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );


	bSizer6->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText8 = new wxStaticText( m_panel_stream, wxID_ANY, _("Search box size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bSizer6->Add( m_staticText8, 0, wxALL, 5 );

	m_staticText_searchBoxSize = new wxStaticText( m_panel_stream, wxID_ANY, _("1° x 1°"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_searchBoxSize->Wrap( -1 );
	bSizer6->Add( m_staticText_searchBoxSize, 0, wxALL, 5 );


	bSizer6->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer6, 1, wxEXPAND, 5 );

	m_slider_searchBoxSize = new wxSlider( m_panel_stream, wxID_ANY, 1, 0, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer2->Add( m_slider_searchBoxSize, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );


	m_panel_stream->SetSizer( bSizer2 );
	m_panel_stream->Layout();
	bSizer2->Fit( m_panel_stream );
	m_notebook->AddPage( m_panel_stream, _("AIS Stream"), false );
	m_panel_mmsiList = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );

	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxVERTICAL );


	bSizer12->Add( 0, 0, 1, wxEXPAND, 5 );

	m_spinCtrl_followedMMSI = new wxSpinCtrl( m_panel_mmsiList, wxID_ANY, wxT("227000000"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100000000, 999999999, 227000000 );
	bSizer12->Add( m_spinCtrl_followedMMSI, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );

	m_textCtrl_mmsiComment = new wxTextCtrl( m_panel_mmsiList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	#ifdef __WXGTK__
	if ( !m_textCtrl_mmsiComment->HasFlag( wxTE_MULTILINE ) )
	{
	m_textCtrl_mmsiComment->SetMaxLength( 20 );
	}
	#else
	m_textCtrl_mmsiComment->SetMaxLength( 20 );
	#endif
	bSizer12->Add( m_textCtrl_mmsiComment, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );

	m_button_addMmsiToList = new wxButton( m_panel_mmsiList, wxID_ANY, _("Add to list"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer12->Add( m_button_addMmsiToList, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer12->Add( 0, 0, 1, wxEXPAND, 5 );

	m_checkBox_activateVesselFollowing = new wxCheckBox( m_panel_mmsiList, wxID_ANY, _("Activate"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkBox_activateVesselFollowing->SetValue(true);
	bSizer12->Add( m_checkBox_activateVesselFollowing, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer12->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer13->Add( bSizer12, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizer15;
	bSizer15 = new wxBoxSizer( wxVERTICAL );

	m_staticText11 = new wxStaticText( m_panel_mmsiList, wxID_ANY, _("MMSI list"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizer15->Add( m_staticText11, 0, wxALIGN_CENTER|wxALL, 5 );

	m_dataViewListCtrl_followedVessels = new wxDataViewListCtrl( m_panel_mmsiList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES|wxDV_ROW_LINES|wxDV_VERT_RULES );
	bSizer15->Add( m_dataViewListCtrl_followedVessels, 1, wxALL|wxEXPAND, 5 );

	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxHORIZONTAL );


	bSizer121->Add( 0, 0, 1, wxEXPAND, 5 );

	m_button_deleteMmsiFromList = new wxButton( m_panel_mmsiList, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer121->Add( m_button_deleteMmsiFromList, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer121->Add( 0, 0, 1, wxEXPAND, 5 );

	m_button_clearMmsiList = new wxButton( m_panel_mmsiList, wxID_ANY, _("Clear all"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer121->Add( m_button_clearMmsiList, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer121->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer15->Add( bSizer121, 0, wxEXPAND, 5 );


	bSizer13->Add( bSizer15, 1, wxEXPAND, 5 );


	m_panel_mmsiList->SetSizer( bSizer13 );
	m_panel_mmsiList->Layout();
	bSizer13->Fit( m_panel_mmsiList );
	m_notebook->AddPage( m_panel_mmsiList, _("Followed vessels"), true );

	bSizer8->Add( m_notebook, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DialogMainGuiBase::OnClose ) );
	m_button_start->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnButtonClick_startStream ), NULL, this );
	m_button_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnButtonClick_stopStream ), NULL, this );
	m_checkBox_followBoatMode->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnCheckBox_FollowBoatMode ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( DialogMainGuiBase::OnScrollThumbRelease_UpdateSearchBoxSize ), NULL, this );
	m_button_addMmsiToList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnButtonClick_AddMmsiToList ), NULL, this );
	m_checkBox_activateVesselFollowing->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DialogMainGuiBase::m_checkBox_activateVesselFollowingOnCheckBox ), NULL, this );
	m_dataViewListCtrl_followedVessels->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler( DialogMainGuiBase::OnDataViewListCtrlItemValueChanged_updateMmsiList ), NULL, this );
	m_button_deleteMmsiFromList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnButtonClick_DeleteMmsiFromList ), NULL, this );
	m_button_clearMmsiList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnButtonClick_ClearMmsiList ), NULL, this );
}

DialogMainGuiBase::~DialogMainGuiBase()
{
}
