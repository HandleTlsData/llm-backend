#include "common.hpp"

static json global_config = {};
static std::string dbDetailsStr = {};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string makeHttpRequest(const std::string &url, const json &requestBody, const std::string &method)
{
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) {
        std::string jsonBody = requestBody.dump();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    return response;
}

std::string hashString(const std::string &source)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, source.c_str(), source.length());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string generateString(int length)
{
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string randomString;
    randomString.reserve(length);

    // Seed the random number generator
    std::srand(std::time(nullptr));

    for (int i = 0; i < length; ++i) {
        randomString += charset[std::rand() % charset.length()];
    }

    return randomString;
}

void prepareDBDetails()
{
    dbDetailsStr = "dbname=" + getConfigValueString("pg.DBNAME") + " user=" + getConfigValueString("pg.DBUSER") 
                    + "  password=" + getConfigValueString("pg.DBPASSWD") + " host=" + getConfigValueString("pg.DBHOST")
                    + " port=" + getConfigValueString("pg.DBPORT");
}

std::string getDBDetails()
{
    return dbDetailsStr;
}

void readConfig(const std::string& filename) 
{
    std::ifstream file(filename);
    file >> global_config;
}

int getConfigValueInt(const std::string& key) {
    // Split the key by '.' to handle nested objects
    std::vector<std::string> key_parts;
    std::string key_part;
    std::istringstream key_stream(key);
    while (std::getline(key_stream, key_part, '.')) {
        key_parts.push_back(key_part);
    }

    // Traverse the JSON object to find the value
    const json* current_obj = &global_config;
    for (const auto& part : key_parts) {
        if (!current_obj->is_object() || !current_obj->contains(part)) {
            return int();
        }
        current_obj = &(*current_obj)[part];
    }

    // Convert the value to the desired type
    try {
        return current_obj->get<int>();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return int();
    }
}

std::string getConfigValueString(const std::string& key) {
    // Split the key by '.' to handle nested objects
    std::vector<std::string> key_parts;
    std::string key_part;
    std::istringstream key_stream(key);
    while (std::getline(key_stream, key_part, '.')) {
        key_parts.push_back(key_part);
    }

    // Traverse the JSON object to find the value
    const json* current_obj = &global_config;
    for (const auto& part : key_parts) {
        if (!current_obj->is_object() || !current_obj->contains(part)) {
            return std::string();
        }
        current_obj = &(*current_obj)[part];
    }

    // Convert the value to the desired type
    try {
        return current_obj->get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return std::string();
    }
}