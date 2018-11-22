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
.. b2-variables::
   :group: KLM Calibration | PID
