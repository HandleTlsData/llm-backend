#include <stdio.h>
#include <iostream>
#include <string>
#include <stddef.h>

#include "json.hpp"
#include <curl/curl.h>
#include <curl/options.h>
#include <openssl/sha.h>
#include <chrono>
#include <thread>

#include <fstream>

using json = nlohmann::json;

std::string makeHttpRequest(const std::string& url, const json& requestBody, const std::string& method);
std::string hashString(const std::string& source);
std::string generateString(int length);

void readConfig(const std::string& filename);

int getConfigValueInt(const std::string& key);
std::string getConfigValueString(const std::string& key);

void prepareDBDetails();
std::string getDBDetails();