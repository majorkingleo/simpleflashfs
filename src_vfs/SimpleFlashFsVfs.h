#pragma once

#include <memory>
#include "../src/SimpleFlashFsFileInterface.h"
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
        virtual void create() = 0;
        virtual bool init() = 0;
        virtual bool initialized() const = 0;
        virtual void cleanup() = 0;
    };

    class VfsServerInterface
    {
    public:
        using list_files_callback_t = std::function<bool(const std::string_view &, std::size_t size )>;

    public:
        virtual ~VfsServerInterface() = default;

        virtual bool register_drive( std::shared_ptr<VfsDriveInterface> drive ) = 0;

        virtual file_handle_t open( const std::string_view & path, std::ios_base::openmode mode ) = 0;
        virtual bool list_files( list_files_callback_t callback, const std::string_view & drive_name = {} ) = 0;
        virtual std::vector<std::string_view> get_drive_names() const = 0;

        virtual void create( const std::string_view & drive_name ) = 0;

        /**
         * @brief Get the current working drive
         * @return the name of the current drive
         */
        virtual std::string get_current_drive() const = 0;

        /**
         * @brief Set the current working drive
         * @param drive_name the name of the drive to set as current
         * @return true if successful, false if drive not found
         */
        virtual bool set_current_drive( const std::string_view & drive_name ) = 0;

        virtual void cleanup() = 0;
    };

} // namespace SimpleFlashFs::Vfs