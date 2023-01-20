Add basf2 Variables Documentation to Sphinx
-------------------------------------------

We can also add documentation for basf2 variables with a very similar syntax to modules:

.. rst:directive:: b2-variables

  Allows to automatically document basf2 variables from the VariableManager. It
  has the following optional parameters. 

  .. note:: ``:regex-filter:`` and ``:noindex:`` can be used also for ``b2-variables`` as described in previous section.


  .. rst:role:: group

     If present show only the variables in the named group

  .. rst:role:: variables

     Can be used to specify a comma separated list of variable names to show, for example

     .. code-block:: rst

        .. b2-variables::
           :variables: x,y,z

     Will only produce documentation for the variables ``x``, ``y``, and ``z``


For this automatic documentation to work all documentation strings passed to
``REGISTER_VARIABLE()`` should be valid reStructuredText_ (see
:ref:`multiline_cpp_strings`) It is also possible to reference variables
elsewhere in the text, for example ``:b2:var:`pidProbabilityExpert```. In most
case it will work even when omitting the ``:b2:var:`` but it is recommended to
add it to make sure it actually links to the correct thing and not a python
function with the same name.

.. important:: Please document any parameters your variable might have using
   :ref:`googlestyle`

.. _reStructuredText: http://www.sphinx-doc.org/en/stable/rest.html