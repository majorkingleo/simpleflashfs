#pragma once

#include <SimpleFlashFsDynamic.h>
#include "SimpleFlashFsVfs.h"
#include <functional>

class FramFsImplDetail : public ::SimpleFlashFs::dynamic::SimpleFlashFs, public SimpleFlashFs::Vfs::VfsDriveInterface
{
public:
	using base_t = ::SimpleFlashFs::dynamic::SimpleFlashFs;

	struct Stat
	{
		std::size_t largest_file_size = 0; // in bytes
		std::size_t trash_size = 0; // in bytes
		std::size_t used_inodes = 0; // number of inodes in use
		std::size_t trash_inodes = 0; // number of inodes to delete
		std::size_t free_inodes = 0;
	};

protected:
	Stat 		m_stat{};
	std::string m_drive_name;
	bool 		m_initialized = false;

public:
	FramFsImplDetail( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_, const std::string_view & drive_name_ )
	: SimpleFlashFs( mem_interface_ ), m_drive_name( drive_name_ )
	{}

	const Stat & get_stat() const {
		return m_stat;
	}

	// read the fs the memory interface points to
	// starting at offset 0
	bool init() override {
		if( !base_t::init() ) {
			m_initialized = false;
			return false;
		}
		m_initialized = true;
		return true;
	}

	bool initialized() const override {
		return m_initialized;
	}

	// override VfsDriveInterface::open
	::SimpleFlashFs::Vfs::file_handle_t open( const std::string_view & path, std::ios_base::openmode mode ) override;

	bool list_files( std::function<bool(const std::string_view &, std::size_t size )> callback ) override;

	std::string_view get_drive_name() const override {
		return m_drive_name;
	}

	void cleanup() override;

private:
	// bring base_t::open into scope (hides the VfsDriveInterface version for direct calls)
	using base_t::open;
	using base_t::init;

protected:

};
