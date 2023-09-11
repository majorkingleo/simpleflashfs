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


