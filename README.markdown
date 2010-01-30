# Barbershop

Barbershop is a fast, lightweight priority queue system for a specific
use case that is undisclosed. The goal is to create a TCP/IP based
service that uses libev(ent) to create a shifting priority queue.

Internally a list of items is tracked, each having a priority. Some
connections to the daemon set/update item priorities while other
connections want to pop something off of the list based on priority.

hash items: item id -> {priority value, last modified}
hash priorities: priority -> list of items
binary tree locks: binary search tree of item ids currently locked
int max: current max priority

As items are added, the hash of items and priorities is updated.

# TODO

 * Find a better way to array/hashmap items->scores
 * Look into moving to libjudy.
 * Add reverse index of scores to items scored such.
 * Add stats structure and expose it.

