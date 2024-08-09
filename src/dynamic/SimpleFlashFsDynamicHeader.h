/**
 * Internal data representation structs
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#ifndef SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICHEADER_H_
#define SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICHEADER_H_

#include <cstdint>
#include <string>
#include <vector>

namespace SimpleFlashFs {
namespace dynamic {

/**
 * Filesystem header
 */
#if 0
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
	uint64_t		filesystem_size = 0; // size in pages
	uint32_t		max_inodes = 0;
	uint16_t		max_path_len = 0;
	CRC_CHECKSUM	crc_checksum_type{CRC_CHECKSUM::CRC32};
};

/**
 * Inode struct
 */
struct Inode
{
	uint64_t 		inode_number{};
	uint64_t		inode_version_number{};
	uint16_t		file_name_len{};
	std::string		file_name;
	uint64_t		attributes{};
	uint64_t		file_len{};
	uint32_t		pages{};

	// the index of data pages
	std::vector<uint32_t> data_pages;

	// data, that can be stored inside the inode
	// will only be filled if pages == 0, so data_pages is also
	// zero and needs no space
	std::vector<std::byte> inode_data;
};
#endif

} // namespace dynamic
} // namespace SimpleFlashFs


#endif /* SRC_DYNAMIC_SIMPLEFLASHFSDYNAMICHEADER_H_ */
