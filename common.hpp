#include <stdio.h>
#include <iostream>
#include <string>
#include <stddef.h>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <curl/curl.h>
#include <curl/options.h>
#include <openssl/sha.h>
#include <chrono>
#include <thread>

#include <fstream>
#include <filesystem>

#define LOG spdlog::info
#define ERRLOG spdlog::error

using json = nlohmann::json;

std::string makeHttpRequest(const std::string& url, const json& requestBody, const std::string& method);
std::string hashString(const std::string& source);
std::string generateString(int length);
std::string urlEncode(const std::string& source);
std::string base64_encode(const std::vector<unsigned char>& data);


std::vector<std::string> split(const std::string& s, char delimiter);
std::vector<std::string> split(const std::string& s, char delimiter, size_t minChunkSize);