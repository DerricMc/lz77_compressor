#include "Trie.h"

Trie::Trie()
{
	root = new trie_node();
	m_iMIN_MATCH = 1;
	m_iSlidingWindow = 4096;
}

Trie::Trie(int iMIN_MATCH)
{
	root = new trie_node();
	if(iMIN_MATCH <= 0)
		m_iMIN_MATCH = 1;
	else
		m_iMIN_MATCH = iMIN_MATCH;
	m_iSlidingWindow = 4096;
}

Trie::~Trie()
{
	clear();
}

void Trie::set_minimum_match(int iMIN_MATCH)
{
	if(iMIN_MATCH <= 0)
		m_iMIN_MATCH = 1;
	else
		m_iMIN_MATCH = iMIN_MATCH;
}
void Trie::set_sliding_window(int32_t iSlidingWindow)
{
	if(iSlidingWindow < 0 )
		m_iSlidingWindow = 4096;
	else
		m_iSlidingWindow = iSlidingWindow;
}

void Trie::clear_helper(trie_node* node)
{
	for(auto iter = node->children.begin(); iter != node->children.end(); iter++)
	{
		clear_helper(iter->second);	
		
	}
	delete node;
}

void Trie::clear()
{
	clear_helper(root);
}

length_offset Trie::slide_and_search(uint8_t* data_begin, uint8_t* cur_pos, uint8_t* la_buffer_end,  uint8_t* data_end)
{
	length_offset lo_pair = {0, 0};
	int offset = static_cast<int32_t>(cur_pos - data_begin);
	if((offset > 0) && (data_end-cur_pos >= m_iMIN_MATCH))
	{
		trie_node* node = find(cur_pos, la_buffer_end);	
		//If the node is equal to root then no prefix was found
		if(node->length > 0)
		{
			lo_pair.length = node->length;
		
			if(!node->offsets.empty())
				lo_pair.offset = offset - node->offsets.back();
			else
				lo_pair.offset = 0;
		}
	}
	int length = lo_pair.length < m_iMIN_MATCH ? 1 : lo_pair.length; //Represents how many prefixes to insert
	//If the the length of the prefix is greater or equal than the minimum match then insert a prefix starting at
	//cur_pos, cur_pos + 1, ..., cur_pos + length
	for(int i = 0; i < length;i++)
	{
		uint8_t* lookahead_end = (la_buffer_end  + i) < data_end ? la_buffer_end + i : data_end;
		insert(offset + i, cur_pos + i, lookahead_end);
	}
	
	for(int i = root->offsets.size(); i > m_iSlidingWindow; i--)
	{
		uint8_t* sliding_begin = data_begin + root->offsets.front();
		uint8_t* sliding_end = (sliding_begin + 18) < data_end ? sliding_begin + 18 : data_end;
		erase(sliding_begin, sliding_end);
	}
	return lo_pair;
}