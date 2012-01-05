#ifndef __TRIE_H__
#define __TRIE_H__
#include <vector>
#include <map>
#include <cstdint>
#include <iterator>
#include <deque>
#include "struct.h"

struct trie_node
{
	std::map<uint8_t, trie_node*> children;
	trie_node* parent;
	std::deque<int32_t> offsets;
	int32_t length;
	
	trie_node()
	{
		length = 0;
		parent = nullptr;	
	}
	
};


class Trie
{
public:
		typedef std::vector<uint8_t>::iterator iterator;
	Trie();
	Trie(int iMIN_MATCH);
	~Trie();
	void clear();
	/*
	@parms
	offset - the beginning of the data to compress buffer
	cur_pos - current position in data to comressed
	la_buffer_end - end of the lookahead buffer
	data_end - end of the data to compress buffer
	
	*/
	length_offset slide_and_search(uint8_t* data_begin, uint8_t* begin, uint8_t* la_buffer_end, uint8_t* data_end);
	void set_minimum_match(int32_t iMIN_MATCH);
	void set_sliding_window(int32_t iSlidingWindow);
	//Insert operations
	template<class InputIterator>
	bool insert(int32_t offset, InputIterator begin, InputIterator end);
	bool insert(int32_t offset, std::vector<uint8_t> vec)
	{
		return insert(offset, vec.begin(), vec.end());
	}
	bool insert(int32_t offset, uint8_t* pointer, uint32_t length)
	{
		return insert(offset, pointer, pointer + length);	
	}
	//end insert operations
	
	//Delete operations
	template<class InputIterator>
	void erase(InputIterator begin, InputIterator end);
	void erase(std::vector<uint8_t> vec)
	{
		return erase(vec.begin(), vec.end());	
	}
	void erase(uint8_t* pointer, uint32_t length)
	{
		return erase(pointer, pointer + length);
	}
	//end delete operations
	
	//Find operations
	template<class InputIterator>
	trie_node* find(InputIterator begin, InputIterator end);
	
	trie_node* find(std::vector<uint8_t> vec)
	{
		return find(vec.begin(), vec.end());
	}
	trie_node* find(uint8_t* pointer, uint32_t length)
	{
		return find(pointer, pointer + length);	
	}
	//end fine operations
private:
	void clear_helper(trie_node* node);
	template<class InputIterator>
	trie_node* find_helper(trie_node* node, InputIterator begin, InputIterator end);
	template<class InputIterator>
	trie_node* insert_helper(trie_node* node, int32_t offset, InputIterator begin, InputIterator end);
	template<class InputIterator>
	bool erase_helper(trie_node* node, InputIterator begin, InputIterator end);
private:
		trie_node* root; //Root node also acts as not found indicator
		int32_t m_iMIN_MATCH;
		int32_t m_iSlidingWindow;
	
};

template<class InputIterator>
trie_node* Trie::find_helper(trie_node* node, InputIterator begin, InputIterator end)
{
	if(begin >= end)
		return node;
	std::map<uint8_t, trie_node*>::const_iterator iter = node->children.find(*begin);
	if(iter == node->children.end())
		return node;
	else
		return find_helper(iter->second, begin + 1, end);
}

template<class InputIterator>
trie_node* Trie::find(InputIterator begin, InputIterator end)
{
	return find_helper(root, begin, end);
}

template<class InputIterator>
trie_node* Trie::insert_helper(trie_node* node, int32_t offset, InputIterator begin, InputIterator end)
{
	node->offsets.push_back(offset);
	if(begin < end)
	{
		trie_node* cur_node;
		std::map<uint8_t, trie_node*>::iterator iter = node->children.lower_bound(*begin);
		if(iter != node->children.end() && !node->children.key_comp()(*begin, iter->first))//Found the node
		{
			cur_node = iter->second;
			
		}
		else
		{
			cur_node = new trie_node();
			cur_node->length = node->length + 1;
			cur_node->parent = node;
			node->children.insert(iter, std::map<uint8_t, trie_node*>::value_type(*begin, cur_node));
		}
		return insert_helper(cur_node, offset, begin + 1, end);
	}
	return node;
  
}

template<class InputIterator>
bool Trie::insert(int32_t offset, InputIterator begin, InputIterator end)
{
	insert_helper(root, offset, begin, end);
	return true;
}

template<class InputIterator>
bool Trie::erase_helper(trie_node* node, InputIterator begin, InputIterator end)
{
	if(begin < end)
	{
		std::map<uint8_t, trie_node*>::iterator iter = node->children.find(*begin);
		if(iter != node->children.end())
		{
			bool can_remove = erase_helper(iter->second, begin + 1, end);
			if(can_remove)
			{
				node->children.erase(iter);
			}
		}
	}
	node->offsets.pop_front();
	if(node->offsets.empty() && node->children.empty())
	{
		delete node;//At this point the node should not have any children
		return true;
	}
	else
		return false;
	
}

template<class InputIterator>
void Trie::erase(InputIterator begin, InputIterator end)
{
	erase_helper(root, begin, end);
}

#endif
