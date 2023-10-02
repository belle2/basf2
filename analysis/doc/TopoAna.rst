-----------------
Topology analysis
-----------------

This section provides some information on the interface, repositories, and
documents of ``TopoAna``,
which is a generic tool for the event type analysis of inclusive MC samples in
high energy physics experiments,
and hence a powerful tool for analysts to investigate the signals and backgrounds
involved in their works.
``TopoAna`` is an offline tool independent of ``basf2``.
It can take the output root files of the :ref:`Analysis <analysis>` module as
input.
The MC truth information for the event type analysis can be stored in the root
files with the utility ``MCGenTopo`` in ``basf2``.
Thus, ``MCGenTopo`` is the interface of ``basf2`` to ``TopoAna``.

.. note::

   Apart from the interface, this section only introduces the ``TopoAna``
   resources outside the Belle II software documentation.
   Inside the documentation, please see :numref:`onlinebook_topoana` for the
   online textbook on ``TopoAna``.
   It is a good idea to start learning the usage of ``TopoAna`` with this online
   textbook.
   Please feel free to contact Xingyu Zhou (zhouxy@buaa.edu.cn) if you have any
   questions or comments on ``TopoAna``.

~~~~~~~~~~~~~
The interface
~~~~~~~~~~~~~

As we mention above, ``MCGenTopo`` is the interface of ``basf2`` to ``TopoAna``.
To be specific, the interface implements the following parameter function
``mc_gen_topo(n)`` in the script ``variables/MCGenTopo.py``.

.. automodule:: variables.MCGenTopo
   :members:
   :noindex:

Below are the steps to use ``mc_gen_topo(n)`` to get the input data to ``TopoAna``.

  1. Append the following statement at the beginning part of your python steering
     script

     .. code-block:: python

        from variables.MCGenTopo import mc_gen_topo

  2. Use the parameter function ``mc_gen_topo(n)`` as a list of variables in the
     steering function ``variablesToNtuple`` as follow

     .. code-block:: python

        variablesToNtuple(particleList, yourOwnVariableList + mc_gen_topo(n), treeName, fieName, path)

  3. Run your python steering script with ``basf2``

~~~~~~~~~~~~
Repositories
~~~~~~~~~~~~

The following three remote repositories of ``TopoAna`` are provided at present.
The one at DESY's GitLab is most convenient to Belle II users.
Nonetheless, the two at GitHub and at GitLab of IHEP are also provided as helpful
alternatives for possible convenience.

* `Repository at DESY GitLab <https://gitlab.desy.de/belle2/software/topoana>`_
* `Repository at GitHub <https://github.com/buaazhouxingyu/topoana>`_
* `Repository at GitLab of IHEP <http://code.ihep.ac.cn/xyzhou/topoana>`_

~~~~~~~~~
Documents
~~~~~~~~~

.. seealso::

   The introduction to the documents can also be found in the file ``README.md``
   in the ``TopoAna`` package, which should be the first document to be read on
   ``TopoAna``.
   For your convenience, a pdf and a html version of the README file are provided
   in the ``TopoAna`` package as ``share/README.pdf`` and ``share/README.html``,
   respectively.

The following three documents of ``TopoAna`` are provided in its package.

  * A **brief** description of the tool is in the document: ``share/quick-start_
    tutorial_v*_Belle_II.pdf``

    * All the examples in the quick-start tutorial can be found in the
      sub-directory ``examples/in_the_quick-start_tutorial``

  * A **detailed** description of the tool is in the document: ``share/user_guide
    _v*.pdf``

    * All the examples in the user guide can be found in the sub-directory
      ``examples/in_the_user_guide``

  * An **essential** description of the tool is in the document:
    ``share/paper_draft_v*.pdf``

    * All the examples in the paper draft can be found in the sub-directory
      ``examples/in_the_paper``

   .. note::

      The paper on the tool has been published by ``Computer Physics
      Communications``.
      You can find this paper and the preprint corresponding to it in the links
      `Comput. Phys. Commun. 258 (2021) 107540 <https://doi.org/10.1016/j.cpc.
      2020.107540>`_ and `arXiv:2001.04016 <https://arxiv.org/abs/2001.04016>`_,
      respectively.
      **If the tool really helps your researches, we would appreciate it very
      much if you could cite the paper in your publications.**

As for the three documents, the quick-start tutorial is the briefest, the user
guide is the most detailed, and the paper draft is composed of the essential
and representative parts of the user guide.

.. tip::

   It is a good practice to learn how to use the tool via the examples 
   in the quick-start tutorial, user guide, and paper draft, 
   in addition to the online textbook in :numref:`onlinebook_topoana`.

~~~~~~~~~~~~~~~~~~~~~
Use cases at Belle II
~~~~~~~~~~~~~~~~~~~~~

At the end of this section, we list two use cases of ``TopoAna`` at Belle II:
one for semitauonic analyses and the other for charm analyses.
You can refer to them if you work in the related analysis groups.

* `Using TopoAna with the semitauonic framework <https://confluence.desy.de/display/BI/Using+topoAna+with+the+semitauonic+framework>`_ by Hannah Marie Wakeling
* `TopoAna Wrapper for Charm Analysis <https://gitlab.desy.de/belle2/physics/charm/software/charm_topoanawrapper>`_ by Guanda Gong
