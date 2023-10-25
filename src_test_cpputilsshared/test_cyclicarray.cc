/**
 * Testcases for cpputils/cpputilsshared/CyclicArray.h
 * @author Copyright (c) 2023 Martin Oberzalek
 */
#include "CyclicArray.h"
#include "test_cyclicarray.h"
#include "ColoredOutput.h"
#include <initializer_list>
#include <variant>
#include <OutDebug.h>
#include <format.h>

using namespace Tools;

template<class T,std::size_t N>
bool operator==( const CyclicArray<T,N> & c, const std::vector<T> & v )
{
    if( c.size() != v.size() ) {
    	//CPPDEBUG( format("c.size != v.size => %d != %d", c.size(), v.size()) );
        return false;
    }

    for( unsigned i = 0; i < c.size(); i++ ) {
        if( c[i] != v[i] ) {
            return false;
        }
    }

    return true;
}

template <class T, std::size_t N>
std::ostream & operator<<( std::ostream & out, const CyclicArray<T,N> & carray )
{
	out << "[" << carray.size() << "]";

	out << "{";

	bool first = true;

	for( const T & t : carray ) {

		if( first ) {
			first = true;
		} else {
			out << ", ";
		}

		out << t;
	}

	out << "}";

	return out;
}

template <class T>
std::ostream & operator<<( std::ostream & out, const std::vector<T> & varray )
{
	out << "[" << varray.size() << "]";

	out << "{";

	bool first = true;

	for( const T & t : varray ) {

		if( first ) {
			first = true;
		} else {
			out << ", ";
		}

		out << t;
	}

	out << "}";

	return out;
}

template <class T, std::size_t N=10>
struct TestEqualToVector : public TestCaseBase<bool>
{
	using CONTAINER = std::variant<CyclicArray<T,N>,std::vector<T>>;
	typedef std::function<void(CONTAINER&)> Func;
	Func func;

    CyclicArray<T,N> c;
    std::vector<T>   v;

    TestEqualToVector( const std::string & descr, Func func, std::initializer_list<T> list = {1,2,3,4,5,6,7,8,9,10} )
	: TestCaseBase<bool>( descr, true, false ),
	  func( func ),
	  c(list),
	  v(list)
	{}

    bool run() override
    {
    	CONTAINER cc = c;
    	CONTAINER cv = v;

        func( cc );
        func( cv );

        return std::get<0>(cc) == std::get<1>(cv);
    }
};

template <class T, std::size_t N=10>
class TestInitEqualToVector : public TestCaseBase<bool>
{
public:
	CyclicArray<T,N> c;
	std::vector<T>   v;

	TestInitEqualToVector( const std::string & descr, const CyclicArray<T,N> & c, const std::vector<T> & v )
	: TestCaseBase<bool>( descr, true ),
	  c(c),
	  v(v)
	{}

	bool run() override {
		return c == v;
	}
};

template <class T, std::size_t N=10>
class TestInitException : public TestCaseBase<bool>
{
public:
	TestInitException( const std::string & descr, bool exception_expected )
	: TestCaseBase<bool>( descr, true, exception_expected )
	{}

	bool run() override {
		CyclicArray<T,N> c({ 1, 2, 3, 4, 5 });
		return !c.empty();
	}
};


std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array1()
{
	return std::make_shared<TestInitEqualToVector<int,10>>(__FUNCTION__,CyclicArray<int,10>({1,2,3}), std::vector<int>({1,2,3}));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array2()
{
	return std::make_shared<TestInitEqualToVector<int,10>>(__FUNCTION__,CyclicArray<int,10>{1,2,3}, std::vector<int>{1,2,3});
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array3()
{
	return std::make_shared<TestInitEqualToVector<int,10>>(__FUNCTION__,CyclicArray<int,10>()={1,2,3}, std::vector<int>()={1,2,3});
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array4()
{
	return std::make_shared<TestInitException<int,10>>(__FUNCTION__,false);
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_cyclic_array5()
{
	return std::make_shared<TestInitException<int,3>>(__FUNCTION__,true);
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_cyclic_array1()
{
	return std::make_shared<TestEqualToVector<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							e.erase(e.begin());
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_cyclic_array2()
{
	return std::make_shared<TestEqualToVector<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							e.erase(e.begin()+3);
						}, v );
			});
}

class TestReverseIterator : public TestCaseBase<bool>
{
public:
	TestReverseIterator( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		CyclicArray<int,10> c({ 1, 2, 3, 4, 5 });
		std::vector<int> v({ 1, 2, 3, 4, 5 });

		if( *c.rbegin() != *v.rbegin() ) {
			return false;
		}

		if( *(--c.rend()) != *(--v.rend()) ) {
			return false;
		}

		return true;
	}
};

std::shared_ptr<TestCaseBase<bool>> test_case_reverse_iterator()
{
	return std::make_shared<TestReverseIterator>(__FUNCTION__);
}
