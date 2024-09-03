//
// Created by zxn on 24-8-23.
//

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <cassert>
#include <string>
#include "TimeStamp.h"
#include <map>

class HttpRequest
{
public:
  //请求方法的标志
  enum Method
  {
    kInvalid, kGet, kPost, kHead, kPut, kDelete
  };

  // 请求版本的标志
  enum Version
  {
    kUnknown, kHttp10, kHttp11
  };

  // 默认构造函数
  HttpRequest()
    : method_(kInvalid),
      version_(kUnknown)
  {
  }

  ~HttpRequest() = default;

  // 设置版本
  void setVersion(const Version v)
  {
    version_ = v;
  }

  // 得到版本
  Version getVersion() const
  { return version_; }

  // 设置方法，之所以需要设计成传入两个指针，是为了方便解析请求行的空格情况
  bool setMethod(const char* start, const char* end)
  {
    assert(method_ == kInvalid);
    std::string m(start, end);
    if (m == "GET")
    {
      method_ = kGet;
    }
    else if (m == "POST")
    {
      method_ = kPost;
    }
    else if (m == "HEAD")
    {
      method_ = kHead;
    }
    else if (m == "PUT")
    {
      method_ = kPut;
    }
    else if (m == "DELETE")
    {
      method_ = kDelete;
    }
    else
    {
      method_ = kInvalid;
    }
    return method_ != kInvalid;
  }

  // 获取方法
  Method method() const
  { return method_; }

  // 方法转为string
  const char* methodString() const
  {
    const char* result = "UNKNOWN";
    switch(method_)
    {
      case kGet:
        result = "GET";
        break;
      case kPost:
        result = "POST";
        break;
      case kHead:
        result = "HEAD";
        break;
      case kPut:
        result = "PUT";
        break;
      case kDelete:
        result = "DELETE";
        break;
      default:
        break;
    }
    return result;
  }

  // GET /index.html?name=1&lang=en HTTP/1.1
  // 对应的就是 /index.html
  // 设置请求路径
  void setPath(const char* start, const char* end)
  {
    path_.assign(start, end);
  }

  // 获取请求路径
  const std::string& path() const
  { return path_; }

  // GET /index.html?name=1&lang=en HTTP/1.1
  // 对应的就是?后面的查询参数
  // 设置查询
  void setQuery(const char* start, const char* end)
  {
    query_.assign(start, end);
  }

  // 得到查询
  const std::string& query() const
  { return query_; }

  // 设置接受时间
  void setReceiveTime(TimeStamp t)
  { receiveTime_ = t; }

  // 返回接受时间
  TimeStamp receiveTime() const
  { return receiveTime_; }

  // 对应的就是下方
  /* Host: www.example.com
   * Accept: text/html, application/xhtml+xml,
   * User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:123.0) Gecko/20100101 Firefox/123.0
   * Cookie: sessionid=abcdefg1234567
   * Connection: keep-alive
  */

  // 给请求头加入其对应的子数据, start为每行的开始, colon为 ':'的所在位置， end为每一行的结尾
  void addHeader(const char* start, const char* colon, const char* end)
  {
    std::string field(start, colon);
    ++colon;
    // 找到:后面不是空格的第一个位置 例如 www的第一个w
    while (colon < end && isspace(*colon))
    {
      ++colon;
    }
    std::string value(colon, end);
    // 去除尾部的空格
    while (!value.empty() && isspace(value[value.size()-1]))
    {
      value.resize(value.size()-1);
    }
    // 加入请求头集合中
    headers_[field] = value;
  }

  // 得到请求头对应key的value数据
  std::string getHeader(const std::string& field) const
  {
    std::string result;
    std::map<std::string, std::string>::const_iterator it = headers_.find(field);
    if (it != headers_.end())
    {
      result = it->second;
    }
    return result;
  }

  // 得到所有请求头的数据
  const std::map<std::string, std::string>& headers() const
  { return headers_; }

  // 用于内部交换
  void swap(HttpRequest& that)
  {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    receiveTime_.swap(that.receiveTime_);
    headers_.swap(that.headers_);
  }

private:
  // 方法
  Method method_;
  // 版本
  Version version_;
  // 请求路径 也就是对应的 /index.html
  std::string path_;
  // 请求查询
  std::string query_;
  // 发出的时间
  TimeStamp receiveTime_;

  // 对应的就是下方
  /* Host: www.example.com
   * Accept: text/html, application/xhtml+xml,
   * User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:123.0) Gecko/20100101 Firefox/123.0
   * Cookie: sessionid=abcdefg1234567
   * Connection: keep-alive
  */
  // 请求头
  std::map<std::string, std::string> headers_;
};

#endif //HTTPREQUEST_H
