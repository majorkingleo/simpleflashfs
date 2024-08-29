/**
 * SimpleFlashFs test case initialisation
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#include "static_fs_h7.h"
#include <CpputilsDebug.h>
#include <format.h>
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>

using namespace Tools;

uint32_t ConfigH7::crc32( const std::byte *bytes, size_t len )
{
	return crcFast( reinterpret_cast<unsigned const char*>(bytes), len );
}

namespace {

class H7TwoFaceImplPc
{
public:
	class File : public SimpleFlashFs::FileInterface
	{
		std::optional<H7TwoFaceImplPc> *fs_instance;
		std::optional<SimpleFs2FlashPages<ConfigH7>::base_t::file_handle_t> file;

	public:
		File( std::optional<H7TwoFaceImplPc> *fs_instance_ )
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
	const std::string file1 = "test_h7_page1.bin";
	const std::string file2 = "test_h7_page2.bin";

	std::optional<SimpleFs2FlashPages<ConfigH7>> fs;
	std::optional<::SimpleFlashFs::SimPc::SimFlashFsFlashMemoryInterface> mem1;
	std::optional<::SimpleFlashFs::SimPc::SimFlashFsFlashMemoryInterface> mem2;
	std::optional<File> file;

public:
	H7TwoFaceImplPc()
	{
		mem1.emplace(file1,SFF_MAX_SIZE);
		mem2.emplace(file2,SFF_MAX_SIZE);
		fs.emplace(&mem1.value(),&mem2.value());

		if( !fs->init() ) {
			throw STDERR_EXCEPTION( Tools::format("cannot create fs (%s,%s)", file1, file2 ) );
		}
	}

	~H7TwoFaceImplPc()
	{

	}

	File & open( std::optional<H7TwoFaceImplPc> * fs_instance, const std::string_view & name, std::ios_base::openmode mode ) {
		 file.emplace( fs_instance );
		 file->open( name, mode );
		 return file.value();
	}

	SimpleFs2FlashPages<ConfigH7> & get_fs() {
		return fs.value();
	}
};

static std::optional<H7TwoFaceImplPc> fs_impl;

} // namespace

H7TwoFace::file_handle_t H7TwoFace::open( const std::string_view & name, std::ios_base::openmode mode )
{
	if( fs_impl ) {
		CPPDEBUG( "An other FS instance is already open" );
		return {};
	}

	fs_impl.emplace();
	H7TwoFaceImplPc::File & f = fs_impl->open( &fs_impl, name, mode );

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

	fs_impl.emplace();

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

