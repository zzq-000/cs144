#ifndef SPONGE_LIBSPONGE_TCP_TIMER_HH
#define SPONGE_LIBSPONGE_TCP_TIMER_HH

#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <queue>
class Timer {
  public:
    Timer(unsigned int _initial_retransmission_timeout)
        : _outstanding_segs(std::queue<TCPSegment>())
        , _RTO(_initial_retransmission_timeout)
        , _initial_RTO(_initial_retransmission_timeout) {}
    bool isrun() const { return _started; }
    void run() {
        _started = true;
        _timestamp = 0;
    }
    void stop() {
        _started = false;
        _timestamp = 0;  // TODO
    }
    bool empty() const { return _outstanding_segs.empty(); }
    bool tick(size_t  ticks) {
        _timestamp += ticks;
        if (_timestamp >= _RTO) {
            set_RTO();
            return false;
        }
        // 未超时
        return true;
    }
    void reset() {
        _timestamp = 0;
        _started = true;
    }

    //! \brief indicate whether should reset consecutive retransmissions
    bool clean(const WrappingInt32 ackno, const WrappingInt32 isn, uint64_t next_seqno) {
        uint64_t abs_seq = unwrap(ackno, isn, next_seqno);
        bool acked_newdata = false;
        while (!_outstanding_segs.empty()) {
            const TCPSegment &seg = _outstanding_segs.front();
            uint64_t ack_data = seg.length_in_sequence_space();
            if (abs_seq - unwrap(seg.header().seqno, isn, next_seqno) >= ack_data) {
                _outstanding_segs.pop();
                acked_newdata = true;
            } else {
                break;
            }
        }
        
        if (acked_newdata) {
            _RTO = _initial_RTO;
            if (!_outstanding_segs.empty()) {
                _timestamp = 0;
                return true;
            }
        }
        return false;
    }
    void push(const TCPSegment& seg) { _outstanding_segs.push(seg); }
    void set_RTO() {
         _RTO *= 2; 
         _timestamp = 0;
    }
    unsigned int get_RTO() { return _RTO; }
    const TCPSegment get_retrans_seg(bool & has_data) {
        if (!empty()) {
            has_data = true;
            return _outstanding_segs.front();
        } else {
            has_data = false;
            return TCPSegment();
        }
    }

  private:
    std::queue<TCPSegment> _outstanding_segs;
    unsigned int _timestamp{0};  // current time
    unsigned int _RTO;
    unsigned int _initial_RTO;
    bool _started{false};
};
// Timer::Timer(unsigned int _initial_retransmission_timeout)
//     : _outstanding_segs(std::queue<TCPSegment>())
//     , _RTO(_initial_retransmission_timeout)
//     , _initial_RTO(_initial_retransmission_timeout) {}

// bool Timer::isrun() const { return _started; }
// void Timer::run() {
//     _started = true;
//     _timestamp = 0;
// }
// void Timer::stop() {
//     _started = false;
//     _timestamp = 0;  // TODO
// }
// bool Timer::empty() const { return _outstanding_segs.empty(); }
// bool Timer::tick(size_t ticks) {
//     _timestamp += ticks;
//     if (_timestamp >= _RTO) {
//         // TODO 超时
//         return false;
//     }
//     // 未超时
//     return true;
// }
// void Timer::reset() {
//     _timestamp = 0;
//     _started = true;
// }
// void Timer::clean(const WrappingInt32 ackno, const WrappingInt32 isn, uint64_t next_seqno) {
//     uint64_t abs_seq = unwrap(ackno, isn, next_seqno);
//     while (!_outstanding_segs.empty()) {
//         const TCPSegment &seg = _outstanding_segs.front();
//         uint64_t ack_data = seg.length_in_sequence_space();
//         if (abs_seq - unwrap(seg.header().seqno, isn, next_seqno) >= ack_data) {
//             _outstanding_segs.pop();
//         } else {
//             break;
//         }
//     }
// }
// void Timer::push(const TCPSegment &seg) { _outstanding_segs.push(seg); }

// void Timer::set_RTO() { _RTO *= 2; }
// const TCPSegment Timer::get_retrans_seg(bool &has_data) {
//     if (!empty()) {
//         has_data = true;
//         return _outstanding_segs.front();
//     } else {
//         has_data = false;
//         return TCPSegment();
//     }
// }
#endif  // !SPONGE_LIBSPONGE_TIMER_HH
