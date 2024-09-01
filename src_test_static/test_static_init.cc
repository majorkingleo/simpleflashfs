#include "test_static_init.h"

#include "static_fs.h"
#include <stderr_exception.h>
#include <format.h>
#include "../src/sim_pc/SimFlashMemoryPc.h"

using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::static_memory;
using namespace ::SimpleFlashFs::SimPc;

namespace {

class TestCaseInitBase : public TestCaseBase<bool>
{
	std::size_t page_size;
	std::size_t size;

public:
	TestCaseInitBase( const std::string & name_,
			std::size_t page_size_ = 1024,
			std::size_t size_ = 100*1024,
			bool expected_result_ = true,
			bool exception_ = false )
	: TestCaseBase<bool>( name_, expected_result_, exception_ ),
	  page_size( page_size_ ),
	  size( size_ )
	{

	}



	bool init()
	{
		const std::string file = "test.bin";
		::SimpleFlashFs::SimPc::SimFlashFsFlashMemory mem(file,size);
		SimpleFs fs(&mem);

		if( !fs.create(fs.create_default_header(page_size, size/page_size)) ) {
			throw STDERR_EXCEPTION( format( "cannot create %s", file ) );
		}

		return true;
	}

	bool run() override
	{
		if( !init() ) {
			return false;
		}

		return true;
	}

};

} // namespace


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


