/**
 * SimpleFlashFs test case initialisation
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#pragma once

#include <src/static/SimpleFlashFsStatic.h>
#include "../src/crc/crc.h"

static constexpr const std::size_t SFF_FILENAME_MAX = 30;
static constexpr const std::size_t SFF_PAGE_SIZE = 1024;
static constexpr const std::size_t SFF_MAX_SIZE = 128*1024*2;

struct ConfigH7 : public SimpleFlashFs::static_memory::Config<SFF_FILENAME_MAX,SFF_PAGE_SIZE>
{
	static uint32_t crc32( const std::byte *bytes, size_t len );
};

class SimpleFsH7 : public SimpleFlashFs::static_memory::SimpleFlashFs<ConfigH7>
{
	using base_t = ::SimpleFlashFs::static_memory::SimpleFlashFs<ConfigH7>;

public:
	SimpleFsH7( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
	: base_t(mem_interface_)
	{
		crcInit();
	}

	bool create()
	{
		header_t header = create_default_header( SFF_PAGE_SIZE, SFF_MAX_SIZE / SFF_PAGE_SIZE );
		header.max_path_len = SFF_FILENAME_MAX;

		return base_t::create(header);
	}
};


