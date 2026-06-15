#pragma once

#include "../src/dynamic/SimpleFlashFsDynamic.h"
#include "SimpleFlashFsVfs.h"
#include <functional>
#include <random>

class FramFsImplDetail : public ::SimpleFlashFs::dynamic::SimpleFlashFs, public SimpleFlashFs::Vfs::VfsDriveInterface
{	
public:
	using base_t = ::SimpleFlashFs::dynamic::SimpleFlashFs;

protected:
	using header_inode_interface_t = ::SimpleFlashFs::base::HeaderInodeRangeInterface<::SimpleFlashFs::dynamic::SimpleFlashFs::config_t>;

	// wear leveling for header inodes
	class InodeHeaderRange : public header_inode_interface_t
	{
	private:
		std::vector<uint32_t> 			m_inode_numbers {};
		std::mt19937 & 					m_rng;
		std::vector<uint32_t>::iterator m_current_it;

	public:
		InodeHeaderRange( std::mt19937 & rng )
		: m_rng( rng )
		{		
		}

		void reinit( const header_t & header ) 
		{
			m_inode_numbers.clear();
			m_inode_numbers.resize( header.max_inodes );

			std::iota( m_inode_numbers.begin(), m_inode_numbers.end(), 0 );
		}

		void reset() override {

			// shuffle the inode numbers to implement wear leveling
			std::shuffle( m_inode_numbers.begin(), m_inode_numbers.end(), m_rng );
			m_current_it = m_inode_numbers.begin();
		}

		bool has_next() const override {
			return m_current_it != m_inode_numbers.end();
		}

		uint32_t next() override {
			return *m_current_it++;
		}

		uint32_t start() override {
			reset();
			return next();
		}
	};


public:	

	struct Stat
	{
		std::size_t largest_file_size = 0; // in bytes
		std::size_t trash_size = 0; // in bytes
		std::size_t used_inodes = 0; // number of inodes in use
		std::size_t trash_inodes = 0; // number of inodes to delete
		std::size_t free_inodes = 0;
	};

protected:
	Stat 					m_stat{};
	std::string 			m_drive_name;
	bool 					m_initialized = false;
	std::random_device 		m_dev;
    std::mt19937			m_rng;
	InodeHeaderRange		m_header_inode_range{m_rng};

public:
	FramFsImplDetail( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_, const std::string_view & drive_name_ )
	: SimpleFlashFs( mem_interface_ ), 
	  m_drive_name( drive_name_ ),
	  m_rng( m_dev() )
	{
		header_inode_range = &m_header_inode_range;
	}

	const Stat & get_stat() const {
		return m_stat;
	}

	// read the fs the memory interface points to
	// starting at offset 0
	bool init() override;

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

	// wear leveling for data pages
	std::optional<uint32_t> allocate_free_data_page() override;
};
