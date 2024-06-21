#include "common.hpp"

static json global_config = {};

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
    if (curl) 
    {
        std::string jsonBody = requestBody.dump();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) 
        {
            ERRLOG("curl_easy_perform() failed: {}", curl_easy_strerror(res));
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
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
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

    for (int i = 0; i < length; ++i) 
    {
        randomString += charset[std::rand() % charset.length()];
    }

    return randomString;
}

std::vector<std::string> split(const std::string& s, char delimiter) 
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delimiter)) 
    {
        result.push_back(item);
    }

    return result;
}

std::vector<std::string> split(const std::string& s, char delimiter, size_t minChunkSize) 
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    std::string chunk;

    while (std::getline(ss, item, delimiter)) 
    {
        if (chunk.length() + item.length() + 1 <= minChunkSize) 
        {
            chunk += item + delimiter;
        } 
        else 
        {
            result.push_back(chunk);
            chunk = item + delimiter;
        }
    }

    if (!chunk.empty()) 
    {
        result.push_back(chunk);
    }

    return result;
}

