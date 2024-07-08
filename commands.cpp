#include "commands.hpp"
#include "comfy_client.hpp"
#include "config.hpp"

void command_handler::registerCommand(const std::string &name,const std::string& description, std::function<std::string(const std::string &)> func)
{
    m_commands[name] = std::make_pair(description, func);
}

std::string command_handler::executeCommand(const std::string &input)
{
    std::regex commandRegex("\\[(\\w+)\\](.*)\\[/\\1\\](.*)");
    std::smatch match;

    if (std::regex_search(input, match, commandRegex)) 
    {
        std::string commandName = match[1].str();
        std::string args = match[2].str();

        auto it = m_commands.find(commandName);
        if (it != m_commands.end()) 
        {
            LOG("command_handler calling {}({})", it->first, args);
            return it->second.second(args);
        } 
        else 
        {
            ERRLOG("Command not found: {}", commandName);
        }
    } 
    else 
    {
        ERRLOG("Invalid command format");
    }
    return std::string();
}

std::vector<std::pair<std::string, std::string>> command_handler::listCommands()
{
    std::vector<std::pair<std::string, std::string>> commandsInfo;
    
    for (const auto& [name, data] : m_commands) 
    {
        commandsInfo.emplace_back(name, data.first); 
    }
    
    return commandsInfo;
}

std::string helper_functions::getCoordinates(const std::string &city)
{
    //to avoid city, country errors
    std::string firstWord = city.substr(0, city.find(" "));
    std::string url = "https://geocoding-api.open-meteo.com/v1/search?name=" + urlEncode(firstWord) + "&count=2&language=en&format=json";
    LOG("getCoordinates {}", url);
    std::string response = makeHttpRequest(url, "", "GET");
    if(!response.empty())
    {
        try 
        {
            json j = json::parse(response);
            if (j.empty()) 
            {
                ERRLOG("No location found for the given city.");
                return std::string();
            }

            // j[0]["lat"];
            // j[0]["lon"];
            return j["results"][0].dump();
        }
        catch (json::parse_error& e) 
        {
            ERRLOG("Error: {}", e.what());
            return std::string();
        }
    }

    return std::string();
}

std::string helper_functions::getWeather(const std::string &city)
{
    const static std::string errorMessage = "Unexpected error";
    auto strCoords = getCoordinates(city);
    // LOG("coords: {}", strCoords);
    if(strCoords.length() < 1)
    {
        return errorMessage;
    }

    try 
    {
        json coordsData = json::parse(strCoords);
        double lat, lon;
        int reportedTemp;
        std::string reportedCity, reportedCountry, reportedUnits;
        reportedCountry = coordsData["country"];
        reportedCity = coordsData["name"];
        lat = coordsData["latitude"];
        lon = coordsData["longitude"];
        std::string url = "https://api.open-meteo.com/v1/forecast?latitude=" + std::to_string(lat) + "&longitude=" + std::to_string(lon) + "&current=temperature_2m,wind_speed_10m&hourly=temperature_2m,relative_humidity_2m,wind_speed_10m";
        std::string response = makeHttpRequest(url, "", "GET");
        // LOG("weather: {}", response);

        json j = json::parse(response);
        reportedTemp = j["current"]["temperature_2m"].get<int>();
        reportedUnits = j["current_units"]["temperature_2m"];

        std::string formattedReturn = "Current Temperature in " + reportedCity + ", " + reportedCountry + " is " + std::to_string(reportedTemp) + reportedUnits;
        return formattedReturn;
    }
    catch (json::parse_error& e) 
    {
        ERRLOG("Error: {}", e.what());
    }
    return std::string();  
}

std::string helper_functions::generateImage(const std::string &prompt)
{
    std::string formattedReturn = "Generating Image of " + prompt;

    auto handler = std::make_unique<comfy_client>(g_config->getValue<std::string>("comfy.base_url"));
    handler->processSingleTxt2Img(prompt, "");

    return formattedReturn;
}
