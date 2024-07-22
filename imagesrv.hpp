#include "common.hpp"
#include "httplib.h"

class imagesrv
{
private:
    bool isAuthorized(const httplib::Request& req);
    bool isAllowedExtension(const std::string& filename);  
public:
    imagesrv();
    ~imagesrv();

    void requestFile(const httplib::Request& req, httplib::Response& res);
};

inline const auto g_imagesrv = std::make_unique< imagesrv >();