:tocdepth: 4

.. _command-line-tools:

Command Line Tools
==================

The Belle2 Software provides a number of command line tools for general use,
including tools to work with files, to help with debugging/testing or for
validation purposes. The general rule is that all tools which are intended to
be usefule for most users should start with the name ``b2`` followed by a usage
category, for example ``b2file`` for all tools for file handling. Sub
categories should be added using ``-`` signs, for example
``b2file-metadata-show`` is a tool intended to show the metadata embeded into
files created by the Belle2 Software.


.. toctree::
    :glob:

    tools/*
