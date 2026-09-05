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

	m_button1 = new wxButton( this, wxID_ANY, _("Start"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_button1, 0, wxALL, 5 );

	m_button2 = new wxButton( this, wxID_ANY, _("Stop"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer21->Add( m_button2, 0, wxALL, 5 );


	bSizer21->Add( 0, 0, 1, wxEXPAND, 5 );


	bSizer2->Add( bSizer21, 1, wxEXPAND, 5 );


	bSizer2->Add( 0, 0, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer2 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( DialogMainGuiBase::OnClose ) );
}

DialogMainGuiBase::~DialogMainGuiBase()
{
}
