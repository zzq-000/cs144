#include "stream_reassembler.hh"

#include <unordered_set>
#define TAILOR 0
#define DISCARD 1
#define BUFFER 2
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , _substring_pool(
          std::priority_queue<std::pair<std::string, size_t>, std::vector<std::pair<std::string, size_t>>, cmp_pair>())
    , _pool_size(0)
    , _assembled_bytes(0)
    , _end(false) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);
    // packet can be 3 category
    // ------|
    //    i.....    1. tailor and then assemble
    //   i...       2. discard
    //         i... 3. buffer
    size_t result = judge(data, index);
    // if (data == "ghX") {
    // cout<<"result: "<<result<<endl;

    // }
    if (result == TAILOR) {
        size_t write_begin = _assembled_bytes - index;
        size_t write_length = min(data.size() - write_begin, remain_capacity());
        // cout<<"remain capacity(): "<< remain_capacity()<<endl;
        // cout<<"datasssssssssssssssssss: "<< remain_capacity()<<endl;
        _output.write(data.substr(write_begin, write_length));
        _assembled_bytes += write_length;
        // if (write_length == data.size() - write_begin) {
        _end = _end || eof;

        // }

        // cout<<data.substr(write_begin, write_length).size()<<" "<<write_length<<endl;
        while (!_substring_pool.empty()) {
            auto slice = _substring_pool.top();
            result = judge(slice.first, slice.second);
            if (slice.first == "b" && slice.second == 1) {
                cout << "b1" << endl;
                cout << "_end: " << _end << endl;
            }
            if (result == DISCARD) {
                _substring_pool.pop();
                _pool_size -= slice.second;
            } else if (result == TAILOR) {
                if (_output.remaining_capacity()) {
                    write_begin = _assembled_bytes - slice.second;
                    write_length = min(slice.first.size() - write_begin, _output.remaining_capacity());
                    if (_output.remaining_capacity() < slice.first.size() - write_begin) {
                        _substring_pool.push(
                            make_pair(data.substr(write_begin + _output.remaining_capacity(),
                                                  data.size() - write_begin - _output.remaining_capacity()),
                                      _assembled_bytes + _output.remaining_capacity()));
                    }
                    _output.write(slice.first.substr(write_begin, write_length));
                    _assembled_bytes += write_length;
                    _substring_pool.pop();
                    _pool_size = _pool_size - write_begin - write_length;
                }

            } else {
                break;
            }
        }
        // cout << "_end: " << _end << endl;
        if (_substring_pool.empty() && _end) {
            _output.end_input();
        }

    } else if (result == BUFFER) {
        size_t current_max_index = _output.bytes_read() + _capacity;
        size_t push_size = min(data.size(), remain_capacity());
        push_size = min(push_size, current_max_index - index);
        // cout << "push size: " << push_size<<endl;
        _substring_pool.push(std::make_pair(data.substr(0, push_size), index));
        _pool_size += push_size;

        if (push_size == data.size()) {
            _end = _end || eof;
        }

    } else {
        _end = _end || eof;

        if (_end && !_pool_size) {
            _output.end_input();
        }
    }
}
size_t StreamReassembler::judge(const string &data, const size_t index) {
    if (index <= _assembled_bytes) {
        if (index + data.size() <= _assembled_bytes) {
            return DISCARD;
        } else {
            return TAILOR;
        }
    }
    if (data.size() == 0) {
        return DISCARD;
    }
    return BUFFER;
}

size_t StreamReassembler::remain_capacity() { return _capacity - _pool_size - _output.buffer_size(); }

size_t StreamReassembler::unassembled_bytes() const {
    std::unordered_set<size_t> bag;
    std::priority_queue<std::pair<std::string, size_t>, std::vector<std::pair<std::string, size_t>>, cmp_pair> tmp =
        _substring_pool;
    while (!tmp.empty()) {
        auto item = tmp.top();
        tmp.pop();
        size_t begin = item.second;
        size_t end = begin + item.first.size();
        for (size_t i = begin; i < end; i++) {
            // cout<<i<<endl;
            bag.insert(i);
        }
    }
    return bag.size();
}

bool StreamReassembler::empty() const { return _end && unassembled_bytes() == 0; }
