#include "tcp_sender.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"
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
        // cout << "balala";
        _segments_out.push(seg);
        // _outstanding_segments.push(seg);
    } else {
        // if (!_window) return;
        // cout << _window << endl;
        if (_fin_acked) return;
        string payload("");
        bool fin = false;
        size_t window_size = max(static_cast<size_t>(1),static_cast<size_t>(_window));
        size_t buffer_size = _stream.buffer_size();
        if (_stream.input_ended()) {
            if (buffer_size <= window_size){
                // cout << "ddd" << endl;
                payload = _stream.read(
                    min(buffer_size, static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE - 1)));
                    // 考虑fin占1位
                fin = _stream.buffer_empty() ? true : false;
                // cout << fin << endl;
            } else {
                payload = _stream.read(min(window_size, 
                static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE)));
            }
        } else {
            payload = _stream.read(
                min(min(buffer_size, window_size),
                static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE)));
        }
        if (!payload.size() && !fin) return;
        if (fin) _fin_sent = true;
        TCPSegment seg = build_segment(payload, false, fin, _isn);
        size_t data_len = seg.length_in_sequence_space();
        WrappingInt32 seqno = wrap(_next_seqno, _isn);
        seg.header().seqno = seqno;
        // cout << "_next_seqno" << _next_seqno << endl;
        _next_seqno += data_len;
        // cout << "_next_seqno" << _next_seqno << endl;
        cout << "fill_window: _byte_flight: " << _byte_flight << endl;

        _byte_flight += data_len;
        cout << "fill_window: _byte_flight: " << _byte_flight << endl;

        _segments_out.push(seg);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    DUMMY_CODE(ackno, window_size);
            // cout << "bbb" << endl;
    if (!_get_ack) {
        _get_ack = true;
    }
    _window = window_size;
    // cout << ackno.raw_value() << " " << _isn.raw_value() << endl;
    uint64_t abs_seq = unwrap(ackno, _isn, _next_seqno);
    cout << "_next_seqno: " << _next_seqno << endl;
    cout << "abs_seqno: " << abs_seq << endl;
    cout << "_acked_seqno: " << _acked_no << endl;
    cout << "_byte_flight: " << _byte_flight << endl;

    //ackno之前的信息已被确认过，所以这次需要忽略
    if (abs_seq <= _acked_no) return;
    
    // ackno过大，不符合sender发送的数据
    if (abs_seq - _acked_no > _byte_flight) return;
    // cout << _byte_flight << endl;
    // cout << abs_seq - _next_seqno << endl;
    _byte_flight -= abs_seq - _acked_no;
    cout << "_byte_flight: " << _byte_flight << endl;
    cout << endl;
    _acked_no = abs_seq;
    // cout << "_byte_flite: " << _byte_flight << endl;
    if(_fin_sent) _fin_acked = true;
    // cout << _fin_sent << " " << _fin_acked << endl; 
    while (!_outstanding_segments.empty()) {
        const TCPSegment seg = _outstanding_segments.front();
        uint64_t ack_data = seg.length_in_sequence_space();
        if (unwrap(ackno, _isn, _next_seqno) - unwrap(seg.header().seqno, _isn, _next_seqno) >= ack_data) {
            _outstanding_segments.pop();
            // cout << "bbb" << endl;
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

