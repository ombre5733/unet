#ifndef DYNAMICADDRESSSERVER_HPP
#define DYNAMICADDRESSSERVER_HPP

namespace uNet
{

/*!

Dynamic Address Service

The Dynamic Address Service (DAS) is used to dynamically assign addresses
to a host.

General message format

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |                Transaction-ID                 |
+---------------+---------------+---------------+---------------+
| Options ...
+---------------
\endcode

DAS Solicitation


Options

Client unique identifier

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   DasOptType  |     Length    |  Unique identifier           ????              |
+---------------+---------------+---------------+---------------+
| ????????????????????????????????????????????????????????????? |
+---------------+---------------+---------------+---------------+
\endcode

DAS option fields
    DasOptType   1

    Length  The length of the option.


*/

class DynamicAddressClient
{
public:
};

class DynamicAddressRelay
{
public:
};

class DynamicAddressServer
{
public:
};

} // namespace uNet

#endif // DYNAMICADDRESSSERVER_HPP
