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

.. b2-variables::
   :group: PID   

ECL Cluster
~~~~~~~~~~~

Here is a list of variables related to ECL cluster.
There is further detailed documentation available `here<https://confluence.desy.de/x/I3I0Aw>`_.

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

MC Matching
~~~~~~~~~~~

Here is a list of truth-matching variables:

.. b2-variables::
   :group: MC Matching

Daughter info
~~~~~~~~~~~~~

Here is a list of variables getting info from particle's daughters:

.. b2-variables::
   :group: DirectDaughterInfo

KLM Cluster and :math:`K_{L}^0` Identification
~~~~~~~~~~~

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

Rest of Event
~~~~~~~~~~~~~

.. b2-variables::
   :group: Rest Of Event

Continuum Suppression
~~~~~~~~~~~~~~~~~~~~~

.. b2-variables::
    :group: Continuum Suppression

Miscellaneous
~~~~~~~~~~~~~

Other variable that can be handy in development:

.. b2-variables::
   :group: Miscellaneous

Flight Information
~~~~~~~~~~~~~~~~~~

Here is a list of flight time and distance variables of a (grand)daughter particle w.r.t. of its (grand)mother decay vertex:

.. b2-variables::
   :group: Flight Information   

Vertex Information
~~~~~~~~~~~~~~~~~~

Here is a list of production and decay vertex variables:

.. b2-variables::
   :group: Vertex Information   


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
