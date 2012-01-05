#ifndef __LOOKUP_TABLE_H
#define __LOOKUP_TABLE_H
#include <map>
#include <deque>
#include <vector>
#include <cstdint>
#include "struct.h"


class lz77LookupTable
{
public:
	lz77LookupTable();
	lz77LookupTable(int32_t iMinimumMatch, int32_t iSlidingWindow=4096, int32_t iLookAheadWindow=18);
	~lz77LookupTable();
	length_offset search(uint8_t* cur_pos, uint8_t* data_begin, uint8_t* data_end);
	void setLookAheadWindow(int32_t iLookAheadWindow);
private:
	typedef std::multimap<std::vector<uint8_t>, int32_t> LookupTable;
	LookupTable table;
	int32_t m_iMinimumMatch;
	int32_t m_iSlidingWindow;
	int32_t m_iLookAheadWindow;
	
	
};

#endif //__LOOKUP_TABLE_H