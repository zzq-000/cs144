// #include "tcp_timer.hh"
// #include "tcp_segment.hh"
// #include "wrapping_integers.hh"
// Timer::Timer(unsigned int _initial_retransmission_timeout)
//     : _outstanding_segs(std::queue<TCPSegment>()), 
//     _RTO(_initial_retransmission_timeout),
//     _initial_RTO(_initial_retransmission_timeout) {}

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

// void Timer::set_RTO() {
//     _RTO *= 2;
// }
// const TCPSegment Timer::get_retrans_seg(bool &has_data) {
//     if (!empty()) {
//         has_data = true;
//         return _outstanding_segs.front();
//     } else {
//         has_data = false;
//         return TCPSegment();
//     }
// }