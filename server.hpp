#include "common.hpp"
#include "httplib.h"
#include "db.hpp"

class ollama_client;
class server
{
private:
    std::string bindAddress = {};
    int listenPort = 0;
    std::unique_ptr<httplib::Server> webSrv;
    std::unique_ptr<pgdb> database;
    std::unordered_map<std::string, std::shared_ptr<ollama_client>> ollamasMap;
    std::mutex ollamasMapMutex;
    void handleOptions(const httplib::Request &req, httplib::Response &res);
    void handleNewChat(const httplib::Request &req, httplib::Response &res);
    void handleSingleMessage(const httplib::Request &req, httplib::Response &res);
    void handleFullChat(const httplib::Request &req, httplib::Response &res);
    void handleLogin(const httplib::Request &req, httplib::Response &res);
    void handleLoginToken(const httplib::Request &req, httplib::Response &res);
    void handleChatLists(const httplib::Request &req, httplib::Response &res);
    void handleChat(const httplib::Request &req, httplib::Response &res);
    void handleChatMessage(const httplib::Request &req, httplib::Response &res);
    void handleDeleteChat(const httplib::Request &req, httplib::Response &res);
    void handleCreateEmbed(const httplib::Request &req, httplib::Response &res);
    void handleGetEmbeds(const httplib::Request &req, httplib::Response &res);

public:
    server(std::string address, int port);
    ~server();

    //blocking
    void listen();
};
