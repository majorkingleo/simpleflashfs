#include "TestCommands.h"

using namespace SimpleFlashFs::Vfs;

CommandResult TestCount::execute(const std::vector<std::string>& args)
{
    if (args.size() < 3) {
        return {false, "format: missing FILENAME or MAX-VALUE", ""};
    }

    std::string filename = get_absolute_path(args[1]);
    std::string max_value = args[2];
    uint32_t max_val = std::stoul(max_value);

    for( uint32_t i = 1; i <= max_val; i++ ) {
        std::string content = std::to_string(i) + "\n";
        try {
            auto handle = m_vfs->open(filename, std::ios::out | std::ios::app);
            if (!handle || !handle->valid()) {
                return {false, "count: cannot open file: " + filename, ""};
            }

            std::byte* data = reinterpret_cast<std::byte*>(content.data());
            size_t size = content.size();

            if (handle->write(data, size) != size) {
                return {false, "count: write error", ""};
            }            
        } catch (const std::exception& e) {
            return {false, "count: " + std::string(e.what()), ""};
        }
    }

    return {true, "", ""};
}