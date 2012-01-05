#ifndef __LZ77BASE__
#define __LZ77BASE__

#include "wx/string.h"
#include "wx/ffile.h"
#include "enum.h"
#include "struct.h"
//class Trie;
class lz77LookupTable;

enum class compressedType:int{
		None=-1,
		LZ00,
		LZ01,
		LZ10,
		LZ11
};

class lzBase {
public:
	lzBase(int MinimumOffset=1,int SlidingWindow=4096,int MinimumMatch=3,int BlockSize=8);
	~lzBase();
	virtual enumCompressionResult Compress(const wxString& inStr, const wxString& outStr,unsigned long offset,unsigned long length)=0;
	virtual enumCompressionResult Decompress(const wxString& inStr, const wxString& outStr,unsigned long offset)=0;
	bool FileIsCompressed(const wxString& inStr, uint8_t encoding, unsigned long offset=0);
	unsigned int decompressedFileLength(const wxString& inStr,unsigned long offset=0);
	static compressedType FileType(const wxString& inStr,unsigned long offset=0);
	
	void setSlidingWindow(int SlidingWindow){m_iSlidingWindow=SlidingWindow;}
	void setReadAheadBuffer(int ReadAheadBuffer){m_iReadAheadBuffer=ReadAheadBuffer;}
	void setMIN_MATCH(int MinimumMatch){m_iMIN_MATCH=MinimumMatch;}
	void setBlockSize(int BlockSize){m_iBlockSize=BlockSize;}
	void setMinimumOffset(unsigned int MinimumOffset){m_uiMinOffset=MinimumOffset;}
	int getSlidingWindow(){return m_iSlidingWindow;}
	int getReadAheadBuffer(){return m_iReadAheadBuffer;}
	int getMIN_MATCH(){return m_iMIN_MATCH;}
	int getBlockSize(){return m_iBlockSize;}
	unsigned getMinimumOffset(){return m_uiMinOffset;}
	
	
private:
	
	int submatch(const uint8_t* str1,const uint8_t* str2,const int len);
	length_offset window_search(uint8_t* beginSearchPtr, uint8_t* searchPosPtr,uint8_t* endLABufferPtr, uint8_t* startLBPtr);
protected:
	length_offset Search(uint8_t* data,uint8_t* posPtr, uint8_t* sizePtr);
	
	int m_iSlidingWindow;
	int m_iReadAheadBuffer;
	int m_iMIN_MATCH;//Minimum number of bytes that have to matched to go through with compression
	int m_iBlockSize;
	unsigned int m_uiMinOffset;
	//Trie* m_trie;
	lz77LookupTable* lz77Table;

};

#endif
