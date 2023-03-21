#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _byte_flight; }

void TCPSender::fill_window() {
    if (!_get_ack) {
        // cout<<"dddd"<<endl;
        string data = "";
        TCPSegment seg = build_segment(data, true, false, _isn);
        _next_seqno += 1;
        _byte_flight += 1;
        // cout << next_seqno_absolute() << endl;;
        // cout << bytes_in_flight() << endl;
        _segments_out.push(seg);
        // _outstanding_segments.push(seg);
    } else {
        
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    DUMMY_CODE(ackno, window_size);
            // cout << "bbb" << endl;

    _window = window_size;
    // _ackno = ackno;
    if (!_get_ack) {
        _get_ack = true;
    }
    uint64_t abs_seq = unwrap(ackno, _isn, _next_seqno);
    _byte_flight = abs_seq - _next_seqno;
    _next_seqno = abs_seq;
    while (!_outstanding_segments.empty()) {
        const TCPSegment seg = _outstanding_segments.front();
        uint64_t ack_data = seg.payload().size() + seg.header().syn ? 1 :0 + seg.header().fin ? 1 : 0;
        if (unwrap(ackno, _isn, _next_seqno) - unwrap(seg.header().seqno, _isn, _next_seqno) >= ack_data) {
            _outstanding_segments.pop();
            cout << "bbb" << endl;
        } else {
            break;
        }
    }


    //1.什么时候设置syn
    //2.什么时候设置fin     _stream.eof() == true  
    //3.payload设置什么
    //4. timer ？
    //5. tick ?
    
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick);

}

unsigned int TCPSender::consecutive_retransmissions() const { return {}; }

void TCPSender::send_empty_segment() {}

TCPSegment TCPSender::build_segment(string &data, bool syn, bool fin, WrappingInt32& seqno) {
        TCPSegment seg;
        seg.payload() = std::string(data);
        seg.header().fin = fin;
        seg.header().syn = syn;
        seg.header().seqno = seqno;
        return seg;
}

