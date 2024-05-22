/**
 * SimpleFlashFs test case initialisation
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#ifndef TEST_DYNAMIC_TEST_DYNAMIC_WRAPPER_H_
#define TEST_DYNAMIC_TEST_DYNAMIC_WRAPPER_H_

#include <TestUtils.h>
#include <memory>

std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen1();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen2();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen3();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen4();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen5();


std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fwrite1();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fwrite2();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fwrite3();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fwrite4();
std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fwrite5();

#endif /* TEST_DYNAMIC_TEST_DYNAMIC_WRAPPER_H_ */
