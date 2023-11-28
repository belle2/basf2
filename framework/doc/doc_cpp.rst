.. _multiline_cpp_strings:

Documentation of Variables and Modules in C++
---------------------------------------------

The description for all variables, modules an module parameters must be valid
reStructuredText_. Parameters for variables should be documented using
:ref:`googlestyle`.

It is advisable to use raw string literals in C++ to be able to easily write multi
line strings. A raw string literal starts with ``R"delimiter(`` and
ends with ``)delimiter"``. It can span multiple lines similar to the python
multi line strings (``"""`` or ``'''``). The ``delimiter`` can be chosen
freely and can also be empty. usually for documentation strings we recommend
something like

.. code-block:: c++

  R"DOC(This will be the actual string
  and it can span multiple lines)DOC"

As an example, the following code to register a variable

.. literalinclude:: variable-docstring-example.rst-fragment
   :language: c++
   :lines: 4-
   :dedent: 2

would show up in the documentation as

.. b2:variable:: example_variable_documentation (param1, param2, ...)

  .. include:: variable-docstring-example.rst-fragment

.. _reStructuredText: http://www.sphinx-doc.org/en/stable/rest.html
