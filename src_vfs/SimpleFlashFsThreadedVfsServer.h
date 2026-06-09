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
    mutable std::mutex                              m_mutex;
    std::atomic<bool>                               m_running {false};
    std::list<std::shared_ptr<VfsDriveInterface>>   m_drives {};
    std::string                                     m_current_drive {};
    std::atomic<std::chrono::seconds>               m_cleanup_interval {std::chrono::seconds(0)}; // 0 = disabled

public:
    SimpleFlashFsThreadedVfsServer();
    ~SimpleFlashFsThreadedVfsServer();

    void start() override;
    void stop() override;

    bool register_drive( std::shared_ptr<VfsDriveInterface> drive ) override;

    file_handle_t open( const std::string_view & path, std::ios_base::openmode mode ) override;
    bool list_files( list_files_callback_t callback, const std::string_view & drive_name ) override;
    std::vector<std::string_view> get_drive_names() const override;

    void create( const std::string_view & drive_name ) override;

    std::string_view get_current_drive() const override;
    bool set_current_drive( const std::string_view & drive_name ) override;

    void cleanup() override;

    void set_cleanup_interval(std::chrono::seconds interval) {
        m_cleanup_interval = interval;
    }

private:
    std::string_view get_drive_name( const std::string_view & path ) const;
    std::string_view parse_drive_name( std::string_view & path ) const;
};

} // namespace SimpleFlashFs::Vfs