/**
 * Testcases for cpputils/cpputilsshared/static_string.h
 * @author Copyright (c) 2023 Martin Oberzalek
 */

#ifndef SRC_TEST_CPPUTILSSHARED_TEST_STATIC_STRING_H_
#define SRC_TEST_CPPUTILSSHARED_TEST_STATIC_STRING_H_

#include <TestUtils.h>
#include <memory>

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_string1();
std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_string2();
std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_string3();


#endif /* SRC_TEST_CPPUTILSSHARED_TEST_STATIC_VECTOR_H_ */
