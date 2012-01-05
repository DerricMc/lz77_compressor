//---------------------------------------------------------------------------
//
// Name:        lz77Frame.h
// Author:      Derric
// Created:     1/27/2009 10:07:25 PM
// Description: lz77Frame class declaration
//
//---------------------------------------------------------------------------

#ifndef __lz77Frame_h__
#define __lz77Frame_h__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif


#include <wx/filedlg.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/regex.h>
#include <wx/ffile.h>
#include <wx/fileconf.h>

#define lz77_FRAME_STYLE wxTAB_TRAVERSAL| wxMAXIMIZE_BOX |wxCLOSE_BOX|wxRESIZE_BORDER|wxMINIMIZE_BOX

class lz77Frame : public wxFrame
{
	
	public:	
		lz77Frame(wxWindow *parent, wxWindowID id = -1, const wxString &title = wxT("lz77 compressor"),const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = lz77_FRAME_STYLE);
		void onOk(wxCommandEvent& event);
		void onClose(wxCommandEvent& event);
		void onFileInput(wxCommandEvent& event);
		void onFileOutput(wxCommandEvent& event);
		void onStartOffset(wxCommandEvent& event);
		void onSize(wxCommandEvent& event);
		
		DECLARE_EVENT_TABLE()
	private:
		void CreateGUIControls();
		wxPanel *lz77Panel;
		wxFileDialog *InfileDialog, *OutfileDialog;
		wxStaticText *tcDesc1, *tcDesc2, *tcDesc3, *tcDesc5;
		wxTextCtrl *Size, *InputStartOff, *Inputfile,*Outputfile;
		wxStaticBox *OutputInfo, *InputInfo;
		wxButton *BrowseButton1, *BrowseButton2, *CloseButton, *OKButton;
		wxRadioBox *compressOptions,*algorithmOptions;
		wxGridBagSizer *sizer;
		wxFileConfig *configfile;
		//Defines the numerical sizes
	  long sizeToCompress;
	  long in_offset;
	  long out_offset;
		
		enum
		{
			ID_tcDesc5 = 1000,
			ID_SIZE,
			ID_OutputInfo,
			ID_BROWSEBUTTON2,
			ID_BROWSEBUTTON1,
			ID_INPUT_OFF,
			ID_tcDesc3,
			ID_tcDesc2,
			ID_CLOSEBUTTON,
			ID_OUTPUTFILE,
			ID_INPUTFILE,
			ID_OKBUTTON,
			ID_tcDesc1,
			ID_INPUT_INFO,
			ID_compressOptions,
			ID_MAIN_PANEL,
			ID_algorithmOptions
		};
		
};

#endif
