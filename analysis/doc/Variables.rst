.. _variables:

Variables
=========

Variables are handled by the VariableManager:

.. _variablemanager:
.. automodule:: variables
   :members:
   :undoc-members:

   .. autodata:: variables.variables


Here is a categorised list of variables known to ``basf2``.
You can also look at the alphabetical index: :ref:`b2-varindex`.
 
Kinematics 
~~~~~~~~~~

.. b2-variables::
   :group: Kinematics

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

More information in `this confluence page <https://confluence.desy.de/display/BI/Physics+charged+particle+identification>`_

.. b2-variables::
   :group: PID   

PID for expert
""""""""""""""
These expert-level variables are metavariable that allow the used to access the
LogLikelihood values, the binary likelihood ratios and the global likelihood
ratios for any arbitrary detector combination of mass hypothesis. The accepted
detector codes are SVD, TOP, CDC, ARICH, ECL, KLM and ALL.

If a likelihood is not available from the selected detector list, `NaN` is returned.

.. warning :: 
  These variables are not to be used in physics analyses, but only by experts doing performance studies.

.. b2-variables::
   :group: PID_expert

PID for B2BII
"""""""""""""
.. warning :: 
  These variables are to be used only when analysing converted Belle samples.

.. b2-variables::
   :group: PID_belle

ECL Cluster
~~~~~~~~~~~

Here is a list of variables related to ECL cluster.
All ECLCluster-based variables return NaN if no ECLCluster is found.
There is further detailed documentation available `here <https://confluence.desy.de/x/I3I0Aw>`_.

.. b2-variables::
   :group: ECL Cluster related

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

.. b2-variables::
   :group: KLM Cluster and KlongID

Time Dependent CPV Analysis Variables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is a list of TDCPV variables:

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

.. b2-variables::
    :group: Continuum Suppression


.. _eventShapeVariables:

Event Shape
~~~~~~~~~~~

These variables are available after adding the event shape builder modules.
This can be done with the function `modularAnalysis.buildEventShape`.

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

Here is a list of flight time and distance variables of a (grand)daughter particle w.r.t. of its (grand)mother decay vertex:

.. b2-variables::
   :group: Flight Information   

.. _vertex_information:

Vertex Information
~~~~~~~~~~~~~~~~~~

Here is a list of production and decay vertex variables:

.. b2-variables::
   :group: Vertex Information   



Miscellaneous
~~~~~~~~~~~~~

Other variable that can be handy in development:

.. b2-variables::
   :group: Miscellaneous

Calibration
~~~~~~~~~~~

There are several variables also available for calibration experts who are working on cdst format files.
Many of these will not work for- and should not be used by- normal analyses.
They have a *[Calibration]* pretag.

.. b2-variables::
   :group: Event (cDST only)
.. b2-variables::
   :group: ECL calibration
.. b2-variables::
   :group: ECL trigger calibration
.. b2-variables::
   :group: KLM Calibration | PID

Variables Collections and Lists
===============================

To avoid very long lists of variable names in `variablesToNtuple <modularAnalysis.variablesToNtuple>`,
it is possible to use collections of variables or lists of variables instead.

Lists of variables are just python lists of variables names. 
One can use the list in the steering file as follows:

.. code:: python

  # Defining the list
  my_list = ['p','E']

  # Passing it as an argumet to variablesToNtuple
  modular_analusis.variablesToNtuple(variables=my_list,
                                     ...)

It is also possible to use `variableCollection`. Name of the variable collection can
be threated as a variable name, and hence one would have the following syntax in the steering file:

.. code:: python

  # Defining the collection
  variables.utils.add_collection(['p','E'],"my_collection")

  # Passing it as an argumet to variablesToNtuple
  modular_analusis.variablesToNtuple(variables=['my_collection'],
                                     ...)

There are several predefined lists of variables and for each predefined list it exists a collection with the same name:

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

.. autofunction:: variales.utils.create_aliases
.. autofunction:: variales.utils.create_daughter_aliases
.. autofunction:: variales.utils.create_aliases_for_selected
.. autofunction:: variales.utils.make_mc
