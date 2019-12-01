#pragma once

#include <stdint.h>
#include "slice.h"
#include "trie.h"

namespace opencdn {
namespace loghub {

struct CompactTrieNode {
    CompactTrieNode(): bitmap(0), has_value(false) {}
    ~CompactTrieNode() = default;
    bool operator==(const CompactTrieNode& rhs) const;

    uint16_t bitmap;
    bool has_value;
    void* children[0];
};

class CompactTrie {
public:
    CompactTrie(const Trie& trie);
    CompactTrie(): _root(nullptr), _size(0), _cnt(0) {}
    ~CompactTrie(); 

    CompactTrieNode* do_compact(const Trie::TrieNode& trie);
    int put(const Slice& key); 

    std::string to_string() const;
    int decode(const std::string& serial);

    bool is_exist(const Slice& key) const;
    inline uint64_t size() const { return _size; }
    inline uint64_t count() const { return _cnt; }
    const CompactTrieNode* root() const { return _root; }

private:
    void destroy(CompactTrieNode* node);

private:
    CompactTrieNode *_root;
    uint64_t _size;
    uint64_t _cnt;
};

} // end of namespace loghub
} // end of namespace opencdn 
