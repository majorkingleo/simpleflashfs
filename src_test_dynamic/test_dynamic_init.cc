#include "../src_test_dynamic/test_dynamic_init.h"

#include <src/dynamic/SimpleFlashFsDynamic.h>
#include <sim_pc/SimFlashMemoryInterfacePc.h>
#include <stderr_exception.h>
#include <format.h>

using namespace Tools;
using namespace SimpleFlashFs;
using namespace SimpleFlashFs::dynamic;
using namespace SimpleFlashFs::SimPc;

class TestCaseInitBase : public TestCaseBase<bool>
{
	std::size_t page_size;
	std::size_t size;

public:
	TestCaseInitBase( const std::string & name_,
			std::size_t page_size_ = 528,
			std::size_t size_ = 100*1024,
			bool exception_ = false )
	: TestCaseBase<bool>( name, true, exception_ ),
	  page_size( page_size_ ),
	  size( size_ )
	{

	}

	bool run() override
	{
		const std::string file = "test.bin";
		SimFlashFsFlashMemoryInterface mem(file,size);
		SimpleFlashFs::dynamic::SimpleFlashFs fs(&mem);

		if( !fs.create(fs.create_default_header(page_size, size/page_size)) ) {
			throw STDERR_EXCEPTION( format( "cannot create %s", file ) );
		}

		return true;
	}

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
#endif


std::shared_ptr<TestCaseBase<bool>> test_case_init1()
{
	return std::make_shared<TestCaseInitBase>("init1");
}

std::shared_ptr<TestCaseBase<bool>> test_case_init2()
{
	return std::make_shared<TestCaseInitBase>("init2", 1);
}



