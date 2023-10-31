/**
 * Testcases for cpputils/cpputilsshared/CyclicArray.h
 * @author Copyright (c) 2023 Martin Oberzalek
 */
#include "static_vector.h"
#include "test_static_vector.h"
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
bool operator==( const static_vector<T,N> & c, const std::vector<T> & v )
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

} // namespace

namespace std {
template <class T, std::size_t N>
std::ostream & operator<<( std::ostream & out, const static_vector<T,N> & carray )
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
	using CONTAINER = std::variant<static_vector<T,N>,std::vector<T>>;
	typedef std::function<void(CONTAINER&)> Func;
	Func func;

	static_vector<T,N> c;
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
	static_vector<T,N> c;
	std::vector<T>   v;

	TestInitEqualToVector( const std::string & descr, const static_vector<T,N> & c, const std::vector<T> & v )
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
		static_vector<T,N> c({ 1, 2, 3, 4, 5 });
		return !c.empty();
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector1()
{
	return std::make_shared<TestInitEqualToVector<int,10>>(__FUNCTION__,static_vector<int,10>({1,2,3}), std::vector<int>({1,2,3}));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector2()
{
	return std::make_shared<TestInitEqualToVector<int,10>>(__FUNCTION__,static_vector<int,10>{1,2,3}, std::vector<int>{1,2,3});
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector3()
{
	return std::make_shared<TestInitEqualToVector<int,10>>(__FUNCTION__,static_vector<int,10>()={1,2,3}, std::vector<int>()={1,2,3});
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector4()
{
	return std::make_shared<TestInitException<int,10>>(__FUNCTION__,false);
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_vector5()
{
	return std::make_shared<TestInitException<int,3>>(__FUNCTION__,true);
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_vector1()
{
	return std::make_shared<TestEqualToVector<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							e.erase(e.begin());
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_vector2()
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
		static_vector<int,10> c({ 1, 2, 3, 4, 5 });
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

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_reverse_iterator()
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
		static_vector<int,10> c({ 1, 2, 3, 4, 5 });
		std::vector<int> v({ 1, 2, 3, 4, 5 });

		v.insert(v.end(),-1);
		c.insert(c.end(),-1);

		CPPDEBUG( format( "c: %s v: %s", c, v ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert1()
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
		static_vector<int,10> c({ 1, 2, 3, 4, 5 });
		std::vector<int> v({ 1, 2, 3, 4, 5 });

		c.insert(c.begin()+2,-1);
		v.insert(v.begin()+2,-1);

		CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert2()
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
		static_vector<int,10> c({ 1, 2, 3, 4, 5 });
		std::vector<int> v({ 1, 2, 3, 4, 5 });

		c.insert(c.begin(),-1);
		v.insert(v.begin(),-1);

		CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert3()
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
		static_vector<InstanceCounter,10> c;
		std::vector<InstanceCounter> v;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace


std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert4()
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
		static_vector<InstanceCounter,10> c;
		std::vector<InstanceCounter> v;
		v.reserve(10);

		c.push_back(InstanceCounter("a", "c"));
		c.insert(c.begin(),InstanceCounter("b", "c"));

		v.push_back(InstanceCounter("a", "v"));
		v.insert(v.begin(),InstanceCounter("b", "v"));

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert5()
{
	return std::make_shared<TestInsert5>(__FUNCTION__);
}

namespace {

class TestInsert6 : public TestCaseBase<bool>
{
public:
	TestInsert6( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		static_vector<int,10> c;
		std::vector<int> v { 1, 2, 3, 4, 5 };
		v.reserve(10);

		c = v;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert6()
{
	return std::make_shared<TestInsert6>(__FUNCTION__);
}


namespace {

class TestInsert7 : public TestCaseBase<bool>
{
public:
	TestInsert7( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		static_vector<int,10> c;
		std::pmr::vector<int> v { 1, 2, 3, 4, 5 };
		v.reserve(10);

		c = v;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert7()
{
	return std::make_shared<TestInsert7>(__FUNCTION__);
}


namespace {

class TestInsert8 : public TestCaseBase<bool>
{
public:
	TestInsert8( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		static_vector<int,10> c { 1, 2, 3, 4, 5 };
		std::pmr::vector<int> v;
		v.reserve(10);

		v = c;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert8()
{
	return std::make_shared<TestInsert8>(__FUNCTION__);
}



namespace {

class TestInsert9 : public TestCaseBase<bool>
{
public:
	TestInsert9( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {
		static_vector<int,10> c { 1, 2, 3, 4, 5 };
		std::vector<int> v;
		v.reserve(10);

		v = c;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_vector_insert9()
{
	return std::make_shared<TestInsert9>(__FUNCTION__);
}
