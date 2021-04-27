.. _doctools:

How to document your code with Sphinx
=====================================

Our sphinx setup is rather simple: First it will include the ``framework``
documentation and then it will go through all packages alphabetically and
include any files ending in ``index*.rst`` in a subdirectory ``doc`` in the package
directory, also alphabetically. It will ignore all other files in this
directory or any files in sub directories.  So for example if we have

.. code-block:: none

  analysis/doc/
      index.rst
      variables.rst
      an_image.png
      variable_groups/01-Kinematics.rst
      variable_groups/02-PID.rst
  calibration/doc/
      index_caf.rst
  framework/doc/
      index-01-install.rst
      index-02-tools.rst

It would include the files in the global table contents in the order

- ``framework/doc/index-01-install.rst``
- ``framework/doc/index-02-tools.rst``
- ``analysis/doc/index.rst``
- ``calibration/doc/index_caf.rst``

.. note:: ``.rst`` files not starting with ``index`` in the sub directory
  ``variable_groups`` are not included in the top level tree

The packages are free to structure their documents as they see fit considering
the following rules:

1. Everything in one ``.rst`` file ends up in one single html page

2. Every top level heading in included ``index*.rst`` will create one top-level
   heading in the global table of contents (aka one part in the pdf version)

3. Every top heading in these included files will create one top level heading
   in the global table of contents

4. Sphinx documentation is intended to be one big documentation tree, that
   means all existing ``.rst`` Files should be included in one `toctree
   directive`_, for example in the ``analysis/index.rst`` we could have
   something like

   .. code-block:: rst

     Analysis Package
     ================

     Some text describing the structure of the package

     .. toctree:
        :glob:

        variables
        variable_groups/*

   This would include first ``analysis/variables.rst`` and then all ``.rst``
   files in ``analysis/variable_groups/`` alphabetically. Top level headings in
   those files will be treated as sub-sections to the "Analysis Package". This
   will be rendered as a nested list of document titles (the first heading in
   those files)

6. Every file can limit the depth to which the table of contents is expanded by
   adding ``:tocdepth: N`` at the top of the file. The global table of contents
   is limited to two levels.

   .. note:: This will not have any effect on latex output. For the PDF output
      the depth of the table of contents is always set to 3 levels

.. _toctree directive: http://www.sphinx-doc.org/en/stable/markup/toctree.html

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
``:ref:`framework/doc/atend-doctools:Referencing Components``` which still
result in :ref:`framework/doc/atend-doctools:Referencing Components`.

Inserting Figures
-----------------

While properly documenting the code itself is the first step to take, you may want to include figures that explain
overall concepts of a package, module, or class (see :numref:`framework_modpath_diagram`).
To do this, first simply place the image file you would like to display into your ``<package>/doc>`` directory.
You can then place the image (in this case ``cat.jpg``) into the documentation by using

.. code-block:: rst

  .. _cat_picture:

  .. figure:: cat.jpg
    :width: 40em
    :align: center

    Why is it always cats?

where we have also included the reference ``cat_picture`` to use later.
This markup would display the image as

.. _cat_picture:

.. figure:: cat.jpg
  :width: 40em
  :align: center

  Why is it always cats?

Just as with other components, we can reference the picture from the text by using ``:ref:`cat_picture```.
Which then appears as a reference using the caption text as :ref:`cat_picture`.
As before, we could use the ``:numref:`` syntax to get a numbered reference displayed as :numref:`cat_picture`.

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




Add Basf2 Modules Documentation to Sphinx
-----------------------------------------

Basf2 Module documentation can be added to sphinx automatically using

.. rst:directive:: b2-modules

  Will automatically document basf2 modules. Without any arguments it will
  document all existing modules. It has the following optional parameters

  .. rst:role:: package

    Only document modules found from a specific package

  .. rst:role:: library

    Only document modules found in the specified library. This is the name of
    the directory inside the ``modules/`` directory but with a prefix "lib"
    and suffix ".so". So if you want to document all modules in
    ``analysis/modules/ParticleCombiner`` this could be

    .. code-block:: rst

      .. b2-modules::
         :library: libParticleCombiner.so


  .. rst:role:: modules

    Explicitly choose the modules to document with a comma separated list of modules:

    .. code-block:: rst

      .. b2-modules::
         :modules: EventInfoSetter, EventInfoGetter

  .. rst:role:: regex-filter

    Can be used to filter the modules to be documented by a python
    :py:mod:`regular expression <re>` For example to show all modules in
    the ``framework`` package which begin with 'Event'

    .. code-block:: rst

      .. b2-modules::
         :package: framework
         :regex-filter: ^Event

  .. rst:role:: no-parameters

    if present do not document module parameters

  .. rst:role:: io-plots

    if present it will try to include a graph showing the required, optional
    and registered DataStore elements.

  .. rst:role:: noindex

    if present the modules will not be added to the index. This is useful if
    the same module is documented multiple times to select which of these
    should show up in the index.


For this automatic documentation to work all documentation strings passed to
``setDescription()`` and ``addParam()`` should be valid reStructuredText_ (see
:ref:`multiline_cpp_strings`). It is also possible to reference modules
elsewhere in the text, for example ``:b2:mod:`EventInfoSetter```. In most case
it will work even when omitting the ``:b2:mod:`` but it is recommended to add
it to make sure it actually links to the correct thing and not a python
function with the same name.


Add Basf2 Variables Documentation to Sphinx
-------------------------------------------

We can also add documentation for basf2 variables with a very similar syntax to modules:

.. rst:directive:: b2-variables

  Allows to automatically document basf2 variables from the VariableManager. It
  has the following optional parameters

  .. rst:role:: group

     If present show only the variables in the named group

  .. rst:role:: variables

     Can be used to specify a comma separated list of variable names to show, for example

     .. code-block:: rst

        .. b2-variables::
           :variables: x,y,z

     Will only produce documentation for the variables ``x``, ``y``, and ``z``


  .. rst:role:: regex-filter

     Can be used to filter the selected variables by a python
     :py:mod:`regular expression <re>` For example to show all variables in
     the group "Kinematics" which begin with x

     .. code-block:: rst

        .. b2-variables::
           :group: Kinematics
           :regex-filter: ^x.*

  .. rst:role:: noindex

    if present the variables will not be added to the index. This is useful if
    the same variable is documented multiple times to select which of these
    should show up in the index.


For this automatic documentation to work all documentation strings passed to
``REGISTER_VARIABLE()`` should be valid reStructuredText_ (see
:ref:`multiline_cpp_strings`) It is also possible to reference variables
elsewhere in the text, for example ``:b2:var:`pidProbabilityExpert```. In most
case it will work even when omitting the ``:b2:var:`` but it is recommended to
add it to make sure it actually links to the correct thing and not a python
function with the same name.

.. important:: Please document any parameters your variable might have using
   :ref:`googlestyle`


Additional Features
-------------------

* All documented basf2 modules and variables are automatically added to a
  separate, alphabetic index page for easy lookup. They can be referenced with

  - ``:ref:`b2-modindex``` (:ref:`b2-modindex`)
  - ``:ref:`b2-varindex``` (:ref:`b2-varindex`)

* We have support for easy linking to JIRA issues by using
  ``:issue:`BII-XXXX```, for example ``:issue:`BII-8``` (:issue:`BII-8`)


Additional boxes for the online lessons
---------------------------------------

.. sidebar:: Overview
    :class: overview

    **Teaching**: 10 min

    **Exercises**: 5 min

    **Prerequisites**: None

    **Questions**:

        * What is a particle list?
        * What are final state particles?
        * How can I specify decays?

    **Objectives**:

        * Reconstruct particles

.. code:: rst

    .. sidebar:: Overview
        :class: overview

        **Teaching**: 10 min

        **Exercises**: 5 min

        **Prerequisites**: None

        **Questions**:

            * What is a particle list?
            * What are final state particles?
            * How can I specify decays?

        **Objectives**:

            * Reconstruct particles


No hands-on training without some nice exercises:

.. code:: rst

    .. admonition:: Question
       :class: exercise stacked

       What's the object-oriented way to get rich?

    .. admonition:: Hint
       :class: toggle xhint stacked

       Think about relationships between classes!

    .. admonition:: Solution
       :class: toggle solution

       Inheritance.

.. admonition:: Question
   :class: exercise stacked

   What's the object-oriented way to get rich?

.. admonition:: Hint
   :class: toggle xhint stacked

   Think about relationships between classes!

.. admonition:: Solution
   :class: toggle solution

   Inheritance.

**Notes**:

* You can also use ``.. admonition:: Exercise`` for an exercise rather than a question
  (in general the content after ``admonition::`` will always be the title).
* The ``stacked`` class removes the space after the question block, so that the solution
  block is directly joined. If you want to write some text after your question, simply
  remove this class.
* Note that the class for the hint box is ``xhint`` (short for exercise-hint),
  not ``hint`` (the latter is already in use for "normal" hint boxes)

.. code:: rst

    .. admonition:: Key points
        :class: key-points

        * There are 10 kinds of people in this world:
          Those who understand binary, those who don't,
          and those who weren't expecting a base 3 joke.

.. admonition:: Key points
    :class: key-points

    * There are 10 kinds of people in this world:
      Those who understand binary, those who don't,
      and those who weren't expecting a base 3 joke.


How to test locally
-------------------

You can test locally your changes in the Sphinx documentation by compiling your
code with the following command:

.. code:: bash

    scons --sphinx html

The output will be produced in ``$BELLE2_LOCAL_DIR/build/html`` and you can
navigate it with your favorite browser to check if the output is what you
expect.

Alternatively you can also build the sphinx documentation without building any
code with

.. code:: bash

    b2code-sphinx-build

which will be slightly faster. If you're only interested in sphinx warnings,
use

.. code:: bash

    b2code-sphinx-warnings

If you are working on a remote server with port ``8XXXX`` forwarded to your local
machine (as when running a Jupyter notebook), you can also start a tiny web server on
the remote machine and access it from your local machine to view the rendered
documentation:

.. code:: bash

    # log in to remote with port forwarding
    # (8XXX is a unique number of your choice)
    ssh login.cc.kek.jp -L 8XXX:localhost:8XXX
    # set up basf2
    cd ${BELLE2_LOCAL_DIR}/build/html
    python3 -m http.server 8XXX

Now navigate to ``http://0.0.0.0:8XXX`` on your local machine to see the rendered
web pages.

Previewing documentation changes in a pull request
--------------------------------------------------

1. Open your PR following the guidelines outlined at :ref:`here <pr_best_practices>`
2. Wait for it to build
3. click on the build status and open the detailed build results
4. open the "Artifacts" tab
5. click "Sphinx Documentation"

