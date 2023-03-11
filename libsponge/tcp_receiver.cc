#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);

    auto &header = seg.header();
    bool eof = false;
    if (_set_isn && header.seqno - _isn <= 0) {
        return;
    } else {
        if (!_set_isn && header.syn) {
            _isn = header.seqno;
            _set_isn = true;
        }
        if (_set_isn) {
            if (_get_fin) {
                eof = true;
                _set_fin = true;
            } else if (header.fin) {
                _get_fin = true;
                eof = true;
                _set_fin = true;
            }
        }
        if (_set_isn) {
            size_t checkpoint = stream_out().bytes_written();
            size_t index = unwrap(header.seqno, _isn, checkpoint);
            index = index == 0 ? 0 : index - 1;
            string data = static_cast<string>(seg.payload().str());

            _reassembler.push_substring(data, index, eof);
        }
    }

    // cout<< "byte_writen: " << stream_out().buffer_size()<<endl;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_set_isn) {
        size_t checkpoint = stream_out().bytes_written();

        size_t abs_seq = checkpoint + 1;

        if (_reassembler.empty()) {
            abs_seq++;
        }
        return wrap(abs_seq, _isn);
    }
    return {};
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
