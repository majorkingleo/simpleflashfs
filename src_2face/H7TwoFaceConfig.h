/*
 * H7TwoFaceConfig.h
 *
 *  Created on: 29.08.2024
 *      Author: martin.oberzalek
 */

#pragma once

#include "../src/static/SimpleFlashFsStatic.h"

static constexpr const std::size_t SFF_FILE_NAME_MAX = 30;
static constexpr const std::size_t SFF_PAGE_SIZE = 512;
static constexpr const std::size_t SFF_MAX_SIZE = 128*1024;
static constexpr const std::size_t SFF_MAX_PAGES = 1024;

struct ConfigH7 : public SimpleFlashFs::static_memory::Config<SFF_FILE_NAME_MAX,SFF_PAGE_SIZE,SFF_MAX_PAGES,SFF_MAX_SIZE>
{
	static uint32_t crc32( const std::byte *bytes, size_t len );
};

