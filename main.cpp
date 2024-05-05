#include "common.hpp"
#include "ollama_client.hpp"
#include "server.hpp"

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

std::string get_current_dir() 
{
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    std::string current_dir(buff);
    return current_dir;
}


void createConfig(const std::string& filename) {
    // Check if the file exists and is not empty
    std::ifstream file(filename);
    if (file.good() && file.peek() != std::ifstream::traits_type::eof()) {
        return;
    }

    printf("Generating config file\n");
    json config;
    config["server"]["bind_address"] = "0.0.0.0";
    config["server"]["bind_port"] = 9090;

    config["pg"]["DBNAME"] = "empty";
    config["pg"]["DBUSER"] = "empty";
    config["pg"]["DBPASSWD"] = "empty";
    config["pg"]["DBHOST"] = "localhost";
    config["pg"]["DBPORT"] = "5432";

    std::ofstream outfile(filename);
    outfile << std::setw(4) << config << std::endl;
}

int main()
{
    printf("Reading config\n");
    createConfig("config.json");
    readConfig("config.json");
    prepareDBDetails();

    printf("Starting backend\n");
    auto webserver = std::make_unique<server>(getConfigValueString("server.bind_address"), getConfigValueInt("server.bind_port"));
    webserver->listen();   
    return 0;
}