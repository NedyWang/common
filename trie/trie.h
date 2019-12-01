#pragma once

#include <stdint.h>
#include <string>
#include <vector>

// level db
#include "slice.h"

namespace common {

class CompactTrie;
class Trie {
public:
    struct TrieNode {
        TrieNode() {
            has_value = false;
            bzero(branch, sizeof(branch));
        }

        bool has_value;
        TrieNode *branch[16];
    };

    Trie(): _size(sizeof(_root)) {}
    ~Trie() = default;
    
    void put(const Slice& key);
    bool is_exist(const Slice& key) const;
    inline uint64_t size() const { return _size; };

friend class CompactTrie;
private:
    TrieNode _root;
    uint64_t _size;
};

} // end of namespace common 
