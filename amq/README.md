
**THIS IS A WORK IN PROGRESS**

The goal is two-fold. First, a simpler C++ API to `ActiveMQ` functionality. Second, to implement this API on an underlying API in C, not `ActiveMQ CMS`, which is C++ unfortunately. Currently, the classes are defined directly on top of `CMS`, treating its objects as if they were "typed void pointers". An underlying C API would in fact provide just those kinds of pointers. The translation of the internals of our wrapper API would be straightforward.

The simpler surface API is based on the KISS principle: _the complex may be harder than the simple, but the simple must be easy_. The code for the `Sender` and `Recver` programs are provided as demonstrations of this. They can be compared with the baroque monstrosities in the `ActiveMQ-CPP` distribution package, such as [`SimpleProducer.cpp`](https://github.com/apache/activemq-cpp/blob/master/activemq-cpp/src/examples/producers/SimpleProducer.cpp) and [`SimpleAyncConsumer.cpp`](https://github.com/apache/activemq-cpp/blob/master/activemq-cpp/src/examples/consumers/SimpleAsyncConsumer.cpp). When you just want to send and receive messages, without bells and whistles, how much "necessary" boilerplate are you willing to tolerate?

The idea of a C interface to a C++ library is not new.  It's called the [Hourglass API pattern](https://www.linkedin.com/pulse/hourglass-interface-cc-apis-c-libraries-jarno-ralli-phd-lfgrf/) (an unfortunate name for search purposes, as there will be a lot of confounding hits to an irrelevant graphic pattern of the same name.)  Here, the aim would be to preserve the guts of the CPP library but expose it via C, not C++.  We can then reconstruct the CMS C++ interface on _that_, rather than directly on the library.

The work is informed by earlier efforts to develop C++ APIs on top of the `Tibco EMS` and `Solace` libraries, both of which are sensible industrial strength implementations. Their only problem is that they cost an arm and a leg, whereas `ActiveMQ`, a well-tested product, is open source and free.  The disadvantage is the lack of a C API.

[Solace](https://github.com/SolaceSamples/solace-samples-c)
[Client](https://github.com/SolaceSamples/solace-samples-c/tree/master/inc/solclient)

(Tibco APIs are not freely available, although their documentation is)

`AmqAPI.h`: The core wrappers of the main `CMS` objects: `Connection`, `Session`, `Destination`, `Producer`, `Consumer`, `Message` (really, only `TextMessage`at this juncture); along with some helpers.

`EndPoint.h`: A helper class for CMS `Destination`s.

`AmqAgent.{h,cpp}`:  An all-in-one simple "Session" class, supporting one included producer and multiple consumers if desired.

`MessageHandler.h`: The basic "listener" class, passed into the `CMS API` to receive messages asynchronously.

`MessageReceiver.h`: A boilerplate encapsulating mixin class to ease the processing of inbound messages.

`Sender.cpp`: A client to send one or more files to a destination. Standard input can be used to send multiple one line messages.

`Recver.cpp`: A client to receive messages from one or more subscripions (either all topics or all queues).
 

**OTHER ITEMS**

Various other "utility" classes are used in these files. They can be found in the parent folder. 



 

 
 