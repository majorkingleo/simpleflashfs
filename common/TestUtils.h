/**
 * Test class
 * @author Copyright (c) 2023 Martin Oberzalek
 */

#ifndef TEST_TESTUTILS_H_
#define TEST_TESTUTILS_H_

#include <string>

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



#endif /* TEST_TESTUTILS_H_ */
