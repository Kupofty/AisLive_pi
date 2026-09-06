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
#include <wx/stattext.h>
#include <wx/statline.h>
#include <wx/slider.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DialogMainGuiBase
///////////////////////////////////////////////////////////////////////////////
class DialogMainGuiBase : public wxDialog
{
	private:

	protected:
		wxButton* m_button_start;
		wxButton* m_button_stop;
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText_streamState;
		wxStaticLine* m_staticline1;
		wxStaticText* m_staticText4;
		wxStaticText* m_staticText_searchLatitude;
		wxStaticText* m_staticText6;
		wxStaticText* m_staticText_searchLongitude;
		wxStaticText* m_staticText8;
		wxStaticText* m_staticText_searchBoxSize;
		wxSlider* m_slider_searchBoxSize;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnButtonClick_startStream( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnButtonClick_stopStream( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnScroll_UpdateSearchBoxSize( wxScrollEvent& event ) { event.Skip(); }


	public:

		DialogMainGuiBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("AisLive Plugin"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 388,208 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER );

		~DialogMainGuiBase();

};

