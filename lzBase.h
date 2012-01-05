#ifndef __LZ77BASE__
#define __LZ77BASE__

#include "wx/string.h"
#include "wx/ffile.h"
#include "enum.h"
#include "struct.h"
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
	lzBase(int32_t MinimumOffset=1,int32_t SlidingWindow=4096,int32_t MinimumMatch=3,int32_t BlockSize=8);
	~lzBase();
	virtual enumCompressionResult Compress(const wxString& inStr, const wxString& outStr,uint64_t offset,uint64_t length)=0;
	virtual enumCompressionResult Decompress(const wxString& inStr, const wxString& outStr,uint64_t offset)=0;
	bool FileIsCompressed(const wxString& inStr, uint8_t encoding, uint64_t offset=0);
	uint32_t decompressedFileLength(const wxString& inStr,uint64_t offset=0);
	static compressedType FileType(const wxString& inStr,uint64_t offset=0);
	
	void setSlidingWindow(int32_t SlidingWindow){m_iSlidingWindow=SlidingWindow;}
	void setReadAheadBuffer(int32_t ReadAheadBuffer){m_iReadAheadBuffer=ReadAheadBuffer;}
	void setMIN_MATCH(int32_t MinimumMatch){m_iMIN_MATCH=MinimumMatch;}
	void setBlockSize(int32_t BlockSize){m_iBlockSize=BlockSize;}
	void setMinimumOffset(uint32_t MinimumOffset){m_uiMinOffset=MinimumOffset;}
	int32_t getSlidingWindow(){return m_iSlidingWindow;}
	int32_t getReadAheadBuffer(){return m_iReadAheadBuffer;}
	int32_t getMIN_MATCH(){return m_iMIN_MATCH;}
	int32_t getBlockSize(){return m_iBlockSize;}
	unsigned getMinimumOffset(){return m_uiMinOffset;}
	
	
private:
	
	int32_t submatch(const uint8_t* str1,const uint8_t* str2,const int32_t len);
	length_offset window_search(uint8_t* beginSearchPtr, uint8_t* searchPosPtr,uint8_t* endLABufferPtr, uint8_t* startLBPtr);
protected:
	length_offset Search(uint8_t* posPtr, uint8_t* data_begin, uint8_t* data_end);
	
	int32_t m_iSlidingWindow;
	int32_t m_iReadAheadBuffer;
	int32_t m_iMIN_MATCH;//Minimum number of bytes that have to matched to go through with compression
	int32_t m_iBlockSize;
	uint32_t m_uiMinOffset;
	lz77LookupTable* lz77Table;

};

#endif
