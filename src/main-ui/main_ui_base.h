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
#include <wx/checkbox.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/dataview.h>
#include <wx/notebook.h>
#include <wx/dialog.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class DialogMainGuiBase
///////////////////////////////////////////////////////////////////////////////
class DialogMainGuiBase : public wxDialog
{
	private:

	protected:
		wxNotebook* m_notebook;
		wxPanel* m_panel_stream;
		wxButton* m_button_start;
		wxButton* m_button_stop;
		wxStaticText* m_staticText2;
		wxStaticText* m_staticText_streamState;
		wxStaticLine* m_staticline1;
		wxCheckBox* m_checkBox_followBoatMode;
		wxStaticLine* m_staticline2;
		wxStaticText* m_staticText9;
		wxStaticText* m_staticText4;
		wxStaticText* m_staticText_searchLatitude;
		wxStaticText* m_staticText10;
		wxStaticText* m_staticText6;
		wxStaticText* m_staticText_searchLongitude;
		wxStaticText* m_staticText8;
		wxStaticText* m_staticText_searchBoxSize;
		wxSlider* m_slider_searchBoxSize;
		wxPanel* m_panel_mmsiList;
		wxSpinCtrl* m_spinCtrl_followedMMSI;
		wxTextCtrl* m_textCtrl_mmsiComment;
		wxButton* m_button_addMmsiToList;
		wxCheckBox* m_checkBox_activateVesselFollowing;
		wxStaticText* m_staticText11;
		wxDataViewListCtrl* m_dataViewListCtrl_followedVessels;
		wxButton* m_button_deleteMmsiFromList;
		wxButton* m_button_clearMmsiList;

		// Virtual event handlers, override them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnButtonClick_startStream( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnButtonClick_stopStream( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCheckBox_FollowBoatMode( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnScroll_UpdateSearchBoxSize( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnScrollThumbRelease_UpdateSearchBoxSize( wxScrollEvent& event ) { event.Skip(); }
		virtual void OnButtonClick_AddMmsiToList( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_checkBox_activateVesselFollowingOnCheckBox( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDataViewListCtrlItemValueChanged_updateMmsiList( wxDataViewEvent& event ) { event.Skip(); }
		virtual void OnButtonClick_DeleteMmsiFromList( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnButtonClick_ClearMmsiList( wxCommandEvent& event ) { event.Skip(); }


	public:

		DialogMainGuiBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("AisLive Plugin"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 400,323 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER );

		~DialogMainGuiBase();

};

