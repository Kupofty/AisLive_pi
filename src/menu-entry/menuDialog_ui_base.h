///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.2.1-0-g80c4cb6)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DialogMenuEntryBase
///////////////////////////////////////////////////////////////////////////////
class DialogMenuEntryBase : public wxDialog
{
	private:

	protected:
		wxButton* m_button_dummy;

		// Virtual event handlers, override them in your derived class
		virtual void OnButtonClick_UpdateDummy( wxCommandEvent& event ) { event.Skip(); }


	public:

		DialogMenuEntryBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Template Plugin Menu"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 233,81 ), long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER );

		~DialogMenuEntryBase();

};

