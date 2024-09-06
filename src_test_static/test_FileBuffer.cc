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
#include "FFile.h"

using namespace Tools;

namespace {

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

