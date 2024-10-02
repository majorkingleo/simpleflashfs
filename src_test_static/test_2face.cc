/**
 * Testcases for SimpleFlashFsDynamic
 * @author Copyright (c) 2024 Martin Oberzalek
 */
#include "ColoredOutput.h"
#include "ColBuilder.h"
#include <arg.h>
#include <iostream>
#include <OutDebug.h>
#include <memory>
#include <format.h>
#include "test_H7TwoFace.h"
#include "test_pageset.h"
#include "test_FileBuffer.h"
#include "test_SimpleIni.h"
#include <limits>

using namespace Tools;

int main( int argc, char **argv )
{
	ColoredOutput co;

	Arg::Arg arg( argc, argv );
	arg.addPrefix( "-" );
	arg.addPrefix( "--" );

	Arg::OptionChain oc_info;
	arg.addChainR(&oc_info);
	oc_info.setMinMatch(1);
	oc_info.setContinueOnMatch( false );
	oc_info.setContinueOnFail( true );

	Arg::FlagOption o_help( "help" );
	o_help.setDescription( "Show this page" );
	oc_info.addOptionR( &o_help );

	Arg::FlagOption o_debug("d");
	o_debug.addName( "debug" );
	o_debug.setDescription("print debugging messages");
	o_debug.setRequired(false);
	arg.addOptionR( &o_debug );

	if( !arg.parse() )
	{
		std::cout << arg.getHelp(5,20,30, 80 ) << std::endl;
		return 1;
	}

	if( o_debug.getState() )
	{
		Tools::x_debug = new OutDebug();
	}

	if( o_help.getState() ) {
		std::cout << arg.getHelp(5,20,30, 80 ) << std::endl;
		return 1;
	}

#if __cpp_exceptions > 0
	try {
#endif

		std::vector<std::shared_ptr<TestCaseBase<bool>>> test_cases;

		test_cases.push_back( test_case_static_TwoFace_write1() );
		test_cases.push_back( test_case_static_TwoFace_write2() );
		test_cases.push_back( test_case_static_TwoFace_write3() );

		test_cases.push_back( test_case_static_TwoFace_delete1() );
		test_cases.push_back( test_case_static_TwoFace_rename1() );

		test_cases.push_back( test_case_static_TwoFace_list_files1() );

		test_cases.push_back( test_case_pageset1() );
		test_cases.push_back( test_case_pageset2() );
		test_cases.push_back( test_case_pageset3() );
		test_cases.push_back( test_case_pageset4() );
		test_cases.push_back( test_case_pageset5() );
		test_cases.push_back( test_case_pageset6() );
		test_cases.push_back( test_case_pageset7() );
		test_cases.push_back( test_case_pageset8() );
		test_cases.push_back( test_case_pageset9() );

		test_cases.push_back( test_case_static_TwoFace_open_speacial1() );

		test_cases.push_back( test_case_static_TwoFace_max_files1() );
		test_cases.push_back( test_case_static_TwoFace_max_files2() );


		test_cases.push_back( test_case_filebuffer_1() );
		test_cases.push_back( test_case_filebuffer_2() );
		test_cases.push_back( test_case_filebuffer_3() );
		test_cases.push_back( test_case_filebuffer_4() );
		test_cases.push_back( test_case_filebuffer_5() );
		test_cases.push_back( test_case_filebuffer_6() );
		test_cases.push_back( test_case_filebuffer_7() );
		test_cases.push_back( test_case_filebuffer_8() );

		test_cases.push_back( test_case_filebuffer_9() );

		test_cases.push_back( test_case_filebuffer_10() );

		test_cases.push_back( test_case_filebuffer_11() );



		test_cases.push_back( test_case_simple_ini_read_1() );
		test_cases.push_back( test_case_simple_ini_read_2() );

		test_cases.push_back( test_case_simple_ini_read_3() );

		test_cases.push_back( test_case_simple_ini_read_4() );
		test_cases.push_back( test_case_simple_ini_read_5() );

		test_cases.push_back( test_case_simple_ini_write_1() );


		test_cases.push_back( test_case_simple_ini_write_2() );
		test_cases.push_back( test_case_simple_ini_write_3() );

		test_cases.push_back( test_case_simple_ini_write_4() );

		test_cases.push_back( test_case_simple_ini_write_5() );

		test_cases.push_back( test_case_simple_ini_write_6() );


		test_cases.push_back( test_case_simple_ini_write_7() );

		test_cases.push_back( test_case_simple_ini_write_8() );

		test_cases.push_back( test_case_static_TwoFace_write_ini1() );

		test_cases.push_back( test_case_simple_ini_read_value_1() );
		test_cases.push_back( test_case_simple_ini_read_value_2() );
		test_cases.push_back( test_case_simple_ini_read_value_3() );

		test_cases.push_back( test_case_simple_ini_read_blob_1() );

		ColBuilder col;

		const int COL_IDX 		= col.addCol( "Idx" );
		const int COL_NAME 		= col.addCol( "Test" );
		const int COL_RESULT    = col.addCol( "Result" );
		const int COL_EXPTECTED = col.addCol( "Expected" );
		const int COL_TEST_RES	= col.addCol( "Test Result" );

		unsigned idx = 0;

		for( auto & test : test_cases ) {

			idx++;

			CPPDEBUG( format( "run test: %s", test->getName() ) );

			col.addColData( COL_IDX, format( "% 2d", idx ) );
			col.addColData( COL_NAME, test->getName() );

			std::string result;
			std::string expected_result = "true";

			if( !test->getExpectedResult() ) {
				expected_result = "false";
			}

			std::string test_result;

#if __cpp_exceptions > 0
			try {
#endif

				if( test->throwsException() ) {
					 expected_result = "exception";
				}

				if( test->run() ) {
					result = "true";
				} else {
					result = "false";
				}

#if __cpp_exceptions > 0
			} catch( const std::exception & error ) {
				result = "exception";
				CPPDEBUG( format( "Error: %s", error.what() ));
			} catch( ... ) {
				result = "exception";
				CPPDEBUG( "Error" );
			}
#endif

			if( result != expected_result ) {
				test_result = co.color_output( ColoredOutput::RED, "failed" );
			} else {
				test_result = co.color_output( ColoredOutput::GREEN, "succeeded" );
			}

			col.addColData( COL_RESULT, result );
			col.addColData( COL_EXPTECTED, expected_result );
			col.addColData( COL_TEST_RES, test_result );

		}

		std::cout << col.toString() << std::endl;

#if __cpp_exceptions > 0
	} catch( const std::exception & error ) {
		std::cout << "Error: " << error.what() << std::endl;
		return 1;
	}
#endif

	return 0;
}





