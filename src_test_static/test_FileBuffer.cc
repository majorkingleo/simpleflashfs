/*
 * test_FileBufer.cc
 *
 *  Created on: 04.09.2024
 *      Author: martin.oberzalek
 */
#include "test_FileBuffer.h"
#include <fstream>
#include "../src_2face/SimpleFlashFsFileBuffer.h"
#include <filesystem>
#include <format.h>
#include <CpputilsDebug.h>
#include <xml.h>
#include <string.h>
#include <string_utils.h>

using namespace Tools;

namespace {

	class FFile : public SimpleFlashFs::FileInterface
	{
		mutable std::fstream file;
		std::string name;
		std::ios_base::openmode openmode;
		bool debug = true;

	public:
		FFile( const std::string & name_, std::ios_base::openmode openmode_ )
		: file( name_.c_str(), openmode_ ),
		  name( name_ ),
		  openmode( openmode_ )
		{
		}

		bool operator!() const override {
			return file.operator!();
		}

		std::size_t write( const std::byte *data, std::size_t size ) override  {
			if( debug ) {
				std::string s( std::string_view( reinterpret_cast<const char*>(data), size ) );
				s = substitude( s,  "\n", "\\n" );
				s = substitude( s,  std::string(1,'\0'), "\\0" );
				CPPDEBUG( format( "%s: writing '%s' at pos: %d", name, s, file.tellg() ) );
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
				CPPDEBUG( format( "%s: seeking to pos: %d", name, pos_ ) );
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

	class TestCaseFuncFileBuffer : public TestCaseBase<bool>
	{
		typedef std::function<void( SimpleFlashFs::FileInterface & file )> Func;
		Func func;
		std::size_t buffer_size;
		std::ios_base::openmode openmode;

	public:
		TestCaseFuncFileBuffer( const std::string & name,
								Func func_,
								std::size_t buffer_size_,
								std::ios_base::openmode openmode_ )
		: TestCaseBase<bool>( name, true ),
		  func( func_ ),
		  buffer_size( buffer_size_ ),
		  openmode( openmode_ )
		  {}

		bool run() override {

			std::string file_name_fstream          = format( ".%s.fstream.txt", name );
			std::string file_name_buffered_fstream = format( ".%s.buffered_fstream.txt", name );
			std::vector<std::byte> buffer(buffer_size);

			std::filesystem::remove(file_name_fstream);
			std::filesystem::remove(file_name_buffered_fstream);

			{
				FFile f_fstream_a( file_name_fstream, openmode );

				if( !f_fstream_a ) {
					CPPDEBUG( format( "cannot open file: '%s'", f_fstream_a.get_file_name() ) );
					return false;
				}

				FFile f_fstream_b( file_name_buffered_fstream, openmode );

				if( !f_fstream_b ) {
					CPPDEBUG( format( "cannot open file: '%s'", f_fstream_b.get_file_name() ) );
					return false;
				}


				SimpleFlashFs::FileBuffer file_buffered_fstream( f_fstream_b, buffer );

				func( f_fstream_a );
				func( file_buffered_fstream );
			}

			std::string s1;
			if( !XML::read_file(file_name_fstream,s1) ) {
				CPPDEBUG( format( "cannot read file: '%s'", file_name_fstream ) );
				return false;
			}

			std::string s2;
			if( !XML::read_file(file_name_buffered_fstream,s2) ) {
				CPPDEBUG( format( "cannot read file: '%s'", file_name_buffered_fstream ) );
				return false;
			}

			if( s1 != s2 ) {
				CPPDEBUG( "content differs" );
				return false;
			}

			return true;
		}
	};


	class TestCaseFuncOneFileBuffer : public TestCaseBase<bool>
	{
		typedef std::function<bool( SimpleFlashFs::FileBuffer & file )> Func;
		Func func;
		std::size_t buffer_size;
		std::ios_base::openmode openmode;

	public:
		TestCaseFuncOneFileBuffer( const std::string & name,
								Func func_,
								std::size_t buffer_size_,
								std::ios_base::openmode openmode_ )
		: TestCaseBase<bool>( name, true ),
		  func( func_ ),
		  buffer_size( buffer_size_ ),
		  openmode( openmode_ )
		  {}

		bool run() override {

			std::string file_name_buffered_fstream = format( ".%s.buffered_fstream.txt", name );
			std::vector<std::byte> buffer(buffer_size);

			std::filesystem::remove(file_name_buffered_fstream);

			{
				FFile f_fstream_b( file_name_buffered_fstream, openmode );

				if( !f_fstream_b ) {
					CPPDEBUG( format( "cannot open file: '%s'", f_fstream_b.get_file_name() ) );
					return false;
				}


				SimpleFlashFs::FileBuffer file_buffered_fstream( f_fstream_b, buffer );

				return func( file_buffered_fstream );
			}

			return true;
		}
	};

	std::span<const std::byte> to_span( const char* data ) {
		return std::span<const std::byte>(reinterpret_cast<const std::byte*>(data), strlen(data)+1);
	}

	std::span<const std::byte> to_span( const std::string_view & data ) {
		return std::span<const std::byte>(reinterpret_cast<const std::byte*>(data.data()), data.size() );
	}

	std::string to_string( const std::span<std::byte> & data ) {
		return reinterpret_cast<const char*>(data.data());
	}


} // namespace


std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_1()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		std::string text = "eins zwei";
		auto s = to_span( text );
		file.write( s.data(), s.size() );

	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}


std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_2()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		std::string text = "11111111111111111111111111111\n"
				"22222222222222222222222222222\n"
				"33333333333333333333333333333\n";
		auto s = to_span( text );
		file.write( s.data(), s.size() );

	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}

std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_3()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		w( "1111111111111111111111\n" );
		w( "2222222222222222222222\n" );
		w( "3333333333333333333333\n" );

	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}


std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_4()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		w( "111111111\n" );
		w( "222222222\n" );
		w( "333333333\n" );

	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}

std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_5()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		w( "111111111\n" );
		w( "222222222\n" );
		w( "333333333\n" );

		file.seek(10);

		w( "444444444\n" );
	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}


std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_6()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		w( "111111111\n" );
		w( "222222222\n" );
		w( "333333333\n" );

		for( unsigned i = 1; i < 9; i += 2 ) {
			file.seek(i);
			w( "2" );
		}
	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}


std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_7()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		auto r = [&file]( std::size_t len ) {
			std::vector<std::byte> buf(len);

			size_t ret_len = file.read( buf.data(), buf.size() );
			return std::string( reinterpret_cast<const char*>(buf.data()), ret_len );
		};

		w( "111111111\n" );
		w( "222222222\n" );
		w( "333333333\n" );

		file.seek(10);

		std::string s = r( 9 );

		if( s != "222222222" ) {
			CPPDEBUG( format( "s: '%s", s ) );
			throw std::out_of_range("invalid read");
		}

		file.seek(20);

		w( s );
	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}

std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_8()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		auto r = [&file]( std::size_t len ) {
			std::vector<std::byte> buf(len);

			size_t ret_len = file.read( buf.data(), buf.size() );
			return std::string( reinterpret_cast<const char*>(buf.data()), ret_len );
		};

		w( "111111111\n" );
		w( "222222222\n" );
		w( "333333333\n" );

		file.seek(10);

		std::string s = r( 9 );

		if( s != "222222222" ) {
			CPPDEBUG( format( "s: '%s", s ) );
			throw std::out_of_range("invalid read");
		}

		file.seek(20);

		w( s );
	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::app );
}



std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_9()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		w( "111111\n" );
		w( "222222\n" );
		w( "333333\n" );
	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::app );
}


std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_10()
{
	auto test_func = []( SimpleFlashFs::FileInterface & file ) {

		auto w = [&file]( const std::string & text ) {
			auto s = to_span( text );
			file.write( s.data(), s.size() );
		};

		w( "111111\n" );
		w( "222222\n" );
		w( "333333\n" );
	};

	return std::make_shared<TestCaseFuncFileBuffer>(__FUNCTION__, test_func,20,std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}

std::shared_ptr<TestCaseBase<bool>> test_case_filebuffer_11()
{
	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		std::vector<const char*> lines = {
				"111111\n" ,
				"222222\n",
				"333333\n"
		};

		file.write( lines.at(0) );
		file.write( std::string( lines.at(1) ) );
		file.write( std::string_view( lines.at(2) ) );

		file.seek(0);

		std::vector<std::string> erg;

		while( !file.eof() ) {
			auto s = file.get_line<static_string<50>>();
			if( s ) {
				CPPDEBUG( format( "got line: '%s'", *s ) );
				erg.push_back( std::string( *s ) );
			} else {
				CPPDEBUG( "no data" );
			}
		}

		if( lines.size() != erg.size() ) {
			CPPDEBUG( "lines differ" );
			return false;
		}

		for( unsigned i = 0; i < lines.size(); ++i ) {
			if( strip(lines[i]) != erg[i] ) {
				CPPDEBUG( "lines differ" );
				return false;
			}
		}

		return true;
	};

	return std::make_shared<TestCaseFuncOneFileBuffer>( __FUNCTION__, test_func, 20, std::ios_base::in | std::ios_base::out | std::ios_base::trunc );
}

