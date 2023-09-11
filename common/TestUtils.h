/**
 * Test class
 * @author Copyright (c) 2023 Martin Oberzalek
 */

#ifndef TEST_TESTUTILS_H_
#define TEST_TESTUTILS_H_

#include <string>
#include <functional>

template<class RESULT> class TestCaseBase
{
protected:
    std::string name;
    bool throws_exception;
    const RESULT expected_result;

public:
    TestCaseBase( const std::string & name_,
    			  const RESULT & expected_result_,
    			  bool throws_exception_ = false )
    : name( name_ ),
      throws_exception( throws_exception_ ),
      expected_result( expected_result_ )
    {}

    virtual ~TestCaseBase() {}

    virtual RESULT run() = 0;

    const std::string & getName() const {
        return name;
    }

    bool throwsException() const {
        return throws_exception;
    }

    RESULT getExpectedResult() const {
    	return expected_result;
    }
};

template<class t_std_string=std::string>
class TestCaseFuncEqual : public TestCaseBase<bool>
{
	typedef std::function<bool(const t_std_string & a, const t_std_string & b)> Func;
	Func func;
	const t_std_string input;
	const t_std_string output;

public:
	TestCaseFuncEqual( const std::string & name,
			const t_std_string & input_,
			const t_std_string & output_,
			Func func_ )
	: TestCaseBase<bool>( name, true ),
	  input( input_ ),
	  output( output_ ),
	  func( func_ )
	  {}

	bool run() override {
		return func( input, output );
	}
};

template<class t_std_string=std::string>
class TestCaseFuncBool : public TestCaseBase<bool>
{
	typedef std::function<bool(const t_std_string & a)> Func;
	Func func;
	const t_std_string input;

public:
	TestCaseFuncBool( const std::string & name,
			const t_std_string & input_,
			bool expected_result_,
			Func func_ )
	: TestCaseBase<bool>( name, expected_result_ ),
	  input( input_ ),
	  func( func_ )
	  {}

	bool run() override {
		return func( input );
	}
};
#endif /* TEST_TESTUTILS_H_ */
