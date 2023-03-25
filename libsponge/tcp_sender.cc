#include "tcp_sender.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"
// #include "tcp_timer.hh"
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , timer(_initial_retransmission_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _byte_flight; }

void TCPSender::fill_window() {
    if (!_send_syn) {
        cout << "syn: " << endl;
        string data = "";
        TCPSegment seg = build_segment(data, true, false, _isn);
        _next_seqno += 1;
        _byte_flight += 1;
        _segments_out.push(seg);
        _send_syn = true;
    } else {
        cout << "after syn: " << endl;
        // 检查是否有超时的packet
        // 每次发送包时，看Timer是否启动，若没有，则启动timer
        // if (!_window) return;
        // cout << _window << endl;
        // cout << _fin_sent << endl;
        if (_fin_sent)
            return;  // 会话已结束
        string payload("");
        bool fin = false;
        // 与1取max：如果接收方宣布窗口为空，自我发送方仍要尝试探测
        while ((static_cast<size_t>(std::max(_window, static_cast<uint16_t>(1))) - bytes_in_flight()) != 0) {
            size_t write_size = static_cast<size_t>(std::max(_window, static_cast<uint16_t>(1))) - bytes_in_flight();
            size_t buffer_size = _stream.buffer_size();
            cout << "write_size: " << write_size << endl;
            cout << "buffer_size: " << write_size << endl;
            // if (buffer_size == 0) return;
            if (_stream.input_ended()) {
                payload = _stream.read(
                    min(write_size, 
                    min(buffer_size, static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE)))
                );
                fin = (buffer_size < write_size) ? true : false;
            } else {
                payload =
                    _stream.read(min(min(buffer_size, write_size), static_cast<size_t>(TCPConfig::MAX_PAYLOAD_SIZE)));
            }
            if (!payload.size() && !fin)
                return;
            if (fin)
                _fin_sent = true;
            TCPSegment seg = build_segment(payload, false, fin, _isn);
            size_t data_len = seg.length_in_sequence_space();
            WrappingInt32 seqno = wrap(_next_seqno, _isn);
            seg.header().seqno = seqno;
            cout << "data_len: " << data_len << endl;
            // cout << "_next_seqno" << _next_seqno << endl;
            _next_seqno += data_len;
            // cout << "_next_seqno" << _next_seqno << endl;
            // cout << "fill_window: _byte_flight: " << _byte_flight << endl;
            _byte_flight += data_len;
            // cout << "fill_window: _byte_flight: " << _byte_flight << endl;
            _segments_out.push(seg);
            timer.push(seg);
            if (fin) {
                return;
            }
        }
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
    // cout << "window size: "<< window_size << endl;
    // cout << ackno.raw_value() << " " << _isn.raw_value() << endl;
    uint64_t abs_seq = unwrap(ackno, _isn, _next_seqno);
    // cout << "_next_seqno: " << _next_seqno << endl;
    // cout << "abs_seqno: " << abs_seq << endl;
    // cout << "_acked_seqno: " << _acked_no << endl;
    // cout << "_byte_flight: " << _byte_flight << endl;

    // ackno之前的信息已被确认过，所以这次需要忽略
    if (abs_seq <= _acked_no)
        return;

    // ackno过大，不符合sender发送的数据
    if (abs_seq - _acked_no > _byte_flight)
        return;
    // cout << _byte_flight << endl;
    // cout << abs_seq - _next_seqno << endl;
    _byte_flight -= abs_seq - _acked_no;
    // cout << "_byte_flight: " << _byte_flight << endl;
    // cout << endl;
    _acked_no = abs_seq;
    // cout << "_byte_flite: " << _byte_flight << endl;
    if (_fin_sent)
        _fin_acked = true;
    // cout << _fin_sent << " " << _fin_acked << endl;
    if (timer.clean(ackno, _isn, _next_seqno)) {
        _consecutive_retransmissions = 0;
    }

    // 1.什么时候设置syn
    // 2.什么时候设置fin     _stream.eof() == true
    // 3.payload设置什么
    // 4. timer ？
    // 5. tick ?
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick);
    if (timer.tick(ms_since_last_tick, _window)) {
        // 未超时
        // _consecutive_retransmissions = std::max(_consecutive_retransmissions, _consecutive_retransmissions_counter);
        // _consecutive_retransmissions_counter = 0;
        // _last_retran = false;
    } else {
        // 超时
        // if (!_last_retran) _last_retran = true;
        // _consecutive_retransmissions += 1;

        uint64_t seg_seq;
        // cout << timer.get_RTO() << endl;
        if (!_get_ack) {
            string data = "";
            TCPSegment seg = build_segment(data, true, false, _isn);
            _segments_out.push(seg);
            seg_seq = unwrap(seg.header().seqno, _isn, _next_seqno);
        } else {
            bool has_data;
            TCPSegment seg = timer.get_retrans_seg(has_data);
            if (has_data) {
                _segments_out.push(seg);
            }
            seg_seq = unwrap(seg.header().seqno, _isn, _next_seqno);
        }
        if (seg_seq == _last_retran_seq) {
            _consecutive_retransmissions_counter += 1;
            _consecutive_retransmissions = std::max(_consecutive_retransmissions, _consecutive_retransmissions_counter);
        } else {
            _last_retran_seq = seg_seq;
            _consecutive_retransmissions_counter = 1;
        }
        // cout << "con: " << _consecutive_retransmissions << endl;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {}

TCPSegment TCPSender::build_segment(string &data, bool syn, bool fin, WrappingInt32 &seqno) {
    TCPSegment seg;
    seg.payload() = std::string(data);
    seg.header().fin = fin;
    seg.header().syn = syn;
    seg.header().seqno = seqno;
    return seg;
}
