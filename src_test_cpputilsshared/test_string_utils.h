/**
 * Testcases for cpputils/cpputilsshared/string_utils.h
 * @author Copyright (c) 2001 - 2023 Martin Oberzalek
 */

#ifndef SRC_TEST_CPPUSTILSSHARED_TEST_STRING_UTILS_H_
#define SRC_TEST_CPPUSTILSSHARED_TEST_STRING_UTILS_H_

#include <TestUtils.h>
#include <string_utils.h>
#include <memory>

template<class Func, class t_std_string>
class TestCaseFuncEqual : public TestCaseBase<bool>
{
	Func func;
	const t_std_string & input;
	const t_std_string & output;

public:
	TestCaseFuncEqual( const std::string & name,
			const t_std_string & input_,
			const t_std_string & output_,
			Func func_ )
	: TestCaseBase<bool>( name, true ),
	  input( input_ ),
	  output( output_ )
	  {}

	bool run() override {
		return func( input, output );
	}
};

std::shared_ptr<TestCaseBase<bool>> test_case_toupper1();
std::shared_ptr<TestCaseBase<bool>> test_case_toupper2();
std::shared_ptr<TestCaseBase<bool>> test_case_toupper3();
std::shared_ptr<TestCaseBase<bool>> test_case_toupper4();

std::shared_ptr<TestCaseBase<bool>> test_case_tolower1();
std::shared_ptr<TestCaseBase<bool>> test_case_tolower2();
std::shared_ptr<TestCaseBase<bool>> test_case_tolower3();
std::shared_ptr<TestCaseBase<bool>> test_case_tolower4();

#endif /* SRC_TEST_CPPUSTILSSHARED_TEST_STRING_UTILS_H_ */
