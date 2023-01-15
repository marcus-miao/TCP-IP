#include "stream_reassembler.hh"

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) 
    : _output(capacity)
    , _capacity(capacity)
    , _windowRightIdx(capacity-1)
    , _unassembled()
    , _nextByteIdx(0)
    , _eof(false)
    , _lastByteIdx(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    // Update _windowRightIdx
    _windowRightIdx = _nextByteIdx + _output.remaining_capacity() - 1;

    size_t dataLen = data.length();
    if (eof) {
        _eof = true;

        // Avoid underflow in edge case where index + dataLen = 0
        size_t temp = index + dataLen;
        if (temp == 0) {
            _output.end_input();
            return;
        }
        _lastByteIdx = temp - 1;
    }

    flush();

    // Case 1: all bytes in the segment are assembled
    if (index < _nextByteIdx && index + dataLen <= _nextByteIdx)
        return;

    size_t upperBound = index + dataLen;

    // Case 2: some bytes in the segment can be directly assembled
    if (index <= _nextByteIdx) {
        string toWrite("");
        for (size_t i = _nextByteIdx; i < upperBound; i++) {
            if (i > _windowRightIdx)
                break;

            const char curr = data.at(i - index);
            toWrite += curr;
            _nextByteIdx++;
            delete_byte(i);
        }
        _output.write(toWrite);
        flush();
        return;
    }

    // Case 3: all bytes in the segment cannot be assembled
    for (size_t i = index; i < upperBound; i++) {
        if (i > _windowRightIdx)
            break;
        add_byte(i, data.at(i - index));
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled.size(); }

bool StreamReassembler::empty() const { return _unassembled.empty(); }

// =============================================================================
// Helper methods implementation
// =============================================================================

void StreamReassembler::add_byte(const size_t index, const char byte) {
    if (_unassembled.find(index) == nullptr)
        _unassembled.insert({index, byte});
}

void StreamReassembler::delete_byte(const size_t index) {
    if (_unassembled.find(index) != nullptr)
        _unassembled.erase(index);
}

void StreamReassembler::flush() {
    string toWrite("");
    while (_unassembled.find(_nextByteIdx) != nullptr) {
        toWrite += _unassembled.find(_nextByteIdx)->second;
        delete_byte(_nextByteIdx);
        _nextByteIdx++;
    }
    _output.write(toWrite);
    if (_eof && _nextByteIdx > _lastByteIdx)
        _output.end_input();
}