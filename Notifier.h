#pragma once

#include <vector>
#include <algorithm>

template<typename Event, typename Client>
struct NotifierMethods
{
    static on_event( Client& client, Event const& event )
    {
        client.on_event( event );
    }
};

template<typename Event, typename Client, typename Methods = NotifierMethods<Event, Client> >
struct Notifier
: std::vector<Client>
{
    Notifier() = default;
    ~Notifier() = default;
    
    void notify( Event const& event )
    {
        for ( auto& _client : *this )
        {
            Methods::on_event( _client, event ); 
        }
    }
    
    template<typename Predicate>
    void notify_if( Predicate const& predicate )
    {
        for ( auto& _client : *this )
        {
            if ( predicate( _client ) ) { Methods::on_event( _client, event ); } 
        }
    }
    
    template<typename Predicate>
    void prune( Predicate const& predicate )
    {
        std::erase( std::remove_if( begin(), end(), predicate ), end() );
    }
};