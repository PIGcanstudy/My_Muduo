//
// Created by zxn on 24-8-23.
//

#include "HttpServer.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "HttpContext.h"
#include "TcpConnection.h"
#include "logger.h"

// 默认的Http回调函数
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);

}

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const std::string& name,
                       TcpServer::Option option)
  : server_(loop, listenAddr, name, option),
    httpCallback_(defaultHttpCallback)
{
  server_.setConnectionCallback(
      std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start()
{
  LOG_INFO("HttpServer[%s] starts listening on %s", server_.name(), server_.ipPort());
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setContext(HttpContext());
  }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           TimeStamp receiveTime)
{

  // 将std::any* 转换为 HttpContext*
  HttpContext* context = std::any_cast<HttpContext>(conn->getMutableContext());

  // 先进行请求解析，看格式是否正确
  if (!context->parseRequest(buf, receiveTime))
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  // 看是否是得到所有状态
  if (context->gotAll())
  {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  const std::string& connection = req.getHeader("Connection");
  // 如果是关闭或者http版本是1.0且不是长连接，就标志为关闭状态
  // 为什么后者也要呢，因为http1.0默认短链接, 发完信息后就得关闭
  bool close = connection == "close" ||
    (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  // 创建一个短链接的响应
  HttpResponse response(close);
  // 根据请求创建响应
  httpCallback_(req, &response);

  Buffer buf;
  // 将响应信息写入buffer
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection())
  {
    conn->shutdown();
  }
}