/*
 * SimpleFlashFsDynamicHeader.h
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */

#ifndef SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICHEADER_H_
#define SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICHEADER_H_

#include <cstdint>
#include <string>

namespace SimpleFlashFs {
namespace dynamic {

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

	std::string 	magic_string{};
	ENDIANESS 		endianess{ENDIANESS::LE};
	uint16_t   		version = 0;
	uint32_t		page_size = 0;
	uint64_t		filesystem_size = 0;
	uint32_t		max_inodes = 0;
	uint16_t		max_path_len = 0;
	CRC_CHECKSUM	crc_checksum_type{CRC_CHECKSUM::CRC32};
};

} // namespace dynamic
} // namespace SimpleFlashFs


#endif /* SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICHEADER_H_ */
