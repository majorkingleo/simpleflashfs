/**
 * Testcases for cpputils/cpputilsshared/string_utils.h
 * @author Copyright (c) 2001 - 2023 Martin Oberzalek
 */
#include "string_utils.h"
#include "test_string_utils.h"

using namespace Tools;

auto test_upper_equal = []( const auto & a, const auto & b) { return toupper(a) == b; };

std::shared_ptr<TestCaseBase<bool>> test_case_toupper1() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__,"hello","HELLO",test_upper_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_toupper2() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L"hello",L"HELLO",test_upper_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_toupper3() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__,"","",test_upper_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_toupper4() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L"",L"",test_upper_equal);
}



auto test_lower_equal = []( const auto & a, const auto & b) { return tolower(a) == b; };

std::shared_ptr<TestCaseBase<bool>> test_case_tolower1() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__,"HELLO","hello",test_lower_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_tolower2() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L"HELLO",L"hello",test_lower_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_tolower3() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__,"","",test_lower_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_tolower4() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L"HELLO",L"hello",test_lower_equal);
}



auto test_strip_equal = []( const auto & a, const auto & b) { return strip(a) == b; };

std::shared_ptr<TestCaseBase<bool>> test_case_strip1() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," ","",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip2() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__,"","",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip3() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," hello","hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip4() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," hello ","hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip5() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" ",L"",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip6() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L"",L"",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip7() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" hello",L"hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip8() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" hello ",L"hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip9() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" hello hello ",L"hello hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip10() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," hello hello ","hello hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip11() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" \thello hello ",L"hello hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip12() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," \thello hello ","hello hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip13() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" \thello hello \n",L"hello hello",test_strip_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip14() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," \thello hello \n","hello hello",test_strip_equal);
}



auto test_strip_view_equal = []( const auto & a, const auto & b) { return strip_view(a) == b; };

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view1() {
	return std::make_shared<TestCaseFuncEqual<std::string_view>>(__FUNCTION__," ","",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view2() {
	return std::make_shared<TestCaseFuncEqual<std::string_view>>(__FUNCTION__,"","",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view3() {
	return std::make_shared<TestCaseFuncEqual<std::string_view>>(__FUNCTION__," hello","hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view4() {
	return std::make_shared<TestCaseFuncEqual<std::string_view>>(__FUNCTION__," hello ","hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view5() {
	return std::make_shared<TestCaseFuncEqual<std::wstring_view>>(__FUNCTION__,L" ",L"",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view6() {
	return std::make_shared<TestCaseFuncEqual<std::wstring_view>>(__FUNCTION__,L"",L"",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view7() {
	return std::make_shared<TestCaseFuncEqual<std::wstring_view>>(__FUNCTION__,L" hello",L"hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view8() {
	return std::make_shared<TestCaseFuncEqual<std::wstring_view>>(__FUNCTION__,L" hello ",L"hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view9() {
	return std::make_shared<TestCaseFuncEqual<std::wstring_view>>(__FUNCTION__,L" hello hello ",L"hello hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view10() {
	return std::make_shared<TestCaseFuncEqual<std::string_view>>(__FUNCTION__," hello hello ","hello hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view11() {
	return std::make_shared<TestCaseFuncEqual<std::wstring_view>>(__FUNCTION__,L" \thello hello ",L"hello hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view12() {
	return std::make_shared<TestCaseFuncEqual<std::string_view>>(__FUNCTION__," \thello hello ","hello hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view13() {
	return std::make_shared<TestCaseFuncEqual<std::wstring_view>>(__FUNCTION__,L" \thello hello \n",L"hello hello",test_strip_view_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_view14() {
	return std::make_shared<TestCaseFuncEqual<std::string_view>>(__FUNCTION__," \thello hello \n","hello hello",test_strip_view_equal);
}



auto test_strip_leading_equal = []( const auto & a, const auto & b) { return strip_leading(a) == b; };

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading1() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," ","",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading2() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__,"","",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading3() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," hello","hello",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading4() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," hello ","hello ",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading5() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" ",L"",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading6() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L"",L"",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading7() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" hello",L"hello",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading8() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" hello ",L"hello ",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading9() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" hello hello ",L"hello hello ",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading10() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," hello hello ","hello hello ",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading11() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" \thello hello ",L"hello hello ",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading12() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," \thello hello ","hello hello ",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading13() {
	return std::make_shared<TestCaseFuncEqual<std::wstring>>(__FUNCTION__,L" \thello hello \n",L"hello hello \n",test_strip_leading_equal);
}

std::shared_ptr<TestCaseBase<bool>> test_case_strip_leading14() {
	return std::make_shared<TestCaseFuncEqual<std::string>>(__FUNCTION__," \thello hello \n","hello hello \n",test_strip_leading_equal);
}



