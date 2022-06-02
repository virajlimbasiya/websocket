#include "websocserver.h"
#include<algorithm>
#include <websocketpp/server.hpp>
#include<functional>
#include<iostream>




// void websocserver::onOpen(Clientconn conn){

// 	{
//         std::lock_guard<std::mutex> lock(this->mutedConnList);
// 	    this->openConn.push_back(conn);
// 	}
// 	for (auto handler : this->connectHandlers) {
// 		handler(conn);
// 	}
    
// }


void websocserver::onOpen(Clientconn conn){
    std::cout<<"new connection id:"<<this->conn_id<<std::endl;
    this->connHandlers.insert({this->conn_id++,conn});
}

void websocserver::onClose(Clientconn conn)
{
	{
		std::lock_guard<std::mutex> lock(this->mutedConnList);
		
		auto connVal = conn.lock();
		auto newEnd = std::remove_if(this->openConn.begin(), this->openConn.end(), [&connVal](Clientconn elem)
		{
			if (elem.expired() == true) {
				return true;
			}
			
			auto elemVal = elem.lock();
			if (elemVal.get() == connVal.get()) {
				return true;
			}

			return false;
		});
		this->openConn.resize(std::distance(openConn.begin(), newEnd));
	}

	for (auto handler : this->disconnectHandlers) {
		handler(conn);
	}
}

void websocserver::onMessage(Clientconn con, server::message_ptr msg){
    // string messageObj = msg->get_charpayload();
	char* request = msg->get_charpayload();
	if(strcmp(request,"trade")==0){
		this->tradeHandlers.push_back(con);

		this->sendmessage(con,"trade update published\0","serverMessage");
		// for(auto conn:this->tradeHandlers){
		// 	this->sendmessage(conn,msg->get_charpayload(), "serverMessage");
		// }
	}
	else if(strcmp(request, "orderbook")==0){
		this->bookHandlers.push_back(con);
		this->sendmessage(con,"orderbook update published\0","serverMessage");		
		// for(auto conn:this->bookHandlers){
		// 	this->sendmessage(conn,msg->get_charpayload(), "serverMessage");
		// }
	}
}




void websocserver::run(int port){
    std::cout<<"listening on "<<port<<std::endl;
    this->endpoint.listen(port);
    this->endpoint.start_accept();
    this->eventLoop.run();
}

void websocserver::sendmessage(Clientconn conn, const char message[], const char messagety[]){

    this->endpoint.send(conn,message,websocketpp::frame::opcode::text);
}

void websocserver::publishdata(vector<int> connList, const char message[], const char messagety[]){
	std::lock_guard<std::mutex> lock(mutedConnList);
    for(auto i: connList){
		auto conn = this->connHandlers[i];
        this->sendmessage(conn,message,messagety);
    }
}

void websocserver::publishtoall(const char message[], const char messagety[]){
    std::lock_guard<std::mutex> lock(mutedConnList);
    for(auto conn: openConn){
        this->sendmessage(conn,message,messagety);
    }
}

size_t websocserver::numOfConn(){
    std::lock_guard<std::mutex>lock(mutedConnList);
    return this->openConn.size();
}