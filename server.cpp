#include "server.hpp"
#include "ollama_client.hpp"
#include "config.hpp"
#include "commands.hpp"

using namespace httplib;

#define ROUTER_POST(uri, handler) m_webSrv->Options(uri, [&](const Request& req, Response& res) {this->handleOptions(req,res);}); \
            m_webSrv->Post(uri, [&](const Request& req, Response& res) {handler(req,res);});

#define DEFAULT_MODEL "llama3:latest"
//#define OLLAMA_URL "http://127.0.0.1:11434"

std::shared_ptr<ollama_client> server::getClientInstance(const std::string &token)
{
    std::lock_guard<std::mutex> lock(this->m_ollamasMapMutex);

    auto it = this->m_ollamasMap.find(token);
    if (it == this->m_ollamasMap.end()) 
    {
        LOG("creating new ollama client");
        auto newOllama = std::make_shared<ollama_client>(m_urlChat, m_urlImage, m_urlEmbed);
        this->m_ollamasMap[token] = newOllama;
        newOllama->accessed();
        return newOllama;
    } 
    else 
    {
        LOG("reusing ollama client");
        it->second->accessed();
        return it->second;     
    }
}

void server::handleOptions(const httplib::Request &req, httplib::Response &res)
{
    res.set_content("", "text/plain");
    res.status = 200;
}

void server::handleNewChat(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    std::string token = json_data["token"];
    // printf("token: %s \n", token.c_str());
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        //res.set_content("Logged in", "text/plain");
        int userID = db->userIDFromToken(token);
        int newChatID = db->createChat(userID, 3);
        auto strChatID = std::to_string(newChatID);
        res.set_content(strChatID.c_str(), "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

void server::handleSingleMessage(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    std::string token = json_data["token"];
    std::string reqText = json_data["text"];
    // printf("token: %s \n", token.c_str());
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        auto chatHandler = getClientInstance(token);

        auto response = chatHandler->processSingleMessage(reqText);
        chatHandler->accessed();

        res.set_content(response.c_str(), "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

void server::handleFullChat(const httplib::Request & req, httplib::Response & res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    std::string token = json_data["token"];
    std::string chatID = json_data["chatID"];
    // printf("token: %s \n", token.c_str());
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        int userID = db->userIDFromToken(token);
        if(userID > 0)
        {
            auto msgs = db->retrieveMessages(userID, std::stoi(chatID));
        }
        else
        {            
            res.set_content("", "text/plain");
        }
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

void server::handleLogin(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    std::string username = json_data["name"];
    std::string password = json_data["password"];
    // printf("login: %s %s\n", username.c_str(), password.c_str());
    auto db = std::make_unique<pgdb>();
    if(db->auth(username, password))
    {
        std::string randomWords = generateString(10);
        std::string uniqueToken = hashString(randomWords);
        db->saveToken(username, uniqueToken);
        res.set_content(uniqueToken.c_str(), "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Wrong login-password combination", "text/plain");
        res.status = 404;
    }
}

void server::handleLoginToken(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    std::string token = json_data["token"];
    // printf("token: %s \n", token.c_str());
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        res.set_content("Logged in", "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

void server::handleChatLists(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    std::string token = json_data["token"];
    // printf("token: %s \n", token.c_str());
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        int userID = db->userIDFromToken(token);
        auto list = db->getChatIdsForUser(userID);

        json response_json = {};
        response_json["chatsArray"] = list;

        res.set_content(response_json.dump(), "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }

}

void server::handleChatMessage(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    // printf("json_data: %s \n", json_data.dump().c_str());
    std::string token = json_data["token"];
    int chatID = json_data["chatID"];
    std::string incomingMessage = json_data["message"];
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        int userID = db->userIDFromToken(token);
        auto list = db->getChatIdsForUser(userID);

        auto it = std::find(list.begin(), list.end(), chatID);
        if (it != list.end()) 
        {
            std::string response = {};
            auto chatHandler = getClientInstance(token);
            auto chatHistory = db->getChatMessages(chatID, userID);

            if(json_data.contains("image") && json_data["image"].get<std::string>().length() > 0)
            {
                //img2txt logic
                std::string imageB64 = json_data["image"];
                LOG("processing img2txt request");
                response = chatHandler->processChatMessageWithImage(incomingMessage, imageB64);
            }
            else if(json_data.contains("doc"))
            {
                //embed logic
                std::string docName = json_data["doc"];
                std::string vecData = db->getDoc(userID, docName);
                LOG("processing embeded request");
                response = chatHandler->processChatMessageWithEmbed(incomingMessage, vecData, chatHistory);
            }
            else
            {
                LOG("processing regular request");
                auto incomingCommand = chatHandler->processMessageWithCommandHandler(incomingMessage);
                LOG("command: {}", incomingCommand);
                auto commandResponse = g_cmd->executeCommand(incomingCommand);
                if(commandResponse.length() > 0)
                {
                    response = commandResponse;
                }
                else
                {
                    response = chatHandler->processChatMessage(incomingMessage, chatHistory);
                }
            }

            db->storeMessage(chatID, userID, incomingMessage);
            db->storeMessage(chatID, 3, response);

            res.set_content(response.c_str(), "text/plain");
            res.status = 200;
        } 
        else 
        {
            ERRLOG("unauthorized chat access");
            res.set_content("unauthorized access", "text/plain");
            res.status = 500;
        }
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }

}

void server::handleDeleteChat(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    std::string token = json_data["token"];
    int chatID = json_data["chatID"];
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        int userID = db->userIDFromToken(token);
        auto list = db->getChatIdsForUser(userID);

        auto it = std::find(list.begin(), list.end(), chatID);

        if (it != list.end()) 
        {
            db->removeChatById(chatID);
            res.set_content("", "text/plain");
            res.status = 200;
            return;
        } 
        else 
        {
            ERRLOG("unauthorized chat access");
            res.set_content("unauthorized access", "text/plain");
            res.status = 500;
        }
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }

}

void server::handleCreateEmbed(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    // printf("json_data: %s \n", json_data.dump().c_str());
    std::string token = json_data["token"];
    std::string emName = json_data["docname"];
    std::string textData = json_data["content"];
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        int userID = db->userIDFromToken(token);
        auto chatHandler = getClientInstance(token);
        auto docs = split(textData, '\n', 450);
        std::vector<std::vector<float>> vecDocs = {};
        for(auto& doc: docs)
        {
            vecDocs.push_back(chatHandler->processNewEmbedding(doc));
        }

        auto vecDB = m_ragHelper->createNewDoc(emName, vecDocs);

        db->saveDoc(userID, emName, textData);
        res.set_content("", "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

void server::handleGetEmbeds(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    // printf("json_data: %s \n", json_data.dump().c_str());
    std::string token = json_data["token"];
    auto db = std::make_unique<pgdb>();
    if(db->auth(token))
    {
        int userID = db->userIDFromToken(token);
        auto list = db->getUserDocs(userID);

        json response_json = {};
        response_json["embeds"] = list;

        res.set_content(response_json.dump(), "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

void server::handleGetModels(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    // printf("json_data: %s \n", json_data.dump().c_str());
    std::string token = json_data["token"];
    auto db = std::make_unique<pgdb>();
    if(db->auth(token))
    {
        auto chatHandler = getClientInstance(token);
        auto ret = chatHandler->processListModels();
        res.set_content(ret, "text/plain");
        res.status = 200;
    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

void server::handleChat(const httplib::Request &req, httplib::Response &res)
{
    std::string body = req.body;
    json json_data = json::parse(body);
    // printf("json_data: %s \n", json_data.dump().c_str());
    std::string token = json_data["token"];
    int chatID = json_data["chatID"];
    auto db = std::make_unique<pgdb>();

    if(db->auth(token))
    {
        int userID = db->userIDFromToken(token);
        auto list = db->getChatIdsForUser(userID);

        auto it = std::find(list.begin(), list.end(), chatID);
        if (it != list.end()) 
        {
            auto msgsArray = db->getChatMessages(chatID, userID);

            json response_json = {};
            response_json["chatMessages"] = msgsArray;

            res.set_content(response_json.dump(), "text/plain");
            res.status = 200;
        } 
        else 
        {
            ERRLOG("unauthorized chat access");
            res.set_content("unauthorized access", "text/plain");
            res.status = 500;
        }

    }
    else
    {
        res.set_content("Unknown token", "text/plain");
        res.status = 404;
    }
}

server::server(std::string address, int port) : m_webSrv(std::make_unique<httplib::Server>()), m_database(std::make_unique<pgdb>()), m_ragHelper(std::make_unique<rag>())
{
    m_bindAddress = address;
    m_listenPort = port;

    m_urlChat = g_config->getValue<std::string>("ollama.chat.base_url");
    m_urlEmbed = g_config->getValue<std::string>("ollama.embed.base_url");
    m_urlImage = g_config->getValue<std::string>("ollama.image.base_url");

}

server::~server()
{
}

void server::listen()
{
    //for CORS we need to handle OPTIONS - return 200. Headers are set by default
    ROUTER_POST("/login", this->handleLogin);
    ROUTER_POST("/loginToken", this->handleLoginToken);
    ROUTER_POST("/createChat", this->handleNewChat);
    ROUTER_POST("/singleMessage", this->handleSingleMessage);
    ROUTER_POST("/getChats", this->handleChatLists);
    ROUTER_POST("/getChat", this->handleChat);
    ROUTER_POST("/chatMessage", this->handleChatMessage);
    ROUTER_POST("/deleteChat", this->handleDeleteChat);
    ROUTER_POST("/createEmbed", this->handleCreateEmbed);
    ROUTER_POST("/getEmbeds", this->handleGetEmbeds);
    ROUTER_POST("/getModels", this->handleGetModels);

    m_webSrv->set_default_headers({
        { "Access-Control-Allow-Origin", "*" },
        { "Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS" },
        { "Access-Control-Allow-Headers", "Origin, Content-Type, X-Auth-Token" }
    });

    m_webSrv->listen(m_bindAddress, m_listenPort);
}