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

.. toctree::
    doc_python
    referencing
    figures
    doc_cpp
    googlestyle
    add_mod_doc
    add_var_doc
    additional_features
    additional_boxes
    test_locally
    preview
