/**
 * SimpleFlashFs test case initialisation
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#ifndef SRC_TEST_STATIC_STATIC_FS_H_
#define SRC_TEST_STATIC_STATIC_FS_H_


#include <src/static/SimpleFlashFsStatic.h>
#include "../src/crc/crc.h"

static constexpr const std::size_t SFF_FILENAME_MAX = 10;
static constexpr const std::size_t SFF_PAGE_SIZE = 4*1024;
static constexpr const std::size_t SFF_MAX_PAGES = 1024;  // space for hash set (set of uint32_t)

struct MyConfig : public SimpleFlashFs::static_memory::Config<SFF_FILENAME_MAX,SFF_PAGE_SIZE,SFF_MAX_PAGES, 1024*128>
{
	static uint32_t crc32( const std::byte *bytes, size_t len );
};

class SimpleFs : public SimpleFlashFs::static_memory::SimpleFlashFs<MyConfig>
{
public:
	SimpleFs( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
	: ::SimpleFlashFs::static_memory::SimpleFlashFs<MyConfig>(mem_interface_)
	{
		crcInit();
	}
};


#endif /* SRC_TEST_STATIC_STATIC_FS_H_ */
