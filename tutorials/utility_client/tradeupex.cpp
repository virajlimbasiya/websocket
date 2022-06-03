/*
 * Copyright (c) 2016, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <boost/asio.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <asio/io_service.hpp>
#include <websocketpp/config/debug_asio.hpp>
#include <websocketpp/server.hpp>
#include <functional>
#include <websocserver.h>
#include <iostream>

typedef websocketpp::client<websocketpp::config::debug_asio_tls> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;


using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

//sing std::bind;
using websocketpp::lib::bind;
using namespace std;
struct okxlevelupdate{
    string contract;
    vector<vector<double>> ask;
    vector<vector<double>> bid;
    uint64_t ts;
    long checksum;
};


struct Tradeupdate
{
    string instId;
    string tradeId;
    double price;
    double quantity;
    int side;  //0 for buy 1 for sell
    uint64_t ts;
};


void getparsed(char m_buff[], int strLength, okxlevelupdate& okx){


    if(m_buff[19]=='b'){
        okxlevelupdate okx;
        string key = "";
        string insType = "";
        vector<vector<double>>ask;
        vector<vector<double>>bid;
        string ts = "";
        string chksum = "";
        uint64_t tStamp = 0;
        long checksum=0;
        int i = 35;
        int c=0;
        while(c<2){
            if(m_buff[i]=='\"')c++;
            insType+=m_buff[i++];
        }

        if(m_buff[i+12]=='u')i+=36;
        else if(m_buff[i+12]=='s')i+=38;
        int b_count = 1;
        i++;
        vector<double> temp;
        while(b_count!=0){
            if(m_buff[i]=='['){
                b_count+=1;
                i++;
                continue;
            }
            else if(m_buff[i]==']'){
                b_count-=1;

                if(b_count==1){
                    ask.push_back(temp);
                    temp.clear();
                }
                i++;
                continue;
            }
            else if(m_buff[i]==','){
                i++;
                continue;
                }
            if(m_buff[i]=='\"'){
                string num = "";
                i++;
                while(m_buff[i]!='\"'){
                    num+=m_buff[i];
                    i++;
                }
                i++;
                double x = 0;
                stringstream s(num);
                s>>x;
                temp.push_back(x);
            }
        }
        temp.clear();



        i+=9;
        b_count = 1;
        while(b_count!=0){
            if(m_buff[i]=='['){
                b_count+=1;
                i++;
                continue;
            }
            else if(m_buff[i]==']'){
                b_count-=1;

                if(b_count==1){
                    bid.push_back(temp);
                    temp.clear();
                }
                i++;
                continue;
            }
            else if(m_buff[i]==','){
                i++;
                continue;
                }
            if(m_buff[i]=='\"'){
                string num = "";
                i++;
                while(m_buff[i]!='\"'){
                    num+=m_buff[i];
                    i++;
                }
                i++;
                double x = 0;
                stringstream s(num);
                s>>x;
                temp.push_back(x);
            }
        }
        temp.clear();
 
    

        i+=7;
        c=0;
        while(c<1){
            if(m_buff[i]=='\"')c++;
            ts+=m_buff[i++];
        }
        i++;
        stringstream s(ts);
        s>>tStamp;
        i+=11;
        //string chksum = "";
        while(m_buff[i]!='}'){
            chksum+=m_buff[i++];
        }

        stringstream s1(chksum);
        s1>>checksum;

        okx.contract = insType;
        okx.ask = ask;
        okx.bid = bid;
        okx.ts = tStamp;
        okx.checksum = checksum;

        cout<<okx.contract<<endl;
    cout<<"ask\n";
    for(auto ar: okx.ask){
        for(auto x: ar)cout<<x<<" ";
        cout<<endl;
    }
    cout<<"bid\n";
    for(auto ar: okx.bid){
        for(auto x: ar)cout<<x<<" ";
        cout<<endl;
    }
    cout<<okx.ts<<" "<<okx.checksum<<endl;

    }else if(m_buff[19]=='t'){
        Tradeupdate tup;
        string instId="";
        string tradeId = "";
        double price;
        double quantity;
        int side;  //0 for buy 1 for sell
        uint64_t ts;
        int i = 36;
        int c = 0;
        while(c<2){
            if(m_buff[i]=='\"'){
                c++;
                i++;
                continue;
            }
            instId+=m_buff[i++];
        }
        i+=35+instId.length()-1;
        
        while(m_buff[i]!='\"'){
            tradeId+=m_buff[i++];
        }
        i+=8;
        string px = "";
        while(m_buff[i]!='\"'){
            if(m_buff[i]!='\"')px+=m_buff[i++];
        }
        stringstream s(px);
        s>>price;
        i+=8;
        string sz="";
        while(m_buff[i]!='\"'){
            if(m_buff[i]!='\"')sz+=m_buff[i++];
        }
        stringstream s1(sz);
        s1>>quantity;
        cout<<"size:"<<quantity<<endl;
        i+=10;
        if(m_buff[i]=='s')side = 1;
        else if(m_buff[i]=='b')side = 0;

        i+=12;
    
        string time_stamp = "";

        while(m_buff[i]!='\"'){
            if(m_buff[i]!='\"')time_stamp+=m_buff[i++];
        }
        stringstream s3(time_stamp);
        s3>>ts;
    
        tup.instId = instId;
        tup.tradeId = tradeId;
        tup.price = price;
        tup.quantity = quantity;
        tup.side = side;
        tup.ts = ts;


        cout<<"instid:"<<tup.instId<<endl;
        cout<<"tradeid:"<<tup.tradeId<<endl;
        cout<<"price:"<<tup.price<<endl;
        cout<<"quantity:"<<tup.quantity<<endl;
        cout<<"side:"<<tup.side<<endl;
        cout<<"ts:"<<tup.ts<<endl;
    }else{
        cerr<<"required data missing!"<<endl;
    }
}





    std::string hostname = "ws.okex.com";
    std::string port = "8443";
    std::string uri = "wss://ws.okex.com:8443/ws/v5/public";


//void connect(client &c);

class okexclient{
    public:    
   // context_ptr on_tls_init(const char * hostname, websocketpp::connection_hdl) ;

    okexclient(std::shared_ptr<websocserver>s1): s(s1){ 
        // connect(c);
    }
    void connect(){
        c.set_access_channels(websocketpp::log::alevel::all);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);
        c.set_error_channels(websocketpp::log::elevel::all);

        // // Initialize ASIO
        c.init_asio();

        // // Register our message handler
        c.set_message_handler(websocketpp::lib::bind(&okexclient::on_message,this,placeholders::_1,placeholders::_2));
        c.set_open_handler(websocketpp::lib::bind(&okexclient::on_open,this,placeholders::_1));
        c.set_tls_init_handler(websocketpp::lib::bind(&okexclient::on_tls_init, this, placeholders::_1));

        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        if (ec) {
            std::cout << "could not create connection because: " << ec.message() << std::endl;
            return;
        }
        c.connect(con);
        c.get_alog().write(websocketpp::log::alevel::app, "Connecting to " + uri);            
            // s = s1;
        c.run();

        //m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &c);
    }


    void on_message(websocketpp::connection_hdl , client::message_ptr msg) {
        char *char_array =msg->get_charpayload();
        int n = strlen(char_array);
        if(char_array[19]=='t')s->publishTrade(char_array,"serverMessage");
        else if(char_array[19]=='b')s->publishOrderBook(char_array,"serverMessage");
    }

    void on_open(websocketpp::connection_hdl hdl) {
        c.send(hdl, "{\"op\": \"subscribe\",\"args\": [{\"channel\": \"books\",\"instId\": \"BTC-USDT\"}]}", websocketpp::frame::opcode::text);
        c.send(hdl, "{\"op\": \"subscribe\",\"args\": [{\"channel\": \"trades\",\"instId\": \"BTC-USDT-SWAP\"}]}", websocketpp::frame::opcode::text);
    }

    context_ptr on_tls_init(websocketpp::connection_hdl) {
        context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                            boost::asio::ssl::context::no_sslv2 |
                            boost::asio::ssl::context::no_sslv3 |
                            boost::asio::ssl::context::single_dh_use);


            ctx->set_verify_mode(boost::asio::ssl::verify_none);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return ctx;
    }
    private:

    std::shared_ptr<websocserver> s;
    client c;
    std::shared_ptr<websocketpp::lib::thread> m_thread;
};



int main(int argc, char* argv[]) {

    std::shared_ptr<websocserver> s = std::make_shared<websocserver>(); 

    boost::asio::io_service mainEventLoop;


    s->connect([&mainEventLoop,&s](Clientconn conn){
        mainEventLoop.post([conn,&s](){
            // s->addConn(conn);
            std::cout<<"connection opened "<<s->numOfConn()<<" connection(s) made\n";
        });
    });
    std::thread serverThread([&s](){s->run(9001);});

    okexclient c(s);
    c.connect();


    try {
        // Set logging to be pretty verbose (everything except message payloads)


        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.


        // Start the ASIO io_service run loop
        // this will cause a single connection to be made to the server. c.run()
        // will exit when this connection is closed.
        //m_endpoint.send(hdl, "", websocketpp::frame::opcode::text);

    } catch (websocketpp::exception const & e) {
        std::cout << e.what() << std::endl;
    }
}