#include "../src_test_dynamic/test_dynamic_init.h"
#include "../src_test_dynamic/test_dynamic_wrapper.h"

#include <src/dynamic/SimpleFlashFsDynamic.h>
#include <src/dynamic/SimpleFlashFsDynamicInstanceHandler.h>
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>
#include <format.h>
#include <src/dynamic/SimpleFlashFsDynamicWrapper.h>
#include <CpputilsDebug.h>

#define fopen( path, mode ) SimpleFlashFs_dynamic_fopen( path, mode )
#define fclose( file ) SimpleFlashFs_dynamic_fclose( file)
#define FILE SIMPLE_FLASH_FS_DYNAMIC_FILE

using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::dynamic;
using namespace SimpleFlashFs::SimPc;

unsigned instance_count = 0;

class TestCaseWrapperBase : public TestCaseBase<bool>
{
	std::size_t page_size;
	std::size_t size;
	std::shared_ptr<SimFlashFsFlashMemoryInterface> mem;
	std::shared_ptr<SimpleFlashFs::dynamic::SimpleFlashFs> fs;
	const std::string instance_name;

public:
	TestCaseWrapperBase( const std::string & name_,
			std::size_t page_size_ = 528,
			std::size_t size_ = 100*1024,
			bool expected_result_ = true,
			bool exception_ = false )
	: TestCaseBase<bool>( name_, expected_result_, exception_ ),
	  page_size( page_size_ ),
	  size( size_ ),
	  instance_name( format( ".test%d.bin", instance_count++ ) )
	{

	}

	~TestCaseWrapperBase()
	{
		InstanceHandler::instance().deregister_instance(instance_name);
	}

	bool init()
	{
		auto & instance_handler = InstanceHandler::instance();

		const std::string file = instance_name;

		mem = std::make_shared<SimFlashFsFlashMemoryInterface>(file,size);
		fs = std::make_shared<SimpleFlashFs::dynamic::SimpleFlashFs>(mem.get());

		instance_handler.register_instance(instance_name, fs );

		if( !fs->create(fs->create_default_header(page_size, size/page_size)) ) {
			throw STDERR_EXCEPTION( format( "cannot create %s", file ) );
		}

		//CPPDEBUG( format( "registering instance: '%s'", instance_name));
		SimpleFlashFs_dynamic_wrapper_register_default_instance_name(instance_name.c_str());

		return true;
	}

	void deinit()
	{
		SimpleFlashFs_dynamic_wrapper_unregister_default_instance_name(instance_name.c_str());
	}
/*
	bool run() override
	{
		if( !init() ) {
			return false;
		}

		return true;
	}
*/
};

#if 0
class TestCaseInit1 : public TestCaseInitBase
{
public:
	TestCaseInit1()
	: TestCaseBase<bool>( "init1" )
	{

	}
};



std::shared_ptr<TestCaseBase<bool>> test_case_init1()
{
	return std::make_shared<TestCaseInitBase>("init1");
}

std::shared_ptr<TestCaseBase<bool>> test_case_init2()
{
	return std::make_shared<TestCaseInitBase>("init2", 1, 100, false, true );
}

std::shared_ptr<TestCaseBase<bool>> test_case_init3()
{
	return std::make_shared<TestCaseInitBase>("init3", 37, 100, false, true );
}
#endif

class TestCaseWrapperFunc : public TestCaseWrapperBase
{
	std::function<bool()> func;

public:
	TestCaseWrapperFunc( const std::string & name_, std::function<bool()>  func_ )
	: TestCaseWrapperBase( name_ ),
	  func( func_ )
	{

	}

	bool run() override
	{
		if( !init() ) {
			CPPDEBUG( "init failed" );
			return false;
		}

		try {
			bool ret = func();

			deinit();

			return ret;
		} catch( std::exception & error ) {
			deinit();
			throw error;
		}
	}
};



std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen1()
{
	return std::make_shared<TestCaseWrapperFunc>("fopen1", []() {
		FILE *f = fopen( "test", "r" );
		if( f == nullptr ) {
			return true;
		}

		fclose( f );
		return false;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen2()
{
	return std::make_shared<TestCaseWrapperFunc>("fopen2", []() {
		FILE *f = fopen( "test", "r+" );
		if( f == nullptr ) {
			return true;
		}
		fclose( f );
		return false;
	});
}


std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen3()
{
	return std::make_shared<TestCaseWrapperFunc>("fopen3", []() {
		FILE *f = fopen( "test", "rw+" );
		if( f == nullptr ) {
			return false;
		}
		fclose( f );
		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen4()
{
	return std::make_shared<TestCaseWrapperFunc>("fopen4", []() {
		FILE *f = fopen( "test", "a" );
		if( f == nullptr ) {
			return false;
		}
		fclose( f );
		return true;
	});
}

std::shared_ptr<TestCaseBase<bool>> test_case_wrapper_fopen5()
{
	return std::make_shared<TestCaseWrapperFunc>("fopen5", []() {
		FILE *f = fopen( "test", "a+" );
		if( f == nullptr ) {
			return false;
		}
		fclose( f );
		return true;
	});
}

