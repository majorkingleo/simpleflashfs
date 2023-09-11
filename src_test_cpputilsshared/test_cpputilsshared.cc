/**
 * Testcases for cpputils/cpputilsshared/string_utils.h
 * @author Copyright (c) 2001 - 2023 Martin Oberzalek
 */
#include "test_string_utils.h"
#include "ColoredOutput.h"
#include "ColBuilder.h"
#include <arg.h>
#include <iostream>
#include <OutDebug.h>
#include <memory>

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

	try {
		std::vector<std::shared_ptr<TestCaseBase<bool>>> test_cases;


		test_cases.push_back( test_case_toupper1() );
		test_cases.push_back( test_case_toupper2() );
		test_cases.push_back( test_case_toupper3() );
		test_cases.push_back( test_case_toupper4() );

		test_cases.push_back( test_case_tolower1() );
		test_cases.push_back( test_case_tolower2() );
		test_cases.push_back( test_case_tolower3() );
		test_cases.push_back( test_case_tolower4() );

		test_cases.push_back( test_case_strip1() );
		test_cases.push_back( test_case_strip2() );
		test_cases.push_back( test_case_strip3() );
		test_cases.push_back( test_case_strip4() );
		test_cases.push_back( test_case_strip5() );
		test_cases.push_back( test_case_strip6() );
		test_cases.push_back( test_case_strip7() );
		test_cases.push_back( test_case_strip8() );
		test_cases.push_back( test_case_strip9() );
		test_cases.push_back( test_case_strip10() );
		test_cases.push_back( test_case_strip11() );
		test_cases.push_back( test_case_strip12() );
		test_cases.push_back( test_case_strip13() );
		test_cases.push_back( test_case_strip14() );

		test_cases.push_back( test_case_strip_view1() );
		test_cases.push_back( test_case_strip_view2() );
		test_cases.push_back( test_case_strip_view3() );
		test_cases.push_back( test_case_strip_view4() );
		test_cases.push_back( test_case_strip_view5() );
		test_cases.push_back( test_case_strip_view6() );
		test_cases.push_back( test_case_strip_view7() );
		test_cases.push_back( test_case_strip_view8() );
		test_cases.push_back( test_case_strip_view9() );
		test_cases.push_back( test_case_strip_view10() );
		test_cases.push_back( test_case_strip_view11() );
		test_cases.push_back( test_case_strip_view12() );
		test_cases.push_back( test_case_strip_view13() );
		test_cases.push_back( test_case_strip_view14() );

		test_cases.push_back( test_case_strip_leading1() );
		test_cases.push_back( test_case_strip_leading2() );
		test_cases.push_back( test_case_strip_leading3() );
		test_cases.push_back( test_case_strip_leading4() );
		test_cases.push_back( test_case_strip_leading5() );
		test_cases.push_back( test_case_strip_leading6() );
		test_cases.push_back( test_case_strip_leading7() );
		test_cases.push_back( test_case_strip_leading8() );
		test_cases.push_back( test_case_strip_leading9() );
		test_cases.push_back( test_case_strip_leading10() );
		test_cases.push_back( test_case_strip_leading11() );
		test_cases.push_back( test_case_strip_leading12() );
		test_cases.push_back( test_case_strip_leading13() );
		test_cases.push_back( test_case_strip_leading14() );

		test_cases.push_back( test_case_strip_trailing1() );
		test_cases.push_back( test_case_strip_trailing2() );
		test_cases.push_back( test_case_strip_trailing3() );
		test_cases.push_back( test_case_strip_trailing4() );
		test_cases.push_back( test_case_strip_trailing5() );
		test_cases.push_back( test_case_strip_trailing6() );
		test_cases.push_back( test_case_strip_trailing7() );
		test_cases.push_back( test_case_strip_trailing8() );
		test_cases.push_back( test_case_strip_trailing9() );
		test_cases.push_back( test_case_strip_trailing10() );
		test_cases.push_back( test_case_strip_trailing11() );
		test_cases.push_back( test_case_strip_trailing12() );
		test_cases.push_back( test_case_strip_trailing13() );
		test_cases.push_back( test_case_strip_trailing14() );

		test_cases.push_back( test_case_is_int1() );
		test_cases.push_back( test_case_is_int2() );
		test_cases.push_back( test_case_is_int3() );
		test_cases.push_back( test_case_is_int4() );
		test_cases.push_back( test_case_is_int5() );
		test_cases.push_back( test_case_is_int6() );

		ColBuilder col;

		const int COL_IDX 		= col.addCol( "Idx" );
		const int COL_NAME 		= col.addCol( "Test" );
		const int COL_RESULT    = col.addCol( "Result" );
		const int COL_EXPTECTED = col.addCol( "Expected" );
		const int COL_TEST_RES	= col.addCol( "Test Result" );

		unsigned idx = 0;

		for( auto & test : test_cases ) {

			idx++;

			DEBUG( format( "run test: %s", test->getName() ) );

			col.addColData( COL_IDX, format( "% 2d", idx ) );
			col.addColData( COL_NAME, test->getName() );

			std::string result;
			std::string expected_result = "true";

			if( !test->getExpectedResult() ) {
				expected_result = "false";
			}

			std::string test_result;

			try {

				if( test->throwsException() ) {
					 expected_result = "exception";
				}

				if( test->run() ) {
					result = "true";
				} else {
					result = "false";
				}

			} catch( const std::exception & error ) {
				result = "exception";
				DEBUG( format( "Error: %s", error.what() ));
			} catch( ... ) {
				result = "exception";
				DEBUG( "Error" );
			}

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

	} catch( const std::exception & error ) {
		std::cout << "Error: " << error.what() << std::endl;
		return 1;
	}

	return 0;
}

