


#include <iostream>

#include <boost/asio.hpp>
#include <map>
#include <functional>

#include "FcgiClient.h"

void getData(std::string&& data) {

	std::cout <<  data << std::endl;
}


int main(/*int argc, char* argv[]*/) {
	try {

		boost::asio::io_service io_service;

		Fcgi::client client(io_service,"127.0.0.1",9000);

		client.onConnect([&client](){


			std::cout << "链接成功" << std::endl;

			std::map<std::string,std::string> header;
			header["REQUEST_METHOD"]    = "GET";
			header["SCRIPT_FILENAME"]   = "/data/www/az/trunk/newgomarket.goapk.com/test.php";


			client.request(std::bind(&getData,std::placeholders::_1),header,str);


		});


		io_service.run();
	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
