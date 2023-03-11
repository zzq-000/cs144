#include "wrapping_integers.hh"
#include <iostream>
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    DUMMY_CODE(n, isn);
    uint32_t bias = static_cast<uint32_t>(n % (static_cast<uint64_t>(UINT32_MAX) + 1));
    return isn + bias;
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    DUMMY_CODE(n, isn, checkpoint);
    uint32_t length = n - isn;

    size_t div = (checkpoint > length? checkpoint - length: length - checkpoint)/
     (static_cast<uint64_t>(UINT32_MAX) + 1);

    uint64_t left = static_cast<uint64_t>(div) * (static_cast<uint64_t>(UINT32_MAX) + 1) + length;
    uint64_t right = static_cast<uint64_t>(div + 1) * (static_cast<uint64_t>(UINT32_MAX) + 1) + length;

    if (checkpoint <= (right - left) / 2 + left) {
        return left;
    }
    return right;
}
