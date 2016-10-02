
#include "StringKeyMap.h"

#include <iostream>

	/**
	 * Demo: map strings to member function pointers and apply on an instance
	 */
	class Demo
	{
	public:
		using FuncMap =  Utility::StringKeyMap<void (Demo::*)(std::string const&) const, Utility::ToUpper>;

		Demo(std::ostream& os, std::string const& prefix)
		: os_(os)
		, prefix_(prefix)
		{}

		static FuncMap const& getMap();
	
	private:
		void output( std::string const& arg1, std::string const& arg2 ) const
		{
			os_ << prefix_ << ": " << arg2 << " [" << arg1 << "]" << std::endl;
		}

		void one  ( std::string const& arg ) const { output( arg, "1" ); }
		void two  ( std::string const& arg ) const { output( arg, "2" ); }
		void three( std::string const& arg ) const { output( arg, "3" ); }
		void dflt ( std::string const& arg ) const { output( arg, "default" ); }

		std::ostream&	os_;
		std::string		prefix_;
	};

	Demo::FuncMap const& Demo::getMap()
	{
		static Demo::FuncMap	_funcMap({
			{ "one", &Demo::one },
			{ "two", &Demo::two },
			{ "three", &Demo::three }
			}, &Demo::dflt);
		return _funcMap;
	}
	
void translate( std::istream& is, Demo const& demo )
{
	Demo::FuncMap		_map(Demo::getMap());
	std::string			_line;

	while ( std::getline( is, _line ) )
	{
		(demo.*_map[_line])( _line );
	}
}

int main()
{
	translate( std::cin, Demo(std::cout, "result") );
	return 0;
}

