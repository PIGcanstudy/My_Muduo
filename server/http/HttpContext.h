//
// Created by zxn on 24-8-23.
//

#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H

#include "HttpRequest.h"

class Buffer;

class HttpContext
{
public:
    // 请求解析的状态, 后续可以用状态机来实现每个部分的解析
    enum HttpRequestParseState
    {
        kExpectRequestLine, // 请求行
        kExpectHeaders,  // 请求头
        kExpectBody,  // 请求身体
        kGotAll,  // 得到所有
      };

    HttpContext()
      : state_(kExpectRequestLine)
    {
    }

    // 用来解析请求
    bool parseRequest(Buffer* buf, TimeStamp receiveTime);

    // 判断是否是得到所有
    bool gotAll() const
    { return state_ == kGotAll; }

    // 重置请求，重置后请求为空
    void reset()
    {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    // 返回不可更改的请求
    const HttpRequest& request() const
    { return request_; }

    // 返回可以更改的请求
    HttpRequest& request()
    { return request_; }

private:
    // 解析请求行
    bool processRequestLine(const char* begin, const char* end);

    // 请求解析的状态码
    HttpRequestParseState state_;

    // 需要解析的请求
    HttpRequest request_;
};



#endif //HTTPCONTEXT_H
