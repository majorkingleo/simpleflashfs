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

using namespace SimpleFlashFs;
using namespace Tools;

namespace {

bool write_default_ini( SimpleFlashFs::FileBuffer & file )
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
			"  key1 = value1 \t\n" \
			"; comment = 2   \n"
			"  key2= value2 \n" \
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

		write_default_ini( file );

		SimpleIni ini( file );

		std::string_view value;
/*
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
*/
		if( !ini.read("section3","key1", value ) || value != "value1" ) {
			CPPDEBUG( "section3/key1 not found" );
			return false;
		}
/*
		if( !ini.read("section3","key2", value ) || value != "value2" ) {
			CPPDEBUG( "section3/key2 not found" );
			return false;
		}

		if( !ini.read("section4","key1", value ) || value != "nolineend" ) {
			CPPDEBUG( "section4/key1 not found" );
			return false;
		}
*/
		return true;
	};

	return std::make_shared<TestCaseFuncOneFileBuffer>(__FUNCTION__, test_func, 20,
			std::ios_base::in | std::ios_base::out | std::ios_base::trunc, false );
}
/*
std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_2()
{

}*/
