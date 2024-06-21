#include "common.hpp"
#include "httplib.h"
#include "db.hpp"
#include "rag.hpp"

class ollama_client;
class server
{
private:
    std::string m_bindAddress = {};
    int m_listenPort = 0;
    std::unique_ptr<httplib::Server> m_webSrv;
    std::unique_ptr<pgdb> m_database;
    std::unordered_map<std::string, std::shared_ptr<ollama_client>> m_ollamasMap;
    std::mutex m_ollamasMapMutex;
    std::unique_ptr<rag> m_ragHelper;
private:
    std::shared_ptr<ollama_client> getClientInstance(const std::string& token);
    std::string processImageToText(const std::string &incomingMessage, const std::string &imageBase64);

private:
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
    void handleGetModels(const httplib::Request &req, httplib::Response &res);

public:
    server(std::string address, int port);
    ~server();

    //blocking
    void listen();
};
