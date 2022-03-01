How to write basf2 code (coding conventions)
============================================

The following rules and conventions should ensure a good usability and maintainability of
the Belle II software. They are written for C++, but have to be followed for Python code
as well, unless other rules for Python are given.

General rules
-------------

Try to keep your code as simple as possible:

1. The interfaces should be easy to use and consistent throughout the entire software.
   Use inheritance and templates with care: try aggregation instead of inheritance, avoid
   multiple inheritance or friends, don't use private inheritance.

2. Aim for high coherence (only things belonging to a class should be there, and the class
   should be complete) and low coupling.

3. The code should compile without warning messages and must be portable (e.g. it should
   run on both 32 and 64 bit machines).

Please, please avoid:

1. Hacks and fancy features.

2. Interfaces to interfaces to interfaces.

3. Too much dependence on external software; check whether a plug-in mechanism can be used
   if external software is required.

4. Flexibility if it significantly increases the possibility of making mistakes: the
   system should behave correctly by default; a user should not be required to change any
   parameters from their default value to get a correct result.

5. Side effects: if code has side effects this must be clearly documented.

6. Preprocessor definitions if possible; in particular don't use them for constants.

Naming conventions
------------------

Names should be precise, colorful and in self-explanatory English:

1. Avoid abbreviations where possible, e.g. use ``TrackFitter`` instead of ``TrFt``.

2. Use plurals for containers (lists, vectors, etc.), e.g. ``particles``, ``tracks``.

3. When combining several words, use the order corresponding to natural English language,
   e.g. ``CDCHit`` instead of ``HitCDC``.

4. Use first-letter capitalization instead of underscore for constructing names, e.g.
   ``getTotalEnergy()`` instead of ``get_total_energy()``.

5. Use the ``get`` prefix for methods that return the value of the data member (getters).
   Use the ``set`` prefix for methods that set the value of a data member by parsing an
   argument (setters).

5. *Python*: except for class names, use lower case words with underscores,
   e.g. ``find_particle()``.

6. Specifically:

   * class and type names start with a capital letter (``CapWords``, same for Python)

   * local variable and method names start with a lower case letter

   * enum types start with ``E`` followed by a capital letter

   * template arguments begin with ``A`` followed by a capital letter

   * use the prefix ``m_`` for member variables, ``s_`` for static member variables, and
     ``c_`` for constants

   * *Python*: use all upper case with underscores for constants; always use
     ``self``/``cls`` as first argument of instance/class methods

   * module parameter names are equal to the corresponding variable name, but without
     ``m_`` prefix

7. If values are not given in standard units the used units have to be stated explicitly
   in the variable name. Standard units are cm, nanosecond, radian, GeV, Kelvin, Tesla,
   elementary charge as defined in `Unit.h`_.

.. _Unit.h: https://github.com/belle2/basf2/blob/main/framework/gearbox/include/Unit.h

8. Executable that are part of the official interface on which users and other tools can
   rely on must have the prefix ``b2`` and must be documented in Sphinx. Other executables
   must have a prefix equal to the name of the package to which they belong.

9. Avoid the usage of "slave/master", "blacklist/whitelist". Prefer instead:
   "controller/agent", "primary/secondary", "denylist/allowlist".

10. In general, avoid anything that might cause distress or feelings of exclusion to other
    collaborators.

In case of doubts, don't hesistate to contact the coordinators.

Namespaces
----------

1. All the definitions should be done in ``namespace Belle2``.

2. Use sub-namespaces with names equal to the package names for everything (e.g.:
   ``namespace Belle2::Something``), except for datastore objects and anything in a
   ``dataobjects``, ``dbobjects`` or ``modules`` subdirectory.

3. ``using namespace`` is forbidden in header files; include statements must be outside
   namespaces.

4. ``enum class`` should be used to implement enumerations to contain the constant within
   the enum’s scope

Classes
-------

Each class implementation should go into a single source code file.

The ordering in the class definition is ``public``, ``protected``, ``private``, and in
each such section elements shall be ordered like:

* typedefs and enums,

* constructors and destructor,

* operators,

* other methods,

* data members, while in case of modules the steerable data members before non-steerable
  ones,

with static methods and members after non-static ones. Public data members are forbidden.

The only exception to the above rule are simple structs with no methods except for a
default constructor. In this case public data members (without ``m_`` prefix) are allowed.
Use the struct instead of the ``class`` keyword for such structures.

*Python*: dollow basically the same rules, but put data members first.

Only very short functions like simple setters and getters should be in the declaration.
Implementation of non-template methods should go to the source file.

Methods should be declared ``const`` if possible. When overriding a virtual method of a
base class, the ``override`` keyword should be used to mark this intention. The ``final``
keyword may be used for methods that should not be overridden or classes from which no
other classes should be derived.
