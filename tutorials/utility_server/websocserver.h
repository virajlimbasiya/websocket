
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>
using std::string;
using std::vector;
using std::map;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl Clientconn;




class websocserver: std::enable_shared_from_this<websocserver>{
    public:
        size_t numOfConn();
        websocserver()
        {
            //Wire up our event handlers
            this->endpoint.set_open_handler(std::bind(&websocserver::onOpen, this, std::placeholders::_1));
            this->endpoint.set_close_handler(std::bind(&websocserver::onClose, this, std::placeholders::_1));
            this->endpoint.set_message_handler(std::bind(&websocserver::onMessage, this, std::placeholders::_1, std::placeholders::_2));
            
            //Initialise the Asio library, using our own event loop object
            this->endpoint.init_asio(&(this->eventLoop));
        }
        void run(int port);

        // void connect(auto handler);

        // void disconnect(auto handler);

        std::shared_ptr<websocserver> get_ptr(){
            return shared_from_this();
        }


        void removeConn(int id){
            for(auto itr=this->connHandlers.begin();itr!=this->connHandlers.end();itr++){
                if(itr->first==id){
                    int close_code = websocketpp::close::status::normal;
                    this->endpoint.close(this->connHandlers[itr->first],close_code,"bad connection");
                    this->connHandlers.erase(itr->first);
                    break;
                }
            }
            std::cout<<"id:"<<id<<" disconnected\n";
            std::cout<<this->connHandlers.size()<<" connection remaining\n";
        }
        template<typename callBack>
        void connect(callBack handler){
            this->eventLoop.post([this,handler](){
                this->connectHandlers.push_back(handler);
            });
        }
        websocketpp::connection_hdl getconn(int id){
            return this->connHandlers[id];
        }

        template<typename callBack>
        void disconnect(callBack handler){
            this->eventLoop.post([this,handler](){
                this->disconnectHandlers.push_back(handler);
            });
        }

        template<typename callBack>
        void message(const char messagety[], callBack handler){
            // auto consthandler = handler;
            this->eventLoop.post([this,messagety,handler](){
                this->messageHandlers[messagety].push_back(handler);
            });
        }

        void total_connections(){
            // auto consthandler = handler;
            std::cout<<"Total connections "<<this->connHandlers.size()<<std::endl;
        }

        // template<typename callBack>
        // void message(const string &messagety,callBack handler){
        //     this->eventLoop.post([this,messagety,handler](){
        //         // std::vector<std::function<void(Clientconn, const string&)> >&a=this->messageHandlers[messagety];
        //         // a.push_back(handler);
        //         std::cout<<this->messageHandlers[messagety].size()<<std::endl;
        //         // this->messageHandlers[messagety].push_back(handler);
        //     });
        // }

        void sendmessage(Clientconn conn, const char message[], const char messagety[]);
        void publishtoall(const char message[], const char messagety[]);
        void publishdata(vector<int> connList, const char message[], const char messagety[]);
    protected:
        void onOpen(Clientconn conn);
        void onClose(Clientconn conn);
        void onMessage(Clientconn conn, server::message_ptr msg);
        void onConnect(Clientconn conn);
        boost::asio::io_service eventLoop;
        // asio::io_service eventLoop;
        vector<Clientconn>openConn;
        server endpoint;
        std::mutex mutedConnList;	
        vector<int> connectionList;
        vector<Clientconn> tradeHandlers;
        vector<Clientconn> bookHandlers;
        int conn_id=0;
        map<int,Clientconn> connHandlers;
        vector<std::function<void(Clientconn)>> connectHandlers;
		vector<std::function<void(Clientconn)>> disconnectHandlers;
		map<string, vector<std::function<void(Clientconn,const char [])>>> messageHandlers;
};

class okexclient{
    public:
        okexclient(std::shared_ptr<websocserver>s1): s(s1){
            // s = s1;
        }
        
    protected:
        std::shared_ptr<websocserver> s;
};