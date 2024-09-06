/*
 * FFile.h
 *
 *  Created on: 06.09.2024
 *      Author: martin.oberzalek
 */
#include <fstream>
#include "../src_2face/SimpleFlashFsFileBuffer.h"
#include <string_utils.h>
#include <CpputilsDebug.h>
#include <filesystem>

class FFile : public SimpleFlashFs::FileInterface
{
	mutable std::fstream file;
	std::string name;
	std::ios_base::openmode openmode;
	bool debug = true;

public:
	FFile( const std::string & name_, std::ios_base::openmode openmode_, bool debug_ = true )
	: file( name_.c_str(), openmode_ ),
	  name( name_ ),
	  openmode( openmode_ ),
	  debug( debug_ )
	{
	}

	bool operator!() const override {
		return file.operator!();
	}

	std::size_t write( const std::byte *data, std::size_t size ) override  {
		if( debug ) {
			std::string s( std::string_view( reinterpret_cast<const char*>(data), size ) );
			s = Tools::substitude( s,  "\n", "\\n" );
			s = Tools::substitude( s,  std::string(1,'\0'), "\\0" );
			CPPDEBUG( Tools::format( "%s: writing '%s' at pos: %d", name, s, file.tellg() ) );
		}

		file.write( reinterpret_cast<const char*>(data), size );
		return size;
	}

	std::size_t read( std::byte *data, std::size_t size ) override {
		file.read( reinterpret_cast<char*>(data), size );
		return file.gcount();
	}

	bool flush() override {
		file.flush();
		return !file.operator!();
	}

	std::size_t tellg() const override {
		return file.tellg();
	}

	std::size_t file_size() const override {
		return std::filesystem::file_size(name);
	}

	bool eof() const override {
		if( file.eof() ) {
			return true;
		}

		if( tellg() >= file_size() ) {
			return true;
		}

		return false;
	}

	void seek( std::size_t pos_ ) override {
		if( debug ) {
			CPPDEBUG( Tools::format( "%s: seeking to pos: %d", name, pos_ ) );
		}

		file.seekg(pos_);
		file.seekp(pos_);
	}

	 bool delete_file() override {
		 file.close();
		 return std::filesystem::remove(name);
	 }

	 bool rename_file( const std::string_view & new_file_name ) override {
		 std::filesystem::rename(name, new_file_name );
		 return true;
	 }

	 bool valid() const {
		return !operator!();
	 }

	 std::string_view get_file_name() const override {
		 return name;
	 }

	 bool is_append_mode() const override {
		 return openmode & std::ios_base::app;
	 }
};

class TestCaseFuncOneFileBuffer : public TestCaseBase<bool>
{
	typedef std::function<bool( SimpleFlashFs::FileBuffer & file )> Func;
	Func func;
	std::size_t buffer_size;
	std::ios_base::openmode openmode;
	bool debug_file_buffer  = true;

public:
	TestCaseFuncOneFileBuffer( const std::string & name,
							Func func_,
							std::size_t buffer_size_,
							std::ios_base::openmode openmode_,
							bool debug_file_buffer_ = true )
	: TestCaseBase<bool>( name, true ),
	  func( func_ ),
	  buffer_size( buffer_size_ ),
	  openmode( openmode_ ),
	  debug_file_buffer( debug_file_buffer_ )
	  {}

	bool run() override {

		std::string file_name_buffered_fstream = Tools::format( ".%s.buffered_fstream.txt", name );
		std::vector<std::byte> buffer(buffer_size);

		std::filesystem::remove(file_name_buffered_fstream);

		{
			FFile f_fstream_b( file_name_buffered_fstream, openmode, debug_file_buffer );

			if( !f_fstream_b ) {
				CPPDEBUG( Tools::format( "cannot open file: '%s'", f_fstream_b.get_file_name() ) );
				return false;
			}


			SimpleFlashFs::FileBuffer file_buffered_fstream( f_fstream_b, buffer );

			return func( file_buffered_fstream );
		}

		return true;
	}
};
