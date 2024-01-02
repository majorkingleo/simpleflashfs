/**
 * SimpleFlashFs main implementation class
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#include "SimpleFlashFsDynamic.h"
#include "../SimpleFlashFsFlashMemoryInterface.h"
#include "../SimpleFlashFsConstants.h"
#include <algorithm>
#include <CpputilsDebug.h>
#include <bit>
#include <cstring>
#include "../crc/crc.h"
#include <format.h>
#include <string_utils.h>
#include <map>

using namespace Tools;

namespace SimpleFlashFs::dynamic {

FileHandle::~FileHandle()
{
	if( fs ) {
		flush();

		fs->free_unwritten_pages( page );
		for( auto page : inode.data_pages ) {
			fs->free_unwritten_pages(page);
		}
	}
}

std::size_t FileHandle::write( const std::byte *data, std::size_t size ) {
	return fs->write( this, data, size );
}

std::size_t FileHandle::read( std::byte *data, std::size_t size ) {
	return fs->read( this, data, size );
}

bool FileHandle::flush() {
	return fs->flush(this);
}

SimpleFlashFs::SimpleFlashFs( FlashMemoryInterface *mem_interface_ )
: mem(mem_interface_)
{
	crcInit();
}

Header SimpleFlashFs::create_default_header( uint32_t page_size, uint64_t filesystem_size )
{
	Header header;

	header.magic_string = MAGICK_STRING;
	header.version = 1;
	header.page_size = page_size;
	header.filesystem_size = filesystem_size;

	header.max_inodes = filesystem_size / 10 * 2;
	header.max_inodes = std::max( header.max_inodes, static_cast<uint32_t>(10) );

	if( header.max_inodes > header.filesystem_size ) {
		header.max_inodes = 2;
	}

	header.max_path_len = 50;

	if( std::endian::native == std::endian::big ) {
		header.endianess = Header::ENDIANESS::BE;
	} else {
		header.endianess = Header::ENDIANESS::LE;
	}

	return header;
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

	mem->erase(0, h.page_size * h.filesystem_size );

	return write( h );
}


bool SimpleFlashFs::write( const Header & header_ )
{
	header = header_;
	Header h = header_;

	std::vector<std::byte> page(header.page_size);

	std::size_t pos = 0;

	std::memcpy( &page[pos], header.magic_string.c_str(), header.magic_string.size() );
	pos += MAGICK_STRING_LEN;

	if( header.endianess == Header::ENDIANESS::LE ) {
		std::memcpy( &page[pos], ENDIANESS_LE, 2 );
	} else {
		std::memcpy( &page[pos], ENDIANESS_BE, 2 );
	}

	pos += 2;

	auto add=[this,&pos,&page]( auto & t ) {
		auto_endianess(t);
		std::memcpy( &page[pos], &t, sizeof(t) );
		pos += sizeof(t);
	};

	add(h.version);
	add(h.page_size);
	add(h.filesystem_size);
	add(h.max_inodes);
	add(h.max_path_len);
	uint16_t chktype = static_cast<uint16_t>(h.crc_checksum_type);
	add(chktype);


	add_page_checksum( page );

	mem->write( 0, page.data(), page.size() );

	return true;
}

bool SimpleFlashFs::swap_endianess()
{
	if( std::endian::native == std::endian::big && header.endianess == Header::ENDIANESS::LE ) {
		return true;
	}
	else if( std::endian::native == std::endian::little && header.endianess == Header::ENDIANESS::BE ) {
		return true;
	}

	return false;
}

std::size_t SimpleFlashFs::get_num_of_checksum_bytes() const
{
	switch( header.crc_checksum_type )
	{
	case Header::CRC_CHECKSUM::CRC32: return sizeof(uint32_t);

	default:
		throw std::out_of_range("unknown checksum type");
	}
}

void SimpleFlashFs::add_page_checksum( std::vector<std::byte> & page )
{
	switch( header.crc_checksum_type )
	{
	case Header::CRC_CHECKSUM::CRC32:
		{
			uint32_t chksum = crcFast( reinterpret_cast<unsigned char*>(&page[0]), page.size() - sizeof(uint32_t));
			auto_endianess(chksum);
			memcpy( &page[page.size() - sizeof(uint32_t)], &chksum, sizeof(chksum));
			return;
		}
	default:
		throw std::out_of_range("unknown checksum type");
	}
}

uint32_t SimpleFlashFs::calc_page_checksum( const std::byte *page, std::size_t size )
{
	switch( header.crc_checksum_type )
	{
	case Header::CRC_CHECKSUM::CRC32:
		return crcFast( reinterpret_cast<const unsigned char*>(page), size - sizeof(uint32_t));
	default:
		throw std::out_of_range("unknown checksum type");
	}
}

uint32_t SimpleFlashFs::get_page_checksum( const std::byte *page, std::size_t size )
{
	switch( header.crc_checksum_type )
	{
	case Header::CRC_CHECKSUM::CRC32:
		{
			uint32_t chksum = 0;
			memcpy( &chksum, page + (size - sizeof(uint32_t)), sizeof(uint32_t) );
			auto_endianess(chksum);
			return chksum;
		}
	default:
		throw std::out_of_range("unknown checksum type");
	}
}


uint32_t SimpleFlashFs::get_checksum_size() const
{
	switch( header.crc_checksum_type )
	{
	case Header::CRC_CHECKSUM::CRC32:
		{
			return sizeof(uint32_t);
		}
	default:
		throw std::out_of_range("unknown checksum type");
	}
}


bool SimpleFlashFs::init()
{
	Header h{};
	std::vector<std::byte> page(MIN_PAGE_SIZE);

	mem->read(0, page.data(), page.size());

	std::size_t pos = 0;
	h.magic_string = std::string_view( reinterpret_cast<char*>(&page[pos]), MAGICK_STRING_LEN );

	if( h.magic_string != MAGICK_STRING ) {
		CPPDEBUG( format( "invalid magick string: '%s'", h.magic_string ));
		return false;
	}

	pos += MAGICK_STRING_LEN;

	std::string_view endianess( reinterpret_cast<char*>(&page[pos]), ENDIANESS_LEN );

	if( endianess == ENDIANESS_BE ) {
		h.endianess = Header::ENDIANESS::BE;
	} else if( endianess == ENDIANESS_LE ) {
		h.endianess = Header::ENDIANESS::LE;
	} else {
		CPPDEBUG( format( "invalid endianess: '%s'", endianess ));
		return false;
	}

	// auto_endianess is looking at header.endianess
	header = h;

	pos += ENDIANESS_LEN;

	auto read=[this,&pos,&page]( auto & t ) {
		std::memcpy(&t, &page[pos], sizeof(t) );
		auto_endianess(t);
		pos += sizeof(t);
	};

	read(h.version);
	read(h.page_size);
	read(h.filesystem_size);
	read(h.max_inodes);
	read(h.max_path_len);
	uint16_t chktype = 0;
	read(chktype);

	switch( chktype ) {
	case static_cast<uint16_t>(Header::CRC_CHECKSUM::CRC32):
		h.crc_checksum_type = Header::CRC_CHECKSUM::CRC32;
		break;
	default:
		CPPDEBUG( format( "invalid chksum type: '%d'", chktype ));
		return false;
	}

	if( h.page_size < MIN_PAGE_SIZE ) {
		CPPDEBUG( format( "invalid page size '%d'", h.page_size ));
		return false;
	}

	// now reread the whole page
	page.resize(h.page_size);
	mem->read(0, page.data(), page.size());

	uint32_t chksum_calc = calc_page_checksum( page );
	uint32_t chksum_page = get_page_checksum( page );

	if( chksum_calc != chksum_page ) {
		CPPDEBUG( format( "chksum_calc: %d != chksum_page: %d", chksum_calc, chksum_page ));
		return false;
	}

	header = h;

	read_all_free_data_pages();

	return true;
 }

std::shared_ptr<FileHandle> SimpleFlashFs::open( const std::string & name, std::ios_base::openmode mode )
{
	auto handle = find_file( name );

	if( !handle ) {
		// file does not exists
		if( !(mode & std::ios_base::out) ) {
			return {};
		}

		handle = allocate_free_inode_page();

		if( !handle ) {
			CPPDEBUG( "cannot allocate new inode page" );
			return {};
		}

		handle->inode.file_name = name;
		handle->inode.file_name_len = name.size();
		handle->modified = true;
		return handle;
	}
	else if( mode & std::ios_base::trunc ) {

		auto new_handle = allocate_free_inode_page();
		new_handle->inode = handle->inode;
		new_handle->inode.file_len = 0;
		new_handle->inode.pages = 0;
		new_handle->inode.data_pages.clear();
		new_handle->inode.inode_data.clear();

		return new_handle;
	}

	return handle;
}

std::shared_ptr<FileHandle> SimpleFlashFs::find_file( const std::string & name )
{
	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		std::vector<std::byte> page(header.page_size);

		if( read_page( i, page, true ) ) {
			auto inode = get_inode( page );
			if( inode->inode.file_name == name ) {
				inode->page = i;
				return inode;
			}
		}
	}

	return {};
}

bool SimpleFlashFs::read_page( std::size_t idx, std::byte *page, std::size_t size, bool check_crc )
{
	std::size_t offset = header.page_size + idx * header.page_size;

	if( mem->read(offset, page, size ) != size ) {
		CPPDEBUG( "cannot read all data" );
		return false;
	}

	if( check_crc ) {
		if( get_page_checksum( page, size ) != calc_page_checksum(page, size) ) {
			//CPPDEBUG( "checksum failed" );
			return false;
		}
	}

	return true;
}

std::shared_ptr<FileHandle> SimpleFlashFs::get_inode( const std::vector<std::byte> & page )
{
	std::shared_ptr<FileHandle> ret = std::make_shared<FileHandle>(this);

	std::size_t pos = 0;

	auto read=[this,&pos,&page]( auto & t ) {
		std::memcpy(&t, &page[pos], sizeof(t) );
		auto_endianess(t);
		pos += sizeof(t);
	};

	read( ret->inode.inode_number );
	read( ret->inode.inode_version_number );
	read( ret->inode.file_name_len );

	std::vector<char> file_name(ret->inode.file_name_len+1);
	std::memcpy(&file_name[0], &page[pos], ret->inode.file_name_len );
	pos += ret->inode.file_name_len;
	ret->inode.file_name = std::string_view( &file_name[0], ret->inode.file_name_len );

	read( ret->inode.attributes );
	read( ret->inode.file_len );
	read( ret->inode.pages );

	if( ret->inode.pages  ) {
		ret->inode.data_pages.resize( ret->inode.pages, 0 );

		for( unsigned i = 0; i < ret->inode.pages; i++ ) {
			read( ret->inode.data_pages[i] );
		}
	}
	/**
	 * read the data, that is directly stored inside the inode
	 */
	else if( ret->inode.pages == 0 && ret->inode.file_len > 0 ) {
		FileHandle *handle = &(*ret);
		ret->inode.inode_data.resize(get_inode_data_space_size(handle));
		memcpy( ret->inode.inode_data.data(), page.data() + pos, ret->inode.file_len );
	}

	return ret;
}

std::shared_ptr<FileHandle> SimpleFlashFs::allocate_free_inode_page()
{
	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		std::vector<std::byte> page(header.page_size);

		if( !read_page( i, page, true ) ) {
			if( allocated_unwritten_pages.count(i) == 0 ) {
				auto ret = std::make_shared<FileHandle>(this);
				ret->page = i;
				allocated_unwritten_pages.insert(i);
				return ret;
			}
		}
	}

	return {};
}

uint32_t SimpleFlashFs::allocate_free_inode_page_number()
{
	std::vector<std::byte> page;
	page.reserve(header.page_size);

	for( unsigned i = 0; i < header.max_inodes; i++ ) {
		page.clear();
		page.resize(header.page_size);

		if( !read_page( i, page, true ) ) {
			if( allocated_unwritten_pages.count(i) == 0 ) {
				allocated_unwritten_pages.insert(i);
				return i;
			}
		}
	}

	return {};
}

uint32_t SimpleFlashFs::allocate_free_data_page()
{
	if( free_data_pages.empty() ) {
		return 0;
	}

	auto it = free_data_pages.begin();
	auto ret = *it;
	free_data_pages.erase(it);

	return ret;
}

std::size_t SimpleFlashFs::write( FileHandle* file, const std::byte *data, std::size_t size )
{
	std::size_t page_idx = file->pos / header.page_size;
	std::size_t bytes_written = 0;

	/**
	 * store the data inside the inode itself, if there is enough space
	 */
	std::size_t space_inside_the_inode = get_inode_data_space_size(file);
	if( space_inside_the_inode > file->pos + size &&
		file->inode.file_len < space_inside_the_inode ) {

		if( file->inode.inode_data.size() < space_inside_the_inode ) {
			file->inode.inode_data.resize(space_inside_the_inode);
		}

		memcpy( file->inode.inode_data.data() + file->pos, data, size );
		file->pos += size;
		file->inode.file_len = std::max( file->pos, file->inode.file_len);
		file->modified = true;

		// the read write will occur during flush
		return size;

	} else if( file->inode.inode_data.size() ) {
		file->inode.inode_data.clear();
		file->modified = true;
	}

	bool target_page_is_a_new_allocated_one = false;

	// allocate new pages
	while( page_idx > file->inode.data_pages.size() ) {
		uint32_t new_page_idx = allocate_free_data_page();

		if( new_page_idx == 0 ) {
			CPPDEBUG( "no space left on device" );
			return 0;
		}

		file->inode.data_pages.push_back( new_page_idx );
		file->pos += header.page_size;
		page_idx = file->pos / header.page_size;
		target_page_is_a_new_allocated_one = true;
	}

	// unaligned data, map the buffer to a complete page
	const std::size_t data_start_at_page = file->pos % header.page_size;

	if( data_start_at_page != 0 ) {
		std::vector<std::byte> page(header.page_size);
		const std::size_t page_number = file->inode.data_pages.at(page_idx);
		if( !read_page( page_number, page, false ) ) {
			CPPDEBUG( format( "reading from pos %d failed", page_number * header.page_size ) );
			return 0;
		}

		const std::size_t len = std::min( size, header.page_size - data_start_at_page );
		memcpy( &page[data_start_at_page], data, len );

		if( !write_page( file, page, target_page_is_a_new_allocated_one, file->inode.data_pages.at(page_idx) ) ) {
			CPPDEBUG( "no space left on device" );
			return 0;
		}

		bytes_written += len;
		file->pos += len;
	}

	while( bytes_written < size ) {
		page_idx = file->pos / header.page_size;
		target_page_is_a_new_allocated_one = false;

		if( page_idx >= file->inode.data_pages.size() ) {
			uint32_t new_page_number = allocate_free_data_page();

			if( new_page_number == 0 ) {
				CPPDEBUG( "no space left on device" );
				return 0;
			}

			file->inode.data_pages.push_back( new_page_number );
			target_page_is_a_new_allocated_one = true;
		}

		// last partial page
		if( bytes_written + header.page_size > size ) {

			std::vector<std::byte> page(header.page_size);

			if( !target_page_is_a_new_allocated_one ) {
				const std::size_t page_number = file->inode.data_pages.at(page_idx);
				if( !read_page( page_number, page, false ) ) {
					CPPDEBUG( format( "reading from pos %d failed", page_number * header.page_size ) );
					return 0;
				}
			}

			const std::size_t len = std::min( static_cast<uint32_t>(size - bytes_written), header.page_size );
			memcpy( page.data(), data + bytes_written, len );

			if( !write_page( file, page, target_page_is_a_new_allocated_one, file->inode.data_pages.at(page_idx) ) ) {
				CPPDEBUG( "no space left on device" );
				return 0;
			}

			bytes_written += len;
			file->pos += len;

		} else {

			std::basic_string_view<std::byte> page( data + bytes_written, header.page_size );

			if( !write_page( file, page, target_page_is_a_new_allocated_one, file->inode.data_pages.at(page_idx) ) ) {
				CPPDEBUG( "no space left on device" );
				return 0;
			}

			bytes_written += header.page_size;
			file->pos += header.page_size;

		} // else

	} // while

	file->modified = true;
	file->inode.file_len = std::max(file->pos, file->inode.file_len) ;

	return bytes_written;
}

bool SimpleFlashFs::write_page( FileHandle* file,
		const std::basic_string_view<std::byte> & page,
		bool target_page_is_a_new_allocated_one,
		uint32_t page_number )
{
	if( target_page_is_a_new_allocated_one ) {
		std::size_t ret = mem->write( header.page_size + header.page_size * page_number, page.data(), page.size() );

		if( ret == page.size() ) {
			return true;
		} else {
			CPPDEBUG( "no space left on device" );
			return false;
		}

	} else {
		uint32_t new_page_number = allocate_free_data_page();

		if( new_page_number == 0 ) {
			CPPDEBUG( "no space left on device" );
			return false;
		}

		uint32_t old_page_number = page_number;

		std::size_t ret = mem->write( header.page_size + header.page_size * new_page_number, page.data(), page.size() );

		if( ret == page.size() ) {
			for( auto & p : file->inode.data_pages ) {
				if( p == old_page_number ) {
					p = new_page_number;
					break;
				}
			}
			return true;
		} else {
			CPPDEBUG( "no space left on device" );
			return false;
		}
	}
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
			auto inode = get_inode( page );
			inode->page = i;
			max_inode_number = std::max( max_inode_number, inode->inode.inode_number );
			CPPDEBUG( format( "found inode %d,%d at page: %d",
					inode->inode.inode_number, inode->inode.inode_version_number, i ) );

			inodes[inode->inode.inode_number].push_back(inode);
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

	CPPDEBUG( format( "free Data pages: %s", IterableToCommaSeparatedString(free_data_pages) ) );
}

bool SimpleFlashFs::flush( FileHandle* file )
{
	if( !file->modified ) {
		return true;
	}

	if( file->inode.inode_number == 0) {
		file->inode.inode_number = max_inode_number + 1;
		max_inode_number = file->inode.inode_number;

		CPPDEBUG( format( "writing new inode %d,%d at page %d",
				file->inode.inode_number,
				file->inode.inode_version_number,
				file->page ));

		auto page = inode2page(file->inode);
		add_page_checksum(page);

		if( !write_page(file, page, true, file->page ) ) {
			CPPDEBUG( format( "cannot write inode %d page %d", file->inode.inode_number, file->page ));
			return false;
		}
		file->modified = false;
		return true;
	}

	FileHandle old_file = FileHandle( *file, false );

	file->page = allocate_free_inode_page_number();
	file->inode.inode_version_number++;

	auto page = inode2page(file->inode);
	add_page_checksum(page);

	CPPDEBUG( format( "writing page %d", file->page ));

	if( !write_page(file, page, true, file->page ) ) {
		CPPDEBUG( format( "cannot write inode %d,%d page %d",
				file->inode.inode_number, file->inode.inode_version_number, file->page ));
		return false;
	}

	file->modified = false;

	erase_inode_and_unused_pages(old_file, *file);

	return true;
}

void SimpleFlashFs::erase_inode_and_unused_pages( FileHandle & inode_to_erase,
		FileHandle & next_inode_version )
{
	CPPDEBUG( format( "cleaning up inode %d,%d comparing with %d,%d",
			inode_to_erase.inode.inode_number,
			inode_to_erase.inode.inode_version_number,
			next_inode_version.inode.inode_number,
			next_inode_version.inode.inode_version_number));

	// all pages, that are used by the next inode are still in use
	auto & dp = inode_to_erase.inode.data_pages;

	// assume to erase all old pages
	std::set<uint32_t> pages_to_erase( dp.begin(), dp.end() );

	// now remove all pages from the inode in the next version
	for( auto page : next_inode_version.inode.data_pages ) {
		pages_to_erase.erase(page);
	}

	// add the inode self too
	pages_to_erase.insert(inode_to_erase.page);

	for( auto page : pages_to_erase ) {
		CPPDEBUG( format( "erasing page: %d", page ) );

		std::size_t address = header.page_size + page * header.page_size;
		mem->erase(address, header.page_size );

		if( page > header.max_inodes ) {
			free_data_pages.insert(page);
		}
	}

	inode_to_erase.modified = false;
}

std::vector<std::byte> SimpleFlashFs::inode2page( const Inode & inode )
{
	std::vector<std::byte> page(header.page_size);
	std::size_t pos = 0;

	auto write=[this,&pos,&page]( auto t ) {
		auto_endianess(t);
		std::memcpy(&page[pos], &t, sizeof(t) );
		pos += sizeof(t);
	};

	write( inode.inode_number );
	write( inode.inode_version_number );
	write( inode.file_name_len );

	std::memcpy(&page[pos], &inode.file_name[0], inode.file_name_len );
	pos += inode.file_name_len;

	write( inode.attributes );
	write( inode.file_len );

	uint32_t pages = inode.data_pages.size();
	write( pages );

	for( unsigned i = 0; i < inode.pages; i++ ) {
		write( inode.data_pages[i] );
	}

	// write small data directly into the inode
	if( pages == 0 && inode.file_len > 0 ) {
		memcpy( page.data() + pos, inode.inode_data.data(), inode.inode_data.size() );
	}

	return page;
}

std::list<std::shared_ptr<FileHandle>> SimpleFlashFs::get_all_inodes()
{
	std::list<std::shared_ptr<FileHandle>> ret;

	for( unsigned i = 0; i < header.max_inodes; i++ ) {

		std::vector<std::byte> page(header.page_size);

		if( read_page( i, page, true ) ) {

			auto inode = get_inode( page );
			inode->page = i;

			ret.push_back(inode);
		}
	}

	return ret;
}

std::size_t SimpleFlashFs::read( FileHandle* file, std::byte *data, std::size_t size )
{
	std::size_t page_idx = file->pos / header.page_size;
	std::size_t bytes_readen = 0;

	if( file->inode.file_len - file->pos < size ) {
		size = file->inode.file_len - file->pos;
	}

	if( size == 0 ) {
		return 0;
	}

	/**
	 * stored data is inside the inode itself, if there is enough space
	 */
	std::size_t space_inside_the_inode = get_inode_data_space_size(file);
	if( space_inside_the_inode > file->pos + size &&
		file->inode.file_len < space_inside_the_inode ) {

		std::size_t len = std::min( file->inode.file_len - file->pos, size );
		memcpy( data, file->inode.inode_data.data() + file->pos, len );
		return len;
	}

	// unaligned data, map the buffer to a complete page
	const std::size_t data_start_at_page = file->pos % header.page_size;

	if( data_start_at_page != 0 ) {
		std::vector<std::byte> page(header.page_size);
		if( !read_page( file->inode.data_pages.at(page_idx), page, false ) ) {
			CPPDEBUG( format( "reading from pos %d failed", page_idx * header.page_size ) );
			return bytes_readen;
		}

		const std::size_t len = std::min( size, header.page_size - data_start_at_page );
		memcpy( data + bytes_readen, &page[data_start_at_page], len );

		bytes_readen += len;
		file->pos += len;
	}

	while( bytes_readen < size ) {
		page_idx = file->pos / header.page_size;

		// last partial page
		if( bytes_readen + header.page_size > size ) {

			std::vector<std::byte> page(header.page_size);

			if( !read_page( file->inode.data_pages.at(page_idx), page, false ) ) {
				CPPDEBUG( format( "reading from pos %d failed", page_idx * header.page_size ) );
				return bytes_readen;
			}

			const std::size_t len = std::min( static_cast<uint32_t>(size - bytes_readen), header.page_size );
			memcpy( data + bytes_readen, page.data(), len );

			bytes_readen += len;
			file->pos += len;

		} else {

			if( !read_page( file->inode.data_pages.at(page_idx), data + bytes_readen, header.page_size ) ) {
				CPPDEBUG( "reading from device failed" );
				return bytes_readen;
			}

			bytes_readen += header.page_size;
			file->pos += header.page_size;

		} // else

	} // while

	return bytes_readen;
}

std::size_t SimpleFlashFs::get_inode_data_space_size( FileHandle* file ) const
{
	return header.page_size - (sizeof(Inode::inode_number)
			+ sizeof(Inode::inode_version_number)
			+ sizeof(Inode::file_name_len)
			+ file->inode.file_name_len
			+ sizeof(Inode::attributes)
			+ sizeof(Inode::file_len)
			+ sizeof(Inode::pages));
}

} // namespace SimpleFlashFs::dynamic

