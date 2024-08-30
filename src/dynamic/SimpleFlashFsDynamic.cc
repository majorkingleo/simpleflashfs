/**
 * SimpleFlashFs main implementation class
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#include "SimpleFlashFsDynamic.h"
#include <CpputilsDebug.h>
#include <bit>
#include <cstring>
#include <format.h>
#include <string_utils.h>
#include <map>
#include <type_traits>
#include "../crc/crc.h"

using namespace Tools;

namespace SimpleFlashFs::dynamic {

uint32_t Config::crc32( const std::byte *bytes, size_t len )
{
	return crcFast( reinterpret_cast<unsigned const char*>(bytes), len );
}



SimpleFlashFs::SimpleFlashFs( FlashMemoryInterface *mem_interface_ )
: base::SimpleFlashFsBase<Config>(mem_interface_)
{
	crcInit();
}

bool SimpleFlashFs::create( const Header & h )
{
	if( h.page_size * h.filesystem_size > mem->size() ) {
		CPPDEBUG( "filesystem too large for memory" );
		return false;
	}

	if( h.page_size == 0 ||
		h.filesystem_size == 0 ) {
		CPPDEBUG( "invalid data" );
		return false;
	}

	if( h.page_size < MIN_PAGE_SIZE ) {
		CPPDEBUG( "invalid data" );
		return false;
	}

	if( h.filesystem_size < 4 ) {
		CPPDEBUG( "invalid data" );
		return false;
	}

	mem->erase(0, h.page_size * h.filesystem_size );

	if( !SimpleFlashFsBase<Config>::write( h ) ) {
		return false;
	}

	return init();
}



void SimpleFlashFs::read_all_free_data_pages()
{
	free_data_pages.clear();

	for( unsigned i = header.max_inodes; i < header.filesystem_size; i++ ) {
		free_data_pages.insert(i);
	}

	std::map<uint64_t,std::list<std::shared_ptr<FileHandle>>> inodes;

	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		std::vector<std::byte> page(header.page_size);

		if( read_page( i, page, true ) ) {
			/*
			CPPDEBUG( format( "inode page: %d data %x%x%x%x%x%x%x%x", i,
					static_cast<unsigned>(page[0]),
					static_cast<unsigned>(page[1]),
					static_cast<unsigned>(page[2]),
					static_cast<unsigned>(page[3]),
					static_cast<unsigned>(page[4]),
					static_cast<unsigned>(page[5]),
					static_cast<unsigned>(page[6]),
					static_cast<unsigned>(page[7]) ) );
					*/
			FileHandle inode = get_inode( page );
			inode.page = i;
			max_inode_number = std::max( max_inode_number, inode.inode.inode_number );
			CPPDEBUG( format( "found inode %d,%d at page: %d",
					inode.inode.inode_number, inode.inode.inode_version_number, i ) );

			auto dyn_inode = std::shared_ptr<FileHandle>(new FileHandle(std::move(inode)));
			inodes[inode.inode.inode_number].push_back(dyn_inode);
			//free_data_pages.insert(inode->inode.data_pages.begin(), inode->inode.data_pages.end());
		}
	}

	// clear all old inodes
	for( auto & pair : inodes ) {
		auto & list = pair.second;
		if( list.size() > 1 ) {
			list.sort([]( auto a, auto b ) {
				return a->inode.inode_version_number < b->inode.inode_version_number;
			});

			while( list.size() > 1 ) {
				auto inode_it = list.begin();
				erase_inode_and_unused_pages( *(*inode_it), *(*(++list.begin())) );
				list.erase( inode_it );
			}
		}
	}

	// remove used pages from free_data_pages list
	for( auto & pair : inodes ) {
		auto & list = pair.second;
		for( auto page : list.front()->inode.data_pages ) {
			free_data_pages.erase(page);
		}
	}

	CPPDEBUG( format( "free Data pages: %s", IterableToCommaSeparatedString(free_data_pages.get_sorted_data()) ) );
}



std::list<std::shared_ptr<::SimpleFlashFs::dynamic::SimpleFlashFs::FileHandle>> SimpleFlashFs::get_all_inodes()
{
	std::list<std::shared_ptr<FileHandle>> ret;

	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		std::vector<std::byte> page(header.page_size);

		if( read_page( i, page, true ) ) {

			auto inode = get_inode( page );
			inode.page = i;

			auto dyn_inode = std::shared_ptr<FileHandle>(new FileHandle(std::move(inode)));
			ret.push_back(dyn_inode);
		}
	}

	return ret;
}

} // namespace SimpleFlashFs::dynamic

