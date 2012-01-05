#ifndef __LZ77TYPE10__
#define __LZ77TYPE10__

#include "lzBase.h"

class lz77Type10 : public lzBase {
public:
	lz77Type10(int MinimumOffset=1, int SlidingWindow=4096, int MinimumMatch=3, int BlockSize=8);
	enumCompressionResult Compress(const wxString& inStr, const wxString& outStr,unsigned long offset,unsigned long length);
	enumCompressionResult Decompress(const wxString& inStr, const wxString& outStr,unsigned long offset);
	
};

#endif
