#pragma once

#include "SimpleFlashFsVfs.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <list>

namespace SimpleFlashFs::Vfs
{

class SimpleFlashFsThreadedVfsServer : public VfsServerInterface
{
private:
    std::thread                                     m_server_thread;
    std::mutex                                      m_mutex;
    std::atomic<bool>                               m_running {false};
    std::list<std::shared_ptr<VfsDriveInterface>>   m_drives {};        

public:
    SimpleFlashFsThreadedVfsServer();
    ~SimpleFlashFsThreadedVfsServer();

    void start() override;
    void stop() override;

    bool register_drive( std::shared_ptr<VfsDriveInterface> drive ) override;

    file_handle_t open( const std::string_view & path, std::ios_base::openmode mode ) override;
    bool list_files( std::function<bool(const std::string_view &, std::size_t size )> callback ) override;
    std::vector<std::string_view> get_drive_names() const override;

private:
    std::string_view get_drive_name( const std::string_view & path ) const;
};

} // namespace SimpleFlashFs::Vfs