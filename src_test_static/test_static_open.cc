/*
 * test_static_open.cc
 *
 *  Created on: 26.08.2024
 *      Author: martin.oberzalek
 */
#include "test_static_open.h"
#include "../src_2face/H7TwoFaceConfig.h"
#include "../src_2face/SimpleFlashFsNoDel.h"
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>
#include <format.h>
#include <memory>

using namespace Tools;
using namespace SimpleFlashFs::static_memory;
using namespace ::SimpleFlashFs::SimPc;

namespace {

class TestCaseOpenBase : public TestCaseBase<bool>
{
protected:
	std::optional<SimpleFsNoDel<ConfigH7>> fs;
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
	std::function<RetType(SimpleFsNoDel<ConfigH7> & fs)> func;

public:
	TestCaseFunc( const std::string & name_, std::function<RetType(SimpleFsNoDel<ConfigH7> & fs)>  func_ )
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
	return std::make_shared<TestCaseFunc<bool>>(__FUNCTION__, [](SimpleFsNoDel<ConfigH7> & fs) {

		auto f = fs.open("test.open1", std::ios_base::in );

		if( !f ) {
			CPPDEBUG( "cannot open file" );
			return true;
		}

		return false;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_open2()
{
	return std::make_shared<TestCaseFunc<bool>>(__FUNCTION__, [](SimpleFsNoDel<ConfigH7> & fs) {

		auto f = fs.open("test.open2", std::ios_base::in | std::ios_base::out );

		if( !f ) {
			CPPDEBUG( "cannot open file" );
			return false;
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_open3()
{
	return std::make_shared<TestCaseFunc<bool>>(__FUNCTION__, [](SimpleFsNoDel<ConfigH7> & fs) {

		auto f = fs.open("test.open3", std::ios_base::in | std::ios_base::app );

		if( !f ) {
			CPPDEBUG( "cannot open file" );
			return false;
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_open4()
{
	return std::make_shared<TestCaseFunc<bool>>(__FUNCTION__, [](SimpleFsNoDel<ConfigH7> & fs) {

		{
			auto f = fs.open("test.open4", std::ios_base::out | std::ios_base::trunc );

			if( !f ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}

			std::string_view sv( "test1" );

			if( f.write( reinterpret_cast<const std::byte*>(sv.data()), sv.size() ) != sv.size() ) {
				CPPDEBUG( "cannot write data to file" );
				return false;
			}
		}

		{
			auto f = fs.open("test.open4", std::ios_base::out | std::ios_base::trunc );

			if( !f ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}

			if( f.file_size() != 0 ) {
				CPPDEBUG( "file size is not 0" );
				return false;
			}
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_open5()
{
	return std::make_shared<TestCaseFunc<bool>>(__FUNCTION__, [](SimpleFsNoDel<ConfigH7> & fs) {

		{
			auto f = fs.open("test.open5", std::ios_base::out | std::ios_base::trunc );
			f.inode.attributes |= static_cast<decltype(f.inode.attributes)>(::SimpleFlashFs::base::InodeAttribute::SPECIAL);

			if( !f ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}
		}
		{
			auto f = fs.open("test.open5", std::ios_base::in );

			if( !f ) {
				CPPDEBUG( "cannot open file" );
				return false;
			}

			CPPDEBUG( Tools::format( "attr: %d",
					static_cast<decltype(f.inode.attributes)>(::SimpleFlashFs::base::InodeAttribute::SPECIAL) ) );

			if( !(f.inode.attributes & static_cast<decltype(f.inode.attributes)>(::SimpleFlashFs::base::InodeAttribute::SPECIAL) ) ) {
				return false;
			}
		}

		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_write1()
{
	return std::make_shared<TestCaseFunc<bool>>(__FUNCTION__, [](SimpleFsNoDel<ConfigH7> & fs) {
		auto f = fs.open("test.write1", std::ios_base::out | std::ios_base::trunc );

		if( !f ) {
			CPPDEBUG( "cannot open file" );
			return false;
		}

		char buffer[1000] = { "Hello" };

		std::size_t bytes_written = f.write( reinterpret_cast<std::byte*>(buffer), sizeof(buffer) );
		if( bytes_written != sizeof(buffer) ) {
			CPPDEBUG( format( "%d bytes written", bytes_written ) );
			return false;
		}

		return true;
	});
}
