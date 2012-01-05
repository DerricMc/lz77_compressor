#ifndef __LZ77TYPE11__
#define __LZ77TYPE11__


#include "lzBase.h"

class lz77Type11 : public lzBase {
public:
	lz77Type11(int32_t MinimumOffset=1,int32_t SlidingWindow=4096,int32_t MinimumMatch=3,int32_t BlockSize=8);
	enumCompressionResult Compress(const wxString& inStr, const wxString& outStr,uint64_t offset,uint64_t length);
	enumCompressionResult Decompress(const wxString& inStr, const wxString& outStr,uint64_t offset);
	
	
};

#endif
