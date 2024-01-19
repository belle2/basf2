Why contribute? Read our propaganda at :doc:`/online_book/contribute`.

.. _onlinebook_how_to_contribute:

How to contribute
=================

* Something is **wrong**/**misleading**: Fix it with a merge request or raise the issue by creating a GitLab issue.
  If you are unsure about it, you can ask on `questions.belle2.org <https://questions.belle2.org>`_ before.
* You notice that something is **missing**:

  * If you're not sure whether it belongs here, ask the librarians, e.g. by creating a GitLab issue
    or per email. You can either assign the GitLab issue to someone you like or submit a merge request for it yourself (preferred).
  * If it is already marked as a todo item, check if someone else is working on it (search on GitLab) and if not,
    submit a merge request

Understanding Sphinx and RST
----------------------------

This webpage is built with Sphinx_ using source files in reStructuredText_ (``*.rst``) format.

.. _Sphinx: https://www.sphinx-doc.org/en/master/
.. _reStructuredText: https://docutils.sourceforge.io/rst.html

See :doc:`/framework/doc/atend-doctools` to learn more about these tools.

.. hint::
   If you want to learn sphinx by looking at the sources of various pages,
   click on the ``View page source`` button on the
   top right of the web page.

.. hint::
   The source file of each file is apparent from the URL. E.g. this page
   has the URL ``.../online_book/how_to_contribute.html``, so your source
   file is in ``${BELLE2_LOCAL_DIR}/online_book/how_to_contribute.rst``.

Submitting a MR
---------------

You need to

1. Create a development setup as explained in
   :ref:`build/tools_doc/cvmfs_setup:Development Setup`
2. Change to a new branch ``feature/describe-your-change`` or ``bugfix/something``
   (if your merge request is based on a GitLab issue, you can name it in the MR description).
   You will not be able to push your branch if it isn't prefixed with ``feature/`` or ``bugfix/``
3. Make your changes
4. Optionally: Preview them locally as described in :doc:`/framework/doc/atend-doctools`
5. Push your branch
6. Create a MR following the guidelines outlined at :ref:`pr_best_practices`

.. seealso::

    You already opened a MR for a test repository in :ref:`onlinebook_git`, remember?
    Basically this is exactly the same procedure.

