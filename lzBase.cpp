//#include "Trie.h"
#include "lookupTable.h"
#include "lzBase.h"


lzBase::lzBase(int MinimumOffset, int SlidingWindow, int MinimumMatch, int BlockSize){
	m_uiMinOffset = MinimumOffset;
	m_iSlidingWindow = SlidingWindow;
	m_iMIN_MATCH = MinimumMatch;
	m_iReadAheadBuffer = m_iMIN_MATCH;
	m_iBlockSize = BlockSize;
	//m_trie = new Trie(m_iMIN_MATCH);
	lz77Table = new lz77LookupTable();
}

lzBase::~lzBase()
{
	//delete m_trie;	
	delete lz77Table;
}
compressedType lzBase::FileType(const wxString& inStr,unsigned long offset){
	wxFFile infile;
	infile.Open(inStr,wxT("rb"));
	infile.Seek(offset);
	uint8_t encodeFlag;
	infile.Read(&encodeFlag,1);
	infile.Close();
	compressedType filetype;
	switch(encodeFlag){
	 case 0x00:
		filetype=compressedType::LZ00;
		break;
	 case 0x01:
	 	filetype=compressedType::LZ01;
	 	break;
	 case 0x10:
		filetype=compressedType::LZ10;
		break;
	 case 0x11:
	 	filetype=compressedType::LZ11;
	 	break;
	 default:
	 	filetype=compressedType::None;
	 	break;
	}
	return filetype;
}

bool lzBase::FileIsCompressed(const wxString& inStr, uint8_t encoding, unsigned long offset){
	wxFFile infile;
	infile.Open(inStr,wxT("rb"));
	infile.Seek(offset);
	uint8_t encodeFlag;
	infile.Read(&encodeFlag,1);
	infile.Close();
	return (encodeFlag == encoding);
}

unsigned int lzBase::decompressedFileLength(const wxString& inStr, unsigned long offset){
	wxFFile infile;
	infile.Open(inStr,wxT("rb"));
	infile.Seek(offset+1);
	unsigned int filesize=0;
	infile.Read(&filesize,3);
	if(filesize==0){
		infile.Read(&filesize,4);
	}
	infile.Close();
	return filesize;
}
/*This search function is my own work and is no way affilated with any one else
  I use the my own window_search function to drastically speed up the search function
  Normally a search for one byte is matched, then two, then three, all the way up
  to the size of the LookAheadBuffer. So I decided to skip the incremental search
  and search for the entire LookAheadBuffer and if I don't find the bytes are equal I return
  the next best match(which means if I look for 18 bytes and they are not found 18 bytess did not match,
  and 17 bytes did match then 17 bytes match is return).

*/
length_offset lzBase::Search(uint8_t* data,uint8_t* posPtr, uint8_t* sizePtr)
{
	length_offset results={0,0};

	//Returns negative 1 for Search failures since the current position is passed the size to be compressed
	if(posPtr >=sizePtr)
	{
		results.length=-1;
		return results;
	}

	uint8_t* search_window;
	//LookAheadBuffer is ReadAheadBuffer Size if there are more bytes than ReadAheadBufferSize waiting
	//to be compressed else the number of remaining bytes is the LookAheadBuffer
	int lookAheadBuffer_len=((int)(sizePtr-posPtr)<m_iReadAheadBuffer) ? (int)(sizePtr-posPtr) :m_iReadAheadBuffer;
	int sliding_buffer=(int)(posPtr - data)-m_iSlidingWindow;
	if(sliding_buffer > 0)
		search_window=data+sliding_buffer;	
	else
		search_window=data;
	
	uint8_t* endPos=posPtr+lookAheadBuffer_len;
	//results2 = m_trie->slide_and_search(data, posPtr, endPos, sizePtr);
	results = lz77Table->search(data, sizePtr, posPtr);
	
	/*
	if(!( (posPtr-data < 1)||( sizePtr-posPtr < m_iMIN_MATCH) ))
		results=window_search(search_window,posPtr,endPos,posPtr-m_uiMinOffset);
	if(results.length < m_iMIN_MATCH)
		assert(results2.compare_equal({0,0}));
	else
		assert(results2.compare_equal(results));
		*/
	return results;
}


//Returns the full length of string2 if they are equal else
//Return the number of characters that were equal before they weren't equal
int lzBase::submatch(const uint8_t* str1,const uint8_t* str2,const int len){
	for(int i=0;i<len;++i)
		if(str1[i]!=str2[i])
			return i;
	
	return len;
}

/*
Normally a search for one byte is matched, then two, then three, all the way up
  to the size of the LookAheadBuffer. So I decided to skip the incremental search
  and search for the entire LookAheadBuffer and if the function doesn't find the bytes are 
  equal the function return the next best match(which means if the function look for 18 bytes and they are not found, return
  the number of bytes that did match before it failed to match. The submatch is function returns the number of bytes that
  were equal, which can result up to the bytes total length if both byte strings are equal.
  
  
  ...[][][][][][][][][][][][]|[][][][][][][][][][][][][][]
                             |
     Search Window       Current Pos   LookAheadBuffer
    Up to 4096 bytes                   Up to 18 bytes
    			Sliding Window
    			Up to 4114 bytes
*/
length_offset lzBase::window_search(uint8_t* beginSearchPtr, uint8_t* searchPosPtr,uint8_t* endLABufferPtr, uint8_t* startLBPtr){
	int size=endLABufferPtr-beginSearchPtr;//Size of the entire sliding window
	int n=endLABufferPtr-searchPosPtr;
	length_offset result={0,0};
	int temp=0;
	if(n>size)//If the string that is being looked for is bigger than the string that is being searched
		return result;

	/*This makes sure that search for the searchPosPtr can be searched if an invalid position is given
	  An invalid position occurs if the amount of characters to search in_beginSearchPtr is less than the size
	  of searchPosPtr. In other words there has to be at least n characters left in the string 
	  to have a chance to find n characters*/
	
	do{	
		temp=submatch(startLBPtr,searchPosPtr,n);
		if(result.length<temp){
			result.length=temp;
			result.offset=(int)(searchPosPtr-startLBPtr);
		}
		if(result.length==n)
			return result;
		
		//ReadAheadBuffer is the maximum size of a character match
		
						
	
	}while(startLBPtr-->beginSearchPtr);
	return result;
}

