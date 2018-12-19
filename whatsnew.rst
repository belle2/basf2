:tocdepth: 1

==========
What's New
==========

This page explains the new features in the current release. This cannot cover
all the changes in all packages but should help users to find out what needs to
be adapted when changing to the new release.

.. contents:: Table of Contents
   :depth: 3
   :local:

.. after release-03 we can just add a new heading here and start collecting.
   That way it will be updated correctly in light releases and we don't have to
   change anything for release-04

Changes since release-02-01
===========================

.. important changes should go here. Especially things that break backwards
   compatibility 

.. rubric:: Moved to C++17

The whole software including the ROOT in the externals is now compiled using
the C++17 standard. This should not affect many users but there are a few rare
cases where this might lead to compilation problems of analysis code as some
deprecated features `have been removed <https://mariusbancila.ro/blog/2018/07/05/c17-removed-and-deprecated-features/>`_.
The most notable are 

  * ``throw(TypeName)`` exception specifiers, just remove them.
  * ``std::auto_ptr`` which should be replaced by ``std::unique_ptr``
  * some older parts of the ``<functional>`` header.

In particular if you compile a standalone program that links against the ROOT
in the Belle2 externals this now also needs to be compiled in C++17 mode. You
can do this by adding ``-std=c++17`` to the compiler arguments.

.. note:: It's best to directly pass the output of ``root-config --cflags`` to
    the compiler. That way you always pass the correct flags needed for the
    particular ROOT version setup.

.. rubric:: Build system moved to Python3

This is a major update of `SCons <https://scons.org>`_ but most users should not
notice any difference except for two instances:

*  If you update an existing working directory from an older release you might
   run into an error

   .. code-block:: text

       scons: *** [...] TypeError : intern() argument 1 must be string, not unicode
       scons: building terminated because of errors.
       TypeError: intern() argument 1 must be string, not unicode: 
       [...]
    
   In this case please remove all ``.scon*`` files in the top level of your
   software directory and rerun ``scons``

*  In the unlikely case that you have custom ``SConscript`` files
   which are not Python 3 compatible you will have to :ref:`update them <python:pyporting-howto>`.

.. Now let's add the detailed changes for the analysis package first, that's
   what user will want to see

.. include:: analysis/doc/whatsnew-since/release-02-01.txt

.. And then for framework as well. If something is more important we can always
   move it directly in here

.. include:: framework/doc/whatsnew-since/release-02-01.txt
