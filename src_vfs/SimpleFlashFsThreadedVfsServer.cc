#include "SimpleFlashFsThreadedVfsServer.h"
#include <CpputilsDebug.h>
#include <format.h>

using namespace SimpleFlashFs::Vfs;

namespace {

class ThreadedFileHandle : public SimpleFlashFs::FileInterface
{
private:
    file_handle_t       m_handle;
    mutable std::mutex  m_mutex;

public:
    ThreadedFileHandle( file_handle_t handle )
    : m_handle( std::move(handle) )
    {}

    bool operator!() const override {
        return !m_handle;
    }

    std::size_t write( const std::byte *data, std::size_t size ) override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->write( data, size );
    }

    std::size_t read( std::byte *data, std::size_t size ) override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->read( data, size );
    }

    bool flush() override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->flush();
    }

    std::size_t tellg() const override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->tellg();
    }

    std::size_t file_size() const override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->file_size();
    }

    bool eof() const override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->eof();
    }

    bool seek( std::size_t pos_ ) override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->seek( pos_ );
    }

    bool delete_file() override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->delete_file();
    }

    bool rename_file( const std::string_view & new_file_name ) override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->rename_file( new_file_name );
    }

    std::string_view get_file_name() const override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->get_file_name();
    }

    bool is_append_mode() const override {
        auto lock = std::scoped_lock(m_mutex);
        return m_handle->is_append_mode();
    }
};

} // namespace

SimpleFlashFsThreadedVfsServer::SimpleFlashFsThreadedVfsServer()
: SimpleFlashFsThreadedVfsServer::VfsServerInterface()
{
}

SimpleFlashFsThreadedVfsServer::~SimpleFlashFsThreadedVfsServer()
{
    stop();
}

void SimpleFlashFsThreadedVfsServer::start()
{
    m_running = true;
    m_server_thread = std::thread( [this] () {
       
        auto cleanup_deadline = std::chrono::steady_clock::now() + m_cleanup_interval.load();

        while( m_running ) {
            std::this_thread::sleep_for( std::chrono::milliseconds(100) );

            if( (m_cleanup_interval.load() > std::chrono::seconds(0)) && (std::chrono::steady_clock::now() >= cleanup_deadline ) ) {
                auto lock = std::scoped_lock(m_mutex);
                for( auto & drive : m_drives ) {
                    drive->cleanup();
                }
                cleanup_deadline = std::chrono::steady_clock::now() + m_cleanup_interval.load();
            }
        }
    } ); 
}

void SimpleFlashFsThreadedVfsServer::stop()
{    
    m_running = false;
    std::this_thread::yield();
    if( m_server_thread.joinable() ) {
        m_server_thread.join();        
    }
}

bool SimpleFlashFsThreadedVfsServer::register_drive( std::shared_ptr<VfsDriveInterface> drive )
{
    auto lock = std::scoped_lock(m_mutex);
    m_drives.push_back( drive );
    return true;
}

file_handle_t SimpleFlashFsThreadedVfsServer::open( const std::string_view & path, std::ios_base::openmode mode )
{    
    std::string_view file_path = path;
    const auto drive_name = parse_drive_name( file_path );

    // CPPDEBUG( Tools::format( "open file: %s at drive: %s", file_path, drive_name ) );

    auto lock = std::scoped_lock(m_mutex);

    bool drive_found = false;

    for( auto & drive : m_drives ) {
        if( drive->get_drive_name() != drive_name ) {
            continue;
        }

        drive_found = true;

        if( !drive->initialized() ) {
            if( !drive->init() ) {
                return {};
            }
        }

        auto file = drive->open( file_path, mode );
        if( file ) {
            return std::make_unique<ThreadedFileHandle>( std::move(file) );
        }
    }

    if( !drive_found ) {
        CPPDEBUG( Tools::format( "drive not found: %s", drive_name ) );
    } else {
        CPPDEBUG( Tools::format( "file not found: %s at drive: %s", file_path, drive_name ) );
    }

    return {};
}

std::string_view SimpleFlashFsThreadedVfsServer::get_drive_name( const std::string_view & path ) const
{
    std::string_view drive_name = path;

    if( drive_name.starts_with('/') ) {
        drive_name.remove_prefix(1);
    }

    const auto pos = drive_name.find( '/' );

    if( pos != std::string_view::npos ) {
        return drive_name.substr( 0, pos );
    }
    return drive_name;
}

std::string_view SimpleFlashFsThreadedVfsServer::parse_drive_name( std::string_view & path ) const
{
    std::string_view            drive_name = path;
    std::string_view::size_type file_name_start_pos = 0;

    if( drive_name.starts_with('/') ) {
        drive_name.remove_prefix(1);
        file_name_start_pos++;
    }

    const auto pos = drive_name.find( '/' );

    if( pos != std::string_view::npos ) {
        path.remove_prefix(pos + file_name_start_pos);

        if( path.starts_with('/') ) {
            path.remove_prefix(1);
        }

        return drive_name.substr( 0, pos );
    }
    return drive_name;
}

std::vector<std::string_view> SimpleFlashFsThreadedVfsServer::get_drive_names() const
{
    auto lock = std::scoped_lock(m_mutex);
    std::vector<std::string_view> drive_names;
    for( auto & drive : m_drives ) {
        drive_names.push_back( drive->get_drive_name() );
    }
    return drive_names;
}

bool SimpleFlashFsThreadedVfsServer::list_files( list_files_callback_t callback, const std::string_view & drive_name )
{
    auto lock = std::scoped_lock(m_mutex);
    for( auto & drive : m_drives ) {

        if( !drive_name.empty() && drive->get_drive_name() != drive_name ) {
            continue;
        }

        if( !drive->initialized() ) {
            if( !drive->init() ) {
                continue;
            }
        }

        if( !drive->list_files( callback ) ) {
            return false;
        }
    }
    return true;
}

void SimpleFlashFsThreadedVfsServer::create( const std::string_view & drive_name )
{
    auto lock = std::scoped_lock(m_mutex);
    for( auto & drive : m_drives ) {
        if( drive->get_drive_name() == drive_name ) {
            drive->create();
            return;
        }
    }
}

std::string_view SimpleFlashFsThreadedVfsServer::get_current_drive() const
{
    auto lock = std::scoped_lock(m_mutex);
    return m_current_drive;
}

bool SimpleFlashFsThreadedVfsServer::set_current_drive( const std::string_view & drive_name )
{
    auto lock = std::scoped_lock(m_mutex);
    
    // Check if the drive exists
    for( auto & drive : m_drives ) {
        if( drive->get_drive_name() == drive_name ) {
            m_current_drive = std::string(drive_name);

            if( !drive->initialized() ) {
                if( !drive->init() ) {
                    return false;
                }
            }

            return true;
        }
    }
    
    return false;
}

void SimpleFlashFsThreadedVfsServer::cleanup()
{
    auto lock = std::scoped_lock(m_mutex);
    for( auto & drive : m_drives ) {
        drive->cleanup();
    }
}