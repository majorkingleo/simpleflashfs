/*
 * H7TwoFace.cc
 *
 *  Created on: 29.08.2024
 *      Author: martin.oberzalek
 */
#include "H7TwoFace.h"
#include "H7TwoFaceConfig.h"
#include "SimpleFlashFs2FlashPages.h"
#include <CpputilsDebug.h>
#include <static_debug_exception.h>

using namespace Tools;

namespace {

static std::function<void(bool)> lock_unlock_instance_cb = []( bool ) {};

class H7TwoFaceImpl
{
public:
	class File : public SimpleFlashFs::FileInterface
	{
		std::optional<H7TwoFaceImpl> *fs_instance;
		std::optional<SimpleFlashFs::static_memory::SimpleFs2FlashPages<ConfigH7>::base_t::file_handle_t> file;

	public:
		File( std::optional<H7TwoFaceImpl> *fs_instance_ )
		: fs_instance( fs_instance_ )
		{

		}

		~File() {

			if( fs_instance ) {
				file.reset();

				// otherwise the destructor is called twice
				auto save_ptr = fs_instance;
				fs_instance = nullptr;
				save_ptr->reset();
			}
		}

		bool operator!() const override {
			return file->operator!();
		}

		virtual std::size_t write( const std::byte *data, std::size_t size ) override {
			return file->write( data, size );
		}

		virtual std::size_t read( std::byte *data, std::size_t size ) override {
			return file->read( data, size );
		}

		virtual bool flush() override {
			return file->flush();
		}

		virtual std::size_t tellg() const override {
			return file->tellg();
		}

		virtual std::size_t file_size() const override {
			return file->file_size();
		}

		virtual bool eof() const override {
			return file->eof();
		}

		virtual bool seek( std::size_t pos ) override {
			return file->seek(pos);
		}


		void open( const std::string_view & name, std::ios_base::openmode mode ) {
			 file.emplace( (*fs_instance)->get_fs().open(name, mode) );
		}

		bool delete_file() override {
			return file->delete_file();
		}

		bool rename_file( const std::string_view & new_file_name ) override {
			return file->rename_file( new_file_name );
		}

		std::string_view get_file_name() const override {
			return file->get_file_name();
		}

		bool is_append_mode() const override {
			return file->is_append_mode();
		}
	};

private:
	std::optional<SimpleFlashFs::static_memory::SimpleFs2FlashPages<ConfigH7>> fs;
	SimpleFlashFs::FlashMemoryInterface* mem1;
	SimpleFlashFs::FlashMemoryInterface* mem2;
	std::optional<File> file;

public:
	/**
	 * do_init = false, makes only sense on reformatting the fs
	 */
	H7TwoFaceImpl(SimpleFlashFs::FlashMemoryInterface *mem1, SimpleFlashFs::FlashMemoryInterface *mem2, bool do_init = true )
	{
		fs.emplace(mem1,mem2);

		if( do_init ) {
			if( !fs->init() ) {
				throw STATIC_DEBUG_EXCEPTION("cannot create fs");
			}
		}
	}

	~H7TwoFaceImpl()
	{
		lock_unlock_instance_cb( false );
	}

	File & open( std::optional<H7TwoFaceImpl> * fs_instance, const std::string_view & name, std::ios_base::openmode mode ) {
		 file.emplace( fs_instance );
		 file->open( name, mode );
		 return file.value();
	}

	SimpleFlashFs::static_memory::SimpleFs2FlashPages<ConfigH7> & get_fs() {
		return fs.value();
	}
};

static std::optional<SimpleFlashFs::FlashMemoryInterface*> fs_mem1;
static std::optional<SimpleFlashFs::FlashMemoryInterface*> fs_mem2;
static std::optional<H7TwoFaceImpl> fs_impl;
static std::function<uint32_t(const std::byte* data, size_t len)> fs_crc32_func = [](const std::byte* data, size_t len) {
	return crcFast( reinterpret_cast<unsigned char const*>(data), len );
};

class AutoFreeFs
{
	bool is_disabled = false;
public:
	~AutoFreeFs() {
		if( !is_disabled ) {
			if( fs_impl ) {
				fs_impl.reset();
			}
		}
	}

	void disable() {
		is_disabled = true;
	}
};

} // namespace


void H7TwoFace::set_lock_unlock_callback( std::function<void(bool)> lock_unlock_cb )
{
	lock_unlock_instance_cb = lock_unlock_cb;
}

uint32_t ConfigH7::crc32( const std::byte *bytes, size_t len )
{
	return fs_crc32_func( bytes, len );
}


H7TwoFace::file_handle_t H7TwoFace::open( const std::string_view & name, std::ios_base::openmode mode )
{
	for( const auto & reserved_name : SimpleFlashFs::static_memory::SimpleFs2FlashPages<ConfigH7>::RESERVED_NAMES ) {
		if( reserved_name == name ) {
			CPPDEBUG( "cannot open or create a special file" );
			return {};
		}
	}

	lock_unlock_instance_cb( true );

	if( fs_impl ) {
		CPPDEBUG( "An other FS instance is already open" );
		lock_unlock_instance_cb( false );
		return {};
	}

	AutoFreeFs autofree;

	fs_impl.emplace(fs_mem1.value(),fs_mem2.value());

	if( (mode & std::ios_base::trunc) && fs_impl->get_fs().get_current_fs()->get_stat().free_inodes > 2) {
		// ok
	} else if( mode == std::ios_base::in ) {
		// read only is also ok
	} else if( fs_impl->get_fs().get_current_fs()->get_stat().free_inodes <= 3 ) {
		CPPDEBUG( "no free inode left" );
		return {};
	}

	H7TwoFaceImpl::File & f = fs_impl->open( &fs_impl, name, mode );

	if( !f ) {
		return {};
	}

	autofree.disable();
	return H7TwoFace::file_handle_t(&f);
}

std::span<std::string_view> H7TwoFace::list_files()
{
	lock_unlock_instance_cb( true );

	if( fs_impl ) {
		CPPDEBUG( "An other FS instance is already open" );
		lock_unlock_instance_cb( false );
		return {};
	}

	fs_impl.emplace(fs_mem1.value(),fs_mem2.value());

	AutoFreeFs autofree;

	static ConfigH7::vector_type<std::string_view> v_file_list;
	v_file_list.clear();

	auto & x_file_list = v_file_list;
	auto fs = fs_impl->get_fs().get_current_fs();

	fs->list_files( [&x_file_list,&fs]( auto & file_handle ) {
		x_file_list.push_back( fs->get_inode_file_name_mapped(file_handle).value() );
		return true;
	} );

	std::span<std::string_view> ret( v_file_list.data(), v_file_list.size() );

	return 	ret;
}

void H7TwoFace::set_memory_interface( SimpleFlashFs::FlashMemoryInterface *mem1, SimpleFlashFs::FlashMemoryInterface *mem2 )
{
	fs_mem1 = mem1;
	fs_mem2 = mem2;
}

H7TwoFace::Stat H7TwoFace::get_stat()
{
	lock_unlock_instance_cb( true );

	if( fs_impl ) {
		CPPDEBUG( "An other FS instance is already open" );
		lock_unlock_instance_cb( false );
		return {};
	}

	fs_impl.emplace(fs_mem1.value(),fs_mem2.value());

	AutoFreeFs autofree;

	const auto & stat = fs_impl->get_fs().get_current_fs()->get_stat();
	const auto & header = fs_impl->get_fs().get_current_fs()->get_header();

	Stat ret;
	ret.free_inodes = stat.free_inodes;
	ret.largest_file_size = stat.largest_file_size;
	ret.trash_inodes = stat.trash_inodes;
	ret.trash_size = stat.trash_size;
	ret.used_inodes = stat.used_inodes;

	std::size_t count = 0;

	fs_impl->get_fs().get_current_fs()->list_files( [&count]( auto & file_handle ) { count++; return true; } );

	ret.number_of_files = count;

	// some special files required for copying fs to second flash page
	// minus 1 inode to delete something
	ret.max_number_of_files = header.max_inodes  - 1 - SimpleFlashFs::static_memory::SimpleFs2FlashPages<ConfigH7>::RESERVED_NAMES.size();
	ret.max_file_size = fs_impl->get_fs().get_current_fs()->get_max_file_size();
	ret.max_path_len = header.max_path_len;
	ret.free_space = (fs_impl->get_fs().get_current_fs()->get_number_of_free_data_pages() * header.page_size) + ret.trash_size;

	return ret;
}

void H7TwoFace::set_crc32_func( std::function<uint32_t(const std::byte* data, size_t len)> crc32_func )
{
	fs_crc32_func = crc32_func;
}


bool H7TwoFace::recreate()
{
	lock_unlock_instance_cb( true );
	if( fs_impl ) {
		CPPDEBUG( "An other FS instance is already open" );
		lock_unlock_instance_cb( false );
		return false;
	}

	fs_impl.emplace(fs_mem1.value(),fs_mem2.value(), false);

	AutoFreeFs autofree;

	return fs_impl->get_fs().recreate();
}





