/**
 * Testcases for cpputils/cpputilsshared/CyclicArray.h
 * @author Copyright (c) 2023 Martin Oberzalek
 */
#include "CyclicArray.h"
#include "test_cyclicarray.h"
#include "ColoredOutput.h"
#include <initializer_list>
#include <variant>
#include <CpputilsDebug.h>
#include <format.h>

#include <memory_resource>
#include <vector>

using namespace Tools;

namespace {

template<class T,std::size_t N>
bool operator==( const CyclicArray<T,N> & c, const std::vector<T> & v )
{
    if( c.size() != v.size() ) {
    	// CPPDEBUG( format("c.size != v.size => %d != %d", c.size(), v.size()) );
        return false;
    }

    for( unsigned i = 0; i < c.size(); i++ ) {
        if( c[i] != v[i] ) {
            return false;
        }
    }

    return true;
}

template<class T,std::size_t N>
bool operator==( const CyclicArray<T,N> & c, const std::list<T> & v )
{
    if( c.size() != v.size() ) {
    	// CPPDEBUG( format("c.size != v.size => %d != %d", c.size(), v.size()) );
        return false;
    }

    auto it_c = c.begin();
    auto it_v = v.begin();

    for( ; it_c != c.end() && it_v != v.end(); ++it_c, ++it_v ) {
    	if( *it_c != *it_v ) {
    		return false;
    	}
    }

    return true;
}

}

namespace std {
template <class T, std::size_t N>
std::ostream & operator<<( std::ostream & out, const CyclicArray<T,N> & carray )
{
	out << "[" << carray.size() << "]";

	out << "{";

	bool first = true;

	for( const T & t : carray ) {

		if( first ) {
			first = false;
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
			first = false;
		} else {
			out << ", ";
		}

		out << t;
	}

	out << "}";

	return out;
}
} // namespace std

namespace {

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

} // namespace

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

namespace {

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

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_reverse_iterator()
{
	return std::make_shared<TestReverseIterator>(__FUNCTION__);
}

namespace {

class TestInsert1 : public TestCaseBase<bool>
{
public:
	TestInsert1( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		CyclicArray<int,10> c({ 1, 2, 3, 4, 5 });
		std::vector<int> v({ 1, 2, 3, 4, 5 });

		c.insert(c.end(),-1);
		v.insert(v.end(),-1);

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_insert1()
{
	return std::make_shared<TestInsert1>(__FUNCTION__);
}

namespace {

class TestInsert2 : public TestCaseBase<bool>
{
public:
	TestInsert2( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		CyclicArray<int,10> c({ 1, 2, 3, 4, 5 });
		std::vector<int> v({ 1, 2, 3, 4, 5 });

		c.insert(c.begin()+2,-1);
		v.insert(v.begin()+2,-1);

		CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_insert2()
{
	return std::make_shared<TestInsert2>(__FUNCTION__);
}

namespace {

class TestInsert3 : public TestCaseBase<bool>
{
public:
	TestInsert3( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		CyclicArray<int,10> c({ 1, 2, 3, 4, 5 });
		std::vector<int> v({ 1, 2, 3, 4, 5 });

		c.insert(c.begin(),-1);
		v.insert(v.begin(),-1);

		CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_insert3()
{
	return std::make_shared<TestInsert3>(__FUNCTION__);
}

namespace {

class InstanceCounter
{
	std::string name;
	std::string pos;
	unsigned count = 0;

public:
	InstanceCounter( const std::string & name = "x", const std::string & pos = "x" )
	: name( name ),
	  pos(pos)
	{
	}

	InstanceCounter( const InstanceCounter & other )
	: name( other.name ),
	  pos(other.pos),
	  count( other.count + 1 )
	{
		CPPDEBUG( format( "%s:%s %s %d", __FUNCTION__, pos, name, count ) );
	}

	InstanceCounter( InstanceCounter && other )
	: name( other.name ),
	  pos(other.pos),
	  count( other.count )
	{
		CPPDEBUG( format( "%s:%s %s %d", __FUNCTION__, pos, name, count ) );
	}

	~InstanceCounter() {
		CPPDEBUG( format( "%s:%s %s %d", __FUNCTION__, pos, name, count ) );
	}

	const InstanceCounter & operator=( const InstanceCounter & other )
	{
		CPPDEBUG( format( "%s:%s %s %d", __FUNCTION__, pos, name, count ) );
		name = other.name;
		pos = other.pos;
		count++;
		return *this;
	}

	bool operator!=( const InstanceCounter & other ) const
	{
		if( count != other.count ) {
			CPPDEBUG( format( "%s:%s %s %d count is differtent", __FUNCTION__, pos, name, count ) );
			return true;
		}

		if( name != other.name ) {
			CPPDEBUG( format( "%s:%s %s %d name is different", __FUNCTION__, pos, name, count ) );
			return true;
		}

		return false;
	}
};

class TestInsert4 : public TestCaseBase<bool>
{
public:
	TestInsert4( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		CyclicArray<InstanceCounter,10> c;
		std::vector<InstanceCounter> v(10);
		v.resize(0);

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace


std::shared_ptr<TestCaseBase<bool>> test_case_insert4()
{
	return std::make_shared<TestInsert4>(__FUNCTION__);
}

namespace {

class TestInsert5 : public TestCaseBase<bool>
{
public:
	TestInsert5( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		CyclicArray<InstanceCounter,2> c;
		std::list<InstanceCounter> v;


		c.push_back(InstanceCounter("a", "c"));
		c.insert(c.begin(),InstanceCounter("b", "c"));

		v.push_back(InstanceCounter("a", "v"));
		v.insert(v.begin(),InstanceCounter("b", "v"));

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_insert5()
{
	return std::make_shared<TestInsert5>(__FUNCTION__);
}

