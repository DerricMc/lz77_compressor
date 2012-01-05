#include "lookupTable.h"


lz77LookupTable::lz77LookupTable()
{
	m_iMinimumMatch = 3;
	m_iSlidingWindow = 4096;	
	m_iLookAheadWindow = 18;
}

lz77LookupTable::lz77LookupTable(int32_t iMinimumMatch, int32_t iSlidingWindow, int32_t iLookAheadWindow)
{
	if(iMinimumMatch > 0 )
		m_iMinimumMatch = iMinimumMatch;
	else
		m_iMinimumMatch = 3;
	if(iSlidingWindow > 0)
		m_iSlidingWindow = iSlidingWindow;
	else
		m_iSlidingWindow = 4096;
	if(iLookAheadWindow > 0)
		m_iLookAheadWindow = iLookAheadWindow;
	else
		m_iLookAheadWindow = 18;
}

lz77LookupTable::~lz77LookupTable()
{}

length_offset lz77LookupTable::search(uint8_t* data_begin, uint8_t* data_end, uint8_t* cur_pos)
{
	length_offset lo_pair = {0,0};
	std::vector<uint8_t> vec(cur_pos, cur_pos + m_iMinimumMatch);
	int32_t currentOffset = static_cast<int32_t>(cur_pos - data_begin);
	//Find code
	if(currentOffset > 0 && (data_end - cur_pos) >= m_iMinimumMatch)
	{
		auto elements = table.equal_range(vec);
		elements.second--;
		elements.first--;
		//Iterate over keys in reverse order. C++11 guarantees that the relative order of elements is maintained for the same key
		for(auto iter = elements.second; iter != elements.first; iter--)
		{
			int32_t matchLength = m_iMinimumMatch;
			int32_t lookAheadBufferLength = ((data_end - cur_pos) < m_iLookAheadWindow) ? static_cast<int32_t>(data_end - cur_pos) : m_iLookAheadWindow;
			for(; matchLength < lookAheadBufferLength; ++matchLength)
			{
				if(*(data_begin + iter->second + matchLength) != *(cur_pos + matchLength))
					break;
			}
			//Store the longest match found so far into length_offset struct.
			//When lengths are the same the closer offset to the lookahead buffer wins
			if(lo_pair.length < matchLength)
			{
				lo_pair.length = matchLength;
				lo_pair.offset = currentOffset - iter->second;
			}
			//Found the longest match so break out of loop
			if(lo_pair.length == m_iLookAheadWindow)
				break;
		}
	}
	//end find code
	//Insert code
	table.insert(std::make_pair(vec, currentOffset));
	for(int32_t i = 1; i < lo_pair.length; i++)
	{
		if(data_end - (cur_pos + i) < m_iMinimumMatch)
			break;
		std::copy(cur_pos + i, cur_pos + m_iMinimumMatch + i, vec.begin());
		table.insert(std::make_pair(vec, currentOffset + i));	
			
	}
	//end insert code
	//Delete code
	int32_t slidingWindowOffset = std::max(0, currentOffset - m_iSlidingWindow);//Absolute offset
	int32_t tablesize=static_cast<int32_t>(table.size());
	for(int32_t i = 0; i < tablesize - m_iSlidingWindow; ++i)
	{
		std::copy(data_begin + slidingWindowOffset + i, data_begin + slidingWindowOffset + m_iMinimumMatch + i, vec.begin());
		auto elements = table.equal_range(vec);
		for(auto iter = elements.first; iter != elements.second; iter++)
		{
			if(slidingWindowOffset + i == iter->second)
			{
				table.erase(iter);
				//There should no occurance of the map with the same value
				break;
			}
		}
	}
	//end delete code
	return lo_pair;
	//break lookupTable.cpp:77 if table.size()> 4096
  //break lookupTable.cpp:92 if table.size()> 4096

}