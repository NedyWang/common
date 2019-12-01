#include <cctype>
#include <deque>
#include <stdint.h>

#include "bits.h"
#include "compact_trie.h"
#include "slice.h"

namespace opencdn {
namespace loghub {

CompactTrie::CompactTrie(const Trie& trie) {
    _size = 0;
    _cnt = 0;
    _root = do_compact(trie._root);
}

CompactTrie::~CompactTrie() {
    destroy(_root);
}

void CompactTrie::destroy(CompactTrieNode* node) {
    if (node && node->bitmap) {
        int branch_num = Bits::ones_count<uint16_t>(node->bitmap);
        for (int i = 0; i < branch_num; ++i) {
            assert(node->children[i]);
            destroy(reinterpret_cast<CompactTrieNode*>(node->children[i]));
        }
        
        free(reinterpret_cast<void*>(node));
    }

    node = nullptr;
}

bool CompactTrieNode::operator==(const CompactTrieNode& rhs) const {
    if (this->bitmap != rhs.bitmap) {
        return false;
    }

    if (this->has_value != rhs.has_value) {
        return false;
    }

    int cnt = Bits::ones_count<uint16_t>(this->bitmap);
    for (int i = 0; i < cnt; ++i) {
        auto& ch = *(reinterpret_cast<CompactTrieNode*>(this->children[i]));
        auto& ch_rhs = *(reinterpret_cast<CompactTrieNode*>(rhs.children[i]));

        if (!(ch == ch_rhs)) {
            return false;
        }
    } 

    return true;
}

CompactTrieNode* CompactTrie::do_compact(const Trie::TrieNode& trie) {
    int cnt = 0;
    for (int i = 0; i < 16; ++i) {
        if (trie.branch[i]) {
            ++cnt;
        }
    }

    int size = sizeof(CompactTrieNode) + (cnt << 3);

    void* mem = malloc(size);
    bzero(mem, size);
    CompactTrieNode* root = new(mem) CompactTrieNode();

    root->has_value = trie.has_value;
    root->bitmap = 0;

    cnt = 0;
    for (int i = 0; i < 16; ++i) {
        if (trie.branch[i]) {
            root->bitmap |= (1 << i);
            root->children[cnt++] = (void*)do_compact(*(trie.branch[i]));
        }
    }

    _size += size;
    ++_cnt;

    return root;
}

bool CompactTrie::is_exist(const Slice& key) const {
    auto ptr = _root;
    auto key_size = key.size(): key.size() << 1;
    int offset = 0;
    for (size_t i = 0; i < key_size; ++i) {

        auto word = Bits::get_bw4(key, i);
        if (!((1 << word) & ptr->bitmap)) {
            return false;
        }

        offset = Bits::ones_count<uint16_t>(ptr->bitmap & ((1 << word) - 1));
        ptr = reinterpret_cast<CompactTrieNode*>(ptr->children[offset]);
    }
    
    return ptr->has_value;
}

std::string CompactTrie::to_string() const {
    // TODO Append checksum at the end of res.
    std::string res;
    std::deque<CompactTrieNode*> node_queue;
    node_queue.push_back(_root);

    if (_root == nullptr) {
        return res;
    }

    while (!node_queue.empty()) {
        CompactTrieNode* p = node_queue.front();
        node_queue.pop_front();

        char buf[sizeof(p->bitmap)];
        std::memcpy(buf, &(p->bitmap), sizeof(p->bitmap));
        res.append(buf, sizeof(buf));

        std::memcpy(buf, &(p->has_value), sizeof(p->has_value));
        res.append(buf, sizeof(p->has_value));

        for (int i = 0; i < Bits::ones_count<uint16_t>(p->bitmap); ++i) {
            node_queue.push_back(reinterpret_cast<CompactTrieNode*>(p->children[i]));
        }
    }

    return res;
}

int CompactTrie::decode(const std::string& serial) {
    // TODO check checksum

    Slice slice(serial);
    std::deque<CompactTrieNode*> node_queue; 
    CompactTrieNode* ptr = nullptr;

    int offset = 0;
    int cnt = 0;

    _cnt = serial.size() / 3;
    
    while (!slice.empty()) {
        uint16_t bitmap;
        std::memcpy(&bitmap, slice.data(), 2);
        slice.remove_prefix(2);

        int size = sizeof(CompactTrieNode) + (Bits::ones_count<uint16_t>(bitmap) << 3);
        char* mem = new char[size];
        CompactTrieNode* node = new(mem) CompactTrieNode();

        _size += size;
        node->bitmap = bitmap;
        std::memcpy(&(node->has_value), slice.data(), 1);

        slice.remove_prefix(1);

        if (_root == nullptr) {
            _root = node;
            ptr = node;
            cnt = Bits::ones_count<uint16_t>(ptr->bitmap);
        } else {
            node_queue.push_back(node);
            ptr->children[offset++] = (void*)node;
            while (offset == cnt && !node_queue.empty()) {
                ptr = node_queue.front();
                node_queue.pop_front();
                offset = 0;
                cnt = Bits::ones_count<uint16_t>(ptr->bitmap);
            }
        }
    }

    return 1;
}

int CompactTrie::put(const Slice& key) {
    if (_root == nullptr) {
        _root = new CompactTrieNode();
    }

    CompactTrieNode* parent = nullptr;
    CompactTrieNode* ptr = _root;
    int key_size = key.size() << 1;
    int slot = 0;

    for (int i = 0 ; i < key_size; ++i) {
        auto word = Bits::get_bw4(key, i);
        if (ptr->bitmap & (1 << word)) {
            auto offset = Bits::ones_count<uint16_t>(ptr->bitmap & ((1 << word) - 1));
            parent = ptr;
            ptr = reinterpret_cast<CompactTrieNode*>(ptr->children[offset]);
            slot = offset;
        } else {
            ptr->bitmap |= (1 << word);
            int offset = Bits::ones_count<uint16_t>(ptr->bitmap & ((1 << word) - 1));
            int cnt = Bits::ones_count<uint16_t>(ptr->bitmap);
            int size = sizeof(CompactTrieNode) + cnt * sizeof(void*);
            void* mem = malloc(size);
            bzero(mem, size);
            CompactTrieNode* node = new(mem)CompactTrieNode();
            node->bitmap = ptr->bitmap;
            node->has_value = ptr->has_value; 

            for (int j = 0; j < offset; ++j) {
                node->children[j] = ptr->children[j];
            }

            node->children[offset] = new CompactTrieNode(); 

            for (int j = offset; j < cnt - 1; ++j) {
                node->children[j + 1] = ptr->children[j];
            }

            if (parent) {
                parent->children[slot] = reinterpret_cast<void*>(node);
            }
            
            if (_root == ptr) {
                _root = node;
            } else {
                ++cnt;
            }

            free(reinterpret_cast<void*>(ptr));

            parent = node;
            ptr = reinterpret_cast<CompactTrieNode*>(parent->children[offset]);
            slot = offset;

            _size += size;
            _size -= sizeof(CompactTrieNode) + sizeof(void*) * (cnt - 1);
        }
    }

    ptr->has_value = true;

    return 0;
}

} // end of namespace loghub
} // end of namespace opencdn 
