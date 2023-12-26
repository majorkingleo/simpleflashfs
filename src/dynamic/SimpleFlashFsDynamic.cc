/*
 * SimpleFlashFsDynamic.cc
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */
#include "SimpleFlashFsDynamic.h"
#include "../SimpleFlashFsFlashMemoryInterface.h"
#include "../SimpleFlashFsConstants.h"
#include <algorithm>
#include <CpputilsDebug.h>
#include <bit>
#include <cstring>
#include "../crc/crc.h"

using namespace Tools;

namespace SimpleFlashFs::dynamic {

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

} // namespace SimpleFlashFs::dynamic

