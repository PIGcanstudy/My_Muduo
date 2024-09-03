//
// Created by zxn on 24-8-23.
//

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <map>

class Buffer;
class HttpResponse
{
public:
    // 返回的状态码集合
    enum HttpStatusCode
    {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
      };

    explicit HttpResponse(bool close)
      : statusCode_(kUnknown),
        closeConnection_(close)
    {
    }

    // 设置状态码
    void setStatusCode(HttpStatusCode code)
    { statusCode_ = code; }

    // 设置状态信息
    void setStatusMessage(const std::string& message)
    { statusMessage_ = message; }

    // 设置关闭连接信息
    void setCloseConnection(bool on)
    { closeConnection_ = on; }

    // 判断是否是保持长期连接
    bool closeConnection() const
    { return closeConnection_; }

    // 设置Content-Type
    void setContentType(const std::string& contentType)
    { addHeader("Content-Type", contentType); }

    // 加入响应KEY-VALUE
    void addHeader(const std::string& key, const std::string& value)
    { headers_[key] = value; }

    // 设置响应体
    void setBody(const std::string& body)
    { body_ = body; }

    // 将整个响应信息都写入buffer中
    void appendToBuffer(Buffer* output) const;

private:
    // 请求头对应的key-value集合
    std::map<std::string, std::string> headers_;
    // 状态码
    HttpStatusCode statusCode_;
    // 表示 HTTP 响应的状态消息，如 "OK"、"Not Found" 等。
    std::string statusMessage_;
    // 标志是否保持长期连接
    bool closeConnection_;
    // 响应体
    std::string body_;
};

#endif //HTTPRESPONSE_H
