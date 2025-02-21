
THIS IS A WORK IN PROGRESS.

The goal is two-fold. First, a simpler C++ API to ActiveMQ functionality. 
Second, to implement this API on an underlying API in C, not ActiveMQ CMS, 
which is C++ unfortunately. Curently, the classes are defined directly on 
top of CMS, treating its objects as if they were "typed void pointers". An 
underlying C API would in fact provide just that kind of pointers. The 
translation of the internals of our wrapper API would be straightforward.

The simpler surface API is based on the KISS principle: the difficult may 
be harder than the easy, but the easy must be simple. The code for the 
Sender and Recver are provided as demonstrations of this. They can be 
compared with the baroque monstrosities in the ActiveMQ-CPP distribution 
package, such as SimpleProducer.cpp and SimpleAyncConsumer.cpp. When you 
just want to send and receive messages, without bells and whistles, how 
much "necessary" boilerplate are you willing to tolerate?



 

 
 