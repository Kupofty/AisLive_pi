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

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer21;
	bSizer21 = new wxBoxSizer( wxHORIZONTAL );


	bSizer21->Add( 0, 0, 1, wxEXPAND, 5 );

	m_button_start = new wxButton( this, wxID_ANY, _("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_button_start, 0, wxALL, 5 );

	m_button_stop = new wxButton( this, wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_button_stop, 0, wxALL, 5 );


	bSizer21->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer21, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxHORIZONTAL );


	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText2 = new wxStaticText( this, wxID_ANY, _("Stream:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText2->Wrap( -1 );
	bSizer3->Add( m_staticText2, 0, wxALIGN_CENTER|wxALL, 5 );

	m_staticText_streamState = new wxStaticText( this, wxID_ANY, _("Not running"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_streamState->Wrap( -1 );
	bSizer3->Add( m_staticText_streamState, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer3, 1, wxEXPAND, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer2->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxHORIZONTAL );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Latitude:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	bSizer4->Add( m_staticText4, 0, wxALL, 5 );

	m_staticText_searchLatitude = new wxStaticText( this, wxID_ANY, _("0°"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_searchLatitude->Wrap( -1 );
	bSizer4->Add( m_staticText_searchLatitude, 0, wxALL, 5 );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText6 = new wxStaticText( this, wxID_ANY, _("Latitude:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	bSizer4->Add( m_staticText6, 0, wxALL, 5 );

	m_staticText_searchLongitude = new wxStaticText( this, wxID_ANY, _("0°"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_searchLongitude->Wrap( -1 );
	bSizer4->Add( m_staticText_searchLongitude, 0, wxALL, 5 );


	bSizer4->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer4, 1, wxEXPAND, 5 );

	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxHORIZONTAL );


	bSizer6->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText8 = new wxStaticText( this, wxID_ANY, _("Search box size:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bSizer6->Add( m_staticText8, 0, wxALL, 5 );

	m_staticText_searchBoxSize = new wxStaticText( this, wxID_ANY, _("1° x 1°"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_searchBoxSize->Wrap( -1 );
	bSizer6->Add( m_staticText_searchBoxSize, 0, wxALL, 5 );


	bSizer6->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer6, 1, wxEXPAND, 5 );

	m_slider_searchBoxSize = new wxSlider( this, wxID_ANY, 1, 1, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer2->Add( m_slider_searchBoxSize, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer2 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DialogMainGuiBase::OnClose ) );
	m_button_start->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnButtonClick_startStream ), NULL, this );
	m_button_stop->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMainGuiBase::OnButtonClick_stopStream ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
	m_slider_searchBoxSize->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( DialogMainGuiBase::OnScroll_UpdateSearchBoxSize ), NULL, this );
}

DialogMainGuiBase::~DialogMainGuiBase()
{
}
