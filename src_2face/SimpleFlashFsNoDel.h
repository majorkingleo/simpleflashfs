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
	bool do_debug = false;

public:
	SimpleFsNoDel( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_, bool do_debug_ = false )
	: base_t(mem_interface_),
	  do_debug( do_debug_ )
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

	/**
	 * reads inode, does no error correction, don't use the resulting
	 * file handle for reading, or writing.
	 *
	 * returns:
	 *    empty optional:      read error occurred
	 *    empty file handle:   crc error occurred, free page
	 *    a valid file handle: inode is used by a file
	 */
	std::optional<typename base_t::FileHandle> read_inode( std::size_t index )
	{
		if( this->mem->can_map_read() ) {
			typename base_t::ReadPageMappedReturn ret = this->read_page_mapped( index, this->header.page_size, true );
			if( !ret ) {
				if( ret.error == base_t::ReadError::ReadError ) {
					if( do_debug ) {
						CPPDEBUG("read_error");
					}
					return {}; // empty optional
				}

				if( ret.error == base_t::ReadError::CrcError ) {
					if( ret.data ) {
						if( is_empty( ret.data->begin(), ret.data->end() ) ) {
							return typename base_t::FileHandle{};
						} else {
							 auto inode = this->get_inode( *ret.data, false );

							 if( do_debug ) {
								CPPDEBUG( Tools::static_format<100>( "not empty: found inode %d,%d at page: %d name: '%s'",
										inode.inode.inode_number, inode.inode.inode_version_number, index, inode.inode.file_name ) );
/*
								uint32_t c_page = this->get_page_checksum( ret.data->data(), this->header.page_size );
								uint32_t c_calc = this->calc_page_checksum(ret.data->data(), this->header.page_size );

								CPPDEBUG( Tools::static_format<100>( "chksum: 0x%X calculated chksum: 0x%X %s",
										c_page, c_calc, c_page == c_calc ? "no diff" : "crcsum error " ) );

								CPPDEBUG( Tools::static_format<100>( "page size: %d start_address: 0x%X crcsum at: 0x%X",
										this->header.page_size,
										(uintptr_t)ret.data->data(),
										(uintptr_t)(ret.data->data() + this->header.page_size - sizeof(uint32_t) )) );
*/
							 }
						}
					}
				}

				return {}; // empty optional
			}
			return this->get_inode( *ret.data, false );
		}

		typename base_t::config_t::page_type page( this->header.page_size );

		typename base_t::ReadPageReturn ret = base_t::read_page( index, page, true );
		if( !ret ) {
			if( ret.error == base_t::ReadError::ReadError ) {
				if( do_debug ) {
					CPPDEBUG("read_error");
				}
				return {};
			}

			if( ret.error == base_t::ReadError::CrcError ) {
				if( is_empty( page.begin(), page.end() ) ) {
					return typename base_t::FileHandle{};
				}
			}

			return {};
		}
		return this->get_inode( page, false );
	}

	bool is_empty( auto it_begin, auto it_end ) const {
		const auto it_ret = std::find_if_not( it_begin, it_end, []( auto b ) { return b == std::byte(0xFF); } );

		if( it_ret == it_end ) {
			return true;
		}

		return false;
	}
};


template <class Config>
void SimpleFsNoDel<Config>::read_all_free_data_pages()
{
	base_t::free_data_pages.clear();
	stat = {};

	for( unsigned i = base_t::header.max_inodes; 
		i < base_t::header.filesystem_size - 1; // -1 ... one page for the header itself
		i++ ) {

		base_t::free_data_pages.unordered_insert(i);
	}

	this->iv_storage.clear();

	for( unsigned i = 0; i < base_t::header.max_inodes; i++ ) {

		auto ret = read_inode( i );

		if( !ret ) {
			// empty optional, read error, remove from free data pages list
			if( do_debug ) {
				CPPDEBUG( Tools::static_format<100>( "read error at page: %d", i ) );
			}
			base_t::free_data_pages.erase(i);
			continue;
		}

		typename base_t::FileHandle & inode = *ret;

		if( !inode ) {
			if( do_debug && i < 10) {
				CPPDEBUG( Tools::static_format<100>( "no inode at page: %d", i ) );
			}
			// free data page, do nothing it is already in the free_data_pages list
			continue;
		}

		inode.page = i;
		base_t::max_inode_number = std::max( base_t::max_inode_number, inode.inode.inode_number );

		if( do_debug ) {
			CPPDEBUG( Tools::static_format<100>( "found inode %d,%d at page: %d name: '%s'",
					inode.inode.inode_number, inode.inode.inode_version_number, i, inode.inode.file_name ) );
		}


		if( this->iv_storage.add( inode ) == base_t::InodeVersionStore::add_ret_t::replaced ) {
			stat.trash_inodes++;
		} else {
			stat.used_inodes++;
		}

		stat.largest_file_size = std::max( stat.largest_file_size, inode.file_size() );

		// remove used pages from free_data_pages list
		for( auto page : inode.inode.data_pages ) {
			stat.trash_size += base_t::header.page_size;
			base_t::free_data_pages.erase(page.page_id);
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



