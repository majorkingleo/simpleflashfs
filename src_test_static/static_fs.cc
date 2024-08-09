/**
 * SimpleFlashFs test case initialisation
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#include "static_fs.h"

uint32_t MyConfig::crc32( const std::byte *bytes, size_t len )
{
	return crcFast( reinterpret_cast<unsigned const char*>(bytes), len );
}
