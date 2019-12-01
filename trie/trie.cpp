#include <stdint.h>
#include "bits.h"
#include "slice.h"
#include "trie.h"

namespace loghub {

void Trie::put(const Slice& key) {
    auto ptr = &_root;
    auto key_size = key.size() << 1;
    for (size_t i = 0; i < key_size; ++i) {
        auto word = Bits::get_bw4(key, i);
        if (ptr->branch[word] == nullptr) {
            ptr->branch[word] = new TrieNode();
            _size += sizeof(TrieNode);
        }

        ptr = ptr->branch[word]; 
    }

    ptr->has_value = true;
}

bool Trie::is_exist(const Slice& key) const {
    auto ptr = &_root;
    auto key_size = key.size() << 1;
    for (size_t i = 0; i < key_size; ++i) {
        auto word = Bits::get_bw4(key, i);
        if (!ptr->branch[word]) {
            return false;
        }
        
        ptr = ptr->branch[word];
    }

    return ptr->has_value;
}

} // end of namespace common 
