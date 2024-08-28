/**
 * Internal data representation structs
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#pragma once

#include "../dynamic/SimpleFlashFsBase.h"
#include <static_vector.h>
#include <static_string.h>
#include <static_list.h>

namespace SimpleFlashFs {
namespace static_memory {

template <size_t SFF_FILE_NAME_MAX, size_t SFF_PAGE_SIZE, size_t SFF_MAX_PAGES, size_t SFF_MAX_SIZE>
struct Config
{
	using magic_string_type = Tools::static_string<MAGICK_STRING_LEN>;
	using string_type = Tools::static_string<SFF_FILE_NAME_MAX>;
	using string_view_type = std::string_view;
	using page_type = Tools::static_vector<std::byte,SFF_PAGE_SIZE>;
	constexpr static size_t PAGE_SIZE = SFF_PAGE_SIZE;
	constexpr static size_t MAX_PAGES = SFF_MAX_PAGES;
	constexpr static size_t FILE_NAME_MAX = SFF_FILE_NAME_MAX;
	constexpr static size_t MAX_SIZE = SFF_MAX_SIZE;

	template<class T> class vector_type : public Tools::static_vector<T,SFF_MAX_PAGES> {};

	template<class T>
	class set_type : public Tools::static_list<T,SFF_MAX_PAGES>
	{
		using base_t = Tools::static_list<T,SFF_MAX_PAGES>;

	public:
		template< class InputIt >
		void insert( InputIt first, InputIt last ) {
			for( InputIt it = first; it != last; ++it ) {
				insert( *it );
			}
		}

		void insert( const T & value ) {

			for( auto it = base_t::begin();
					  it !=base_t::end(); ++it ) {
				if( *it == value ) {
					return;
				}
			}

			base_t::push_back(value);
		}

		base_t::size_type erase( const T & value ) {

			for( auto it = base_t::begin();
					  it != base_t::end(); ++it ) {
				if( *it == value ) {
					base_t::erase(it);
					return 1;
				}
			}

			return 0;
		}

		base_t::iterator erase( base_t::const_iterator it ) {
			return base_t::erase(it);
		}

		base_t::size_type count(const T & member) const {
			for( auto it = base_t::begin(); it != base_t::end(); ++it ) {
				if( *it == member ) {
					return 1;
				}
			}

			return 0;
		}
	};

	static uint32_t crc32( const std::byte *bytes, size_t len );
};

template <class Config>
class SimpleFlashFs : public base::SimpleFlashFsBase<Config>
{
public:
	using base_t = base::SimpleFlashFsBase<Config>;
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

	void list_files( typename Config::vector_type<typename Config::string_type> & file_names )
	{
		typename base_t::InodeVersionStore iv_store;

		for( unsigned i = 0; i < base_t::header.max_inodes; i++ ) {
			typename Config::page_type page(base_t::header.page_size);

			if( base_t::read_page( i, page, true ) ) {
				auto file_handle = base_t::get_inode( page );
				file_handle.page = i;
				iv_store.add( file_handle );
			}
		}

		const auto & data = iv_store.get_data();

		for( const auto & iv : data ) {
			typename Config::page_type page(base_t::header.page_size);
			if( base_t::read_page( iv.page, page, true ) ) {
				auto file_handle = base_t::get_inode( page );
				file_handle.page = iv.page;

				// files witout a name are deleted files
				if( file_handle.inode.file_name.empty() ) {
					continue;
				}

				file_names.push_back( file_handle.inode.file_name );
			}
		}
	}


	friend class base::FileHandle<Config,SimpleFlashFs>;
};



} // namespace dynamic
} // namespace SimpleFlashFs

