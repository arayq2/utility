/** 
 * @file hsmtest.c
 * @brief Hierarchical State Machine test.
 * This is an implementation of the example found in
 * Practical StateCharts in C/C++ by Miro Samek.
 */

#include "Hsm.h"
#include <stdio.h>

typedef int Event;
struct Msg
{
    Event evt;
};

class HsmTest
: public Utility::Hsm<HsmTest, Msg>
{
public:
    HsmTest();
    // state handlers
    Msg const *topHndlr( Msg const *msg );
    Msg const *s1Hndlr( Msg const *msg );
    Msg const *s11Hndlr( Msg const *msg );
    Msg const *s2Hndlr( Msg const *msg );
    Msg const *s21Hndlr( Msg const *msg );
    Msg const *s211Hndlr( Msg const *msg );
protected:
    using State = typename Hsm<HsmTest, Msg>::State;
    State top{"top", nullptr, &HsmTest::topHndlr};
    // states: indentation shows hierarchy
    State s1{"s1", &top, &HsmTest::s1Hndlr};
        State s11{"s11", &s1, &HsmTest::s11Hndlr};
    State s2{"s2", &top, &HsmTest::s2Hndlr};
        State s21{"s21", &s2, &HsmTest::s21Hndlr};
            State s211{"s211", &s21, &HsmTest::s211Hndlr};
private:
    int myFoo;
};

#define ENTRY_EVT ((Event)(-1))
#define START_EVT ((Event)(-2))
#define EXIT_EVT  ((Event)(-3))

static Msg const entryMsg = { ENTRY_EVT };
static Msg const startMsg = { START_EVT };
static Msg const exitMsg  = { EXIT_EVT };

Utility::Hsm<HsmTest, Msg>::SpecMsg specMsgs = { &entryMsg, &startMsg, &exitMsg };

HsmTest::HsmTest()
: Hsm<HsmTest, Msg>("HsmTest", top, specMsgs)
, myFoo(0)
{}

enum HsmTestEvents
{
    A_SIG, B_SIG, C_SIG, D_SIG, E_SIG, F_SIG, G_SIG, H_SIG
};

void output( char const* txt )
{
    fputs( txt, stdout ); fflush( stdout );
}

Msg const *HsmTest::topHndlr( Msg const *msg )
{
    switch ( msg->evt )
    {
    case ENTRY_EVT:
        output( "top-ENTRY;" );
        return nullptr;
    case START_EVT:
        output( "top-INIT;" );
        STATE_START( &s1 );
        return nullptr;
    case EXIT_EVT:
        output( "top-EXIT;" );
        return nullptr;
    case E_SIG:
        output( "top-E;" );
        STATE_TRAN( &s211 );
        return nullptr;
    } 
    return msg;
}

Msg const *HsmTest::s1Hndlr( Msg const *msg )
{
    switch ( msg->evt )
    {
    case ENTRY_EVT:
        output( "s1-ENTRY;" );
        return nullptr;
    case START_EVT:
        output( "s1-INIT;" );
        STATE_START( &s11 );
        return nullptr;
    case EXIT_EVT:
        output( "s1-EXIT;" );
        return nullptr;
    case A_SIG:
        output( "s1-A;" );
        STATE_TRAN( &s1 );
        return nullptr;
    case B_SIG:
        output( "s1-B;" );
        STATE_TRAN( &s11 );
        return nullptr;
    case C_SIG:
        output( "s1-C;" );
        STATE_TRAN( &s2 );
        return nullptr;
    case D_SIG:
        output( "s1-D;" );
        STATE_TRAN( &top );
        return nullptr;
    case F_SIG:
        output( "s1-F;" );
        STATE_TRAN( &s211 );
        return nullptr;
    } 
    return msg;
}

Msg const *HsmTest::s11Hndlr( Msg const *msg )
{
    switch ( msg->evt )
    {
    case ENTRY_EVT:
        output( "s11-ENTRY;" );
        return nullptr;
    case EXIT_EVT:
        output( "s11-EXIT;" );
        return nullptr;
    case G_SIG:
        output( "s11-G;" );
        STATE_TRAN( &s211 );
        return nullptr;
    case H_SIG:
        if ( myFoo ) {
            output( "s11-H;" );
            myFoo = 0;
            return nullptr;
        }
        break;
    } 
    return msg;
}

Msg const *HsmTest::s2Hndlr( Msg const *msg )
{
    switch ( msg->evt )
    {
    case ENTRY_EVT:
        output( "s2-ENTRY;" );
        return nullptr;
    case START_EVT:
        output( "s2-INIT;" );
        STATE_START( &s21 );
        return nullptr;
    case EXIT_EVT:
        output( "s2-EXIT;" );
        return nullptr;
    case C_SIG:
        output( "s2-C;" );
        STATE_TRAN( &s1 );
        return nullptr;
    case F_SIG:
        output( "s2-F;" );
        STATE_TRAN( &s11 );
        return nullptr;
    } 
    return msg;
}

Msg const *HsmTest::s21Hndlr( Msg const *msg )
{
    switch ( msg->evt )
    {
    case ENTRY_EVT:
        output( "s21-ENTRY;" );
        return nullptr;
    case START_EVT:
        output( "s21-INIT;" );
        STATE_START( &s211 );
        return nullptr;
    case EXIT_EVT:
        output( "s21-EXIT;" );
        return nullptr;
    case B_SIG:
        output( "s21-B;" );
        STATE_TRAN( &s211 );
        return nullptr;
    case H_SIG:
        if ( !myFoo ) {
            output( "s21-H;" );
            myFoo = 1;
            STATE_TRAN( &s21 );
            return nullptr;
        }
        break;
    } 
    return msg;
}

Msg const *HsmTest::s211Hndlr( Msg const *msg )
{
    switch ( msg->evt )
    {
    case ENTRY_EVT:
        output( "s211-ENTRY;" );
        return nullptr;
    case EXIT_EVT:
        output( "s211-EXIT;" );
        return nullptr;
    case D_SIG:
        output( "s211-D;" );
        STATE_TRAN( &s21 );
        return nullptr;
    case G_SIG:
        output( "s211-G;" );
        STATE_TRAN( &top );
        return nullptr;
    } 
    return msg;
}


const Msg HsmTestMsg[] =
{
    A_SIG,B_SIG,C_SIG,D_SIG,E_SIG,F_SIG,G_SIG,H_SIG
};

int main() {
    HsmTest hsmTest;
    hsmTest.on_start();
    for (;;) {
        char c;
        output("\nEvent<-");
        c = getc( stdin );
        getc( stdin );
        if (c < 'a' || 'h' < c) {
            break;
        }
        hsmTest.dispatch( &HsmTestMsg[c - 'a'] );
    }
    return 0;
}
