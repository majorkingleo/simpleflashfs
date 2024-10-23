/*
 * SimpleFlashFs2FlashPages.h
 *
 *  Created on: 29.08.2024
 *      Author: martin.oberzalek
 */
#pragma once

#include "SimpleFlashFsNoDel.h"
#include <span>

namespace SimpleFlashFs::static_memory {

template<class Config>
class SimpleFs2FlashPages
{
public:
	using base_t = SimpleFsNoDel<Config>;

	static constexpr std::string_view COPY_COMPLETED_FILE_NAME = ".FS_COPY_COMPLETED";
	static constexpr std::string_view FILESYSTEM_SEALED_FILE_NAME = ".FS_SEALED";
	static constexpr std::array<std::string_view,2> RESERVED_NAMES = { COPY_COMPLETED_FILE_NAME, FILESYSTEM_SEALED_FILE_NAME };

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
		const std::string_view name;

		bool valid() const {
			return !(!fs);
		}

		Component( const std::string_view name_ )
		: name( name_ )
		{}

		Component( const Component & other ) = delete;
	};


	class SpecialFilesFileFilter : public FileFilter<Config>
	{
	public:
		bool operator()( const base::FileHandle<Config,base::SimpleFlashFsBase<Config>> & handle ) override {
			if( handle.inode.attributes & static_cast<decltype(handle.inode.attributes)>(base::InodeAttribute::SPECIAL) ) {
				return false;
			}

			return true;
		}
	};

	Component c1;
	Component c2;
	SpecialFilesFileFilter special_file_filter;

	SimpleFsNoDel<Config> *fs = nullptr;
public:
	SimpleFs2FlashPages( ::SimpleFlashFs::FlashMemoryInterface *mem_interface1,
						 ::SimpleFlashFs::FlashMemoryInterface *mem_interface2 )
	: c1("fs1"),
	  c2("fs2")
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

		{
			Component & c = get_component(Component::Type::active);
			fs = &c.fs.value();
			CPPDEBUG( Tools::static_format<100>( "active fs is: %s", c.name ) );
		}


		if( should_cleanup() ) {
			cleanup();
		}

		{
			Component & c = get_component(Component::Type::active);
			fs = &c.fs.value();
			CPPDEBUG( Tools::static_format<100>( "active fs is: %s", c.name ) );
		}

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

		const typename SimpleFsNoDel<Config>::Stat & stat = fs->get_stat();
		const unsigned inode_usage = 100 - ( 100.0 / header.max_inodes * stat.free_inodes );

		CPPDEBUG( Tools::static_format<100>( "data_pages_usage: %d%%, inode_usage: %d%%", data_pages_usage, inode_usage ) );

		// cleanup makes only sense, if we have something to free from trash
		if( stat.trash_inodes > 0 || stat.trash_size > 0 ) {
			if( inode_usage > treshold_percentage ) {
				return true;
			}

			if( data_pages_usage > treshold_percentage ) {
				return true;
			}
		}

		return false;
	}

	SimpleFsNoDel<Config>* get_current_fs() {
		return fs;
	}

	bool recreate() {
		c1.fs.reset();
		c2.fs.reset();

		if( !create( c1 ) ) {
			CPPDEBUG( "cannot create fs1" );
			return false;
		}
		if( !create( c2 ) ) {
			CPPDEBUG( "cannot create fs2" );
			return false;
		}

		return init();
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
			return true;
		} else if( c2.valid() && !c1.valid() ) {
			return true;
		}

		auto fs1_file_copy_completed = c1.fs->open( COPY_COMPLETED_FILE_NAME, std::ios_base::in );
		auto fs1_file_sealed = c1.fs->open( FILESYSTEM_SEALED_FILE_NAME, std::ios_base::in );

		auto fs2_file_copy_completed = c2.fs->open( COPY_COMPLETED_FILE_NAME, std::ios_base::in );
		auto fs2_file_sealed = c2.fs->open( FILESYSTEM_SEALED_FILE_NAME, std::ios_base::in );

		/*
		CPPDEBUG( Tools::format( "FS1: %s %s",
				!fs1_file_copy_completed ? "" : COPY_COMPLETED_FILE_NAME,
				!fs1_file_sealed ? "" : FILESYSTEM_SEALED_FILE_NAME ));

		CPPDEBUG( Tools::format( "FS2: %s %s",
				!fs2_file_copy_completed ? "" : COPY_COMPLETED_FILE_NAME,
				!fs2_file_sealed ? "" : FILESYSTEM_SEALED_FILE_NAME ));
		*/

		// conversion from fs2 to fs1 completed
		if( fs1_file_copy_completed.valid() && fs2_file_sealed.valid() ) {
			c2.fs.reset();
			return true;
		}
		// conversion from fs1 to fs2 completed
		else if( fs2_file_copy_completed.valid() && fs1_file_sealed.valid() ) {
			c1.fs.reset();
			return true;
		}
		// copying from fs2 to fs1 didn't finished
		else if( !fs1_file_copy_completed && fs2_file_sealed.valid() ) {
			// stay on fs2
			c1.fs.reset();
			return true;
		}
		// copying from fs1 to fs2 didn't finished
		else if( !fs2_file_copy_completed && fs1_file_sealed.valid() ) {
			// stay on fs1
			c2.fs.reset();
			return true;
		}

		// both filesystems are valid choose the newest one
		// this happens on a fresh created filesystem
		uint64_t m1 = c1.fs->get_max_inode_number();
		uint64_t m2 = c2.fs->get_max_inode_number();

		if( m1 > m2 ) {
			c2.fs.reset();
		} else {
			c1.fs.reset();
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

		component.fs->set_file_filter(&special_file_filter);

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

		component.fs->set_file_filter(&special_file_filter);

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
		// CPPDEBUG( "============ cleaning up =====================" );

		Component & inactive_component = get_component( Component::Type::inactive );
		Component & active_component = get_component( Component::Type::active );

		// seal fs
		{
			auto fs_sealed = active_component.fs->open( FILESYSTEM_SEALED_FILE_NAME, std::ios_base::out | std::ios_base::trunc );

			if( !fs_sealed ) {
				CPPDEBUG( "cannot seal fs");
				return false;
			}

			fs_sealed.inode.attributes |= static_cast<decltype(fs_sealed.inode.attributes)>(base::InodeAttribute::SPECIAL);
			fs_sealed.modified = true;
		}

		if( !create( inactive_component ) ) {
			CPPDEBUG( "cannot recreate fs!" );
			return false;
		}

		inactive_component.fs->set_max_inode_number( active_component.fs->get_max_inode_number() + 1 );

		bool copy_error = false;

		active_component.fs->list_files( [&copy_error,&inactive_component,this]( auto & file_source ) {
			auto file_target = inactive_component.fs->open( file_source.inode.file_name,
					std::ios_base::out | std::ios_base::trunc | std::ios_base::app | std::ios_base::binary );

			if( !copy( file_source, file_target ) ) {
				copy_error = true;
				return false;
			}

			return true;
		} );

		if( copy_error ) {
			CPPDEBUG( "cannot recreate fs by copying files!" );
			return false;
		}

		{
			// copy process complete create a file to notify
			auto copy_completed = inactive_component.fs->open( COPY_COMPLETED_FILE_NAME, std::ios_base::out | std::ios_base::trunc | std::ios_base::app | std::ios_base::binary );
			copy_completed.inode.attributes |= static_cast<decltype(copy_completed.inode.attributes)>(base::InodeAttribute::SPECIAL);
			copy_completed.modified = true;

			if( !copy_completed ) {
				CPPDEBUG( "cannot create copy complete file name" );
				return false;
			}
		}

		// active, becomes now inactive
		// inactive is already active, so nothing to do
		active_component.fs.reset();

		fs = &inactive_component.fs.value();

		return true;
	}

	bool copy( base_t::base_t::FileHandle & source,  base_t::base_t::FileHandle & target )
	{
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


} // namespace SimpleFlashFs::static_memory
