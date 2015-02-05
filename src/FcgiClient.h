/**
 * fcgi_client.h
 * @package         fcgi_client
 * @copyright       Copyright
 * @author          houwei
 *
 */

#pragma once

#include <string>
#include <map>
#include <functional>
#include <boost/asio.hpp>

namespace Fcgi {
/**
 *
 * @typedef std::map<std::string, std::string>
 */
typedef std::map<std::string, std::string> array_t;

typedef std::function<void (std::string)> DataFunc;



using tcp = boost::asio::ip::tcp;


/**
 * 文字 ASCII码
 */
inline unsigned int ord(int val) {
	return ((unsigned int) val & 0xff);
}

inline void echoHex(char* data, int len) {
	for(int i = 0;i < len;++i){
		std::cout << ":" << ord(data[i]);
	}
	 std::cout << std::endl;
}

/**
 * 协议头结构
 * @typedef struct header_t
 */
typedef struct {
	unsigned int version;
	unsigned int type;
	unsigned int requestId;
	unsigned int contentLength;
	unsigned int paddingLength;
	unsigned int reserved;
	unsigned int flag;
} header_t;



typedef std::map<std::string,std::string> HeaderData;


/**
 * 版本
 * @const unsigned int
 */
const unsigned int VERSION = 1;

/**
 * 类型：请求开始
 * @const unsigned int
 */
const unsigned int BEGIN_REQUEST = 1;

/**
 * 类型：中断请求
 * @const unsigned int
 */
const unsigned int ABORT_REQUEST = 2;

/**
 * 类型：请求结束
 * @const unsigned int
 */
const unsigned int END_REQUEST = 3;

/**
 * 类型：参数
 * @const unsigned int
 */
const unsigned int PARAMS = 4;

/**
 * 类型：标准输入
 * @const unsigned int
 */
const unsigned int STDIN = 5;

/**
 * 类型：标准输出
 * @const unsigned int
 */
const unsigned int STDOUT = 6;

/**
 * 类型：错误
 * @const unsigned int
 */
const unsigned int STDERR = 7;

/**
 * 类型：数据
 * @const unsigned char
 */
const int DATA = 8;
const int GET_VALUES = 9;
const int GET_VALUES_RESULT = 10;
const int UNKNOWN_TYPE = 11;
const int MAXTYPE = 11;

const int RESPONDER = 1;
const int AUTHORIZER = 2;
const int FILTER = 3;

/**
 * 标志：完成
 * @const unsigned char
 */
const unsigned char REQUEST_COMPLETE = 0;
const unsigned char CANT_MPX_CONN = 1;
const unsigned char OVERLOADED = 2;
const unsigned char UNKNOWN_ROLE = 3;

/**
 * メッセージ
 * @const char*
 */
const char MAX_CONNS[] = "MAX_CONNS";
const char MAX_REQS[] = "MAX_REQS";
const char MPXS_CONNS[] = "MPXS_CONNS";

/**
 * 请求头长度
 * @const integer
 */
const int HEADER_LEN = 8;


/**
 * 最大値
 * @const integer
 */
const int MAX_LENGTH = 0xffff;

/**
 * FCGI协议异步客户端
 *
 * @package     FCGIClient
 * @author      Yujiro Takahashi <yujiro3@gmail.com>
 */
class client {
public:


public:


	/**
	 *
	 *
	 */
	client(boost::asio::io_service& io_service, std::string ip, int port);



	/**
	 *
	 *
	 * @access public
	 */
	~client();



	void onConnect(std::function<void ()> fun);



	/**
	 * Execute a request to the FastCGI application
	 *
	 * @access public
	 * @param String stdin Content
	 * @return String
	 */
	void request(DataFunc fun, const std::map<std::string,std::string>& header, std::string& content);

private:
	/**
	 * 构建协议数据
	 *
	 * @access public
	 * @param String stdin Content
	 * @return void
	 */
	std::string buildRecord(HeaderData headerData,std::string& stdin);
	/**
	 * 构建协议
	 *
	 * @access private
	 * @param  int type
	 * @param  string content
	 * @param  int requestId
	 */
	std::string buildPacket(int type, std::string& content, int requestId);
	/**
	 * FastCGI
	 *
	 * @access private
	 * @param string name Name
	 * @param string value Value
	 * @return string FastCGI Name value pair
	 */
	std::string buildNvpair(std::string name, std::string value);

	/**
	 * 读协议头
	 *
	 * @access private
	 * @return void
	 */


	void readPacketHeader(DataFunc fun, std::shared_ptr<header_t> header, std::shared_ptr<std::string> response);


	/**
	 * 读协议体
	 *
	 * @access private
	 * @return void
	 */
	void readPacket(DataFunc fun, std::shared_ptr<header_t> header, std::shared_ptr<std::string> response);

	/**
	 * 读填充
	 *
	 * @access private
	 * @return void
	 */

	void readPadding(DataFunc fun, int length);




	std::string serverHost;
	int serverPort = 0;

	boost::asio::ip::tcp::socket socket_;
	//boost::asio::ip::tcp::resolver resolver_;
	//boost::asio::ip::tcp::endpoint endpoint_;
	boost::asio::ip::tcp::resolver::iterator iterator_;



};
}

