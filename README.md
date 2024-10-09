# Virtual memory allocator

### Description

  The project consists of a vector of doubly linked lists that simulates a heap with free memory zones, and another doubly linked list that manages the allocated memory zones.

There are 7 main functions that help in creating the heap, managing memory zones using malloc() and free(), storing information and displaying it in the allocated zones via read() and write(), displaying memory information via dump_memory(), and releasing the heap with destroy_heap().

To create these functions, several generic subprograms were necessary, which work with lists and manipulate blocks depending on the type of list (SFL = segregated free list and AML = allocated-memory list). My main focus was on adding blocks, removing them, and handling the addresses associated with each.

The memory allocator is virtual, as it only simulates the use of addresses from the heap. In reality, the free(), malloc(), read(), and write() functions themselves use the official functions that have the same functionalities.

The main goal of this project was to deepen knowledge about data structures and how heap memory can be organized.

