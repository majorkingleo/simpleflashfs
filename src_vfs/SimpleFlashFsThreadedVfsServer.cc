#include "SimpleFlashFsThreadedVfsServer.h"

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
        while( m_running ) {
            std::this_thread::sleep_for( std::chrono::milliseconds(100) );
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
    const auto drive_name = get_drive_name( path );
    const auto file_path = path.substr( drive_name.size() );

    auto lock = std::scoped_lock(m_mutex);

    for( auto & drive : m_drives ) {
        if( drive->get_drive_name() != drive_name ) {
            continue;
        }

        auto file = drive->open( file_path, mode );
        if( file ) {
            return std::make_unique<ThreadedFileHandle>( std::move(file) );
        }
    }

    return {};
}

std::string_view SimpleFlashFsThreadedVfsServer::get_drive_name( const std::string_view & path ) const
{
    const auto pos = path.find( '/' );

    if( pos != std::string_view::npos ) {
        return path.substr( 0, pos );
    }
    return path;
}