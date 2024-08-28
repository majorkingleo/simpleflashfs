/*
 * test_static_open.h
 *
 *  Created on: 26.08.2024
 *      Author: martin.oberzalek
 */

#pragma once

#include <TestUtils.h>
#include <memory>

std::shared_ptr<TestCaseBase<bool>> test_case_static_2pages_write1();

std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_write1();
std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_write2();
std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_write3();

std::shared_ptr<TestCaseBase<bool>> test_case_static_TwoFace_delete1();
