/*
 * test_SimpleIni.h
 *
 *  Created on: 06.09.2024
 *      Author: martin.oberzalek
 */
#pragma once

#include <TestUtils.h>
#include <memory>

std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_1();
std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_2();
std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_3();
std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_4();
std::shared_ptr<TestCaseBase<bool>> test_case_simple_ini_read_5();
