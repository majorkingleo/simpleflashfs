/**
 * SimpleFlashFs test case initialisation
 * @author Copyright (c) 2023-2024 Martin Oberzalek
 */
#ifndef TEST_DYNAMIC_TEST_DYNAMIC_INIT_H_
#define TEST_DYNAMIC_TEST_DYNAMIC_INIT_H_

#include <TestUtils.h>
#include <memory>

std::shared_ptr<TestCaseBase<bool>> test_case_init1();
std::shared_ptr<TestCaseBase<bool>> test_case_init2();
std::shared_ptr<TestCaseBase<bool>> test_case_init3();


#endif /* TEST_DYNAMIC_TEST_DYNAMIC_INIT_H_ */
