// #define ASIO_STANDALONE
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "websocserver.h"
#include <asio/io_service.hpp>
#include <functional>
#include<thread>
#include<string>



typedef websocketpp::connection_hdl Clientconn;
// typedef websocketpp::client<websocketpp::config::asio_client> client;


class server_metadata{
    public:
        typedef websocketpp::lib::shared_ptr<server_metadata>ptr;
        server_metadata(int id, websocketpp::connection_hdl hdl)
          : m_id(id)
          , m_hdl(hdl)
          , m_status("Connecting")
          , m_client("N/A")
            {}

        void on_open(server * s, websocketpp::connection_hdl hdl) {
            m_status = "Open";
            server::connection_ptr con = s->get_con_from_hdl(hdl);
            m_client = con->get_response_header("Server");
        }
        
        void on_message(websocketpp::connection_hdl, server::message_ptr msg){
            if(msg->get_opcode()==websocketpp::frame::opcode::text){
                m_messages.push_back("<< "+((std::string)msg->get_charpayload()));
                // std::cout<<msg->get_charpayload()<<std::endl;
            }
        }

        void record_sent_message(std::string message) {
            m_messages.push_back(">> " + message);
        }
        friend std::ostream &operator<<(std::ostream &out, server_metadata const &data);
    
    
    private:
        int m_id;
        std::string m_status;
        std::string m_client;
        websocketpp::connection_hdl m_hdl;
        std::vector<std::string> m_messages;
};

std::ostream &operator<<(std::ostream &out, server_metadata const &data){
    for(auto it = data.m_messages.begin(); it!=data.m_messages.end();++it){
        out<<*it<<std::endl;
    }
    return out;
}



// class utility_server {
// public:

//     utility_server() {
//          // Set logging settings
//         m_endpoint.set_error_channels(websocketpp::log::elevel::all);
//         m_endpoint.set_access_channels(websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload);

//         // Initialize Asio
//         m_endpoint.init_asio();
//     }

//     // void on_fail(client * c, websocketpp::connection_hdl hdl) {
//     //     m_status = "Failed";

//     //     client::connection_ptr con = c->get_con_from_hdl(hdl);
//     //     m_server = con->get_response_header("Server");
//     //     m_error_reason = con->get_ec().message();
//     // }

//     // void set_fail_handler(utility_server::on_fail);

//     websocketpp::connection_hdl get_client_handler(int id){
//         auto handler = serv.getconn(id);
//         return handler;
//     }

//     server_metadata::ptr get_metadata(websocketpp::connection_hdl hdl){
//         server *endp;
//         return std::static_pointer_cast<server_metadata>(endp->get_con_from_hdl(hdl));
//     }

//     void run() {
//         // Listen on port 9002
//         m_endpoint.listen(9002);

//         // Queues a connection accept operation
//         m_endpoint.start_accept();
//         // Start the Asio io_service run loop
//         m_endpoint.run();
//     }
//     server m_endpoint;
// private:
//     typedef std::map<int,server_metadata::ptr> con_list;
//     websocserver serv; 
// };

int main(){
    websocserver sp;
    std::shared_ptr<websocserver> s = std::make_shared<websocserver>();
    okexclient cli(s);
    boost::asio::io_service mainEventLoop;

    s->connect([&mainEventLoop,&s](Clientconn conn){
        mainEventLoop.post([conn,&s](){
            // s->addConn(conn);
            std::cout<<"connection opened "<<s->numOfConn()<<" connection(s) made\n";
        });
    });

    char msgtype[] = "message";
    // auto lambda = [msgtype,&mainEventLoop,&s](Clientconn conn, string &message){
    //     mainEventLoop.post([msgtype,&message,&conn,&s](){
    //         s->sendmessage(conn,msgtype,message);
    //     });
    // };
    s->message(msgtype,[msgtype,&mainEventLoop,&s](Clientconn conn, const char message[]){
        mainEventLoop.post([msgtype,&message,&conn,&s](){
            s->sendmessage(conn,msgtype,message);
        });
    });
    std::thread serverThread([&s](){s->run(9002);});

    std::thread inputThread([&s,&mainEventLoop](){
        char input[100];

        while(1){
            std::cin>>input;
            if(std::string(input)=="publish"){
                int id;
                int num;
                std::cin>>num;
                vector<int> connList;
                while(num--&&std::cin>>id)connList.push_back(id);
                char msg[100];
                std::cin.ignore();
                std::cin.getline(msg,100);
                s->publishdata(connList, msg, "serverMessage");
                // cli.s->total_connections();
            }
            else if(std::string(input)=="sendAll"){
                string line;
                std::getline(std::cin,line);
                
                s->publishtoall(line.c_str(),"serverMessage");
            }
            else if(std::string(input)=="disconnect"){
                int id;
                std::cin>>id;
                // s->disconnect([id,&mainEventLoop,&s](Clientconn conn){
                //     
                // });

                s->sendmessage(s->getconn(id),"closing at connection id "+ char(id),"serverMessage");
                mainEventLoop.post([&s,id](){
                    s->removeConn(id);
                });
                //s->removeConn(id);
            }
            else if(std::string(input)=="quit")break;
        }
    }
    );
    boost::asio::io_service::work work(mainEventLoop);
    mainEventLoop.run();
    std::cout<<"connection broke\n";
    // s->run(9002);
    return 0;
}