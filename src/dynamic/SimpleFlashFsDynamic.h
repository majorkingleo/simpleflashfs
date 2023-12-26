/*
 * SimpleFlashFsDynamic.h
 *
 *  Created on: 25.12.2023
 *      Author: martin
 */

#ifndef SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_
#define SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_

#include "SimpleFlashFsDynamicHeader.h"
#include <vector>
#include <memory>

namespace SimpleFlashFs {

class FlashMemoryInterface;

namespace dynamic {


// https://stackoverflow.com/a/35092546
template<typename T> inline static T swapByteOrder(const T& val) {
    int totalBytes = sizeof(val);
    T swapped = (T) 0;
    for (int i = 0; i < totalBytes; ++i) {
        swapped |= (val >> (8*(totalBytes-i-1)) & 0xFF) << (8*i);
    }
    return swapped;
}

struct FileHandle
{
	Inode inode;
	std::size_t pos{0};
};

class SimpleFlashFs
{
	Header header;
	FlashMemoryInterface *mem;
public:

	SimpleFlashFs( FlashMemoryInterface *mem_interface );

	/** creates a new fs
	 *
	 * following values has to be set
	 * header.page_size,
	 * header.filesystem_size
	 *
	 */
	bool create( const Header & header );
	static Header create_default_header( uint32_t page_size, uint64_t filesystem_size );

	// read the fs the memory interface points to
	// starting at offset 0
	bool init();

	const Header & get_header() const {
		return header;
	}

	std::shared_ptr<FileHandle> open( const std::string & name, std::ios_base::openmode mode );

protected:
	bool write( const Header & header );
	bool swap_endianess();

	template<class T> void auto_endianess( T & val ) {
		if( swap_endianess() ) {
			val = swapByteOrder( val );
		}
	}

	std::size_t get_num_of_checksum_bytes() const;

	void add_page_checksum( std::vector<std::byte> & page );
	uint32_t calc_page_checksum( const std::vector<std::byte> & page );
	uint32_t get_page_checksum( const std::vector<std::byte> & page );

	std::shared_ptr<FileHandle> find_file( const std::string & name );

	bool read_page( std::size_t idx, std::vector<std::byte> & data, bool check_crc = false );

	std::shared_ptr<FileHandle> get_inode( const std::vector<std::byte> & data );
};

} // namespace dynamic
} // namespace SimpleFlashFs



#endif /* SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_ */
