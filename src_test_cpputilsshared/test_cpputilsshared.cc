/**
 * Testcases for cpputils/cpputilsshared/string_utils.h
 * @author Copyright (c) 2001 - 2023 Martin Oberzalek
 */
#include "test_string_utils.h"
#include "test_cyclicarray.h"
#include "test_static_vector.h"
#include "test_static_list.h"
#include "test_static_string.h"
#include "ColoredOutput.h"
#include "ColBuilder.h"
#include <arg.h>
#include <iostream>
#include <OutDebug.h>
#include <memory>
#include <format.h>

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
#if 0
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


		test_cases.push_back( test_case_init_cyclic_array1() );
		test_cases.push_back( test_case_init_cyclic_array2() );
		test_cases.push_back( test_case_init_cyclic_array3() );
		test_cases.push_back( test_case_init_cyclic_array4() );
		test_cases.push_back( test_case_init_cyclic_array5() );

		test_cases.push_back( test_case_modify_cyclic_array1() );
		test_cases.push_back( test_case_modify_cyclic_array2() );
		test_cases.push_back( test_case_reverse_iterator() );

		test_cases.push_back( test_case_insert1() );

		test_cases.push_back( test_case_insert2() );

		test_cases.push_back( test_case_insert3() );
		test_cases.push_back( test_case_insert4() );

		test_cases.push_back( test_case_insert5() );


		test_cases.push_back( test_case_init_static_vector1() );
		test_cases.push_back( test_case_init_static_vector2() );
		test_cases.push_back( test_case_init_static_vector3() );
		test_cases.push_back( test_case_init_static_vector4() );
		test_cases.push_back( test_case_init_static_vector5() );

		test_cases.push_back( test_case_modify_static_vector1() );
		test_cases.push_back( test_case_modify_static_vector2() );
		test_cases.push_back( test_case_static_vector_reverse_iterator() );
		test_cases.push_back( test_case_static_vector_insert1() );

		test_cases.push_back( test_case_static_vector_insert2() );
		test_cases.push_back( test_case_static_vector_insert3() );
		test_cases.push_back( test_case_static_vector_insert4() );
		test_cases.push_back( test_case_static_vector_insert5() );
		test_cases.push_back( test_case_static_vector_insert6() );
		test_cases.push_back( test_case_static_vector_insert7() );
		test_cases.push_back( test_case_static_vector_insert8() );
		test_cases.push_back( test_case_static_vector_insert9() );



		test_cases.push_back( test_case_init_static_list1() );

		test_cases.push_back( test_case_init_static_list2() );
		test_cases.push_back( test_case_init_static_list3() );
		test_cases.push_back( test_case_init_static_list4() );
		test_cases.push_back( test_case_init_static_list5() );
		test_cases.push_back( test_case_init_static_list6() );
		test_cases.push_back( test_case_init_static_list7() );
		test_cases.push_back( test_case_init_static_list8() );
		test_cases.push_back( test_case_init_static_list9() );
		test_cases.push_back( test_case_init_static_list10() );
		test_cases.push_back( test_case_init_static_list11() );


		test_cases.push_back( test_case_modify_static_list1() );
		test_cases.push_back( test_case_modify_static_list2() );
		test_cases.push_back( test_case_modify_static_list3() );

		test_cases.push_back( test_case_modify_static_list4() );
		test_cases.push_back( test_case_modify_static_list5() );
		test_cases.push_back( test_case_modify_static_list6() );

		test_cases.push_back( test_case_static_list_reverse_iterator1() );
		test_cases.push_back( test_case_static_list_reverse_iterator2() );
		test_cases.push_back( test_case_static_list_reverse_iterator3() );

		test_cases.push_back( test_case_static_list_insert1() );

		test_cases.push_back( test_case_static_list_insert2() );
		test_cases.push_back( test_case_static_list_insert3() );
		test_cases.push_back( test_case_static_list_insert4() );

		test_cases.push_back( test_case_static_list_insert5() );

		test_cases.push_back( test_case_static_list_insert6() );
		test_cases.push_back( test_case_static_list_insert7() );
		test_cases.push_back( test_case_static_list_insert8() );
		test_cases.push_back( test_case_static_list_insert9() );
		test_cases.push_back( test_case_static_list_insert10() );

		test_cases.push_back( test_case_static_list_insert11() );

		test_cases.push_back( test_case_static_list_insert12() );

		test_cases.push_back( test_case_static_list_insert13() );
		test_cases.push_back( test_case_static_list_insert14() );
		test_cases.push_back( test_case_static_list_insert15() );
		test_cases.push_back( test_case_static_list_insert16() );

		test_cases.push_back( test_case_static_list_front1() );
		test_cases.push_back( test_case_static_list_front2() );

		test_cases.push_back( test_case_static_list_back1() );
		test_cases.push_back( test_case_static_list_back2() );

		test_cases.push_back( test_case_static_list_emplace1() );
		test_cases.push_back( test_case_static_list_emplace2() );

		test_cases.push_back( test_case_static_list_push_front1() );

		test_cases.push_back( test_case_static_list_reverse1() );

		test_cases.push_back( test_case_static_list_remove1() );
		test_cases.push_back( test_case_static_list_remove2() );

		test_cases.push_back( test_case_static_list_sort1() );
		test_cases.push_back( test_case_static_list_sort2() );

		test_cases.push_back( test_case_static_list_unique1() );
		test_cases.push_back( test_case_static_list_unique2() );
#endif

		test_cases.push_back( test_case_modify_static_string1() );
		test_cases.push_back( test_case_modify_static_string2() );
		test_cases.push_back( test_case_modify_static_string3() );

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

