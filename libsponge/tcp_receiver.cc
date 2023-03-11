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
        return;  //检测seq是否合法   1.syn未到，则合法 2.syn到了，但seq_no > isn，合法
    } else {
        if (!_set_isn && header.syn) {
            _isn = header.seqno;
            _set_isn = true;
        }
        if (_set_isn) {  // 若fin先到，syn未到，需使用_get_fin标记已收到fin
            if (_get_fin) {
                eof = true;
            } else if (header.fin) {
                _get_fin = true;
                eof = true;
            }
        }
        if (_set_isn) {
            size_t checkpoint = stream_out().bytes_written();
            size_t index = unwrap(header.seqno, _isn, checkpoint);
            index = index == 0 ? 0 : index - 1;  //1. seqno != isn ,由于isn的缘故，需要减1  
            // 2. seqno==isn, 此时index=0,无法减1
            string data = static_cast<string>(seg.payload().str());

            _reassembler.push_substring(data, index, eof);
        }
    }

    // cout<< "byte_writen: " << stream_out().buffer_size()<<endl;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_set_isn) {
        size_t checkpoint = stream_out().bytes_written();

        size_t abs_seq = checkpoint + 1; //加上syn

        if (_reassembler.empty()) { //如果会话已经结束了，那么ackno 应加上额外的fin
            abs_seq++;
        }
        return wrap(abs_seq, _isn);
    }
    return {};
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
