/*
 * SimpleFlashFs2FlashPages.h
 *
 *  Created on: 29.08.2024
 *      Author: martin.oberzalek
 */
#pragma once

#include "SimpleFlashFsNoDel.h"

namespace SimpleFlashFs::static_memory {

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

		const typename SimpleFsNoDel<Config>::Stat & stat = fs->get_stat();
		const unsigned inode_usage = 100 - ( 100.0 / header.max_inodes * stat.free_inodes );

		CPPDEBUG( Tools::format( "data_pages_usage: %d%%, inode_usage: %d%%", data_pages_usage, inode_usage ) );

		if( inode_usage > treshold_percentage ) {
			return true;
		}

		if( data_pages_usage > treshold_percentage ) {
			return true;
		}

		return false;
	}

	SimpleFsNoDel<Config>* get_current_fs() {
		return fs;
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
