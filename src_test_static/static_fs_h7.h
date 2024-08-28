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

	bool rename_file( base_t::FileHandle* file, const std::string_view & new_file_name ) override;

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

	typename base_t::InodeVersionStore iv_store;

	for( unsigned i = 0; i < base_t::header.max_inodes; i++ ) {

		typename base_t::config_t::page_type page(base_t::header.page_size);

		if( base_t::read_page( i, page, true ) ) {
			typename base_t::FileHandle inode = base_t::get_inode( page );
			inode.page = i;
			base_t::max_inode_number = std::max( base_t::max_inode_number, inode.inode.inode_number );
			CPPDEBUG( Tools::format( "found inode %d,%d at page: %d name: '%s'",
					inode.inode.inode_number, inode.inode.inode_version_number, i, inode.inode.file_name ) );

			if( iv_store.add( inode ) == base_t::InodeVersionStore::add_ret_t::replaced ) {
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

template <class Config>
bool SimpleFsNoDel<Config>::rename_file( base_t::FileHandle* file, const std::string_view & new_file_name )
{
	// delete it by setting the filename to an empty string
	// next cleanup process will skip it
	if( new_file_name.empty() ) {
		file->inode.file_name.clear();
		file->inode.file_name_len = 0;
		file->modified = true;
		if( !file->flush() ) {
			return false;
		}

		// invalidate the handle
		file->disconnect();

		return true;
	}

	auto other_file = base_t::find_file( new_file_name );

	if( other_file.valid() ) {
		other_file.delete_file();
	}

	file->inode.file_name = new_file_name;
	file->inode.file_name_len = file->inode.file_name.size();
	file->modified = true;

	return true;
}

template<class Config>
class SimpleFs2FlashPages
{
public:
	using base_t = SimpleFsNoDel<Config>;

protected:
	struct Component
	{
		enum class Type
		{
			inactive,
			active
		};

		std::optional<SimpleFsNoDel<Config>> fs;
		::SimpleFlashFs::FlashMemoryInterface *mem = nullptr;

		bool valid() const {
			return !(!fs);
		}

		Component() = default;
		Component( const Component & other ) = delete;
	};

	Component c1;
	Component c2;

	SimpleFsNoDel<Config> *fs = nullptr;
public:
	SimpleFs2FlashPages( ::SimpleFlashFs::FlashMemoryInterface *mem_interface1,
						 ::SimpleFlashFs::FlashMemoryInterface *mem_interface2 )
	{
		c1.mem = mem_interface1;
		c2.mem = mem_interface2;
	}

	~SimpleFs2FlashPages()
	{

	}

	bool init()
	{
		if( !init_fs() ) {
			return false;
		}

		if( should_cleanup() ) {
			cleanup();
		}

		CPPDEBUG( Tools::format( "active fs is: fs%d", c1.valid() ? 1 : 2 ) );

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
		init_fs( c1 );
		init_fs( c2 );

		if( !c1.valid() && !c2.valid() ) {
			if( !create( c1 ) ) {
				CPPDEBUG( "cannot create fs1" );
				return false;
			}
			if( !create( c2 ) ) {
				CPPDEBUG( "cannot create fs2" );
				return false;
			}
		} else if( c1.valid() && !c2.valid() ) {
			fs = &c1.fs.value();
			return true;
		} else if( c2.valid() && !c1.valid() ) {
			fs = &c2.fs.value();
			return true;
		}

		// both filesystems are valid choose the newest one
		uint64_t m1 = c1.fs->get_max_inode_number();
		uint64_t m2 = c2.fs->get_max_inode_number();

		if( m1 > m2 ) {
			c2.fs.reset();
			fs = &c1.fs.value();
		} else {
			c1.fs.reset();
			fs = &c2.fs.value();
		}

		return true;
	}

	bool init_fs( Component & component )
	{
		component.fs.emplace(component.mem);
		if( !component.fs->init() ) {
			component.fs.reset();
			return false;
		}

		return true;
	}

	bool create( Component & component )
	{
		component.mem->erase( 0, component.mem->size() );
		component.fs.emplace(component.mem);

		if( !component.fs->create() ) {
			component.fs.reset();
			return false;
		}

		if( !component.fs->init() ) {
			component.fs.reset();
			return false;
		}

		return true;
	}

	Component & get_component( Component::Type type )
	{
		switch( type )
		{
		case Component::Type::active:
			if( c1.valid() ) {
				return c1;
			} else {
				return c2;
			}
			break;

		case Component::Type::inactive:
			if( !c1.valid() ) {
				return c1;
			} else {
				return c2;
			}
			break;
		}

		throw std::out_of_range("should never been reached");
	}

	bool cleanup()
	{
		Component & inactive_component = get_component( Component::Type::inactive );
		Component & active_component = get_component( Component::Type::active );

		if( !create( inactive_component ) ) {
			CPPDEBUG( "cannot recreate fs!" );
			return false;
		}

		inactive_component.fs->set_max_inode_number( active_component.fs->get_max_inode_number() + 1 );

		typename Config::vector_type<typename Config::string_type> file_names;
		active_component.fs->list_files( file_names );

		for( auto & file_name : file_names ) {
			auto file_source = active_component.fs->open( file_name, std::ios_base::in | std::ios_base::binary );
			auto file_target = inactive_component.fs->open( file_name, std::ios_base::out | std::ios_base::trunc | std::ios_base::app | std::ios_base::binary );

			if( !copy( file_source, file_target ) ) {
				CPPDEBUG( "cannot recreate fs by copying files!" );
				return false;
			}
		}

		// active, becomes now inactive
		// inactive is already active, so nothing todo
		active_component.fs.reset();

		return true;
	}

	bool copy( base_t::base_t::FileHandle & source,  base_t::base_t::FileHandle & target )
	{
		std::size_t data_already_read = 0;
		typename Config::page_type buffer;

		for( size_t data_already_read = 0; data_already_read < source.file_size(); ) {
			const int32_t max_read = std::min( source.file_size() - data_already_read, buffer.capacity() );
			buffer.resize(max_read);

			size_t data_read = source.read(&buffer[0],buffer.size());
			buffer.resize(data_read);
			data_already_read += data_read;

			size_t data_written = target.write( buffer.data(), buffer.size() );
			if( data_written != buffer.size() ) {
				CPPDEBUG( "failed writing data" );
				return false;
			}
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
