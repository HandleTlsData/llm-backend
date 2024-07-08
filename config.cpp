#include "config.hpp"

void appConfig::createConfigFile()
{
    std::ifstream file(m_configFilename);
    if (file.good() && file.peek() != std::ifstream::traits_type::eof()) 
    {
        return;
    }

    LOG("Generating config file");
    json config;
    config["server"]["bind_address"] = "0.0.0.0";
    config["server"]["bind_port"] = 9090;

    config["pg"]["DBNAME"] = "empty";
    config["pg"]["DBUSER"] = "empty";
    config["pg"]["DBPASSWD"] = "empty";
    config["pg"]["DBHOST"] = "localhost";
    config["pg"]["DBPORT"] = "5432";

    //In case you want to have an individual instance for different types of models.
    //At the moment, ollama can only keep one model in memory; when processing is requested with another model, 
    //the old one will be unloaded from memory and the new one will be loaded. 
    //Therefore, having multiple instances can be useful if you have enough memory for multiple models.
    config["ollama"]["chat"]["base_url"] = "http://127.0.0.1:11434";
    config["ollama"]["image"]["base_url"] = "http://127.0.0.1:11434";
    config["ollama"]["embed"]["base_url"] = "http://127.0.0.1:11434";

    config["comfy"]["base_url"] = "http://127.0.0.1:8188";
    config["comfy"]["fs_location"] = "/home/comfy/";

    std::ofstream outfile(m_configFilename);
    outfile << std::setw(4) << config << std::endl;
}

void appConfig::readConfigFile()
{
    std::ifstream file(m_configFilename);
    file >> m_jsonData;
}

void appConfig::readDBConnStr()
{
    m_dbStr = "dbname=" + getValue<std::string>("pg.DBNAME") + " user=" + getValue<std::string>("pg.DBUSER") 
                + "  password=" + getValue<std::string>("pg.DBPASSWD") + " host=" + getValue<std::string>("pg.DBHOST")
                + " port=" + getValue<std::string>("pg.DBPORT");
}

void appConfig::fromFile(const std::string &filename)
{
    m_configFilename = filename;
    createConfigFile();
    readConfigFile();
    readDBConnStr();
}

std::string appConfig::getDBConnString()
{
    return m_dbStr;
}
