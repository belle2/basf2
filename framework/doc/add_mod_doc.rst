Add basf2 Modules Documentation to Sphinx
-----------------------------------------

basf2 Module documentation can be added to sphinx automatically using

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
    the ``framework`` package which begin with 'Event'.

    .. code-block:: rst

      .. b2-modules::
         :package: framework
         :regex-filter: ^Event

    .. note::  Can be used also for variables (see next section). For example
       to show all variables in the group "Kinematics" which begin with x

    .. code-block:: rst

      .. b2-variables::
         :group: Kinematics
         :regex-filter: ^x.*

  .. rst:role:: no-parameters

    if present do not document module parameters

  .. rst:role:: io-plots

    if present it will try to include a graph showing the required, optional
    and registered DataStore elements.

  .. rst:role:: noindex

    if present the modules will not be added to the index. This is useful if
    the same module is documented multiple times to select which of these
    should show up in the index.

    .. note:: This can be used also with variables (see next
       section) in that case if present the variables will not be added to the
       index. This is useful if the same variable is documented multiple times to
       select which of these should show up in the index.

For this automatic documentation to work all documentation strings passed to
``setDescription()`` and ``addParam()`` should be valid reStructuredText_ (see
:ref:`multiline_cpp_strings`). It is also possible to reference modules
elsewhere in the text, for example ``:b2:mod:`EventInfoSetter```. In most case
it will work even when omitting the ``:b2:mod:`` but it is recommended to add
it to make sure it actually links to the correct thing and not a python
function with the same name.

.. _reStructuredText: http://www.sphinx-doc.org/en/stable/rest.html