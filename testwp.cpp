
#include "WorkPile.h"
#include <iostream>
//#include <unistd.h>

    class Handler
    {
    public:
        using ItemType = char*;

        Handler(std::ostream& os)
        : os_(os)
        {}

        void operator() ( ItemType ptr )
        {
            os_ << "Thread [" << std::this_thread::get_id() << "]: " << ptr << std::endl;
            std::this_thread::yield();
            //::sleep( 1 );
        }

    private:
        std::ostream&   os_;
    };

    using WorkPile = Utility::WorkPile<Handler>;
    
    int main( int ac, char *av[] )
    {
        WorkPile   _workPile(Handler(std::cerr), 5);

        _workPile.put( av + 1, av + ac );
        _workPile.wait();
        return 0;
    }

