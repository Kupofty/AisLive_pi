///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "menuDialog_ui_base.h"

///////////////////////////////////////////////////////////////////////////

DialogMenuEntryBase::DialogMenuEntryBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );


	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );

	m_button_dummy = new wxButton( this, wxID_ANY, _("Dummy"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( m_button_dummy, 0, wxALIGN_CENTER|wxALL, 5 );


	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	m_button_dummy->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( DialogMenuEntryBase::OnButtonClick_UpdateDummy ), NULL, this );
}

DialogMenuEntryBase::~DialogMenuEntryBase()
{
}
