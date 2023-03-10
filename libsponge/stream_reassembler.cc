#include "stream_reassembler.hh"
#include <unordered_set>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _substring_pool({}), _assembled_bytes(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);
    if (unassembled_bytes() + _assembled_bytes + data.size() <= _capacity) {
        if (index <= _assembled_bytes) {
            if (index + data.size() >= _assembled_bytes) {
                // 1. assemble the slice
                // 2. check the subtring_pool if more slices can be assembled
                // 3. clean useless substring in the substring_pool
                // 4. update assembled_bytes
            } else {
                // just discard the substring
            }
        } else {
            _substring_pool.push_back(std::make_pair(data, index));
        }
    }
}

size_t StreamReassembler::unassembled_bytes() const {
    std::unordered_set<size_t> bag;
    for (auto slice:_substring_pool) {
        size_t begin = slice.second;
        size_t end = slice.second + slice.first.size();
        for (size_t i = begin; i < end; i++) {
            bag.insert(i);
        }
    }
    return bag.size();
}

bool StreamReassembler::empty() const { return {}; }
