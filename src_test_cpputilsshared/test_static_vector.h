/**
 * Testcases for cpputils/cpputilsshared/static_vector.h
 * @author Copyright (c) 2023 Martin Oberzalek
 */

#ifndef SRC_TEST_CPPUTILSSHARED_TEST_STATIC_VECTOR_H_
#define SRC_TEST_CPPUTILSSHARED_TEST_STATIC_VECTOR_H_

#include <TestUtils.h>
#include <memory>


std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector1();
std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector2();
std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector3();
std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector4();
std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector5();

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_vector1();
std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_vector2();

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_reverse_iterator();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert1();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert2();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert3();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert4();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert5();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert6();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert7();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert8();
std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert9();

#endif /* SRC_TEST_CPPUTILSSHARED_TEST_STATIC_VECTOR_H_ */
