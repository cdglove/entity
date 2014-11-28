entity
======

Entity/Component System for C++

There are several entity/component systems out there for C++.  They all suffer from the same issue:  The component list is carries with the entity which creates a performance issue in code trying to get components.

'entity' takes a different approache.  Here, an entity is basically an int and a component is a corresponding entry in an array somewhere.  This simple approach leads to a near zero performance penalty for the abstraction with the only caveat being that the code must have access to the pools as well as the entity in order to do any work.  This can actually lead to better codee because not all code will be able to access all components because the pools themselves may be private.
