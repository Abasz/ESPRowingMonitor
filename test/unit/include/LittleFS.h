// NOLINTBEGIN
#include <string>

class LittleFSFS
{
public:
    LittleFSFS();
    ~LittleFSFS();
    bool begin(bool formatOnFail = false, const char *basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char *partitionLabel = "spiffs");
    bool format();
    size_t totalBytes();
    size_t usedBytes();
    void end();
    std::string open(std::string fileName);
};

extern LittleFSFS LittleFS;
// NOLINTEND