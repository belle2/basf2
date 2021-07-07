Belle2 Physics List
===================

.. _Geant4-provided reference physics lists : https://geant4.web.cern.ch/node/155
.. _Belle2PhysicsList : https://stash.desy.de/projects/B2/repos/software/browse/simulation/physicslist/src/Belle2PhysicsList.cc
.. _geant4/source/physics_lists/constructors : https://geant4.kek.jp/lxr/source/physics_lists/constructors/

`Belle2PhysicsList`_ is a Geant4 modular physics list designed to fit the requirements of
Belle2 simulation.  It is an alternative to the `Geant4-provided reference physics lists`_,
and provides several options to further customize the physics that it invokes.

Belle2PhysicsList was developed "from scratch", rather than modify one of the Geant4
reference physics lists.  This allows for simpler code which is easier to read,
understand and modify.  Templating is avoided as much as possible and a more modular
structure is followed.

Physics modules
---------------

*Belle2PhysicsList* is derived from G4VModularPhysicsList.  This allows the construction
of Geant4 processes, models and cross sections to be grouped into modules called physics
constructors.  In *Belle2PhysicsList* there are eleven such constructors:

- G4EmStandardPhysics_option1
- G4DecayPhysics
- G4OpticalPhysics
- ProtonPhysics
- NeutronPhysics
- PionPhysics
- KaonPhysics
- HyperonPhysics
- AntiBaryonPhysics
- IonPhysics
- GammaLeptoNuclearPhysics

The first three physics constructors are part of the standard Geant4 distribution and
can be found in `geant4/source/physics_lists/constructors`_ .
They handle the purely electromagnetic interactions, weak and electromagnetic decays,
and optical photon interactions, respectively.  The next seven constructors deal with
purely hadronic interactions and the last one handles gamma-nuclear and lepto-nuclear
interactions.

Electromagnetic physics
-----------------------

This physics list is implemented with the so-called "option1" electromagnetic physics.
The physics models used here are older and less precise, but faster, than those used by
most physics lists.  While almost all of the Geant4 reference physics lists, such as
*FTFP_BERT*, use the G4EmStandardPhysics constructor, G4EmStandardPhysics_option1 was
chosen for Belle2 because of its speed.  The LHC CMS experiment observed a 30% speed-up
with only a small loss in precision.  It remains to be seen through validation whether
this small loss of precision is acceptable for Belle2.  If not, the physics list
provides the option to switch back to G4EmStandardPhysics.

An option also exists to add optical photon physics.  By default, this is not added in
order to save CPU time.  

Decay physics
-------------

The decay physics constructor implements the weak and electromagnetic decays of all
long-lived hadrons and leptons.  Hadronic decays, such as the decay of rho or delta
resonances, etc., are not included here but are part of the various hadronic models.
The decays of heavy flavor particles, such as D and B mesons are also not implemented,
assuming that these decays are handled by generators external to Geant4.
  
Although a radioactive decay package is available in Geant4, no such process is assigned
to particles in this physics list.

Hadronic physics
----------------

The choice of hadronic physics in *Belle2PhysicsList* was tuned to match the Belle2 energy
range.  In most of the Geant4 reference physics lists, including *FTFP_BERT*, this choice
was governed more by high energy physics data from test beam experiments.  

The implementation of inelastic hadronic collisions of hadrons with nuclei is quite
detailed because many different models and sub-models are required to cover the entire
energy range and the set of particles involved.  For *Belle2PhysicsList*, and most of the
Geant4 reference physics lists, these processes are usually implemented by two main 
hadronic models, the Bertini cascade and the FTFP QCD string model.  QCD string models
are excellent for high energies where the incident hadron has more than a few GeV of
kinetic energy, sufficient to form a QCD string.  Below such energies, QCD strings do not
form and a larger part of the target nucleus contributes to the reaction.  Hence
intra-nuclear cascade models such as Bertini are used.  Because the dividing line
between these two regimes is not sharp, and because a smooth transition between one and
the other is desired, Geant4 hadronic models are often overlapped in energy.  The
following list shows the energy ranges over which the hadronic models are applied.

+-------------------+-------------------------------------------------------+
| hadron            |                     Energy ranges                     |
+===================+=======================================================+
| protons, neutrons | Bertini from 0 to 12 GeV, FTFP from 5 GeV to 100 TeV  |
+-------------------+-------------------------------------------------------+
|    pions, kaons   | Bertini from 0 to 12 GeV, FTFP from 10 GeV to 100 TeV |
+-------------------+-------------------------------------------------------+
|     hyperons      | Bertini from 0 to 6 GeV,  FTFP from 4 GeV to 100 TeV  |
+-------------------+-------------------------------------------------------+
|   anti-baryons    | FTFP from 0 to 100*TeV                                |
+-------------------+-------------------------------------------------------+
|                   | Binary light ion from 0 to 110 MeV                    |
|                   +-------------------------------------------------------+
|       ions        | QMD nucleus-nucleus model from 100 MeV to 10 GeV      |
|                   +-------------------------------------------------------+
|                   | FTFP from 10.01 GeV to 1 TeV                          |
+-------------------+-------------------------------------------------------+

Where two models overlap in energy, Geant4 throws a random number to see which model is
applied.  At the low energy edge of the overlap region, the low energy model is chosen
100% of the time.  At the upper energy edge, the high energy model is chosen 100% of
the time.  In between, the probability that the low energy model is chosen ramps down
linearly with energy while that for the high energy model ramps up.

The overlap regions in *Belle2PhysicsList* differ from those in the Geant4 reference
physics lists.  Since the hadrons in the Belle2 detector are lower in energy than those
in the LHC detectors, the Bertini cascade is used more and FTFP is used less.  This means
that transition between Bertini and FTFP is moved up in energy.  This change should
improve the agreement between simulation and Belle2 showers, and reduce the required
simulation time since the Bertini cascade is faster than FTFP at the same energies.

A similar scheme is used for the elastic processes.  Fewer models are required and they
tend to be simpler than in the inelastic case.  Hadron capture processes are also
implemented.

*Belle2PhysicsList* also offers the option to use the Geant4 high precision neutron
package below 20 MeV.  This option is useful if background or beamline issues are being
studied, but would not be used in the standard simulation due to the large amount of CPU
time required.

Secondary production thresholds
-------------------------------

This physics list allows the Geant4 secondary production thresholds to be set either
globally or separately for each detector envelope (cuts per region).  The current global
default value is 0.07 cm.  Using the steering script, a different threshold may be set 
for each of the following detectors:

- PXD   0.01 cm
- SVD   0.02 cm
- CDC   0.10 cm
- TOP + ARICH  0.05 cm  (currently one value for both TOP and ARICH envelopes)
- ECL   0.10 cm
- KLM   0.10 cm

The values shown here are suggestions and may be changed in order to optimize the simulation.
See below for how to set these for a run.

Using this physics list 
-----------------------

*Belle2PhysicsList* is currently invoked in **FullSimModule**.  It can be chosen by setting
``PhysicsList="Belle2"`` 
in ``main.add_module`` for ``FullSim``.
Secondary production thresholds may also be set in ``main.add_module``.
If no cut values are specified in this file, the default value of 0.07 cm is
used everywhere in the Belle2 geometry.  This global default can be changed by setting 
the parameter ``ProductionCut``.  Each detector envelope can have a cut value distinct from 
the global value by setting the parameters ``PXDProductionCut``, ``SVDProductionCut``, etc.
Cut values which are not explicitly set for a given detetcor envelope assume the value 
set by the ``ProductionCut`` parameter. 

Example useage::

   main.add_module("FullSim",PhysicsList="Belle2", RunEventVerbosity=0,
                  RegisterOptics=False,
                  # StandardEM=True,
                  # UseHighPrecisionNeutrons=True,
                  # ProductionCut=0.07,
                  StoreBremsstrahlungPhotons=True, BremsstrahlungPhotonsEnergyCut=10.0)

Here the physics list is invoked with its default EM and neutron options since these
lines are commented out.

.. note::
    
  Above example is part of test script, `Belle2PhyslistTestBrems.py <https://stash.desy.de/projects/B2/repos/software/browse/simulation/examples/Belle2PhyslistTestBrems.py>`_, which is identical to `BremsstralungPhotons.py <https://stash.desy.de/projects/B2/repos/software/browse/simulation/examples/BremsstralungPhotons.py>`_,:: 

   main.add_module("FullSim", StoreBremsstrahlungPhotons=True, BremsstrahlungPhotonsEnergyCut=10.0)

  except that it invokes *Belle2PhysicsList*

