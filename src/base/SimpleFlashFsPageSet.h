/*
 * SimpleFlashFsPageSet.h
 *
 *  Created on: 30.08.2024
 *      Author: martin.oberzalek
 */
#include <limits>
#include <algorithm>

#pragma once

namespace SimpleFlashFs::base {

template <class Config>
class PageSet
{
public:
	static constexpr uint32_t NO_DATA = std::numeric_limits<uint32_t>::max();
    
    using size_type      = typename Config::template vector_type<uint32_t>::size_type;
	using iterator       = typename Config::template vector_type<uint32_t>::iterator;
	using const_iterator = typename Config::template vector_type<uint32_t>::const_iterator;
	using vector_type    = typename Config::template vector_type<uint32_t>;
    
private:
    typename Config::template vector_type<uint32_t> data;

	bool unsorted = false;
	bool unshrinked = false;

public:

	template< class InputIt >
	void unordered_insert( InputIt first, InputIt last ) {
		data.insert( data.end(), first, last );
		unsorted = true;
		unshrinked = true;
	}

	void sort() {
		std::sort( data.begin(), data.end() );

		// remove NO_DATA values from the end
		for( auto it = data.rbegin();
			 it != data.rend() && *it == NO_DATA && !data.empty();
			 it = data.rbegin() )
		{
			data.resize(data.size()-1);
		}

		unsorted = false;
		unshrinked = false;
	}

	void unordered_insert( const uint32_t value ) {
		data.push_back( value );
		unsorted = true;
	}

	void insert( const uint32_t value ) {

		if( unsorted ) {
			sort();
		}

		iterator prev = data.end();

		for( auto it = data.begin();
				  it != data.end(); prev = it, ++it ) {
			if( *it == value ) {
				return;
			}

			if( *it == NO_DATA ) {
				continue;
			}

			if( *it > value ) {
				if( prev != data.end() && *prev == NO_DATA ) {
					*prev = value;
					return;
				}

				data.insert(it, value);
				return;
			}
		}

		data.push_back(value);
	}

	size_type erase( const uint32_t value )
	{
		if( unsorted ) {
			sort();
		}

		for( auto it = data.begin();
				  it != data.end(); ++it ) {
			if( *it == value ) {
				*it = NO_DATA;
				unshrinked = true;
				return 1;
			}

			if( *it == NO_DATA ) {
				unshrinked = true;
				continue;
			}

			if( *it > value ) {
				return 0;
			}
		}

		return 0;
	}

	void erase( const_iterator const_it ) {
		auto it = data.erase(const_it, const_it);
		*it = NO_DATA;
		unshrinked = true;
	}

	size_type count(const uint32_t member) {

		if( unsorted ) {
			sort();
		}

		for( auto it = data.begin(); it != data.end(); ++it ) {
			if( *it == member ) {
				return 1;
			}
			if( *it != NO_DATA && *it > member ) {
				return 0;
			}
		}

		return 0;
	}

	const typename Config::template vector_type<uint32_t> & get_data() const {
		return data;
	}

	const typename Config::template vector_type<uint32_t> & get_data() {
		if( unsorted || unshrinked ) {
			sort();
		}
		return data;
	}

	typename Config::template vector_type<uint32_t> get_sorted_data() const {
		typename Config::template vector_type<uint32_t> ret( data );
		std::sort( ret.begin(), ret.end() );

		for( unsigned i = 0; i < ret.size(); ++i ) {
			if( ret[i] == NO_DATA ) {
				ret.resize(i);
				break;
			}
		}

		return ret;
	}

	void clear() {
		data.clear();
		unsorted = false;
		unshrinked = false;
	}

	size_type size() {
		if( unsorted ) {
			sort();
		}

		return data.size();
	}

	size_type size() const {
		if( unsorted == false && unshrinked == false) {
			return data.size();
		}

		return std::count_if(data.begin(), data.end(), [](const uint32_t n) { return n != SimpleFlashFs::base::PageSet<Config>::NO_DATA; });
	}

	bool empty() {
		if( unsorted || unshrinked ) {
			sort();
		}

		return data.empty();
	}

	bool empty() const {
		if( unsorted == false && unshrinked == false ) {
			return data.empty();
		}

		return !std::find(data.begin(), data.end(), [](const uint32_t n) { return n != SimpleFlashFs::base::PageSet<Config>::NO_DATA; });
	}

	const_iterator begin() const {
		return data.cbegin();
	}
};

} // namespace SimpleFlashFs::base
