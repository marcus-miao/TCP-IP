#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    WrappingInt32 seqno = seg.header().seqno;

    // Check SYN flag and set initial sequence number (isn)
    if (seg.header().syn) {
        _synReceived = true;
        _isn = seg.header().seqno;
    }

    // If SYN flag hasn't been received, do nothing
    if (!_synReceived)
        return;

    // Push payload to reassembler. The index of Reassembler::push_substring() 
    // is the stream index which equals absolute index - 1. The absolute index
    // of the first byte in the payload is unwrapped sequence number if there
    // isn't a syn flag. Else, it will be unwrapp(seqno + 1). 

    uint64_t index; 
    if (seg.header().syn)
        index = unwrap(seqno + 1, _isn, _checkpoint) - 1;
    else
        index = unwrap(seqno, _isn, _checkpoint) - 1;
    size_t initRemainingCapacity = stream_out().remaining_capacity();
    _reassembler.push_substring(seg.payload().copy(), index, seg.header().fin);

    // Update checkpoint
    _checkpoint += initRemainingCapacity - stream_out().remaining_capacity();
    if (stream_out().input_ended())
        _checkpoint++;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_synReceived)
        return wrap(_checkpoint + 1, _isn);
    return {};
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
