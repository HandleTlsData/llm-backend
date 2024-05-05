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
    /* data */
public:
    //dbdetails template: "dbname=DBNAME user=DBUSER password=DBPASSWD host=DBHOST port=DBPORT"
    pgdb(); 
    ~pgdb();

    std::string hashPassword(const std::string& username, const std::string& password);

    bool createUser(const std::string& username, const std::string& pwd);
    pgdb::req_state usernameExists(const std::string& username);

    bool auth(std::string& username, std::string& password);
    bool auth(std::string& token);
    int userIDFromToken(std::string& token);

    void saveToken(const std::string& username, const std::string& token);
    int createChat(int user1Id, int user2Id);
    void storeMessage(int chatId, int senderId, const std::string& message);
    std::vector<std::string> retrieveMessages(int ownerId, int chatId);
    std::vector<std::pair<std::string, bool>> getChatMessages(int chatId, int userId);
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

