/**
 * SimpleFlashFs main implementation class
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */

#ifndef SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_
#define SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_

#include "SimpleFlashFsBase.h"
#include "SimpleFlashFsDynamicHeader.h"
#include <vector>
#include <memory>
#include <set>
#include <list>

namespace SimpleFlashFs {

class FlashMemoryInterface;

namespace dynamic {


// https://stackoverflow.com/a/35092546
/**
 * swaps the byte order of a value
 */
template<typename T> inline static T swapByteOrder(const T& val) {
    int totalBytes = sizeof(val);
    T swapped = (T) 0;
    for (int i = 0; i < totalBytes; ++i) {
        swapped |= (val >> (8*(totalBytes-i-1)) & 0xFF) << (8*i);
    }
    return swapped;
}

class SimpleFlashFs;

/**
 * FileHandle class
 */
class FileHandle
{
public:
	Inode inode;
	uint32_t page   {0};
	std::size_t pos {0};
	bool modified   {false};

protected:
	SimpleFlashFs *fs;

public:
	FileHandle( SimpleFlashFs *fs_ )
	: fs( fs_ )
	{}

	/**
	 * automatically calls flush()
	 */
	~FileHandle();

	FileHandle( const FileHandle & other ) = delete;

	FileHandle & operator=( const FileHandle & other ) = delete;

	std::size_t write( const std::byte *data, std::size_t size );
	std::size_t read( std::byte *data, std::size_t size );
	bool flush();

	std::size_t tellg() const {
		return pos;
	}

	std::size_t file_size() const {
		return inode.file_len;
	}

	bool eof() const {
		return pos == inode.file_len-1;
	}

	void seek( std::size_t pos_ ) {
		pos = pos_;
	}

	friend class SimpleFlashFs;

private:
	/**
	 * For creating a backup copy, disconnect the pointer to fs.
	 */
	FileHandle( const FileHandle & other, bool no_fs_connection )
	: inode( other.inode ),
	  page( other.page ),
	  pos( other.pos ),
	  modified( other.modified ),
	  fs( nullptr )
	{

	}

};

struct Config
{
	using string_type = std::string;
};

class SimpleFlashFs : public base::SimpleFlashFsBase<Config>
{
public:
	using Header = base::Header<Config>;

protected:
	FlashMemoryInterface *mem;

	std::set<uint32_t> allocated_unwritten_pages;
	std::set<uint32_t> free_data_pages;
	uint64_t max_inode_number = 0;

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

	// read the fs the memory interface points to
	// starting at offset 0
	bool init();

	const Header & get_header() const {
		return header;
	}

	std::shared_ptr<FileHandle> open( const std::string & name, std::ios_base::openmode mode );

	std::size_t write( FileHandle* file, const std::byte *data, std::size_t size );
	std::size_t read( FileHandle* file, std::byte *data, std::size_t size );

	bool flush( FileHandle* file );

	friend class FileHandle;

	uint64_t get_max_inode_number() const {
		return max_inode_number;
	}

	std::size_t get_number_of_free_data_pages() const {
		return free_data_pages.size();
	}

	std::list<std::shared_ptr<FileHandle>> get_all_inodes();

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

	uint32_t calc_page_checksum( const std::vector<std::byte> & page ) {
		return calc_page_checksum( page.data(), page.size() );
	}
	uint32_t calc_page_checksum( const std::byte *page, std::size_t size );

	uint32_t get_page_checksum( const std::vector<std::byte> & page ) {
		return get_page_checksum( page.data(), page.size() );
	}
	uint32_t get_page_checksum( const std::byte *page, std::size_t size );
	uint32_t get_checksum_size() const;

	std::shared_ptr<FileHandle> find_file( const std::string & name );

	bool read_page( std::size_t idx, std::vector<std::byte> & page, bool check_crc = false ) {
		return read_page( idx, page.data(), page.size(), check_crc );
	}

	bool read_page( std::size_t idx, std::byte *data, std::size_t size, bool check_crc = false );

	std::shared_ptr<FileHandle> get_inode( const std::vector<std::byte> & data );
	std::shared_ptr<FileHandle> allocate_free_inode_page();
	uint32_t allocate_free_inode_page_number();
	uint32_t allocate_free_data_page();
	uint32_t allocate_free_data_page( const FileHandle *file );

	void free_unwritten_pages( uint32_t page ) {
		allocated_unwritten_pages.erase(page);
	}

	void read_all_free_data_pages();

	bool write_page( FileHandle* file,
			const std::basic_string_view<std::byte> & page,
			bool target_page_is_a_new_allocated_one,
			uint32_t page_number );

	bool write_page( FileHandle* file,
			const std::vector<std::byte> & page,
			bool target_page_is_a_new_allocated_one,
			uint32_t page_number ) {

		return write_page( file,
				std::basic_string_view<std::byte>( page.data(), page.size() ),
				target_page_is_a_new_allocated_one,
				page_number );
	}

	void erase_inode_and_unused_pages( FileHandle & inode_to_erase, FileHandle & next_inode_version );

	std::vector<std::byte> inode2page( const Inode & inode );

	/**
	 * returns the maximum size of data, that fits inside an spacific inode
	 * depends, on the filename len
	 */
	std::size_t get_inode_data_space_size( const FileHandle* file ) const;

	/**
	 * The length of the filename is dynamic, so we have to calculate
	 * it per file.
	 */
	std::size_t get_max_inode_data_pages( const FileHandle* file ) const;
};

} // namespace dynamic
} // namespace SimpleFlashFs



#endif /* SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_ */
