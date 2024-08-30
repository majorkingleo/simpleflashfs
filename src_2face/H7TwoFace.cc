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
#include <format.h>
#include <stderr_exception.h>

using namespace Tools;


uint32_t ConfigH7::crc32( const std::byte *bytes, size_t len )
{
	return crcFast( reinterpret_cast<unsigned const char*>(bytes), len );
}


namespace {

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

		virtual void seek( std::size_t pos ) override {
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
	};

private:
	std::optional<SimpleFlashFs::static_memory::SimpleFs2FlashPages<ConfigH7>> fs;
	SimpleFlashFs::FlashMemoryInterface* mem1;
	SimpleFlashFs::FlashMemoryInterface* mem2;
	std::optional<File> file;

public:
	H7TwoFaceImpl(SimpleFlashFs::FlashMemoryInterface *mem1, SimpleFlashFs::FlashMemoryInterface *mem2 )
	{
		fs.emplace(mem1,mem2);

		if( !fs->init() ) {
			throw STDERR_EXCEPTION("cannot create fs");
		}
	}

	~H7TwoFaceImpl()
	{

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

} // namespace


H7TwoFace::file_handle_t H7TwoFace::open( const std::string_view & name, std::ios_base::openmode mode )
{
	for( const auto & reserved_name : SimpleFlashFs::static_memory::SimpleFs2FlashPages<ConfigH7>::RESERVED_NAMES ) {
		if( reserved_name == name ) {
			CPPDEBUG( "cannot open or create a special file" );
			return {};
		}
	}


	if( fs_impl ) {
		CPPDEBUG( "An other FS instance is already open" );
		return {};
	}

	fs_impl.emplace(fs_mem1.value(),fs_mem2.value());
	H7TwoFaceImpl::File & f = fs_impl->open( &fs_impl, name, mode );

	if( !f ) {
		fs_impl.reset();
		return {};
	}

	return H7TwoFace::file_handle_t(&f);
}

std::span<std::string_view> H7TwoFace::list_files()
{
	if( fs_impl ) {
		CPPDEBUG( "An other FS instance is already open" );
		return {};
	}

	fs_impl.emplace(fs_mem1.value(),fs_mem2.value());

	static ConfigH7::vector_type<ConfigH7::string_type> file_list;
	static ConfigH7::vector_type<std::string_view> v_file_list;
	file_list.clear();
	v_file_list.clear();

	fs_impl->get_fs().get_current_fs()->list_files( file_list );
	fs_impl.reset();

	v_file_list.insert( v_file_list.end(), file_list.begin(), file_list.end() );

	std::span<std::string_view> ret( v_file_list.data(), v_file_list.size() );

	return 	ret;
}

void H7TwoFace::set_memory_interface( SimpleFlashFs::FlashMemoryInterface *mem1, SimpleFlashFs::FlashMemoryInterface *mem2 )
{
	fs_mem1 = mem1;
	fs_mem2 = mem2;
}



