/**
 * SimpleFlashFs main implementation class
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */

#ifndef SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_
#define SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_

#include "../base/SimpleFlashFsBase.h"
#include <memory>
#include <list>

namespace SimpleFlashFs {

namespace dynamic {

struct Config
{
	using magic_string_type = std::string;
	using string_type = std::string;
	using string_view_type = std::string_view;
	using page_type = std::vector<std::byte>;

	static constexpr uint32_t PAGE_SIZE = 0; // no limit

	template<class T> class vector_type : public std::vector<T> {};

	static uint32_t crc32( const std::byte *bytes, size_t len );
};


class SimpleFlashFs : public base::SimpleFlashFsBase<Config>
{
public:
	using Header = base::Header<Config>;
	using Inode = base::Inode<Config>;
	using FileHandle = base::FileHandle<Config,base::SimpleFlashFsBase<Config>>;

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
	bool init() {
		if( !base::SimpleFlashFsBase<Config>::init() ) {
			return false;
		}

		read_all_free_data_pages();
		return true;
	}

	friend class base::FileHandle<Config,SimpleFlashFs>;

	std::list<std::shared_ptr<FileHandle>> get_all_inodes();

protected:
	void read_all_free_data_pages();
};

} // namespace dynamic
} // namespace SimpleFlashFs



#endif /* SRC_DYNAMIC_SIMPLEFLASHFSDYNAMIC_H_ */
