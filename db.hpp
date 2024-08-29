#include "common.hpp"
#include <pqxx/pqxx>

class pgdb
{
private:
    enum req_state {
        error = 0,
        found,
        not_found
    };
    std::unique_ptr<pqxx::connection> conn;
public:
    //dbdetails template: "dbname=DBNAME user=DBUSER password=DBPASSWD host=DBHOST port=DBPORT"
    pgdb(); 
    ~pgdb();

public:
    struct messageBody
    {
        std::string msg;
        bool isLocal;
        std::string imgUri;

    };

    static std::string hashPassword(const std::string& username, const std::string& password);

    bool createUser(const std::string& username, const std::string& pwd);
    pgdb::req_state usernameExists(const std::string& username);

    bool auth(const std::string& username, const std::string& password);
    bool auth(const std::string& token);
    int userIDFromToken(const std::string& token);
    std::string usernameFromToken(const std::string& token);

    void saveToken(const std::string& username, const std::string& token);
    int createChat(int user1Id, int user2Id);
    void storeMessage(int chatId, int senderId, const std::string& message, const std::string& filename = "");
    std::vector<std::pair<std::string, bool>> getChatMessages(int chatId, int userId);
    std::vector<messageBody> getChatMessagesWithImgs(int chatId, int userId);
    std::vector<int> getChatIdsForUser(int userId);
    std::string getFirstMessageOfChat(int chatId);
    void removeChatsForUser(int userId);
    void removeChatById(int chatId);

    void saveDoc(int userId, const std::string& docName, const std::string& docContent);
    std::string getDoc(int userId, const std::string& docName);
    std::vector<std::string> getUserDocs(int userId);
    void deleteDoc(int userId, const std::string& docName);
    void deleteAllUserDocs(int userId);
};

