/*
 * SimpleFlashFsNoDel.h
 *
 *  Created on: 29.08.2024
 *      Author: martin.oberzalek
 */

#pragma once

#include "../src/static/SimpleFlashFsStatic.h"
#include "../src/crc/crc.h"
#include <CpputilsDebug.h>

namespace SimpleFlashFs::static_memory {

template<class Config>
class SimpleFsNoDel : public SimpleFlashFs<Config>
{
public:
	using base_t = SimpleFlashFs<Config>;

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
	Stat stat{};

public:
	SimpleFsNoDel( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
	: base_t(mem_interface_)
	{
		crcInit();
	}

	bool create()
	{
		typename base_t::header_t header = base_t::create_default_header( Config::PAGE_SIZE, Config::MAX_SIZE / Config::PAGE_SIZE );
		header.max_path_len = Config::FILE_NAME_MAX;

		return base_t::create(header);
	}

	// read the fs the memory interface points to
	// starting at offset 0
	bool init() override {
		if( !base_t::init() ) {
			return false;
		}

		read_all_free_data_pages();
		return true;
	}

	const Stat & get_stat() const {
		return stat;
	}

protected:
	void read_all_free_data_pages();

	virtual void erase_inode_and_unused_pages( base_t::file_handle_t & inode_to_erase, base_t::file_handle_t & next_inode_version ) override {
		// do nothing
	}

	typename base_t::FileHandle read_inode( std::size_t index )
	{
		if( this->mem->can_map_read() ) {
			auto page = this->read_page_mapped( index, this->header.page_size, true );
			if( !page ) {
				return {};
			}
			return this->get_inode( *page );
		}

		typename base_t::config_t::page_type page( this->header.page_size );
		if( !base_t::read_page( index, page, true ) ) {
			return {};
		}
		return this->get_inode( page );
	}
};


template <class Config>
void SimpleFsNoDel<Config>::read_all_free_data_pages()
{
	base_t::free_data_pages.clear();
	stat = {};

	for( unsigned i = base_t::header.max_inodes; i < base_t::header.filesystem_size; i++ ) {
		base_t::free_data_pages.unordered_insert(i);
	}

	this->iv_storage.clear();

	for( unsigned i = 0; i < base_t::header.max_inodes; i++ ) {

		typename base_t::FileHandle inode = read_inode( i );

		if( !inode ) {
			continue;
		}

		inode.page = i;
		base_t::max_inode_number = std::max( base_t::max_inode_number, inode.inode.inode_number );

/*
		CPPDEBUG( Tools::format( "found inode %d,%d at page: %d name: '%s'",
				inode.inode.inode_number, inode.inode.inode_version_number, i, inode.inode.file_name ) );
*/

		if( this->iv_storage.add( inode ) == base_t::InodeVersionStore::add_ret_t::replaced ) {
			stat.trash_inodes++;
		} else {
			stat.used_inodes++;
		}

		stat.largest_file_size = std::max( stat.largest_file_size, inode.file_size() );

		// remove used pages from free_data_pages list
		for( auto page : inode.inode.data_pages ) {
			stat.trash_size += base_t::header.page_size;
			base_t::free_data_pages.erase(page);
		}
	}

	// CPPDEBUG( Tools::format( "max_inodes: %d", base_t::header.max_inodes ) );

	stat.free_inodes = base_t::header.max_inodes - stat.used_inodes - stat.trash_inodes;

//	CPPDEBUG( Tools::format( "free Data pages: %s", Tools::IterableToCommaSeparatedString(base_t::free_data_pages) ) );
/*
	CPPDEBUG( Tools::format( "free Data pages: %d", base_t::free_data_pages.size() ) );
	CPPDEBUG( Tools::format( "largest file size: %dB", stat.largest_file_size ) );
	CPPDEBUG( Tools::format( "trash size size: %dB", stat.largest_file_size ) );
	CPPDEBUG( Tools::format( "used inodes: %d trash inodes: %d free inodes: %d",
			stat.used_inodes,
			stat.trash_inodes,
			stat.free_inodes ));
*/
}

} // namespace SimpleFlashFs::static_memory



