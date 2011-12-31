#ifndef __TRIE_H__
#define __TRIE_H__
#include <vector>
#include <map>

struct trie_node
{
	std::vector<char_t> prefix;
	std::map<std::vector<uint8_t>, trie_node*> children;
	trie_node* parent;
	int offset;
	
};


class Trie
{
	public:
	Trie();
	~Trie();
	tire_node* insert(uint8_t prefix);
	private:
	void delete_node(trie_node*);
	
	private:
		trie_node* root;
	
};
#endif
