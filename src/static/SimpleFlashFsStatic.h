/**
 * Internal data representation structs
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#pragma once

#include "../dynamic/SimpleFlashFsBase.h"
#include <static_vector.h>
#include <static_string.h>

namespace SimpleFlashFs {
namespace static_memory {

template <size_t SFF_FILENAME_MAX, size_t SFF_PAGE_SIZE>
struct Config
{
	using string_type = Tools::static_string<SFF_FILENAME_MAX>;
	using page_type = Tools::static_vector<std::byte,SFF_PAGE_SIZE>;

	template<class T> class vector_type : public Tools::static_vector<T,SFF_PAGE_SIZE> {};

	static uint32_t crc32( const std::byte *bytes, size_t len );
};

template <class Config>
class SimpleFlashFs : public base::SimpleFlashFsBase<Config>
{
public:
	using Header = base::Header<Config>;
	using Inode = base::Inode<Config>;
	using FileHandle = base::FileHandle<Config,base::SimpleFlashFsBase<Config>>;

public:

	SimpleFlashFs( FlashMemoryInterface *mem_interface_ )
	: base::SimpleFlashFsBase<Config>(mem_interface_)
	{

	}
	/** creates a new fs
	 *
	 * following values has to be set
	 * header.page_size,
	 * header.filesystem_size
	 *
	 */
	bool create( const Header & h )
	{
		if( h.page_size * h.filesystem_size > base::SimpleFlashFsBase<Config>::mem->size() ) {
			CPPDEBUG( "filesystem too large for memory" );
			return false;
		}

		if( h.page_size == 0 ||
			h.filesystem_size == 0 ) {
			CPPDEBUG( "invalid data" );
			return false;
		}

		if( h.page_size < MIN_PAGE_SIZE ) {
			CPPDEBUG( "invalid data" );
			return false;
		}

		if( h.filesystem_size < 4 ) {
			CPPDEBUG( "invalid data" );
			return false;
		}

		base::SimpleFlashFsBase<Config>::mem->erase(0, h.page_size * h.filesystem_size );

		if( !base::SimpleFlashFsBase<Config>::write( h ) ) {
			return false;
		}

		return base::SimpleFlashFsBase<Config>::init();
	}

	friend class base::FileHandle<Config,SimpleFlashFs>;
};



} // namespace dynamic
} // namespace SimpleFlashFs
