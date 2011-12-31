//---------------------------------------------------------------------------
//
// Name:        lz77Frame.cpp
// Author:      Derric
// Created:     1/27/2009 10:07:25 PM
// Description: lz77Frame class implementation
//
//---------------------------------------------------------------------------
#include "lz77Frame.h"
#include "lz77Type10.h"
#include "lz77Type11.h"


BEGIN_EVENT_TABLE(lz77Frame,wxFrame)
	EVT_TEXT(ID_SIZE,lz77Frame::onSize)
	EVT_BUTTON(ID_BROWSEBUTTON2,lz77Frame::onFileOutput)
	EVT_BUTTON(ID_BROWSEBUTTON1,lz77Frame::onFileInput)
	EVT_TEXT(ID_INPUT_OFF,lz77Frame::onStartOffset)
	EVT_BUTTON(ID_CLOSEBUTTON,lz77Frame::onClose)
	EVT_BUTTON(ID_OKBUTTON,lz77Frame::onOk)
	EVT_TEXT_ENTER(ID_OUTPUTFILE,lz77Frame::onOk)
	EVT_TEXT_ENTER(ID_INPUTFILE,lz77Frame::onOk)
	EVT_TEXT_ENTER(ID_INPUT_OFF,lz77Frame::onOk)
	EVT_TEXT_ENTER(ID_SIZE,lz77Frame::onOk)
END_EVENT_TABLE()


lz77Frame::lz77Frame(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint& pos, const wxSize& size, long style)
: wxFrame(parent, id, title,pos,size,style)
{
	CreateGUIControls();
	sizeToCompress=0;
	in_offset=0;
	out_offset=0;
	
}





void lz77Frame::CreateGUIControls()
{
	lz77Panel=new wxPanel(this,ID_MAIN_PANEL,wxDefaultPosition,wxDefaultSize,wxTAB_TRAVERSAL);
	SetTitle(wxS("FE lz77 compressor"));
	SetIcon(wxNullIcon);
	configfile=new wxFileConfig(wxEmptyString,wxEmptyString,wxS("lz77.conf"),wxEmptyString,
		wxCONFIG_USE_LOCAL_FILE|wxCONFIG_USE_RELATIVE_PATH);
	wxConfigBase::Set(configfile);
	configfile->SetRecordDefaults(true);
	wxString DefaultInputDir;
	configfile->Read(wxS("input-dir"),&DefaultInputDir,wxEmptyString);
	wxString DefaultOutputDir;
	configfile->Read(wxS("output-dir"),&DefaultOutputDir,wxEmptyString);
	
	wxArrayString includeList;
	includeList.Alloc(24);
	includeList.Add(wxS('0'));includeList.Add(wxS('1'));includeList.Add(wxS('2'));includeList.Add(wxS('3'));includeList.Add(wxS('4'));
	includeList.Add(wxS('5'));includeList.Add(wxS('6'));includeList.Add(wxS('7'));includeList.Add(wxS('8'));includeList.Add(wxS('9'));
	includeList.Add(wxS('a'));includeList.Add(wxS('b'));includeList.Add(wxS('c'));includeList.Add(wxS('d'));includeList.Add(wxS('e'));includeList.Add(wxS('f'));
	includeList.Add(wxS('A'));includeList.Add(wxS('B'));includeList.Add(wxS('C'));includeList.Add(wxS('D'));includeList.Add(wxS('E'));includeList.Add(wxS('F'));
	includeList.Add(wxS('$'));includeList.Add(wxS('%'));
	wxTextValidator hexValidator(wxFILTER_INCLUDE_CHAR_LIST);
	hexValidator.SetIncludes(includeList);

	InfileDialog =  new wxFileDialog(lz77Panel, wxS("Choose a file"), DefaultInputDir, wxS(""), 
		wxS("All Files|*|LZ77 Type 10 Files(*.cms;*.tpl)|*.cms;*.tpl|LZ77 Type 11 Files(*.one;*.onz)|*.one;*.onz"), wxFD_OPEN);

	OutfileDialog =  new wxFileDialog(lz77Panel, wxS("Choose a file"), DefaultOutputDir, wxS(""), 
		wxS("All Files|*|LZ77 Type 10 Files(*.cms;*.tpl)|*.cms;*.tpl|LZ77 Type 11 Files(*.one;*.onz)|*.one;*.onz"), wxFD_SAVE);

	InputInfo = new wxStaticBox(lz77Panel, ID_INPUT_INFO, wxS("Input file info"), wxDefaultPosition, wxSize(-1, -1));
	OutputInfo = new wxStaticBox(lz77Panel, ID_OutputInfo, wxS("Output file info"), wxDefaultPosition, wxSize(-1, -1));
	tcDesc1 = new wxStaticText(lz77Panel, ID_tcDesc1, wxS("Offset(HEX)"), wxDefaultPosition, wxDefaultSize, 0, wxS("tcDesc1"));
	tcDesc2 = new wxStaticText(lz77Panel, ID_tcDesc2, wxS("Outputfile"), wxDefaultPosition, wxDefaultSize, 0, wxS("tcDesc2"));
	tcDesc3 = new wxStaticText(lz77Panel, ID_tcDesc3, wxS("Inputfile"), wxDefaultPosition, wxDefaultSize, 0, wxS("tcDesc3"));
	tcDesc5 = new wxStaticText(lz77Panel, ID_tcDesc5, wxS("Size from Offset(DEC)"), wxDefaultPosition, wxDefaultSize, 0, wxS("tcDesc5"));

	wxArrayString compressArray;
	compressArray.Add(wxS("Compress"));
	compressArray.Add(wxS("DeCompress"));
	
	compressOptions = new wxRadioBox(lz77Panel, ID_compressOptions, wxS("Options"), wxDefaultPosition, wxSize(-1, -1), compressArray, 1, wxRA_SPECIFY_COLS);
	compressOptions->SetSelection(0);
	
	wxArrayString algorithmArray;
	algorithmArray.Add(wxS("Best LZ77 Type 10"));
	algorithmArray.Add(wxS("Exact Wii LZ77 Type 10"));
	algorithmArray.Add(wxS("Wii LZ77 Type 11"));
	algorithmOptions = new wxRadioBox(lz77Panel, ID_algorithmOptions, wxS("Compression Algorithm"), wxDefaultPosition, wxSize(-1, -1), algorithmArray, 2, wxRA_SPECIFY_COLS);
	algorithmOptions->SetSelection(0);
	
	Inputfile = new wxTextCtrl(lz77Panel, ID_INPUTFILE, wxS(""), wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER, wxDefaultValidator, wxS("Inputfile"));
	BrowseButton1 = new wxButton(lz77Panel, ID_BROWSEBUTTON1, wxS("Browse"), wxDefaultPosition, wxSize(-1, -1), 0, wxDefaultValidator, wxS("BrowseButton1"));
	InputStartOff = new wxTextCtrl(lz77Panel, ID_INPUT_OFF, wxS(""), wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER, hexValidator, wxS("InputStartOff"));
	Size = new wxTextCtrl(lz77Panel, ID_SIZE, wxS(""), wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC), wxS("Size"));
	Size->SetMaxLength(10);
	Outputfile = new wxTextCtrl(lz77Panel, ID_OUTPUTFILE, wxS(""), wxDefaultPosition, wxSize(-1, -1), wxTE_PROCESS_ENTER, wxDefaultValidator, wxS("Outputfile"));
	BrowseButton2 = new wxButton(lz77Panel, ID_BROWSEBUTTON2, wxS("Browse"),wxDefaultPosition, wxSize(-1, -1), 0, wxDefaultValidator, wxS("BrowseButton2"));
	OKButton = new wxButton(lz77Panel, ID_OKBUTTON, wxS("OK"), wxDefaultPosition, wxSize(-1, -1), 0, wxDefaultValidator, wxS("OKButton"));
	CloseButton = new wxButton(lz77Panel, ID_CLOSEBUTTON, wxS("Close"), wxDefaultPosition, wxSize(-1, -1), 0, wxDefaultValidator, wxS("CloseButton"));
	/*
	Inputfile->MoveAfterInTabOrder(compressOptions);
	BrowseButton1->MoveAfterInTabOrder(Inputfile);
	Outputfile->MoveAfterInTabOrder(BrowseButton1);
	BrowseButton2->MoveAfterInTabOrder(Outputfile);
	InputStartOff->MoveAfterInTabOrder(BrowseButton2);
	Size->MoveAfterInTabOrder(InputStartOff);
	OKButton->MoveAfterInTabOrder(Size);
	CloseButton->MoveAfterInTabOrder(OKButton);
	*/
	
	sizer=new wxGridBagSizer(15,15);
	lz77Panel->SetSizer(sizer);
	wxGridBagSizer *gbSizer=new wxGridBagSizer(5,5);
	gbSizer->Add(compressOptions,wxGBPosition(0,0),wxGBSpan(1,1));
	gbSizer->Add(algorithmOptions,wxGBPosition(0,1),wxGBSpan(1,1));
	sizer->Add(gbSizer,wxGBPosition(0,0),wxGBSpan(1,1));
	
	wxStaticBoxSizer *sbSizer=new wxStaticBoxSizer(InputInfo,wxHORIZONTAL);
	gbSizer=new wxGridBagSizer(5,5);
	gbSizer->Add(tcDesc3,wxGBPosition(0,0),wxGBSpan(1,1),wxEXPAND);
	gbSizer->Add(150,0,wxGBPosition(0,1),wxGBSpan(1,1),wxEXPAND);
	gbSizer->Add(Inputfile,wxGBPosition(1,0),wxGBSpan(1,2),wxEXPAND);
	gbSizer->Add(BrowseButton1,wxGBPosition(1,2),wxGBSpan(1,1),wxSHAPED);
	
	gbSizer->Add(tcDesc1,wxGBPosition(2,0),wxGBSpan(1,1),wxEXPAND);
	gbSizer->Add(InputStartOff,wxGBPosition(3,0),wxGBSpan(1,1),wxSHAPED);
	gbSizer->Add(tcDesc5,wxGBPosition(2,1),wxGBSpan(1,1),wxEXPAND);
	gbSizer->Add(Size,wxGBPosition(3,1),wxGBSpan(1,1),wxSHAPED);
	sbSizer->Add(gbSizer,1);
	sizer->Add(sbSizer,wxGBPosition(1,0),wxGBSpan(1,1),wxSHAPED);
	
	sbSizer=new wxStaticBoxSizer(OutputInfo,wxHORIZONTAL);
	gbSizer=new wxGridBagSizer(5,5);
	gbSizer->Add(tcDesc2,wxGBPosition(0,0),wxGBSpan(1,1),wxEXPAND);
	gbSizer->Add(150,0,wxGBPosition(0,1),wxGBSpan(1,1),wxEXPAND);
	gbSizer->Add(Outputfile,wxGBPosition(1,0),wxGBSpan(1,2),wxEXPAND);
	gbSizer->Add(BrowseButton2,wxGBPosition(1,2),wxGBSpan(1,1),wxSHAPED);
	sbSizer->Add(gbSizer,1);
	sizer->Add(sbSizer,wxGBPosition(2,0),wxGBSpan(1,1),wxSHAPED);
		
	sizer->Add(OKButton,wxGBPosition(3,0),wxDefaultSpan,wxSHAPED);
	sizer->Add(CloseButton,wxGBPosition(4,0),wxDefaultSpan,wxSHAPED);
	sizer->SetFlexibleDirection(wxBOTH);
	sizer->SetSizeHints(this);
	Fit();
}




/*
	Return true if the file is to be compressed 
	Return false if the file is not to be compressed.
*/
void lz77Frame::onOk(wxCommandEvent& event)
{
	wxString message;
	unsigned int filesize;
	unsigned long bytesToCompress=sizeToCompress;
	wxFFile infile, outfile;
	
	infile.Open(Inputfile->GetValue(), wxS("rb"));
		
	if(infile.IsOpened()==false)
	{
		wxMessageBox(wxS("Error opening input file\nfile may be in use by another program or may not exist"),wxS("Error"),wxOK,this);
		return;
	}

	filesize=infile.Length();
	infile.Close();
	
	if(filesize<in_offset)
	{
		wxMessageBox(wxS("Offset is bigger than the size of the file"),wxS("Error"),wxOK,this);
		return;
	}
	else if(filesize<sizeToCompress)
	{
		wxMessageBox(wxS("Size to compress is bigger than the total size of the file"),wxS("Error"),wxOK,this);
		return;
	}
	else if(filesize<in_offset+sizeToCompress)
	{
		wxMessageBox(wxS("Offset + size to compress is bigger than the total size of the file"),wxS("Error"),wxOK,this);
		return;
	}
	
	//Making sure the number of bytes to compress is not more than the file
	if(sizeToCompress<=0 || sizeToCompress>(filesize-in_offset))
	{
		bytesToCompress=filesize-in_offset;
	}

		
	 enumCompressionResult result = enumCompressionResult::SUCCESS;
	
	if((enumCompressionMode)compressOptions->GetSelection()==enumCompressionMode::compressSelect){
		switch(algorithmOptions->GetSelection()){
		 case  enumCompressionType::BEST_LZ77_TYPE_10:
		 	{
		 		lz77Type10 lz77(18,1);
		 		result=lz77.Compress(Inputfile->GetValue(),Outputfile->GetValue(),in_offset,bytesToCompress);
		 	}
		 	break;
		 case  enumCompressionType::EXACT_WII_LZ77_TYPE_10:
		 	{
		 		lz77Type10 lz77(18,2);
		 		result=lz77.Compress(Inputfile->GetValue(),Outputfile->GetValue(),in_offset,bytesToCompress);
		 	}
		 	break;
		 case  enumCompressionType::WII_LZ77_TYPE_11:
		 	{
		 		lz77Type11 lz77;
		 		result=lz77.Compress(Inputfile->GetValue(),Outputfile->GetValue(),in_offset,bytesToCompress);
		 	}
		 	break;
		}
		
	}
	else{
		switch(lzBase::FileType(Inputfile->GetValue(),in_offset)){
		 case compressedType::LZ00:
		 case compressedType::LZ01:
		 	result= enumCompressionResult::COMPRESSED_FORMAT_NOT_SUPPORTED;
		 	break;
		 case compressedType::LZ10:
		 	{lz77Type10 lz77;
		 	result=lz77.Decompress(Inputfile->GetValue(),Outputfile->GetValue(),in_offset);}
		 	break;
		 case compressedType::LZ11:
		 	{lz77Type11 lz77;
		 	result=lz77.Decompress(Inputfile->GetValue(),Outputfile->GetValue(),in_offset);	}
		 	break;
		 case compressedType::None:
		 	result= enumCompressionResult::FILE_NOT_COMPRESSED;
		 	break;
		}
				
	}

	

	switch(result){
	 case enumCompressionResult::SUCCESS:
	 	wxMessageBox(wxS("Success"),wxS("Results"),wxOK,this);
	 	break;
	 case enumCompressionResult::COULD_NOT_READ_LENGTH_BYTES:
	 	wxMessageBox(wxString::Format(wxS("Could not read %u from input file"),bytesToCompress),wxS("Error"),wxOK,this);
		break;
	 case enumCompressionResult::FILE_NOT_OPENED:
		wxMessageBox(wxS("Output file could not be opened"),wxS("Error"),wxOK,this);
		break;
	 case enumCompressionResult::FILE_NOT_COMPRESSED:
		wxMessageBox(wxS("Input file is not compressed"),wxS("Error"),wxOK,this);
		break;
	 case enumCompressionResult::INVALID_COMPRESSED_DATA:
		wxMessageBox(wxS("Compressed Data is invalid"),wxS("Error"),wxOK,this);
		break;
	 case enumCompressionResult::UNCOMPRESSED_SIZE_INVALID:
		wxMessageBox(wxS("Uncompressed file size is not correct"),wxS("Error"),wxOK,this);
		break;
	 case enumCompressionResult::COMPRESSED_FORMAT_NOT_SUPPORTED:
		wxMessageBox(wxS("Compressed format is not supported yet"),wxS("Info"),wxOK,this);
		break;
	 default:
		break;
	 }
		
	
	
}

void lz77Frame::onClose(wxCommandEvent& event)
{
	Close();
}


void lz77Frame::onFileInput(wxCommandEvent& event)
{
	// Gets input filename from user

	if(InfileDialog->ShowModal()==wxID_OK){
		if(InfileDialog->GetFilename()==wxEmptyString)
			return;

	Inputfile->SetValue(InfileDialog->GetPath());
	}
	configfile->Write(wxS("input-dir"),InfileDialog->GetDirectory());
}

void lz77Frame::onFileOutput(wxCommandEvent& event)
{
	if(OutfileDialog->ShowModal()==wxID_OK){
		if(OutfileDialog->GetFilename()==wxEmptyString)
			return;

	Outputfile->SetValue(OutfileDialog->GetPath());
	}
	configfile->Write(wxS("output-dir"),OutfileDialog->GetDirectory());
}


void lz77Frame::onStartOffset(wxCommandEvent& event)
{
	wxRegEx match(wxS("(^[%|$]?[0-9]|[a-f]|[A-F]+$)"));
	bool results=false;
	results=match.Matches(InputStartOff->GetValue());
	if (results)
		InputStartOff->GetValue().ToLong(&in_offset,16);
	else
		in_offset=0;
}


void lz77Frame::onSize(wxCommandEvent& event)
{
	wxRegEx match(wxS("(^[0-9]+$)"));
	bool results=false;
	results=match.Matches(Size->GetValue());
	if (results)
		Size->GetValue().ToLong(&sizeToCompress,10);
	else
		sizeToCompress=0;


}

