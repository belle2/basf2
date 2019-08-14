
VariableManager
===============

The VariableManager handles all variables in ``basf2`` analysis.
It is implemented as a `singleton <https://en.wikipedia.org/wiki/Singleton_pattern>`_
C++ class with a python interface.

The C++ documentation is `here <https://b2-master.belle2.org/software/development/classBelle2_1_1Variable_1_1Manager.html>`_.

.. tip::

        For (unfortunate) historical reasons, the python accessor to the VariableManager
        singleton is called ``variables`` and is in the python ``variables`` module.
        This leads to strange-looking python ``import`` commands.

        For example:

        .. code-block:: python

            from variables import variables
                
        To avoid confusion, example/tutorial scripts often use a namespace alias ``vm``.
        You might want to use this in your scripts.

        .. code-block:: python

            from variables import variables as vm # shorthand for VariableManager


.. py:class:: VariableManager

   Singleton class to hold all variables and aliases in the current scope.

   .. py:method:: addAlias(alias, expression)

      Create a new alias.

      Variable names are deliberately verbose and explicit (to avoid ambiguity).
      However, it is often not desirable to deal with long unwieldy variable names particularly in the context of `VariableManagerOutput`.

      Example:

          Aliases to a verbose variable may be set with:

          >>> from variables import variables as vm
          >>> vm.addAlias("shortname", "aReallyLongAndSpecificVariableName(1, 2, 3)")

      .. seealso:: 

          `variables.utils.create_aliases` and `variables.utils.create_aliases_for_selected` 
          might be helpful if you find yourself setting many aliases in your analysis script.

      .. warning:: 

          The VariableManager instance is configured independently of the `basf2.Path`. 
          In case of adding the same alias twice, the configuration just before calling `basf2.process` is what wins.

      :param str alias: New alias to create
      :param str expression: The expression the alias should evaluate to

      :return: True if the alias was successfully added

   .. py:method:: getAliasNames()

      Get a list of all alias names (in reverse order added)

      .. tip:: 

          This returns a ``ROOT.vector`` which you will probably 
          need to convert into a python ``list(str)``.

          >>> my_aliases = list(vm.getAliasNames())

      :returns: ``ROOT.vector`` list of alias names

   .. py:method:: addCollection(collection, variables)

      Create a new variable collection.

      .. tip:: 

         This method takes a ``ROOT.vector<string>`` as input.
         It's probably easier to use `variables.utils.add_collection` which wraps this function for you.

      :param str collection: The new collection to create.
      :param ROOT.vector(str) variables: A ``ROOT.vector<string>`` of variables to add as the variable collection.

      :returns: True if the collection was successfully added

   .. py:method:: getCollection(collection)
  
      Get a list of all variables in the ``collection``.

      :param str collection: The name of the existing variable collection

      :returns: ``ROOT.vector`` list of variable names

   .. py:method:: printAliases()
 
      Prints all aliases currently registered.
      Useful to call just before calling `basf2.process` on an analysis `basf2.Path` when debugging.


.. _variablesByGroup:

Variables by group
==================

Here is a categorised list of variables known to ``basf2``.
You can also look at the alphabetical index: :ref:`b2-varindex`.
 
Kinematics 
~~~~~~~~~~

.. b2-variables::
   :group: Kinematics

Helicity 
~~~~~~~~

.. b2-variables::
      :group: Helicity variables

Tracking
~~~~~~~~

Here is a list of track variables:

.. b2-variables::
   :group: Tracking

PID
~~~

Here is a list of particle identification variables:

.. warning ::
  The **definitions** of the default PID variables have changed between
  release-01 and release-02. 

  Prior to release-02-00-00 (i.e. in release-01-XX-YY) each ID was calculated
  against the pion likelihood alone, or the kaon in the case of the pion itself.
  Namely the pair probability (also known as the binary probability) was returned: 

    * for all particles: :math:`\text{<Part>ID}=\mathcal{L}_{\text{<Part>}}/\mathcal{L}_\pi`, where :math:`\text{<Part>}\in[e,\mu,K,p,d]`.

    * for pions: :math:`\text{PionID}=\mathcal{L}_\pi/\mathcal{L}_K`.

  In other words, pionID was sensitive only to the pion-kaon mis-id, and not to
  the pion-proton or pion-muon mis-identification.

.. b2-variables::
   :group: PID   

Basic particle information
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. b2-variables::
   :group: Basic particle information

PID for expert
""""""""""""""
These expert-level variables are metavariable that allow the used to access the
LogLikelihood values, the binary likelihood ratios and the global likelihood
ratios for any arbitrary detector combination of mass hypothesis. The accepted
detector codes are SVD, TOP, CDC, ARICH, ECL, KLM and ALL.

If a likelihood is not available from the selected detector list, **NaN** is returned.

.. warning :: 
  These variables are not to be used in physics analyses, but only by experts doing performance studies.

.. b2-variables::
   :group: PID_expert

ECL Cluster
~~~~~~~~~~~

Here is a list of variables related to ECL cluster.
All ECLCluster-based variables return NaN if no ECLCluster is found.

.. _importantNoteECL:

.. note::
    All floating type variables in the mdst dataobject ECLCluster use ROOT Double32_t types with
    specific range declaration to save disk storage. This has two important consequences for a user:
        
        - All ECL cluster variables have a limited precision. This precision is always better than
          the intrinsic ECL data acquisition precision. However, if these variables are histogrammed,
          binning effects are likely.
        - All ECL cluster variables are clipped at the lower and upper boundaries: Values below (above)
          these boundaries will be set to the lower (upper) bound.
    
    Lower and uppper limits, and precision of these variables are mentioned inside the note box below them.
    One should note this in the context of binning effects.


.. b2-variables::
   :group: ECL Cluster related

There are also some special variables related to the MC matching of ECL clusters (specifically).

.. b2-variables::
   :group: MC Matching for ECLClusters

Acceptance
~~~~~~~~~~

Here is a list of variables for acceptance cuts:

.. b2-variables::
   :group: Acceptance

Trigger
~~~~~~~

Here is a list of trigger variables:

.. b2-variables::
   :group: L1 Trigger

.. b2-variables::
   :group: Software Trigger

Event
~~~~~

Here is a list of event variables:

.. b2-variables::
   :group: Event   

Parameter Functions
~~~~~~~~~~~~~~~~~~~

Here is a list of variables that require a parameter:

.. b2-variables::
   :group: ParameterFunctions   

Meta Functions
~~~~~~~~~~~~~~

Here is a list of variables that returns extra info of a given particle:

.. b2-variables::
   :group: MetaFunctions   

MC matching and MC truth
~~~~~~~~~~~~~~~~~~~~~~~~

Here is a list of MC truth-related variables.
For some variables, you will need to run truth matching in order to get sensible results.

.. code-block:: python

    from modularAnalysis import matchMCTruth
    matchMCTruth("B0:myCandidates")  # for example


Variables will also work on generator-level particles:

.. code-block:: python

    from modularAnalysis import fillParticleListFromMC
    fillParticleListFromMC("B0:generator", "") # the generator-level B particles


.. b2-variables::
   :group: MC matching and MC truth


.. b2-variables::
   :group: MC particle seen in subdetectors


Daughter info
~~~~~~~~~~~~~

Here is a list of variables getting info from particle's daughters:

.. b2-variables::
   :group: DirectDaughterInfo

KLM Cluster and :math:`K_{L}^0` Identification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is a list of KLM Cluster and :math:`K_{L}^0` identification variables:

.. warning ::
  Please note that these variables refer to KLMClusters, which are designed to reconstruct :math:`K_{L}^0` and other
  neutral particles with the KLM subdetector. These variables **must not be used to do particle identification of
  charged tracks** (for example, they must not be used to identify muons), otherwise there is a serious risk to spoil
  a physics analysis. 
  
  For particle identification of charged tracks, please use the canonical PID variables.

.. b2-variables::
   :group: KLM Cluster and KlongID

Time Dependent CPV Analysis Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use most of the variables in this section on need to run `vertex.TagV` method:

.. b2-variables::
   :group: Time Dependent CPV Analysis Variables

Flavor Tagger
~~~~~~~~~~~~~

.. b2-variables::
   :group: Flavor Tagger Variables
.. b2-variables::
   :group: Flavor Tagger MetaFunctions

.. _restOfEventVariables:

Rest of Event
~~~~~~~~~~~~~

.. b2-variables::
   :group: Rest Of Event

Continuum Suppression
~~~~~~~~~~~~~~~~~~~~~

For a detailed description of the continuum suppression, see `ContinuumSuppression`

.. b2-variables::
    :group: Continuum Suppression


.. _eventShapeVariables:

Event Shape
~~~~~~~~~~~

These variables are available after adding the event shape builder modules.
This can be done with the function `modularAnalysis.buildEventShape`.
For a detailed description of the event shape variables,  see `EventShape`

.. b2-variables::
    :group: EventShape

.. _eventKinematicsVariables:

Event Kinematics
~~~~~~~~~~~~~~~~

These variables are available after adding the event kinematics modules.
This can be done with the function `modularAnalysis.buildEventKinematics`.

.. b2-variables::
    :group: EventKinematics
         
.. _flight_information:

Flight Information
~~~~~~~~~~~~~~~~~~

Here is a list of flight time and distance variables of a (grand)daughter particle w.r.t. its (grand)mother decay vertex:

.. b2-variables::
   :group: Flight Information   

.. _vertex_information:

Vertex Information
~~~~~~~~~~~~~~~~~~

Here is a list of production and decay vertex variables:

.. b2-variables::
   :group: Vertex Information   

Belle and ``b2bii`` variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Several legacy Belle variables are provided. 

.. note::
   These are intended for studies with ``b2bii`` and for comparison between Belle and Belle II.

.. b2-variables::
   :group: Belle Variables

PID for B2BII
"""""""""""""

.. warning:: 
   These variables are to be used only when analysing converted Belle samples.

.. b2-variables::
   :group: PID_belle


Miscellaneous
~~~~~~~~~~~~~

Other variable that can be handy in development:

.. b2-variables::
   :group: Miscellaneous

Calibration
~~~~~~~~~~~

There are several variables also available for calibration experts who are working on ``cdst`` format files.

.. warning:: Many of these will not work for- and should not be used by- normal analyses.

They have a **[Calibration]** pretag.

.. b2-variables::
   :group: Event (cDST only)
.. b2-variables::
   :group: ECL calibration
.. b2-variables::
   :group: ECL trigger calibration
.. b2-variables::
   :group: KLM Calibration | PID


Collections and Lists
=====================

To avoid very long lists of variable names in `variablesToNtuple <modularAnalysis.variablesToNtuple>`,
it is possible to use collections of variables or lists of variables instead.

Lists of variables are just python lists of variables names. 
One can use the list in the steering file as follows:

.. code:: python

  # Defining the list
  my_list = ['p','E']

  # Passing it as an argument to variablesToNtuple
  modularAnalysis.variablesToNtuple(variables=my_list, ...)

It is also possible to create user-defined variable collections.
The name of the variable collection can be treated as a variable name.

.. autofunction:: variables.utils.add_collection


Predefined collections
~~~~~~~~~~~~~~~~~~~~~~

We provide several predefined lists of variables.
For each predefined list, there is a collection with the same name:

.. automodule:: variables.collections
   :members:


Operations with variable lists
==============================

It is possible to create new variable lists using meta-variables.  For example,
one can define list of kinematic variables in LAB frame and create another
lists of kinematic variables in CMS using ``useCMSFrame(variable)`` meta-variable:

.. code:: python

  from variables.utils import create_aliases
  # Replacement to Kinematics tool
  kinematics = ['px', 'py', 'pz', 'pt', 'p', 'E']

  # Kinematic variables in CMS
  cms_kinematics = create_aliases(kinematics, "useCMSFrame({variable})", "CMS")

Now we can use the list of aliases ``cms_kinematics`` and add them to the
output in one go or modify them further. The following functions are provided
to help to easily create aliases.

.. we don't document all of the stuff on this module and we choose the order
   because the remaining functions will not be helpful for users and this is a
   user manual after all.

.. autofunction:: variables.utils.create_aliases
.. autofunction:: variables.utils.create_aliases_for_selected
.. autofunction:: variables.utils.create_daughter_aliases
.. autofunction:: variables.utils.create_mctruth_aliases

Miscellaneous helpers for using variables
=========================================

.. autofunction:: variables.getAllTrgNames
.. autofunction:: variables.std_vector
.. autofunction:: variables.printVars
