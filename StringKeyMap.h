#pragma once

#include <string>
#include <initializer_list>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <boost/algorithm/string/case_conv.hpp>

namespace Utility
{
	struct ToUpper { static std::string normalize( std::string const& item ) { return boost::to_upper_copy( item ); } };
	struct ToLower { static std::string normalize( std::string const& item ) { return boost::to_lower_copy( item ); } };
	struct NoConv  { static std::string normalize( std::string const& item ) { return item; } };
	
    template<typename Value>
    struct DefaultValue
    {
        Value   dflt_;
        DefaultValue(Value const& value = Value()) : dflt_(value) {}
        Value operator() ( std::string const& ) const { return dflt_; }
    };

    template<typename Value>
    struct DefaultThrow
    {
        DefaultThrow(Value const& value = Value()) {}
        Value operator() ( std::string const& key ) const { throw std::out_of_range(key); }
    };


	// StringKeyMap. 
	// Wrapper around STL map with value defaults, key case conversions, helpers. 
	//
	template <
		typename Value,
		typename Case = NoConv,
        template<typename> class DefaultPolicy = DefaultValue,
		typename Map = std::unordered_map<std::string, Value>
		>
	class StringKeyMap
	{
	public:
        using Policy = DefaultPolicy<Value>;
		explicit
		StringKeyMap(Policy const& policy = Policy())
		: map_()
		, policy_(policy)
		{}
		
		StringKeyMap(std::initializer_list<typename Map::value_type> il, Policy const& policy = Policy())
		: StringKeyMap(il.begin(), il.end(), policy)
		{}

		template <typename Iterator>
		StringKeyMap(Iterator begin, Iterator end, Policy const& policy = Policy())
		: map_()
		, policy_(policy)
		{
			std::for_each( begin, end, std::ref(*this) );
		}
		
		StringKeyMap& operator() ( typename Map::value_type const& pair )
		{
			return (*this)( pair.first, pair.second );
		}

		StringKeyMap& operator() ( std::string const& key, Value const& value )
		{
			map_[Case::normalize( key )] = value;
			return *this;
		}
		
		Value operator[] ( std::string const& key ) const
		{
			auto	_it(map_.find( Case::normalize( key ) ));
			return _it != map_.end() ? _it->second : policy_( key );
		}

		Value operator() ( std::string const& key ) const
		{
			return (*this)[key];
		}
		
	private:
		Map			map_;
        Policy      policy_;
	};

} // namespace Utility

