#include "Buffer.h"

#include <unistd.h>
#include <sys/uio.h>

Buffer::Buffer(size_t initsize)
    : buffer_(initsize)
    , readerIndex_(kCheapPrepend)
    , writerIndex_(kCheapPrepend)
{

}

// 考虑两种扩容情况
// 1. 预留空间 加上 可写空间不足的的话 就直接扩容
// 2. 否则 就不直接扩容，而是重新分配下 内部结构
// 这样可以避免内存空间的浪费
void Buffer::makespace(size_t len) {
    if(prependAbleBytes() + writeAbleBytes() < len + kCheapPrepend) {
        buffer_.resize(writerIndex_ + len);
    }else {
        size_t readable = readAbleBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
    }
}

void Buffer::ensureWriterableBytes(size_t len) {
    // 不够写就扩容
    if(writeAbleBytes() < len) {
        makespace(len);
    }
}

void Buffer::append(const char *data, size_t len) {
    ensureWriterableBytes(len);
    std::copy(data, data + len, beginWrite());
    writerIndex_ += len;
}

void Buffer::retrieve(size_t len) {
    if(len < readAbleBytes())
    {
        //已经读的小于可读的，只读了一部分len
        //还剩readerIndex_ += len 到 writerIndex_
        readerIndex_ += len;
    }
    else //len == readableBytes()
    {
        retrieveAll();
    }
}

void Buffer::retrieveAll() {
    readerIndex_ = writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readAbleBytes());//应用可读取数据的长度
}

std::string Buffer::retrieveAsString(size_t len) {
    // 从起始位置读len长
    std::string result{peek(), len};

    // 上面一句吧缓冲区可读的数据读出来，这里对缓冲区进行复位操作
    retrieve(len);

    return result;
}

ssize_t Buffer::readFd(int fd, int *saveErrno) {
    // 栈上的临时空间，分配64K
    char extrabuf[65536] = { 0 };
    struct iovec vec[2];
    // buffer底层缓冲区剩余可以写的空间
    const size_t writable = writeAbleBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    // writable < sizeof extrabuf就选2块，否则一块就够用
    const int iovcnt = (writable < sizeof extrabuf)? 2 : 1;

    const ssize_t n = readv(fd, vec, iovcnt);

    if(n < 0)
    {
        *saveErrno = errno;
    }
    else if(n <= writable) //buffer可写的缓冲区已经够存储读取出来的数据
    {
        writerIndex_ += n;
    }
    else //extrabufl里面也写入了数据
    {
        writerIndex_ = buffer_.size();

        //writerIndex_ 开始写n-writable的数据
        append(extrabuf,n-writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
    const ssize_t n = write(fd, peek(), readAbleBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}
