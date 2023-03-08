#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _capacity(capacity)
    , _bytes_in(0)
    , _bytes_written(0)
    , _bytes_poped(0)
    , _input_ended(false)
    , _output_ended(false)
    , _data({}) {
    DUMMY_CODE(capacity);
}

size_t ByteStream::write(const string &data) {
    DUMMY_CODE(data);
    if (!input_ended()) {
        size_t remaining = remaining_capacity();
        int bytes_written = remaining <= data.size() ? remaining : data.size();
        for (int i = 0; i < bytes_written; i++) {
            _data.push_back(data[i]);
            _bytes_in++;
            _bytes_written++;
        }
        // if (_bytes_in > 0) {
        //     _output_ended = false;
        // }
        return bytes_written;
    }

    return 0;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    DUMMY_CODE(len);
    string result = "";
    size_t copy_size = len <= buffer_size() ? len : buffer_size();
    for (size_t i = 0; i < copy_size; i++) {
        result.push_back(_data[i]);
    }
    return result;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    DUMMY_CODE(len);
    size_t pop_size = len <= buffer_size() ? len : buffer_size();
    while (pop_size > 0) {
        _data.pop_front();
        _bytes_poped++;
        _bytes_in--;
        pop_size--;
    }
    if (buffer_empty() && input_ended()) {
        _output_ended = true;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    DUMMY_CODE(len);
    if (!eof()) {
        string result = peek_output(len);
        pop_output(len);
        return result;
    }
    return "";
}

void ByteStream::end_input() {
    _input_ended = true; 
    if (buffer_empty()) {
        _output_ended = true;
    }
}

bool ByteStream::input_ended() const { return _input_ended; }

size_t ByteStream::buffer_size() const { return _bytes_in; }

bool ByteStream::buffer_empty() const { return buffer_size() == 0 ? true : false; }

bool ByteStream::eof() const { return _output_ended; }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_poped; }

size_t ByteStream::remaining_capacity() const { return _capacity - _bytes_in; }
