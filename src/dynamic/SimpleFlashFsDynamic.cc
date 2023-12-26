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
#include <format.h>

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

uint32_t SimpleFlashFs::calc_page_checksum( const std::vector<std::byte> & page )
{
	switch( header.crc_checksum_type )
	{
	case Header::CRC_CHECKSUM::CRC32:
		return crcFast( reinterpret_cast<const unsigned char*>(&page[0]), page.size() - sizeof(uint32_t));
	default:
		throw std::out_of_range("unknown checksum type");
	}
}

uint32_t SimpleFlashFs::get_page_checksum( const std::vector<std::byte> & page )
{
	switch( header.crc_checksum_type )
	{
	case Header::CRC_CHECKSUM::CRC32:
		{
			uint32_t chksum = 0;
			memcpy( &chksum, &page[page.size() - sizeof(uint32_t)], sizeof(uint32_t) );
			auto_endianess(chksum);
			return chksum;
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

	return true;
 }

} // namespace SimpleFlashFs::dynamic

