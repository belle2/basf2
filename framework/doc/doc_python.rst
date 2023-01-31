Documentation of Python Code
----------------------------

Python code can be documented mostly automatically if the module containing the
functions/classes can be imported without error or modification of the path.
Please have a look at the autodoc_ documentation how to automatically add
python documentation to the sphinx document tree.

.. _autodoc: http://www.sphinx-doc.org/en/stable/ext/autodoc.html

For this automatic documentation to work the docstrings must be valid reStructuredText_:

.. literalinclude:: docstring-example.rst-fragment
   :lines: 4-
   :dedent: 2

Using

.. code-block:: rst

  .. autofunction:: examplemodule.dummy_function_example

this would render something like

.. py:module:: examplemodule

.. py:function:: dummy_function_example(name, foo=None)

   .. include:: docstring-example.rst-fragment

.. _reStructuredText: http://www.sphinx-doc.org/en/stable/rest.html