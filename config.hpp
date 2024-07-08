#include "common.hpp"

class appConfig
{
private:
    std::string m_configFilename = {};
    std::string m_dbStr = {};
    json m_jsonData = {};

private:
    void createConfigFile();
    void readConfigFile();
    void readDBConnStr();
public:
    void fromFile(const std::string &filename);
public:
    std::string getDBConnString();
    template<class T>
    T getValue(const std::string& key)
    {
        std::vector<std::string> key_parts;
        std::string key_part;
        std::istringstream key_stream(key);
        while (std::getline(key_stream, key_part, '.')) 
        {
            key_parts.push_back(key_part);
        }

        const json* current_obj = &m_jsonData;
        for (const auto& part : key_parts) 
        {
            if (!current_obj->is_object() || !current_obj->contains(part)) 
                return T();
                
            current_obj = &(*current_obj)[part];
        }

        try 
        {
            return current_obj->get<T>();
        } 
        catch (const std::exception& e) 
        {
            ERRLOG("Error: {}", e.what());
            return T();
        }
    }
};

inline const auto g_config = std::make_unique< appConfig >();