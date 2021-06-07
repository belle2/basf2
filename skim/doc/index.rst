.. _skim:

=====
Skims
=====

The skim package is a collection of high-level analysis scripts that reduce the data set to a manageable size by applying a simple selection.
The input to a skim are :ref:`mdst` files of processed data.
The output are so-called :ref:`analysis_udstoutput` (udst) files.
These files actually contain more information but fewer events.

.. tip:: Analysts are recommended to use skimmed udst files as input to their analysis.

To get information about existing skim, ask the current skim group liaison.

.. tip:: For an introductory lesson, take a look at :numref:`onlinebook_skim`.

.. versionchanged:: release-06-00-00

    The skim package was reorganised between releases 5 and 6. If you need your skim steering files to work with both release 5 and release 6, then please use the following type of try-except block:

    .. code-block:: python

        try:
            # release 6+ imports
            from skim import BaseSkim, CombinedSkim
            from skim.WGs.ewp import BtoXll
        except (ImportError, ModuleNotFoundError):
            # release 5 imports
            from skimExpertFunctions import BaseSkim, CombinedSkim
            from skim.ewp import BtoXll

Systematics skims
-----------------

.. toctree:: 01-systematics


Physics skims
-------------

.. toctree:: 02-physics
    

Standard skim lists
-------------------

.. toctree:: 03-lists
    

Information for skim experts
----------------------------

.. toctree:: 04-experts
    

