.. _referencing_things:

Referencing Components
----------------------

Much of the documentation done by Sphinx involves referencing other components of the documentation.
For example, when writing the command ``:py:func:`examplemodule.dummy_function_example```, you are
referencing a documented Python function of this name.
Sphinx then automatically creates a reference link to this function for you, displayed as
:py:func:`examplemodule.dummy_function_example`.
You can also create your own references to most other components of the documentation, such as sections, code-blocks,
and figures.
To create a custom reference you should put the reference directive just prior to the component you want to reference.
For example, in order to create a reference to this section this code was used

.. code-block:: rst

  .. _referencing_things:

  Referencing Components
  ----------------------

.. important:: Notice that the reference name ``_referencing_things`` has a leading underscore.
               This is *not part of the name*. When using the reference you omit this underscore.

We can then make a reference to this section by using ``:ref:`referencing_things``` which displays as :ref:`referencing_things`.
If you prefer to have a numbered reference, we could instead use ``:numref:`referencing_things``` which displays as
:numref:`referencing_things`.

We also have enabled a extension to automatically define references for all
sections. So without adding anything this section could also be referenced by
``:ref:`framework/doc/referencing:Referencing Components``` which still
result in :ref:`framework/doc/referencing:Referencing Components`.
