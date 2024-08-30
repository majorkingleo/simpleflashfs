/*
 * test_pageset.cc
 *
 *  Created on: 30.08.2024
 *      Author: martin.oberzalek
 */
#include "test_pageset.h"
#include <base/SimpleFlashFsPageSet.h>
#include <stderr_exception.h>
#include <format.h>
#include <static_vector.h>
#include <string_utils.h>
#include <CpputilsDebug.h>

#include <limits>

using namespace Tools;
using namespace SimpleFlashFs::base;

namespace {

template<class T> class static_vector_20 : public Tools::static_vector<T,20> {};

template<class page_set>
typename page_set::vector_type to_vec( const page_set & ps )
{
	typename page_set::vector_type ret;
	ret.reserve(ps.get_data().size());

	for( auto & value : ps.get_sorted_data() ) {
		if( value == page_set::NO_DATA ) {
			continue;
		}

		ret.push_back( value );
	}

	return ret;

}

struct ConfigDynamic {
	template<class T> class vector_type : public std::vector<T> {};
};

struct ConfigStatic {
	template<class T> class vector_type : public Tools::static_vector<T,20> {};
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_pageset1()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigDynamic> ps;
		const std::initializer_list<uint32_t> il = { 1, 2, 5, 6 };

		ps.insert(5);
		ps.insert(1);
		ps.insert(2);
		ps.insert(6);

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == std::vector<uint32_t>( il );
	});
}


std::shared_ptr<TestCaseBase<bool>> test_case_pageset2()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigDynamic> ps;
		const std::initializer_list<uint32_t> il = { 1, 2, 5, 6 };
		const std::initializer_list<uint32_t> il_unordered = { 5, 1, 6, 2 };

		ps.unordered_insert( il_unordered.begin(), il_unordered.end() );

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == std::vector<uint32_t>( il );
		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_pageset3()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigDynamic> ps;
		const std::initializer_list<uint32_t> il = { 1, 2, 5, 6 };

		ps.unordered_insert(5);
		ps.unordered_insert(1);
		ps.unordered_insert(2);
		ps.unordered_insert(6);


		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == std::vector<uint32_t>( il );
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_pageset4()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigDynamic> ps;
		const std::initializer_list<uint32_t> il = { 1, 2, 6 };
		const std::initializer_list<uint32_t> il_unordered = { 5, 1, 6, 2 };

		ps.unordered_insert( il_unordered.begin(), il_unordered.end() );

		ps.erase(5);

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == std::vector<uint32_t>( il );
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_pageset5()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigStatic> ps;
		const std::initializer_list<uint32_t> il = { 1, 2, 4, 6 };
		const std::initializer_list<uint32_t> il_unordered = { 5, 1, 6, 2 };

		ps.unordered_insert( il_unordered.begin(), il_unordered.end() );

		ps.erase(5);
		ps.insert(4);

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == static_vector_20<uint32_t>( il );
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_pageset6()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigStatic> ps;
		const std::initializer_list<uint32_t> il = { 1, 2, 6, 7 };
		const std::initializer_list<uint32_t> il_unordered = { 5, 1, 6, 2 };

		ps.unordered_insert( il_unordered.begin(), il_unordered.end() );

		ps.erase(5);
		ps.insert(7);

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == static_vector_20<uint32_t>( il );
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_pageset7()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigStatic> ps;
		const std::initializer_list<uint32_t> il = { 0, 1, 2, 6, 7 };
		const std::initializer_list<uint32_t> il_unordered = { 5, 1, 6, 2 };

		ps.unordered_insert( il_unordered.begin(), il_unordered.end() );

		ps.erase(5);
		ps.insert(7);
		ps.insert(0);
		ps.insert(PageSet<ConfigStatic>::NO_DATA);

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == static_vector_20<uint32_t>( il );
	});
}



std::shared_ptr<TestCaseBase<bool>> test_case_pageset8()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigStatic> ps;
		const std::initializer_list<uint32_t> il = { 0, 1, 2, 6 };
		const std::initializer_list<uint32_t> il_unordered = { 5, 1, 6, 2 };

		ps.unordered_insert( il_unordered.begin(), il_unordered.end() );

		ps.erase(5);
		ps.insert(7);
		ps.insert(0);
		ps.insert(PageSet<ConfigStatic>::NO_DATA);
		ps.erase(7);


		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );
		ps.sort();

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return to_vec(ps) == static_vector_20<uint32_t>( il );
	});
}


std::shared_ptr<TestCaseBase<bool>> test_case_pageset9()
{
	return std::make_shared<TestCaseFuncNoInp>(__FUNCTION__, true, []() {

		PageSet<ConfigStatic> ps;
		const std::initializer_list<uint32_t> il_unordered = { 5, 1, 6, 2 };

		ps.unordered_insert( il_unordered.begin(), il_unordered.end() );

		ps.erase(5);
		ps.erase(1);
		ps.erase(6);
		ps.erase(2);

		CPPDEBUG( format( "%s", IterableToCommaSeparatedString(ps.get_data())) );

		return ps.empty();
	});
}
