#pragma once

#include <vector>
#include <string>

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
public:

    //缓冲区头部
    static constexpr size_t kCheapPrepend = 8;

    //缓冲区读写初始大小
    static constexpr size_t kInitialSize = 1024;

    explicit Buffer(size_t initsize = kInitialSize);
    ~Buffer() = default;

    // 返回可读的缓冲区大小
    [[nodiscard]] size_t readAbleBytes() const { return writerIndex_ - readerIndex_;}

    // 返回可写的缓冲区大小
    [[nodiscard]] size_t writeAbleBytes() const { return buffer_.size() - writerIndex_;}

    // 返回缓冲区头部大小
    [[nodiscard]] size_t prependAbleBytes() const { return readerIndex_;}

    // 返回缓冲区中可读数据的起始地址
    [[nodiscard]] const char* peek() const{ return begin() + readerIndex_;}

    // 返回缓冲区可以写的起始地址
    [[nodiscard]] char* beginWrite() { return begin() + writerIndex_; }

    [[nodiscard]] const char* beginWrite() const { return begin() + writerIndex_; }

    // 把[data ,data+len]内存上的数据，添加到writeable缓冲区当中
    void append(const char* data, size_t len);

    // 确定是否可以写入
    void ensureWriterableBytes(size_t len);

    // 读len长的数据，并进行移动位置
    void retrieve(size_t len); //len表示已经读了的

    // 如果都读完了
    void retrieveAll();

    // 把onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllAsString();

    //从起始位置读len长
    std::string retrieveAsString(size_t len);

    //从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);

    //通过fd发送数据
    ssize_t writeFd(int fd, int* saveErrno);

private:
    // 返回数据的首地址
    [[nodiscard]] const char* begin() const {
        //it.operator*() 首元素  it.operator*().operator&() 首元素的地址
        return &*buffer_.begin();
    }

    [[nodiscard]] char* begin() {
        //it.operator*() 首元素  it.operator*().operator&() 首元素的地址
        return &*buffer_.begin();
    }

    // 扩容操作
    void makespace(size_t len);

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};



