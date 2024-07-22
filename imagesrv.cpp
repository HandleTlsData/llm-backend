#include "imagesrv.hpp"
#include "config.hpp"
#include "server.hpp"

bool imagesrv::isAuthorized(const httplib::Request &req)
{
    std::string username = req.matches[1];
    if(username.empty())
        return false;

    auto token = server::getCookie(req, "token");
    if(token.empty())
        return false;

    auto db = std::make_unique<pgdb>();
    if(!db->auth(token))
        return false;

    std::string tokenOwner = db->usernameFromToken(token);
    return username == tokenOwner;
}

bool imagesrv::isAllowedExtension(const std::string &filename)
{
    static const std::unordered_set<std::string> allowed_extensions = {".jpg", ".jpeg", ".png", ".gif"};
    std::string extension = std::filesystem::path(filename).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    return allowed_extensions.find(extension) != allowed_extensions.end();
}

imagesrv::imagesrv()
{
}

imagesrv::~imagesrv()
{
}

void imagesrv::requestFile(const httplib::Request &req, httplib::Response &res)
{
    std::string username = req.matches[1];
    std::string filename = req.matches[2];

    if(!isAuthorized(req))
    {
        res.status = 401;
        res.set_content("Unauthorized", "text/plain");
        return;
    }

    if (!isAllowedExtension(filename)) 
    {
        res.status = 403;
        res.set_content("Forbidden", "text/plain");
        return;
    }

    std::string path = g_config->getValue<std::string>("comfy.fs_location") + "output/" + username + "/" + filename;
    LOG("requested image path: {}", path);
    if (httplib::detail::is_file(path)) 
    {
        auto mm = std::make_shared<httplib::detail::mmap>(path.c_str());
        if (!mm->is_open()) 
            return;
        res.set_header("Cache-Control", "public, max-age=3600");
        res.set_content_provider(
            mm->size(),
            httplib::detail::find_content_type(path, {},"application/octet-stream"),
            [mm](size_t offset, size_t length, httplib::DataSink &sink) -> bool {
            sink.write(mm->data() + offset, length);
            return true;
        });
    }

}
