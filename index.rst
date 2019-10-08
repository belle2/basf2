Belle 2 Software Documentation
==============================

This document contains documentation of the Belle 2 Software, its command line
tools and the Python API.

.. note:: Generated on |today| for release |release|, commit |version|.

This Documentation is not yet complete. In case of questions regarding the
Belle 2 Software or for additional information, please

* look at the `Confluence Pages <https://confluence.desy.de/display/BI>`_
* check `Belle 2 Questions <https://questions.belle2.org>`_

----

.. include all files in the table of content, sorted by package
   first the documentation of the framework package
   then any *.rst files found in package/doc
   then any sphinx_*.rst files in package/
.. toctree::
   :glob:
   :maxdepth: 2
   :numbered: 3

   whatsnew
   build/tools_doc/index*
   framework/doc/index*
   */doc/index*
   framework/doc/atend*

.. only:: format_html

  Indices and tables
  ==================

  * :ref:`genindex`
  * :ref:`modindex`
  * :ref:`b2-modindex`
  * :ref:`b2-varindex`
  * :ref:`search`
