/**
 * SimpleFlashFs test case initialisation
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#pragma once

#include <src/static/SimpleFlashFsStatic.h>
#include "../src/crc/crc.h"
#include <optional>
#include <memory>
#include <CpputilsDebug.h>
#include <format.h>

static constexpr const std::size_t SFF_FILE_NAME_MAX = 30;
static constexpr const std::size_t SFF_PAGE_SIZE = 1024;
static constexpr const std::size_t SFF_MAX_SIZE = 128*1024;
static constexpr const std::size_t SFF_MAX_PAGES = 1024;

struct ConfigH7 : public SimpleFlashFs::static_memory::Config<SFF_FILE_NAME_MAX,SFF_PAGE_SIZE,SFF_MAX_PAGES,SFF_MAX_SIZE>
{
	static uint32_t crc32( const std::byte *bytes, size_t len );
};

template<class Config>
class SimpleFsNoDel : public SimpleFlashFs::static_memory::SimpleFlashFs<Config>
{
public:
	using base_t = ::SimpleFlashFs::static_memory::SimpleFlashFs<Config>;

protected:
	class InodeVersionStore
	{
	public:

		enum class add_ret_t
		{
			replaced,
			inserted
		};

		struct InodeVersion
		{
			uint32_t inode;
			uint32_t version;
		};

	protected:
		typename Config::vector_type<InodeVersion> data;

	public:

		add_ret_t add( uint32_t inode, uint32_t version )
		{
			for( auto & iv : data ) {
				if( iv.inode == inode ) {
					iv.version = std::max( iv.version, version );
					return add_ret_t::replaced;
				}
			}

			data.push_back( InodeVersion{ inode, version } );
			return add_ret_t::inserted;
		}

	};

public:
	struct Stat
	{
		std::size_t largest_file_size = 0; // in bytes
		std::size_t trash_size = 0; // in bytes
		std::size_t used_inodes = 0; // number of inodes in use
		std::size_t trash_inodes = 0; // number of inodes to delete
		std::size_t free_inodes = 0;
	};

protected:
	Stat stat{};

public:
	SimpleFsNoDel( ::SimpleFlashFs::FlashMemoryInterface *mem_interface_ )
	: base_t(mem_interface_)
	{
		crcInit();
	}

	bool create()
	{
		typename base_t::header_t header = base_t::create_default_header( Config::PAGE_SIZE, Config::MAX_SIZE / Config::PAGE_SIZE );
		header.max_path_len = Config::FILE_NAME_MAX;

		return base_t::create(header);
	}

	// read the fs the memory interface points to
	// starting at offset 0
	bool init() override {
		if( !base_t::init() ) {
			return false;
		}

		read_all_free_data_pages();
		return true;
	}

	const Stat & get_stat() const {
		return stat;
	}

protected:
	void read_all_free_data_pages();

	virtual void erase_inode_and_unused_pages( base_t::file_handle_t & inode_to_erase, base_t::file_handle_t & next_inode_version ) override {
		// do nothing
	}
};


template <class Config>
void SimpleFsNoDel<Config>::read_all_free_data_pages()
{
	base_t::free_data_pages.clear();
	stat = {};

	for( unsigned i = base_t::header.max_inodes; i < base_t::header.filesystem_size; i++ ) {
		base_t::free_data_pages.insert(i);
	}

	InodeVersionStore iv_store;

	for( unsigned i = 0; i < base_t::header.max_inodes; i++ ) {

		typename base_t::config_t::page_type page(base_t::header.page_size);



		if( base_t::read_page( i, page, true ) ) {
			typename base_t::FileHandle inode = base_t::get_inode( page );
			inode.page = i;
			base_t::max_inode_number = std::max( base_t::max_inode_number, inode.inode.inode_number );
			CPPDEBUG( Tools::format( "found inode %d,%d at page: %d",
					inode.inode.inode_number, inode.inode.inode_version_number, i ) );

			if( iv_store.add( inode.inode.inode_number, inode.inode.inode_version_number ) == InodeVersionStore::add_ret_t::replaced ) {
				stat.trash_inodes++;
			} else {
				stat.used_inodes++;
			}

			stat.largest_file_size = std::max( stat.largest_file_size, inode.file_size() );

			// remove used pages from free_data_pages list
			for( auto page : inode.inode.data_pages ) {
				stat.trash_size += base_t::header.page_size;
				base_t::free_data_pages.erase(page);
			}
		}
	}

	stat.free_inodes = base_t::header.max_inodes - stat.used_inodes - stat.trash_inodes;

	CPPDEBUG( Tools::format( "free Data pages: %s", Tools::IterableToCommaSeparatedString(base_t::free_data_pages) ) );
	CPPDEBUG( Tools::format( "largest file size: %dB", stat.largest_file_size ) );
	CPPDEBUG( Tools::format( "trash size size: %dB", stat.largest_file_size ) );
	CPPDEBUG( Tools::format( "used inodes: %d trash inodes: %d free inodes: %d",
			stat.used_inodes,
			stat.trash_inodes,
			stat.free_inodes ));
}

template<class Config>
class SimpleFs2FlashPages
{
public:
	using base_t = SimpleFsNoDel<Config>;

protected:
	std::optional<SimpleFsNoDel<Config>> fs1;
	std::optional<SimpleFsNoDel<Config>> fs2;

	::SimpleFlashFs::FlashMemoryInterface *mem_interface1;
	::SimpleFlashFs::FlashMemoryInterface *mem_interface2;

	SimpleFsNoDel<Config> *fs = nullptr;
public:
	SimpleFs2FlashPages( ::SimpleFlashFs::FlashMemoryInterface *mem_interface1_,
						 ::SimpleFlashFs::FlashMemoryInterface *mem_interface2_ )
	: mem_interface1(mem_interface2_),
	  mem_interface2(mem_interface2_)
	{
	}

	~SimpleFs2FlashPages()
	{

	}

	bool init()
	{
		if( !init_fs() ) {
			return false;
		}

		should_cleanup();

		return true;
	}

	base_t::file_handle_t open( const Config::string_view_type & name, std::ios_base::openmode mode )
	{
		return fs->open( name, mode );
	}

	bool should_cleanup( unsigned treshold_percentage = 80 )
	{
		const typename SimpleFsNoDel<Config>::base_t::Header & header = fs->get_header();
		const unsigned all_data_pages = header.filesystem_size - header.max_inodes;
		const unsigned data_pages_usage = 100 - (100.0 / all_data_pages * fs->get_number_of_free_data_pages());

		CPPDEBUG( Tools::format( "data_pages_usage: %d%%", data_pages_usage ) );

		const typename SimpleFsNoDel<Config>::Stat & stat = fs->get_stat();
		const unsigned inode_usage = 100 - ( 100.0 / header.max_inodes * stat.free_inodes );

		CPPDEBUG( Tools::format( "inode_usage:      %d%%", inode_usage ) );

		if( inode_usage > treshold_percentage ) {
			return true;
		}

		if( data_pages_usage > treshold_percentage ) {
			return true;
		}

		return false;
	}

protected:
	bool init_fs()
	{
		init( fs1, mem_interface1 );
		init( fs2, mem_interface2 );

		if( !fs1 && !fs2 ) {
			if( !create( fs1, mem_interface1 ) ) {
				CPPDEBUG( "cannot create fs1" );
				return false;
			}
			if( !create( fs2, mem_interface2 ) ) {
				CPPDEBUG( "cannot create fs2" );
				return false;
			}
		} else if( fs1 && !fs2 ) {
			fs = &fs1.value();
			return true;
		} else if( fs2 && !fs1 ) {
			fs = &fs2.value();
			return true;
		}

		// both filesystems are valid choose the newest one
		uint64_t m1 = fs1->get_max_inode_number();
		uint64_t m2 = fs2->get_max_inode_number();

		if( m1 > m2 ) {
			fs2.reset();
			fs = &fs1.value();
		} else {
			fs1.reset();
			fs = &fs2.value();
		}

		return true;
	}

	bool init( std::optional<SimpleFsNoDel<Config>> & fs, ::SimpleFlashFs::FlashMemoryInterface *mem )
	{
		fs.emplace(mem);
		if( !fs->init() ) {
			fs.reset();
			return false;
		}

		return true;
	}

	bool create( std::optional<SimpleFsNoDel<Config>> & fs, ::SimpleFlashFs::FlashMemoryInterface *mem )
	{
		mem->erase( 0, mem->size() );
		fs.emplace(mem);

		if( !fs->create() ) {
			fs.reset();
			return false;
		}

		if( !fs->init() ) {
			fs.reset();
			return false;
		}

		return true;
	}
};


class H7TwoFace
{
	class Destroyer
	{
	public:
		void operator()( SimpleFlashFs::FileInterface* fs ) const {
			fs->~FileInterface();
		}
	};

public:
	using file_handle_t = std::unique_ptr<SimpleFlashFs::FileInterface,Destroyer>;

public:
	static file_handle_t open( const std::string_view & name, std::ios_base::openmode mode );
};
