/*
 * test_static_open.cc
 *
 *  Created on: 26.08.2024
 *      Author: martin.oberzalek
 */
#include "test_static_open.h"
#include "static_fs_h7.h"
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>
#include <format.h>

using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::static_memory;
using namespace ::SimpleFlashFs::SimPc;

namespace {

class TestCaseOpenBase : public TestCaseBase<bool>
{
protected:
	std::optional<SimpleFsH7> fs;
	std::optional<::SimpleFlashFs::SimPc::SimFlashFsFlashMemoryInterface> mem;

public:
	TestCaseOpenBase( const std::string & name_,
			bool expected_result_ = true,
			bool exception_ = false )
	: TestCaseBase<bool>( name_, expected_result_, exception_ )
	{

	}


	void init()
	{
		const std::string file = "test_h7.bin";
		mem.emplace(file,SFF_MAX_SIZE);
		fs.emplace(&mem.value());

		if( !fs->init() ) {
			CPPDEBUG( format( "recreating fs %s", file ) );
			if( !fs->create() ) {
				throw STDERR_EXCEPTION( format( "cannot create %s", file ) );
			}
		}
	}

	void deinit() {
		fs.reset();
		mem.reset();
	}
};

template <class RetType>
class TestCaseFunc : public TestCaseOpenBase
{
	std::function<RetType(SimpleFsH7 & fs)> func;

public:
	TestCaseFunc( const std::string & name_, std::function<RetType(SimpleFsH7 & fs)>  func_ )
	: TestCaseOpenBase( name_ ),
	  func( func_ )
	{

	}

	bool run() override
	{
		init();

		try {
			RetType ret = func(*fs);
			deinit();

			return ret;
		} catch( std::exception & error ) {
			deinit();
			throw error;
		}
	}
};

} // namespace


std::shared_ptr<TestCaseBase<bool>> test_case_static_open1()
{
	return std::make_shared<TestCaseFunc<bool>>("open1", [](SimpleFsH7 & fs) {

		auto f = fs.open("test.open1", std::ios_base::in );

		if( !f ) {
			return true;
		}

		return false;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_open2()
{
	return std::make_shared<TestCaseFunc<bool>>("open2", [](SimpleFsH7 & fs) {

		auto f = fs.open("test.open2", std::ios_base::in | std::ios_base::out );

		if( !f ) {
			return false;
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_open3()
{
	return std::make_shared<TestCaseFunc<bool>>("open3", [](SimpleFsH7 & fs) {

		auto f = fs.open("test.open3", std::ios_base::in | std::ios_base::app );

		if( !f ) {
			return false;
		}

		return true;
	});
}

