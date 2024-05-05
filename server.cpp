#include "server.hpp"
#include "ollama_client.hpp"

using namespace httplib;

#define ROUTER_POST(uri, handler) webSrv->Options(uri, [&](const Request& req, Response& res) {this->handleOptions(req,res);}); \
            webSrv->Post(uri, [&](const Request& req, Response& res) {handler(req,res);});

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
        //res.set_content("Logged in", "text/plain");
        std::lock_guard<std::mutex> lock(this->ollamasMapMutex);

        auto handleSingle = [&](std::shared_ptr<ollama_client> chatHandler) 
        {
            printf("\n>> loading model llama2:7b");
            while(!chatHandler->loadModel())
            {
                printf("\n>> loading model");
            }
            printf("\n>> model loaded");
            auto response = chatHandler->processSingleMessage(reqText);
            chatHandler->accessed();

            res.set_content(response.c_str(), "text/plain");
            res.status = 200;

        };

        auto it = this->ollamasMap.find(token);
        if (it == this->ollamasMap.end()) {
            printf("\n>> creating new ollama client");
            auto newOllama = std::make_shared<ollama_client>("http://127.0.0.1:11434", "llama3:latest");
            this->ollamasMap[token] = newOllama;
            handleSingle(newOllama);
        } else {
            printf("\n>> reusing ollama client");
            handleSingle(it->second);          
        }
        
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

        if (it != list.end()) {
            std::string response = {};
            
            auto handleAIConversation = [&](std::shared_ptr<ollama_client> chatHandler, std::vector<std::pair<std::string, bool>>& chatHistory) 
            {
                chatHandler->accessed();
                bool imageProcessing = false;
                std::string imageB64 = {};
                if(json_data.contains("image"))
                {
                    imageB64 = json_data["image"];
                    imageProcessing = imageB64.length() > 0;
                }
                
                if(json_data.contains("doc"))
                {
                    std::string docName = json_data["doc"];
                    std::string vecData = db->getDoc(userID, docName);
                    printf("\nprocessing embeded request\n");
                    response = chatHandler->processChatMessageWithEmbed(incomingMessage, vecData, chatHistory);
                }
                else
                {
                    response = imageProcessing ? chatHandler->processChatMessageWithImage(incomingMessage, imageB64, chatHistory) : chatHandler->processChatMessage(incomingMessage, chatHistory);
                }

                res.set_content(response.c_str(), "text/plain");
                res.status = 200;

            };

            {
                std::lock_guard<std::mutex> lock(this->ollamasMapMutex);
                auto it = this->ollamasMap.find(token);
                if (it == this->ollamasMap.end()) {
                    printf("\n>> creating new ollama client");
                    auto newOllama = std::make_shared<ollama_client>("http://127.0.0.1:11434", "llama3:latest");
                    this->ollamasMap[token] = newOllama;
                }
            }

            auto msgsArray = db->getChatMessages(chatID, userID);
            handleAIConversation(this->ollamasMap[token], msgsArray);
            db->storeMessage(chatID, userID, incomingMessage);
            db->storeMessage(chatID, 3, response);

        } else {
            printf("unauthorized chat access");
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
            printf("unauthorized chat access");
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

        auto handleEmbed = [&](std::shared_ptr<ollama_client> chatHandler) 
        {
            chatHandler->accessed();
            //auto response = chatHandler->processNewEmbedding(textData);
            db->saveDoc(userID, emName, textData);

            res.set_content("", "text/plain");
            res.status = 200;
        };

        auto it = this->ollamasMap.find(token);
        if (it == this->ollamasMap.end()) {
            printf("\n>> creating new ollama client");
            auto newOllama = std::make_shared<ollama_client>("http://127.0.0.1:11434", "llama3:latest");
            this->ollamasMap[token] = newOllama;
            handleEmbed(newOllama);
        } else {
            printf("\n>> reusing ollama client");
            handleEmbed(it->second);          
        }

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

        if (it != list.end()) {
            auto msgsArray = db->getChatMessages(chatID, userID);

            json response_json = {};
            response_json["chatMessages"] = msgsArray;

            res.set_content(response_json.dump(), "text/plain");
            res.status = 200;
        } else {
            printf("unauthorized chat access");
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

server::server(std::string address, int port) : webSrv(std::make_unique<httplib::Server>()), database(std::make_unique<pgdb>())
{
    this->bindAddress = address;
    this->listenPort = port;

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

    webSrv->set_default_headers({
        { "Access-Control-Allow-Origin", "*" },
        { "Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS" },
        { "Access-Control-Allow-Headers", "Origin, Content-Type, X-Auth-Token" }
    });

    webSrv->listen(this->bindAddress, this->listenPort);
}