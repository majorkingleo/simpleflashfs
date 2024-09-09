/*
 * test_SimpleIni.cc
 *
 *  Created on: 06.09.2024
 *      Author: martin.oberzalek
 */
#include "test_SimpleIni.h"
#include <CpputilsDebug.h>
#include "FFile.h"
#include "../src_2face/SimpleIni.h"
#include <xml.h>
#include <fstream>

using namespace SimpleFlashFs;
using namespace Tools;

namespace {

class TestCaseFuncWriteIni : public TestCaseBase<bool>
{
	typedef std::function<bool( SimpleFlashFs::FileBuffer & file )> Func;
	Func func;
	std::size_t buffer_size;
	std::ios_base::openmode openmode;
	bool debug_file_buffer  = true;
	std::string expected_ini_text;

public:
	TestCaseFuncWriteIni( const std::string & name,
							Func func_,
							std::size_t buffer_size_,
							std::ios_base::openmode openmode_,
							bool debug_file_buffer_,
							std::string expected_ini_text_ )
	: TestCaseBase<bool>( name, true ),
	  func( func_ ),
	  buffer_size( buffer_size_ ),
	  openmode( openmode_ ),
	  debug_file_buffer( debug_file_buffer_ ),
	  expected_ini_text( expected_ini_text_ )
	  {}

	bool run() override {

		std::string file_name_buffered_fstream = Tools::format( ".%s.buffered_fstream.ini", name );
		std::vector<std::byte> buffer(buffer_size);
		std::span<std::byte> sbuffer(buffer);

		std::filesystem::remove(file_name_buffered_fstream);

		{
			FFile f_fstream_b( file_name_buffered_fstream, openmode, debug_file_buffer );

			if( !f_fstream_b ) {
				CPPDEBUG( Tools::format( "cannot open file: '%s'", f_fstream_b.get_file_name() ) );
				return false;
			}


			SimpleFlashFs::FileBuffer file_buffered_fstream( f_fstream_b, sbuffer );

			if( !func( file_buffered_fstream ) ) {
				return false;
			}
		}

		std::string data;
		if( !XML::read_file( file_name_buffered_fstream, data ) ) {
			return false;
		}

		if( data != expected_ini_text ) {
			CPPDEBUG( "output text differs ");
			std::string expected_out_file_name = file_name_buffered_fstream + ".expected.ini";
			std::ofstream out( expected_out_file_name, std::ios_base::trunc | std::ios_base::out );
			if( !out ) {
				CPPDEBUG( format( "cannot open file '%s'", expected_out_file_name ) );
				return false;
			}

			out << expected_ini_text;

			return false;
		}

		return true;
	}
};


bool write_default_ini1( SimpleFlashFs::FileBuffer & file )
{
	if( file.write(
			"[section1]\n" \
			"  key1=value1\n" \
			"  key2=value2\n" \
			"\n"
			"[section2]\n" \
			"   \n"
			"  key1 = value1 \t\n" \
			" \t key2= value2 \n" \
			"[section3 ]\n" \
			"# comment 1   \n"
			"  key3 = value3 \t\n" \
			"; comment = 2   \n"
			"  key4= value4 \n" \
			"[section4 ]\n" \
			"  key1=nolineend" ) <= 0 ) {
		return false;
	}

	return true;
}

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_1()
{
	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		write_default_ini1( file );

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.read("section1","key1", value ) || value != "value1" ) {
			CPPDEBUG( format( "value: %d", value ) );
			CPPDEBUG( "section1/key1 not found" );
			return false;
		}

		if( !ini.read("section1","key2", value ) || value != "value2" ) {
			CPPDEBUG( "section1/key2 not found" );
			return false;
		}

		if( ini.read("section1","key3", value ) ) {
			CPPDEBUG( "section1/key3 found" );
			return false;
		}

		if( !ini.read("section2","key1", value ) || value != "value1" ) {
			CPPDEBUG( format( "section2/key1 not found (value:'%s')", value ) );
			return false;
		}

		if( !ini.read("section2","key2", value ) || value != "value2" ) {
			CPPDEBUG( format( "section2/key2 not found (value:'%s')", value ) );
			return false;
		}

		if( !ini.read("section3","key3", value ) || value != "value3" ) {
			CPPDEBUG( "section3/key3 not found" );
			return false;
		}

		if( !ini.read("section3","key4", value ) || value != "value4" ) {
			CPPDEBUG( "section3/key4 not found" );
			return false;
		}

		if( !ini.read("section4","key1", value ) || value != "nolineend" ) {
			CPPDEBUG( "section4/key1 not found" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncOneFileBuffer>(__FUNCTION__, test_func, 20,
			std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false );
}

namespace {

bool write_default_ini2( SimpleFlashFs::FileBuffer & file )
{
	if( file.write(
			"[section4 ]\n" \
			"  key1=nolineend" ) <= 0 ) {
		return false;
	}

	return true;
}

}

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_2()
{
	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		write_default_ini2( file );

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.read("section4","key1", value ) || value != "nolineend" ) {
			CPPDEBUG( format( "section4/key1 not found (value:'%s')", value ) );
			return false;
		}
		return true;
	};

	return std::make_shared<TestCaseFuncOneFileBuffer>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false );
}

namespace {

bool write_default_ini3( SimpleFlashFs::FileBuffer & file )
{
	if( file.write(
			"[section4 ]\n" \
			"  key1=" ) <= 0 ) {
		return false;
	}

	return true;
}

}


std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_3()
{
	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		write_default_ini3( file );

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.read("section4","key1", value ) || value != "" ) {
			CPPDEBUG( format( "section4/key1 not found (value:'%s')", value ) );
			return false;
		}
		return true;
	};

	return std::make_shared<TestCaseFuncOneFileBuffer>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false );
}



namespace {

bool write_default_ini4( SimpleFlashFs::FileBuffer & file )
{
	if( file.write(
			"[section4 ]\n" \
			"  key1 " ) <= 0 ) {
		return false;
	}

	return true;
}

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_4()
{
	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		write_default_ini4( file );

		SimpleIni ini( file );

		std::string_view value;

		if( ini.read("section4","key1", value ) ) {
			CPPDEBUG( format( "section4/key1 found (value:'%s')", value ) );
			return false;
		}
		return true;
	};

	return std::make_shared<TestCaseFuncOneFileBuffer>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false );
}



namespace {

bool write_default_ini5( SimpleFlashFs::FileBuffer & file )
{
	if( file.write(
			" key1 " ) <= 0 ) {
		return false;
	}

	return true;
}

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_5()
{
	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		write_default_ini5( file );

		SimpleIni ini( file );

		std::string_view value;

		if( ini.read("section4","key1", value ) ) {
			CPPDEBUG( format( "section4/key1 found (value:'%s')", value ) );
			return false;
		}
		return true;
	};

	return std::make_shared<TestCaseFuncOneFileBuffer>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false );
}

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_1()
{
	std::string expected_text =
			"[section1]\n" \
			"\tkey1 = value1\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.write("section1","key1", "value1" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}
		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false, expected_text );
}


std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_2()
{
	std::string expected_text =
			"[section1]\n" \
			"\tkey1 = value1\n" \
			"\n" \
			"[section2]\n" \
			"\tkey2 = value2\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.write("section1","key1", "value1" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}

		if( !ini.write("section2","key2", "value2" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, true, expected_text );
}


std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_3()
{
	std::string expected_text =
			"[section1]\n" \
			"#\tcomment1\n"
			"\tkey1 = value1\n" \
			"\n" \
			"[section2]\n" \
			"#\tcomment 2\n"
			"\tkey2 = value2\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.write("section1","key1", "value1", "comment1" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}

		if( !ini.write("section2","key2", "value2", "comment 2" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, true, expected_text );
}

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_4()
{
	std::string expected_text =
			"[section1]\n" \
			"\tkey1 = value1\n" \
			"\tkey3 = value3\n" \
			"\n" \
			"[section2]\n" \
			"\tkey2 = value2\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		std::string init_text =
				"[section1]\n" \
				"\tkey1 = value1\n" \
				"\n" \
				"[section2]\n" \
				"\tkey2 = value2\n";


		if( !file.write( init_text ) ) {
			return false;
		}

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.write("section1","key3", "value3" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false, expected_text );
}



std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_5()
{
	std::string expected_text =
			"[section1]\n" \
			"\tkey1 = value1\n" \
			"#\tcomment 3\n"
			"\tkey3 = value3\n" \
			"\n" \
			"[section2]\n" \
			"\tkey2 = value2\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		std::string init_text =
				"[section1]\n" \
				"\tkey1 = value1\n" \
				"\n" \
				"[section2]\n" \
				"\tkey2 = value2\n";


		if( !file.write( init_text ) ) {
			return false;
		}

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.write("section1","key3", "value3", "comment 3" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false, expected_text );
}

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_6()
{
	std::string expected_text =
			"[section1]\n" \
			"#\tcomment 3\n"
			"\tkey3 = value3\n" \
			"\n" \
			"[section2]\n" \
			"\tkey2 = value2\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {

		std::string init_text =
				"[section1]\n" \
				"\n" \
				"[section2]\n" \
				"\tkey2 = value2\n";


		if( !file.write( init_text ) ) {
			return false;
		}

		SimpleIni ini( file );

		std::string_view value;

		if( !ini.write("section1","key3", "value3", "comment 3" ) ) {
			CPPDEBUG( "writing failed" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false, expected_text );
}


std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_7()
{
	std::string expected_text =
			"[section1]\n" \
			"\tkey1 = value1\n" \
			"#\tcomment3\n"
			"\tkey3 = value3\n" \
			"\n" \
			"[section2]\n" \
			"\tkey2 = value2\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {


		SimpleIni ini( file );

		if( !ini.write( "section1", "key1", "value1"  ) ) {
			CPPDEBUG( "writing key1 failed" );
			return false;
		}

		if( !ini.write( "section2", "key2", "value2"  ) ) {
			CPPDEBUG( "writing key2 failed" );
			return false;
		}

		if( !ini.write( "section1", "key3", "value3", "comment3"  ) ) {
			CPPDEBUG( "writing key3 failed" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 20,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false, expected_text );
}

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_write_8()
{
	std::string expected_text =
			"[section1]\n" \
			"\tkey1 = value1\n" \
			"#\tcomment3\n"
			"\tkey3 = value3\n" \
			"\n" \
			"[section2]\n" \
			"\tkey2 = value2\n";

	auto test_func = []( SimpleFlashFs::FileBuffer & file ) {


		SimpleIni ini( file );

		if( !ini.write( "section1", "key1", "value1"  ) ) {
			CPPDEBUG( "writing key1 failed" );
			return false;
		}

		if( !ini.write( "section2", "key2", "value2"  ) ) {
			CPPDEBUG( "writing key2 failed" );
			return false;
		}

		if( !ini.write( "section1", "key3", "value3", "comment3"  ) ) {
			CPPDEBUG( "writing key3 failed" );
			return false;
		}

		return true;
	};

	return std::make_shared<TestCaseFuncWriteIni>(__FUNCTION__, test_func, 512,
				std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false, expected_text );
}
