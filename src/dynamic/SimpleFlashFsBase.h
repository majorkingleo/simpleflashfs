/**
 * SimpleFlashFs  base class
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */

#pragma once

#include <cstdint>
#include <algorithm>
#include "../SimpleFlashFsConstants.h"
#include "../SimpleFlashFsFlashMemoryInterface.h"

namespace SimpleFlashFs {

class FlashMemoryInterface;

namespace base {

/**
 * Filesystem header
 */
template <class Config>
struct Header
{
	enum class ENDIANESS
	{
		LE,
		BE
	};

	enum class CRC_CHECKSUM
	{
		CRC32 = 0
	};

	Config::string_type 	magic_string{};
	ENDIANESS 		endianess{ENDIANESS::LE};
	uint16_t   		version = 0;
	uint32_t		page_size = 0;
	uint64_t		filesystem_size = 0; // size in pages
	uint32_t		max_inodes = 0;
	uint16_t		max_path_len = 0;
	CRC_CHECKSUM	crc_checksum_type{CRC_CHECKSUM::CRC32};
};

template <class Config>
class SimpleFlashFsBase
{
protected:
	using header_t = Header<Config>;
	header_t header;
	FlashMemoryInterface *mem;

public:
	SimpleFlashFsBase( FlashMemoryInterface *mem_interface_ )
	: mem(mem_interface_)
	{

	}

	Header<Config> create_default_header( uint32_t page_size, uint64_t filesystem_size );
};

template <class Config>
Header<Config> SimpleFlashFsBase<Config>::create_default_header( uint32_t page_size, uint64_t filesystem_size )
{
	header_t header;

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
		header.endianess = header_t::ENDIANESS::BE;
	} else {
		header.endianess = header_t::ENDIANESS::LE;
	}

	return header;
}


} // namespace base

} // namespace SimpleFlashFs


