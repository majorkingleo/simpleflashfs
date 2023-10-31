/**
 * Testcases for cpputils/cpputilsshared/CyclicArray.h
 * @author Copyright (c) 2023 Martin Oberzalek
 */

#ifndef SRC_TEST_CPPUTILSSHARED_TEST_CYCLICARRAY_H_
#define SRC_TEST_CPPUTILSSHARED_TEST_CYCLICARRAY_H_

#include <TestUtils.h>
#include <memory>


std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array1();
std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array2();
std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array3();
std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array4();
std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array5();

std::shared_ptr<TestCaseBase<bool>> test_case_modify_cyclic_array1();
std::shared_ptr<TestCaseBase<bool>> test_case_modify_cyclic_array2();

std::shared_ptr<TestCaseBase<bool>> test_case_reverse_iterator();
std::shared_ptr<TestCaseBase<bool>> test_case_insert1();
std::shared_ptr<TestCaseBase<bool>> test_case_insert2();
std::shared_ptr<TestCaseBase<bool>> test_case_insert3();
std::shared_ptr<TestCaseBase<bool>> test_case_insert4();
std::shared_ptr<TestCaseBase<bool>> test_case_insert5();
std::shared_ptr<TestCaseBase<bool>> test_case_insert6();

#endif /* SRC_TEST_CPPUTILSSHARED_TEST_CYCLICARRAY_H_ */
