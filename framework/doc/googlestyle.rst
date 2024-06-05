.. _googlestyle:

Google Style Docstrings
-----------------------

The original reStructuredText style to define parameters with ``:param name:``
becomes a bit unreadable in plain text form. To make sure that the
documentation is human readable also in plain text we recommend using `Google
style docstrings`_. Simply put, instead of writing

.. code-block:: rst

   :param type1 name1: description 1
   :param name2: description 2
   :returns: return value description

you should write

.. code-block:: none

   Parameters:
     name1 (type1): description 1
     name2: description 2

   Returns:
     return value description


The known sections you can and should use if appropriate are

* ``Attributes``
* ``Example``
* ``Examples``
* ``Keyword Arguments``
* ``Methods``
* ``Note``
* ``Other Parameters``
* ``Parameters``
* ``Returns``
* ``Raises``
* ``References``
* ``See Also``
* ``Todo``
* ``Warning``
* ``Warns``
* ``Yields``


You can see a more complete example of this format at
http://www.sphinx-doc.org/en/stable/ext/example_google.html

.. _Google style docstrings: http://www.sphinx-doc.org/en/stable/ext/napoleon.html
