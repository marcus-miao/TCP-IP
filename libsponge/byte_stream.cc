#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    :_buffer(), _capacity(capacity), _bytesRead(0), _bytesWritten(0), _inputEnded(false) {}

size_t ByteStream::write(const string &data) {
    size_t size = 0;
    for (auto c : data) {
        if (_buffer.size() >= _capacity) {
            break;
        }
        _buffer.push_back(c);
        size++;
    }
    _bytesWritten += size;
    return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string output("");
    auto upperBound = min(len, _buffer.size());
    for (size_t i = 0; i < upperBound; i++) {
        output += _buffer.at(i);
    }
    return output;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    auto upperBound = min(len, _buffer.size());
    _bytesRead += upperBound;
    for (size_t i = 0; i < upperBound; i++) {
        _buffer.pop_front();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string output = peek_output(len);
    pop_output(len);
    return output;
}

void ByteStream::end_input() { _inputEnded = true; }

bool ByteStream::input_ended() const { return _inputEnded; }

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return _buffer.empty(); }

bool ByteStream::eof() const { return _inputEnded && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _bytesWritten; }

size_t ByteStream::bytes_read() const { return _bytesRead; }

size_t ByteStream::remaining_capacity() const { 
    return _capacity - _buffer.size();
}
