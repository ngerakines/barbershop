# Barbershop

Barbershop is a fast, lightweight priority queue system for a specific
use case that is undisclosed. The goal is to create a TCP/IP based
service that uses libev(ent) to create a shifting priority queue.


Internally a list of items is tracked, each having a priority. Some
connections to the daemon set/update item priorities while other
connections want to pop something off of the list based on priority.

As items are added, the hash of items and priorities is updated.

# TODO

 * Add next functionality.
 * Add stats structure and expose it.
 * Make fast and lightweight.
 * Clean up the binary search tree interface and make it items specific.
 * Move the scores container to it's own source file.
 * Create a legit benchmark tool.
 * Add input args to the client tool.
