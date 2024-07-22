#include "common.hpp"
#include <unordered_set>

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
            ERRLOG("curl_easy_perform() failed: {} for URL: {}", curl_easy_strerror(res), url);
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

std::string urlEncode(const std::string &source)
{
    std::string response = {};
    CURL* curl = curl_easy_init();
    if (curl) 
    {
        char* encoded_city = curl_easy_escape(curl, source.c_str(), source.length());
        if (!encoded_city) 
        {
            std::cerr << "Failed to encode city name" << std::endl;
            curl_easy_cleanup(curl);
            return response;
        }
        response = std::string(encoded_city);
        return response;
    }
    return response;
}

std::string base64_encode(const std::vector<unsigned char> &data)
{
    static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

    std::string encoded;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    int in_len = data.size();

    while (in_len--) 
    {
        char_array_3[i++] = data[j++];
        if (i == 3) 
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                encoded += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) 
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; j++)
            encoded += base64_chars[char_array_4[j]];

        while(i++ < 3)
            encoded += '=';
    }

    return encoded;
}

std::string get_mime_type(const std::string& filename) 
{
    static const std::map<std::string, std::string> mime_types = 
    {
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".bmp", "image/bmp"},
        {".webp", "image/webp"},
        {".svg", "image/svg+xml"}
    };

    std::string extension = std::filesystem::path(filename).extension().string();
    auto it = mime_types.find(extension);
    if (it != mime_types.end()) 
    {
        return it->second;
    }
    return "application/octet-stream";
}

std::string base64_imageFile(const std::string &base_dir, const std::string &filename)
{
    std::filesystem::path file_path = std::filesystem::path(base_dir) / filename;

    LOG("Base64 filepath: {}", file_path.string());
    
    if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path)) 
    {
        throw std::runtime_error("File does not exist: " + file_path.string());
    }

    std::ifstream file(file_path, std::ios::binary);
    if (!file) 
    {
        throw std::runtime_error("Cannot open file: " + file_path.string());
    }

    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
    std::string base64_data = base64_encode(buffer);
    
    std::string mime_type = get_mime_type(filename);
    
    return "data:" + mime_type + ";base64," + base64_data;
}

std::string currentTime(const std::string &format)
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
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

