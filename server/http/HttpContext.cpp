//
// Created by zxn on 24-8-23.
//

#include "HttpContext.h"
#include "Buffer.h"

bool HttpContext::parseRequest(Buffer* buf, TimeStamp receiveTime) { // buf里存的是我们接收到的整个请求
    bool ok = true;
    bool hasMore = true;
    while (hasMore)
    {
        // 如果是需要解析请求行
        if (state_ == kExpectRequestLine)
        {
            // 首先找到换行符的位置
            const char* crlf = buf->findCRLF();
            if (crlf)
            {
                // 找到了就解析请求行
                ok = processRequestLine(buf->peek(), crlf);
                if (ok)
                {
                    request_.setReceiveTime(receiveTime);
                    // 移动缓冲区表示已经读取了
                    buf->retrieveUntil(crlf + 2);
                    // 更新解析状态
                    state_ = kExpectHeaders;
                }
                else
                {
                    hasMore = false;
                }
            }
            else
            {
                hasMore = false;
            }
        }
        else if (state_ == kExpectHeaders) // 开始处理请求头状态
        {
            // 找到下一个换行符
            const char* crlf = buf->findCRLF();
            if (crlf)
            {
                // 找到:的位置
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf)
                {
                    // 加入请求头的一项
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else
                {
                    // 空行，是请求头的结束位置
                    state_ = kGotAll;
                    hasMore = false;
                }
                // 更新缓冲区
                buf->retrieveUntil(crlf + 2);
            }
            else
            {
                hasMore = false;
            }
        }
        else if (state_ == kExpectBody)
        {
            // 解析请求身体，解析逻辑需要用户自行编写逻辑
        }
    }
    return ok;
}

bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    // 找第一个空格，第一个空格前是请求方法
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space))
    {
        // 将起点设置为空格之后的位置
        start = space+1;
        // 找第二个空格，表示请求路径
        space = std::find(start, end, ' ');
        if (space != end)
        {
            // 看是否有带查询参数
            const char* question = std::find(start, space, '?');
            if (question != space)
            {
                //带了就设置查询参数
                request_.setPath(start, question);
                request_.setQuery(question, space);
            }
            else
            {
                request_.setPath(start, space);
            }
            start = space+1;
            // 找版本Http 版本
            succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
            if (succeed)
            {
                if (*(end-1) == '1')
                {
                    request_.setVersion(HttpRequest::kHttp11);
                }
                else if (*(end-1) == '0')
                {
                    request_.setVersion(HttpRequest::kHttp10);
                }
                else
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}
