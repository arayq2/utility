
#include "Scoring.h"

#include <iostream>

using namespace bridge;

int main( int ac, char* av[])
{
    auto    _level{Contract::Level::L1};
    auto    _rank{Contract::Rank::NT};
    auto    _dbld{Contract::Dbld::NO};
    bool    _vul{false};
    int     _rslt{0};

    if ( ac < 2 ) { std::cerr << "Need a contract and result!" << std::endl; return 1; }

    char*   _ptr = av[1];

    switch ( *_ptr )
    {
        case '1' : _level = Contract::Level::L1; _rslt =  7; break;
        case '2' : _level = Contract::Level::L2; _rslt =  8; break;
        case '3' : _level = Contract::Level::L3; _rslt =  9; break;
        case '4' : _level = Contract::Level::L4; _rslt = 10; break;
        case '5' : _level = Contract::Level::L5; _rslt = 11; break;
        case '6' : _level = Contract::Level::L6; _rslt = 12; break;
        case '7' : _level = Contract::Level::L7; _rslt = 13; break;
        default: std::cerr << "Bad format at level" << std::endl; return 2;
    }

    switch ( *++_ptr )
    {
        case 'N':
        case 'n': _rank = Contract::Rank::NT; break;
        case 'D':
        case 'd':
        case 'C':
        case 'c': _rank = Contract::Rank::MN; break;
        case 'H':
        case 'h':
        case 'S':
        case 's': _rank = Contract::Rank::MJ; break;
        default: std::cerr << "Bad format at rank" << std::endl; return 3;
    }

    switch ( *++_ptr )
    {
    case 'X':
    case 'x': _dbld = Contract::Dbld::YES; ++_ptr; break;
    case 'R':
    case 'r': _dbld = Contract::Dbld::AGAIN; ++_ptr; break;
    }

    if ( *_ptr == 'V' || *_ptr == 'v' )
    {
        _vul = true;
        ++_ptr;
    }

    switch ( *_ptr )
    {
    default:
    case '=': break;
    case '+': _rslt += ::atoi( ++_ptr );
    case '-': _rslt -= ::atoi( ++_ptr );
    }

    std::cout << Contract(_level, _rank, _dbld, _vul).score( _rslt ) << std::endl;

    return 0;
}
