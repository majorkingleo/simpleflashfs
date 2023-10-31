/**
 * Testcases for cpputils/cpputilsshared/static_list.h
 * @author Copyright (c) 2023 Martin Oberzalek
 */
#include "test_static_list.h"
#include "ColoredOutput.h"
#include <initializer_list>
#include <variant>
#include <CpputilsDebug.h>
#include <string_utils.h>
#include <format.h>

#include <memory_resource>
#include <list>
#include "static_list.h"

using namespace Tools;

namespace {

template<class T,std::size_t N>
bool operator==( const static_list<T,N> & c, const std::list<T> & v )
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

template<class T,std::size_t N>
bool operator==( const static_list<T,N> & c, const std::pmr::list<T> & v )
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

} // namespace

namespace std {
template <class T, std::size_t N>
std::ostream & operator<<( std::ostream & out, const static_list<T,N> & carray )
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
std::ostream & operator<<( std::ostream & out, const std::list<T> & varray )
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
struct TestEqualTolist : public TestCaseBase<bool>
{
	using CONTAINER = std::variant<static_list<T,N>,std::list<T>>;
	typedef std::function<void(CONTAINER&)> Func;
	Func func;

	static_list<T,N> c;
    std::list<T>   v;

    TestEqualTolist( const std::string & descr, Func func, std::initializer_list<T> list = {1,2,3,4,5,6,7,8,9,10} )
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
class TestInitEqualTolist : public TestCaseBase<bool>
{
public:
	static_list<T,N> c;
	std::list<T>   v;

	TestInitEqualTolist( const std::string & descr, const static_list<T,N> & c, const std::list<T> & v )
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
		static_list<T,N> c({ 1, 2, 3, 4, 5 });
		return !c.empty();
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list1()
{
	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>({1,2,3}), std::list<int>({1,2,3}));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list2()
{
	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>{1,2,3}, std::list<int>{1,2,3});
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list3()
{
	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>()={1,2,3}, std::list<int>()={1,2,3});
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list4()
{
	return std::make_shared<TestInitException<int,10>>(__FUNCTION__,false);
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list5()
{
	return std::make_shared<TestInitException<int,3>>(__FUNCTION__,true);
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list6()
{
	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>(5), std::list<int>(5));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list7()
{
	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>(5,7), std::list<int>(5,7));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list8()
{
	std::vector<int> v = { 1, 2, 3 };

	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>(v.begin(),v.end()), std::list<int>(v.begin(),v.end()));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list9()
{
	static_list<int,10> c = { 1, 2, 3 };
	std::list<int> v = { 1, 2, 3 };

	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>(c), std::list<int>(v));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list10()
{
	std::list<int> v = { 1, 2, 3 };

	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>(v), std::list<int>(v));
}

std::shared_ptr<TestCaseBase<bool>> test_case_init_static_list11()
{
	std::pmr::list<int> v = { 1, 2, 3 };

	return std::make_shared<TestInitEqualTolist<int,10>>(__FUNCTION__,static_list<int,10>(v), std::list<int>({1,2,3}));
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_list1()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							auto it_new_first = ++e.begin();
							e.erase(e.begin());
							*it_new_first = -1;
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_list2()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							auto it_somewhere = --e.end();
							e.erase(++e.begin());
							*it_somewhere = -1;
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_list3()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							e.clear();
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_list4()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							//CPPDEBUG( format( "%s", typeid(e).name() ) );
							auto is_end = e.erase(--e.end());
							//CPPDEBUG( format( "%s end: %s", typeid(e).name(), is_end == e.end() ) );
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_list5()
{
#if 0
	{
		std::list<int> v{ 1, 2, 3, 4, 5 };
		auto it = v.erase(++v.begin());
		CPPDEBUG( format("after first delete: %s it points to: %d", v, *it ) );
		it = v.erase(it);
		CPPDEBUG( format("it %s points to: %d", v, *it ) );
		it = v.erase(it);
		CPPDEBUG( format("it %s points to: %d", v, *it ) );
		it = v.erase(it);
		CPPDEBUG( format("it %s points to: %d", v, it == v.end() ? "end" : x2s(*it) ) );

		//it = v.erase(it);
		//CPPDEBUG( format("it %s points to: %d", v, it == v.end() ? "end" : x2s(*it) ) );
	}

	{
		static_list<int,10> v{ 1, 2, 3, 4, 5 };
		auto it = v.erase(++v.begin());
		CPPDEBUG( format("after first delete: %s it points to: %d", v, *it ) );
		it = v.erase(it);
		CPPDEBUG( format("it %s points to: %d", v, *it ) );
		it = v.erase(it);
		CPPDEBUG( format("it %s points to: %d", v, *it ) );
		it = v.erase(it);
		CPPDEBUG( format("it %s points to: %d", v, it == v.end() ? "end" : x2s(*it) ) );
		// it = v.erase(it);
		// CPPDEBUG( format("it %s points to: %d", v, it == v.end() ? "end" : x2s(*it) ) );
	}
#endif

	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							auto it_first = e.begin();
							auto it_last = --e.end();
							auto is_end = e.erase(++e.begin(), --it_last);
							*it_first = -1;
							*it_last = -2;
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_modify_static_list6()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){

							auto is_end = e.erase(e.begin(), e.end());

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
		static_list<int,10> c({ 1, 2, 3, 4, 5 });
		std::list<int> v({ 1, 2, 3, 4, 5 });

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

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_reverse_iterator()
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
		static_list<int,10> c({ 1, 2, 3, 4, 5 });
		std::list<int> v({ 1, 2, 3, 4, 5 });

		v.insert(v.end(),-1);
		c.insert(c.end(),-1);

		CPPDEBUG( format( "c: %s v: %s", c, v ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert1()
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
		static_list<int,10> c({ 1, 2, 3, 4, 5 });
		std::list<int> v({ 1, 2, 3, 4, 5 });

		c.insert(++c.begin(),-1);
		v.insert(++v.begin(),-1);

		CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert2()
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
		static_list<int,10> c({ 1, 2, 3, 4, 5 });
		std::list<int> v({ 1, 2, 3, 4, 5 });

		c.insert(c.begin(),-1);
		v.insert(v.begin(),-1);

		CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert3()
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
		static_list<InstanceCounter,10> c;
		std::list<InstanceCounter> v;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace


std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert4()
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
		static_list<InstanceCounter,10> c;
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

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert5()
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
		static_list<int,10> c;
		std::list<int> v { 1, 2, 3, 4, 5 };

		c = v;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert6()
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
		static_list<int,10> c;
		std::pmr::list<int> v { 1, 2, 3, 4, 5 };

		c = v;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert7()
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
		static_list<int,10> c { 1, 2, 3, 4, 5 };
		std::pmr::list<int> v;

		v = c;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert8()
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
		static_list<int,10> c { 1, 2, 3, 4, 5 };
		std::list<int> v;

		v = c;

		//CPPDEBUG( format("v: %s c: %s", v, c ) );

		return c == v;
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert9()
{
	return std::make_shared<TestInsert9>(__FUNCTION__);
}


std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert10()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							auto it_begin = e.begin();

							auto it = e.insert(++e.begin(),static_cast<unsigned>(3),3);

							// test if we have got back the correct it, so modify the value
							// it points to
							*it = -1;
							*it_begin = 0;
						}, v );
			});
}


std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert11()
{
	static_list<int,10> l{ 1, 2, 3 };

	//auto it_b = l.begin();
	auto it_e1 = --l.end();
	auto it_e = l.end();

	l.push_back( 4 );

	CPPDEBUG( format( "end: %d rbegin(): %d", it_e == l.end(), *it_e1 ) );

	l = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	l.pop_back();
	l.pop_back();
	l.pop_back();
	l.pop_back();

	std::vector<int> v{ 1, 2, 3 };

	auto it_start = l.begin();
	it_start++;

	CPPDEBUG( format( "start: %d", *it_start ) );

	auto it = l.insert(it_start,v.begin(),v.end());

	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){

							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							auto it_begin = e.begin();

							std::vector<int> v{ 1, 2, 3 };
							auto it = e.insert(++e.begin(),v.begin(),v.end());

							// test if we have got back the correct it, so modify the value
							// it points to
							*it = -1;
							*it_begin = 0;
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert12()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							auto it = e.insert(++e.begin(),{ 1, 2, 3 });

							CPPDEBUG( format( "%s %s" , typeid(e).name(), e ) );

							// test if we have got back the correct it, so modify the value
							// it points to
							*it = -1;
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert13()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							auto iterator_to_the_end = e.rbegin();
							auto value_at_the_end = *e.rbegin();

							CPPDEBUG( format( "%s %s ", typeid(e).name(), e ) );

							auto it = e.insert(++e.begin(),{ 1, 2, 3 });

							// test if we have got back the correct it, so modify the value
							// it points to
							*it = -1;

							CPPDEBUG( format( "%s %s ", typeid(e).name(), e ) );
							CPPDEBUG( format( "itend: %s == vend before: %s", *iterator_to_the_end, value_at_the_end ) );

							*iterator_to_the_end = -2;
						}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert14()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							std::list<int> v{ 1, 2, 3 };
							auto it = e.insert(++e.begin(),v.begin(), v.begin());

				}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert15()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							std::list<int> v{ 1, 2, 3 };
							auto it = e.insert(++e.begin(),v.begin(), ++v.begin());

				}, v );
			});
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_insert16()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							auto it = e.insert(++e.begin(),{});

				}, v );
			});
}

namespace {

class TestFront1 : public TestCaseBase<bool>
{
public:
	TestFront1( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {

		static_list<int,10> c { 1, 2, 3, 4, 5 };
		std::list<int> v { 1, 2, 3, 4, 5 };

		return c.front() == v.front();
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_front1()
{
	return std::make_shared<TestFront1>(__FUNCTION__);
}


namespace {

class TestFront2 : public TestCaseBase<bool>
{
public:
	TestFront2( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {

		const static_list<int,10> c { 1, 2, 3, 4, 5 };
		const std::list<int> v { 1, 2, 3, 4, 5 };

		return c.front() == v.front();
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_front2()
{
	return std::make_shared<TestFront2>(__FUNCTION__);
}


namespace {

class TestBack1 : public TestCaseBase<bool>
{
public:
	TestBack1( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {

		static_list<int,10> c { 1, 2, 3, 4, 5 };
		std::list<int> v { 1, 2, 3, 4, 5 };

		return c.back() == v.back();
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_back1()
{
	return std::make_shared<TestBack1>(__FUNCTION__);
}


namespace {

class TestBack2 : public TestCaseBase<bool>
{
public:
	TestBack2( const std::string & descr )
	: TestCaseBase<bool>( descr, true, false )
	{}

	bool run() override {

		const static_list<int,10> c { 1, 2, 3, 4, 5 };
		const std::list<int> v { 1, 2, 3, 4, 5 };

		return c.back() == v.back();
	}
};

} // namespace

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_back2()
{
	return std::make_shared<TestBack2>(__FUNCTION__);
}

std::shared_ptr<TestCaseBase<bool>> test_case_static_list_emplace1()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){
							// make space
							e.resize(4);
							e.resize(6);

							auto it_before = ++e.begin();
							auto it = e.emplace(++e.begin(),11);
							*it_before = -1;
				}, v );
			});
}


std::shared_ptr<TestCaseBase<bool>> test_case_static_list_emplace2()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){

							auto it_end = --e.end();
							// make space
							e.pop_front();
							e.pop_front();
							e.pop_front();
							e.pop_front();

							auto it_before = ++e.begin();
							auto & ele = e.emplace_front(11);
							*it_before = -1;
							ele = 12;
							*it_end = 13;
				}, v );
			});
}


std::shared_ptr<TestCaseBase<bool>> test_case_static_list_push_front1()
{
	return std::make_shared<TestEqualTolist<int,10>>(__FUNCTION__,
			[]( auto & v ) {
				std::visit(
						[](auto & e){

							auto it_before = ++e.begin();

							// make space
							e.pop_back();
							e.pop_back();
							e.pop_back();
							e.pop_back();

							int x = 11;
							e.push_front(x);
							e.push_front(12);
							*it_before = -1;
				}, v );
			});
}

