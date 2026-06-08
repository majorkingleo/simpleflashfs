#pragma once

#include <memory>
#include <SimpleFlashFsFileInterface.h>
#include <functional>

namespace SimpleFlashFs::Vfs
{
    using file_handle_t = std::unique_ptr<FileInterface>;

    class VfsDriveInterface
    {
    public:
        virtual ~VfsDriveInterface() = default;

        virtual file_handle_t open( const std::string_view & path, std::ios_base::openmode mode ) = 0;
        virtual bool list_files( std::function<bool(const std::string_view &, std::size_t size )> callback ) = 0;
        virtual std::string_view get_drive_name() const = 0;
    };

    class VfsServerInterface
    {
    public:
        virtual ~VfsServerInterface() = default;

        virtual void start() = 0;
        virtual void stop() = 0;

        virtual bool register_drive( std::shared_ptr<VfsDriveInterface> drive ) = 0;

        virtual file_handle_t open( const std::string_view & path, std::ios_base::openmode mode ) = 0;
        virtual bool list_files( std::function<bool(const std::string_view &, std::size_t size )> callback ) = 0;
        virtual std::vector<std::string_view> get_drive_names() const = 0;
    };

} // namespace SimpleFlashFs::Vfs