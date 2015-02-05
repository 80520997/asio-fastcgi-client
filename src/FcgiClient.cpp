/**
 * fcgi_client.cpp
 *
 * C++ versions 4.4.5
 *
 *      fcgi_client : https://github.com/Yujiro3/fcgicli
 *      Copyright (c) 2011-2013 sheeps.me All Rights Reserved.
 *
 * @package         fcgi_client
 * @copyright       Copyright (c) 2011-2013 sheeps.me
 * @author          Yujiro Takahashi <yujiro3@gmail.com>
 * @filesource
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <stdexcept>

#include <cstring>
#include <cassert>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "FcgiClient.h"

namespace Fcgi {

/**
 *
 *
 * @access public
 * @param string listen
 */
client::client(boost::asio::io_service& io_service, std::string ip, int port) :
		socket_(io_service)//,
		//resolver_(io_service),
		//iterator_(boost::asio::ip::address::from_string(ip),short(555)),

		//serverHost(ip),
		//serverPort(port)
{

	//boost::asio::ip::tcp::resolver(io_service);

	iterator_ = tcp::resolver(io_service).resolve(tcp::resolver::query(ip, std::to_string(port)));


}

/**
 * 链接成功事件
 *
 * @access public
 * @param string listen
 */
void client::onConnect(std::function<void()> fun) {

	boost::asio::async_connect(socket_, iterator_,
		[this,fun](boost::system::error_code ec, boost::asio::ip::tcp::resolver::iterator)
		{
			if (!ec)
			{
				fun();
			}
		}
	);
}


void client::request(DataFunc fun, const std::map<std::string,std::string>& header, std::string& content) {

	HeaderData headerdata(header);

	auto record = buildRecord(headerdata, content);

	boost::asio::async_write(socket_,
		boost::asio::buffer(record.data(), record.size()),
		[this,fun](boost::system::error_code ec, std::size_t len)
		{

			if (!ec) {

				readPacketHeader(fun,std::make_shared<header_t>(),std::make_shared<std::string>());

			} else {
				throw std::runtime_error("write fastcgi data error");
			}
		}
	);

}



/**
 *
 *
 * @access public
 */
client::~client() {

}




/**
 * 构建请求头
 *
 * @access public
 * @param String stdin Content
 * @return void
 */
std::string client::buildRecord(HeaderData headerData,std::string& stdin) {


	headerData["CONTENT_LENGTH"] = std::to_string(stdin.length());

	std::string head("");
	head += char(0);
	head += char(1);    // Responder
	head += char(0);    // Keep alive
	head += char(0);
	head += char(0);
	head += char(0);
	head += char(0);
	head += char(0);

	auto record = buildPacket(BEGIN_REQUEST, head, 1);
	std::string paramsRequest("");
	for(auto item : headerData) {
		paramsRequest += buildNvpair(item.first, item.second);
	}

	std::string empty("");
	if (paramsRequest.size() > 0) {
		record += buildPacket(PARAMS, paramsRequest, 1);
	}
	record += buildPacket(PARAMS, empty, 1);

	if (stdin.size() > 0) {
		record += buildPacket(STDIN, stdin, 1);
	}
	record += buildPacket(STDIN, empty, 1);

	return record;

}





/**
 * 构建协议
 *
 * @access private
 * @param  int type
 * @param  string content
 * @param  int requestId
 */
std::string client::buildPacket(int type, std::string& content, int requestId) {
	int contentLength = content.size();

	assert(contentLength >= 0 && contentLength <= MAX_LENGTH);

	std::string record;

	record += char(VERSION);                            // version
	record += char(type);                               // type
	record += char((requestId >> 8) & 0xff);        // requestIdB1
	record += char((requestId) & 0xff);        // requestIdB0
	record += char((contentLength >> 8) & 0xff);        // contentLengthB1
	record += char((contentLength) & 0xff);        // contentLengthB0
	record += char(0);                                  // paddingLength
	record += char(0);                                  // reserved
	record.append(content.data(), content.size());    // content

	return record;

}







/**
 * FastCGI 协议构建
 *
 * @access private
 * @param string name Name
 * @param string value Value
 * @return string FastCGI Name value pair
 */
std::string client::buildNvpair(std::string name, std::string value) {
	std::string nvpair("");

	int nlen = name.size();
	if (nlen < 128) {
		nvpair = (unsigned char) nlen;                     // name LengthB0
	} else {
		nvpair = (unsigned char) ((nlen >> 24) | 0x80);    // name LengthB3
		nvpair += (unsigned char) ((nlen >> 16) & 0xff);    // name LengthB2
		nvpair += (unsigned char) ((nlen >> 8) & 0xff);    // name LengthB1
		nvpair += (unsigned char) (nlen & 0xff);            // name LengthB0
	}

	int vlen = value.size();
	if (vlen < 128) {
		nvpair += (unsigned char) vlen;                     // value LengthB0
	} else {
		nvpair += (unsigned char) ((vlen >> 24) | 0x80);    // value LengthB3
		nvpair += (unsigned char) ((vlen >> 16) & 0xff);    // value LengthB2
		nvpair += (unsigned char) ((vlen >> 8) & 0xff);    // value LengthB1
		nvpair += (unsigned char) (vlen & 0xff);            // value LengthB0
	}
	nvpair += name + value;

	/* nameData & valueData */
	return nvpair;
}





/**
 * 读取packet头
 *
 * @access private
 * @return void
 */
void client::readPacketHeader(DataFunc fun, std::shared_ptr<header_t> header, std::shared_ptr<std::string> response) {

	auto* pack =new char[HEADER_LEN];



	boost::asio::async_read(socket_,boost::asio::buffer(pack, sizeof pack),[this,fun,pack,header,response](boost::system::error_code ec, std::size_t len){


		if(!ec) {



			header->version = ord(pack[0]);
			header->type = ord(pack[1]);
			header->requestId = (ord(pack[2]) << 8) + ord(pack[3]);
			header->contentLength = (ord(pack[4]) << 8) + ord(pack[5]);
			header->paddingLength = ord(pack[6]);
			header->reserved = ord(pack[7]);


			delete [] pack;


			readPacket(fun,header,response);


		}else {
			delete [] pack;
			throw std::runtime_error("Unable to read response header.");
		}




	});

}




/**
 * 读取数据体
 *
 * @access private
 * @return void
 */
void client::readPacket(DataFunc fun, std::shared_ptr<header_t> header, std::shared_ptr<std::string> response) {





	if (header->contentLength > 0) {


		char* buff = new char[header->contentLength];
		boost::asio::async_read(socket_,boost::asio::buffer(buff, header->contentLength),[this,fun,buff,header,response](boost::system::error_code ec, std::size_t len){

			//echoHex(buff,len);

			if(!ec) {
				if (header->type == STDOUT || header->type == STDERR) {
					response->append(buff,len);
				} else if (header->type == END_REQUEST) {
					header->flag = buff[4];

					fun(std::string(response->data(),response->length()));

					return;
				}
				readPacketHeader(fun,header,response);

			}else {
				throw std::runtime_error("Unable to read response content.");
			}
			delete [] buff;
		});
	}

	/* Padding读取 */
	if (header->paddingLength > 0) {
		readPadding(fun, header->paddingLength);
	}
}



/**
 * 读取padding
 *
 * @access private
 * @return void
 */
void client::readPadding(DataFunc fun, int length) {

	auto* buff = new char[length];

	boost::asio::async_read(socket_,boost::asio::buffer(buff, length),[this,fun,buff](boost::system::error_code ec, std::size_t len){
		delete [] buff;
		if(!ec) {

		}else {
			throw std::runtime_error("Unable to read response padding.");
		}
	});
}


} // namespace fcgi
