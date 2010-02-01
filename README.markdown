# Barbershop

Barbershop is a fast, lightweight priority queue system for a specific
use case that is undisclosed. The goal is to create a TCP/IP based
service that uses libev(ent) to create a shifting priority queue.

Internally a list of items is tracked, each having a priority. Some
connections to the daemon set/update item priorities while other
connections want to pop something off of the list based on priority.

As items are added, the hash of items and priorities is updated.

# Protocol

Clients of barbershop communicate with server through TCP connections.
A given running barbershop server listens on some port; clients connect
to that port, send commands to the server, read responses and eventually
close the connection.

There is no need to send any command to end the session. A client may
just close the connection at any moment it no longer needs it. Note,
however, that clients are encouraged to cache their connections rather
than reopen them every time they need to store or retrieve data. Caching
connections will eliminate the overhead associated with establishing a
TCP connection (the overhead of preparing for a new connection on the
server side is insignificant compared to this).

Commands lines are always terminated by \r\n. Unstructured data is also
terminated by \r\n, even though \r, \n or any other 8-bit characters
may also appear inside the data. Therefore, when a client retrieves
data from a server, it must use the length of the data block (which it
will be provided with) to determine where the data block ends, and not
the fact that \r\n follows the end of the data block, even though it
does.

## Items and Priority Values

Items and their priority scores are unsigned 32bit integers.

## Commands

There are three types of commands. The update command is used to create
or update an item's priority. If the item given is being introduced then
it's base priority is set, else the priority is incremented by the given
amount.

The next command is used to return the next ordered by priority.

Last, the stats command is used to get babershop daemon stats and usage
information.

# Error responses

Each command sent by a client may be answered with an error string
from the server. These error strings come in three types:

"ERROR\r\n"

The client sent a nonexistent command name.

"CLIENT_ERROR <error>\r\n"

Some sort of client error in the input line, i.e. the input doesn't
conform to the protocol in some way. <error> is a human-readable error
string.

"SERVER\_ERROR <error>\r\n"

Some sort of server error prevents the server from carrying out the
command. <error> is a human-readable error string. In cases of severe
server errors, which make it impossible to continue serving the client
(this shouldn't normally happen), the server will close the connection
after sending the error line. This is the only case in which the server
closes a connection to a client.

In the descriptions of individual commands below, these error lines
are not again specifically mentioned, but clients must allow for their
possibility.

## Storage commands

First, the client sends a command line which looks like this:

    <command name> <item id> <priority>\r\n

* <command name> is "update".
* <item id> is the unique 32bit integer representing the item.
* <priority> is a 32bit integer that the priority will either be set to or increment by.

After sending the command line the client awaits the reply which may be:

* "OK\r\n", to indicate success.
* One of the possible client or server error messages listed above.


## Retrieval command

The retrieval commands "next" operates like this:

	next\r\n

After sending the command line the client awaits the reply which may be:

* "-1\r\n", indicates that there are no items to act on.
* "<32u>\r\n", the item id for the client to process.
* One of the possible client or server error messages listed above.

## Statistics

The command "stats" is used to query the server about statistics it
maintains and other internal data.

    stats\r\n

Upon receiving the "stats" command the server sents a number of lines
which look like this:

    STAT <name> <value>\r\n

The server terminates this list with the line:

    END\r\n

In each line of statistics, <name> is the name of this statistic, and
<value> is the data.  The following is the list of all names sent in
response to the "stats" command, together with the type of the value
sent for this name, and the meaning of the value.

In the type column below, "32u" means a 32-bit unsigned integer, "64u"
means a 64-bit unsigner integer. '32u:32u' means two 32-but unsigned
integers separated by a colon.

* 'uptime' (32u) Number of seconds this server has been running.
* 'version' (string) Version string of this server.
* 'updates' (32u) Number of update commands received by this server.
* 'items' (32u) Number of non-garbage collected items.
* 'items\_gc' (32u) Number of items that would remain after garbage collection.
* 'pools' (32u) Number of non-garbage collected pools.
* 'pools_gc' (32u) Number of pools that would remain after garbage collection.

# TODO

 * Start running it through memory leak detection tools.
 * Add more info to stats interface.
 * Make fast and lightweight.
 * Clean up the binary search tree interface and make it items specific.
 * Add ability to put items into a limbo pool until worker response with a comfirmation message that item has been processed.
 * Create a legit benchmark tool.
 * Add input args to the client tool.
