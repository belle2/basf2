.. _event_generators_validation:

Validation
----------

The validation of the MC generators is performed to ensure that updates to the framework do not break the generator
level particle multiplicities and event shapes. These validations are run automatically when updates are added to the
framework and can also be run manually. The output histograms are compared to reference histograms to make sure that
nothing has changed.

.. warning:: In the case of updating to new releases or if changes to the DECAY_BELLE2.dec are implemented it is very
   likely that the histograms will not agree with the reference. This validation is only a good metric when comparing
   updates to the same release as the reference histograms.

The different python scripts used for the validation are located at ``generators/validation/decayfile/``.

Reconstruction Scripts
^^^^^^^^^^^^^^^^^^^^^^

There are two reconstruction scripts divided between charged and mixed MC:

* ``MultiplicitiesEventShapeCharged.py``
* ``MultiplicitiesEventShapeMixed.py``

Parameters
""""""""""

* **input**: the input root file, default file for MultiplicitiesEventShapeCharged.py and
    MultiplicitiesEventShapeMixed.py are ``charged.cdst.root`` and ``mixed.cdst.root`` respectively
* **output**: the output file name, default names  for MultiplicitiesEventShapeCharged.py and
    MultiplicitiesEventShapeMixed.py are ``MCvalidationCharged.root`` or ``MCvalidationMixed.root`` respectively

The output file contains three TTrees. The first TTree is the particle multiplicities on generator level, the second
TTree is the particle multiplicities split between if the particle is a daughter
of :math:`B^+`, :math:`B^-`, :math:`B^0` or :math:`\overline{B^0}`. They split multiplicity variables are defined as
eventExtraInfo variables in ``SplitMultiplicities.py``. Finally the last TTree saves the generator level event shapes
defined in the ``event_shape`` collection in :ref:`analysis/doc/Variables:Predefined collections`.


Validation Histograms
^^^^^^^^^^^^^^^^^^^^^

The validation histograms are created with the following python scripts:

* ``MultiplicityPlotsCharged.py`` 
* ``MultiplicityPlotsMixed.py``
* ``SplitMultiplicityPlotsCharged.py``
* ``SplitMultiplicityPlotsMixed.py``
* ``EventShapePlotsCharged.py``
* ``EventShapePlotsMixed.py``


Multiplicity Plots
^^^^^^^^^^^^^^^^^^

Save validation histograms of particle multiplicities for genUpsilon4S daughters.

Parameters
""""""""""

* **input**: the input root file, default file for MultiplicityPlotsCharged.py and MultiplicityPlotsMixed.py are
    ``MCvalidationCharged.root`` and ``MCvalidationMixed.root`` respectively
* **output**: the output file name, default names for MultiplicityPlotsCharged.py and MultiplicityPlotsMixed.py are
    ``MultiplicityPlotsCharged.root`` or ``MultiplicityPlotsMixed.root`` respectively

Split Multiplicity Plots
^^^^^^^^^^^^^^^^^^^^^^^^

Save validation histograms of particle multiplicities split between if the particle is a daughter
of :math:`B^+`, :math:`B^-`, :math:`B^0` or :math:`\overline{B^0}`.

Parameters
""""""""""

* **input**: the input root file, default file for SplitMultiplicityPlotsCharged.py and SplitMultiplicityPlotsMixed.py
    are ``MCvalidationCharged.root`` and ``MCvalidationMixed.root`` respectively
* **output**: the output file name, default names for SplitMultiplicityPlotsCharged.py and
    SplitMultiplicityPlotsMixed.py are ``SplitMultiplicityPlotsCharged.root`` or ``SplitMultiplicityPlotsMixed.root``
    respectively

Event Shapes
^^^^^^^^^^^^

Save validation histograms of event shape variables.

Parameters
""""""""""

* **input**: the input root file, default file for EventShapePlotsCharged.py and EventShapePlotsMixed.py are
    ``MCvalidationCharged.root`` and ``MCvalidationMixed.root`` respectively
* **output**: the output file name, default names for EventShapePlotsCharged.py and EventShapeMixed.py are
    ``EventShapePlotsCharged.root`` or ``EventShapelotsMixed.root`` respectively


Validation of New Releases
^^^^^^^^^^^^^^^^^^^^^^^^^^

This section will detail how the MC validation between release 5 and release 6 was performed. The MC samples were
constructed using ``feature/enable-evtgen-for-using-old-decfiles-old-pythia`` (`see GitLab for more information
<https://gitlab.desy.de/belle2/software/basf2/-/tree/feature/enable-evtgen-for-using-old-decfiles?ref_type=heads>`_)
which enables release 6 to simulate MC with release 5 DECAY_BELLE2.dec. The three samples investigated in the
validation was:

* **R5D5**: Old Evtgen version, Rel 5 dec file, Pythia 6 (generated with release-05-01-10)
* **R6D6**: New Evtgen version (2.00), Rel 6 dec file, Pythia 8 (generated with prerelease-06-00-00b)
* **R6D5**: New Evtgen version (2.00), Rel 5 dec file, Pythia 6 internally converted to Pythia 8 (generated with prerelease-06-00-00b)

This makes it easier to investigate where differences between releases originate from. It is also a good check to see if
changes to the DECAY_BELLE2.dec are seen in the MC as expected.
