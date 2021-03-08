#include "net/http/HttpContext.h"
#include "net/Buffer.h"
#include <gtest/gtest.h>

using namespace ham;
using namespace ham::net;

TEST(testParseRequestAllInOne, parseRequestAllInOne)
{
  HttpContext context;
  Buffer input;
  input.append("GET /index.html HTTP/1.1\r\n"
       "Host: www.chenshuo.com\r\n"
       "\r\n");

  EXPECT_EQ(context.parseRequest(input, Timestamp::now()), true);
  EXPECT_EQ(context.gotAll(), true);
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::kGet);
  EXPECT_EQ(request.path(), string("/index.html"));
  EXPECT_EQ(request.version(), HttpRequest::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), string("www.chenshuo.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), string(""));
}
 
TEST(testParseRequestInTwoPieces, parseRequestInTwoPieces)
{
  string all("GET /index.html HTTP/1.1\r\n"
       "Host: www.chenshuo.com\r\n"
       "\r\n");

  for (size_t sz1 = 0; sz1 < all.size(); ++sz1)
  {
    HttpContext context;
    Buffer input;
    // printf("sz1 = %d", static_cast<int>(sz1));
    input.append(all.c_str(), sz1);
    if(sz1 <= 25)
    {
        EXPECT_EQ(context.parseRequest(input, Timestamp::now()), false);
    }
    EXPECT_EQ(!context.gotAll(), true);

    size_t sz2 = all.size() - sz1;
    input.append(all.c_str() + sz1, sz2);
    EXPECT_EQ(context.parseRequest(input, Timestamp::now()), true);
    EXPECT_EQ(context.gotAll(), true);
    const HttpRequest& request = context.request();
    EXPECT_EQ(request.method(), HttpRequest::kGet);
    EXPECT_EQ(request.path(), string("/index.html"));
    EXPECT_EQ(request.version(), HttpRequest::kHttp11);
    EXPECT_EQ(request.getHeader("Host"), string("www.chenshuo.com"));
    EXPECT_EQ(request.getHeader("User-Agent"), string(""));
  }
}

TEST(testParseRequestEmptyHeaderValue, parseRequestEmptyHeaderValue)
{
  HttpContext context;
  Buffer input;
  input.append("GET /index.html HTTP/1.1\r\n"
       "Host: www.chenshuo.com\r\n"
       "User-Agent:\r\n"
       "Accept-Encoding: \r\n"
       "\r\n");

  EXPECT_EQ(context.parseRequest(input, Timestamp::now()), true);
  EXPECT_EQ(context.gotAll(), true);
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::kGet);
  EXPECT_EQ(request.path(), string("/index.html"));
  EXPECT_EQ(request.version(), HttpRequest::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), string("www.chenshuo.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), string(""));
  EXPECT_EQ(request.getHeader("Accept-Encoding"), string(""));
}

int main(int argc, char **argv) 
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}