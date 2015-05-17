User Manual       {#mainpage}
===========

\tableofcontents

\section tutorial-preface Preface

--------------------------------------------
ECS for C++
For more information on what an ECS is, visit
[ECS](http://en.wikipedia.org/wiki/Entity_component_system) on Wikipedia.

**Development Status:**

This code is alpha quality at best.  Though it does contain a reasonable test
suit, it's not been proven to be of use to any real systems as of yet.  The
being said, I believe the concept is solid and the library just needs
refinement.

\subsection tutorial-installation Installation

--------------------------------------------

This library is currently header-only but depends on boost 1.55 or higher. The
easiest installation is, if you use cmake, to auto configure it using my handy
[cmake tools](https://github.com/cdglove/cmaketools).  An example might look
like this;

> Download cmaketools to project folder
~~~~~~~{.sh}
mkdir my_program
cd my_program
git clone https://github.com/cdglove/cmaketools contrib/cmaketools
~~~~~~~

> Add the following to a CMakeLists.txt
~~~~~~~{.cmake}
include(contrib/cmaketools/external_git.cmake)
include(contrib/cmaketools/enable_cpp11.cmake)

set(ENTITY_BUILD_TESTS OFF CACHE BOOL "Disable libentity tests" FORCE)
add_external_git_repo(
	"https://github.com/cdglove/entity.git"
	"master" # master breach at HEAD.  Or a specific commit.
	"contrib/entity")
find_package(entity)

add_executable(my_executable main.cpp)
target_include_directories( my_executable PUBLIC ${ENTITY_INCLUDE_DIRS})
target_link_libraries( my_executable ${ENTITY_LIBRARIES})
~~~~~~~

That's all that's required.  entity will autoconfigure itself and your project
with the appropriate dependencies.  To get started,

~~~~~~~{.cpp}
#include <entity/all.hpp>
~~~~~~~

or each individual file as necessary;

~~~~~~~{.cpp}
#include <entity/component/dense_pool.hpp>
#include <entity/**/*.hpp>
~~~~~~~

Alternatively, you can get the source from the [entity
repository](https://github.com/cdglove/entity) on github and then point your
compiler at it via -I, /I, et al.

\subsection tutorial-license License

-------------------------------------------- 

Most of the source code in this project are mine, and those are under the
Boost Software License. Please see the attached LICENSE file for the
licensing.

\subsection tutorial-compilers Supported Compilers

--------------------------------------------
The code is known to work on the following compilers:

- clang 3.4.0
- GCC 4.9.0
- MSVC 2015

\section tutorial-what-is-entity What is entity?

--------------------------------------------

entity implements an ECS system which is usually described as a method to
compose object-like things from component-like things in a dynamic, yet
typesafe fashion.

There are several other C++ ECS systems, but many of them suffer from some
performance issues due to the cost of the entity carrying a list of type
erased components.  Fetching a component from the entity means walking a list
fo see if such a type exists in the list. Because such system usually work
based on types, this also means than an entity can only have one instance of
each component type attached to it

To fix these two problems, entity takes a slightly different approach: nstead
treating the entity as an object, it's actually just an id from which to
retrieve and modify components from various pools.  entity, therefore, simply
coordinates the lifetime management of the entities and it's components while
also providing algorithsm to assist in working with the system.

## Why Use an ECS?

### Flexibility

Inheritance is probably the most overused abstraction in C++ and indeed most
modern languages.  Despite the fact that most problems don't fit into such a
nice hierarchy, most programmers reach for inheritance too soon when first
building a new system resulting in overly rigid architcturses that are
difficult to change.

An ECS on the otherhand is flexible and dynamic.

### Performance

The virtual function mehanism implies heap allocated objects (or at least,
pointers to objects). This in turn implies jumping around in memory a lot,
which is hard on the CPU cache.

But, we like composability, so an ECS aims to fix offer that without the cache
problem.

## OOP vs. ECS

~~~~~~~{.cpp}
    // Put sample OOP code here.
~~~~~~~

with

~~~~~~~{.cpp}
    // Put sample ECS code here.
~~~~~~~

