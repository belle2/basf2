Secondary Particles
===================

In the *Belle II* terminology, primary particles are the ones created by the generators, which are 
subsequently fed into the *Geant4* simulation as the input information. 

Secondary particles are the ones created during the *Geant4* simulation afterwards. For example, 
in-flight decays of Ks or Cerenkov lights (i.e., optical photons) created inside the TOP detector 
are secondary particles.To reduce the size of the output files created by the simulation package, 
the generator level information on the secondary particles is not stored in the output ``MCParticle`` 
block. This is the default option (note : The life of each secondary particle is still simulated). 

If needed, users/software coders are allowed to store the information in the 
``MCParticle`` block for individual secondary particle by flipping the ignore flag assigned to the particle.
Note that it is subdetector group's responsibility to make sure the ``MCParticle`` 
relation of a hit originated from a secondary partcle is assigned in the correct way. 
Otherwise, the MC track finder routines do not work properly.

The Ignore Flag
----------------

In general, after a new ``MCParticle`` (or ``MCParticleGraph``, the internal variable) 
entry is added, its ignore flag is set to true. The generator level information is blocked 
from being written in the output file. The exceptions are the following cases. The ignore 
flag is set to false or should be flipped to false, making their generator level information 
avaliable in the output file.

1. Virtual particles. For example, Upsilon(4S).
2. Primary particles with a simulated Geant4 track.
3. Decays-in-flight secondary particles with a simulated Geant4 track.
4. Secondary particles leaving hits in the sensitive detector area.

For Case 4, the flipping operation should be done by each subdetector group. 
Logically, we can set the flag by directly calling the ``MCParticleGraph`` variable such as,

.. code-block:: cpp

   MCParticleGraph.setIgnore(false);

But it may not be easy to find the path to the ``MCParticleGraph`` variable when you are located 
in the sensitive detector area. There is an easier way to set the flag by using the G4Track variable, 
which is connected to the ``MCParticleGraph`` variable via the TrackInfo method internally. For example,

.. code-block:: cpp

   G4Track& track = *step->GetTrack(); 
   Simulation::TrackInfo::getInfo(track).setIgnore(false);


will do the job.

For Cases 1-3, the flag is already set to false in the appropriate places.

MCParticle Relation
--------------------

When each subdetector group implements the sensitive detector area, they should decide how to 
reassign the ``MCParticle`` relation of the hits originated from the ignored secondary particles: 
Either reattribute the element, set the ``MCParticle`` Relation weight to zero or a negative value, 
or drop the relation element completely for the ignored particle. 

This is how the ``MCParticle`` relation is registered for the hits in the sensitive detector,

.. code-block:: cpp

   registerMCParticleRelation(relation, RelationArray::option); 


where the options are:

- ``c_doNothing``
  same as before, i.e., reassigns the relation to the parent particle. (This was the default option until 2013 summer).
- ``c_zeroWeight`` 
  set weight to zero and reassigns the relation to the parent particle.
- ``c_negativeWeight`` 
  make weight negative (indicating it is originated from an ignored secondary particle) and 
  reassigns the relation to the parent particle. The default option since 2013 summer.
- ``c_deleteElement``  delete the element


If You Want to Save All (or Most of) the Secondary Particle Information in the MCParticles Block
-------------------------------------------------------------------------------------------------

From time to time, you may want to save all the secondary particle information in the output ``MCParticle`` 
block for the detector response studies such as shower shape anlaysis, etc. Here is the way to do as you want. 
A certain ``FullSim`` parameters should be modified to steer the writing process for the secondary particles 
in the correct way. In the python steering file for your ``basf2`` job script, use the following lines in place 
of the corresponding ``add_simulation`` lines or the equivalent:

.. code-block:: python

   g4sim = register_module('FullSim')
   g4sim.param('StoreAllSecondaries', True)
   g4sim.param('SecondariesEnergyCut', 1.0)
   path.add_module(g4sim)

Since there could be too many very low energy secondaries which may blow up the size of the MCParticles block 
into an uncontrollable number, we have an additional parameter called ``SecondaryEnergyCut``. The default value 
is 1.0 MeV. If the energy of a secondary particle is below this threshhold, the particle information will not be 
saved in the MCParticles block, even though the ``StoreAllSecondaries`` parameter is set to True. Otherwise, 
if you do not mind the size of the MCParticles block being too large, you can set this threshhold parameter as 0.

Physics Process
----------------

The *Geant4* provides the information on via which physics process (and its subtype) a secondary particle is created. 
This is stored in the MCParticles and can be retrieved as

.. code-block:: python

   MCParticle.getSecondaryPhysicsProcess();


Between the physics process type and subtype, the subtype parameter gives more detailed information. 
What is implemented in ``basf2`` is the subtype information. Unfortunately, the *Geant4* people did not store the 
definition of subtypes in one file, so one should check several Geant4 files for these numbers. The following is 
the list of constants implemented in *Geant4 v9.6*. They used the same list for v10, too.

.. table:: Physics Process and its subtype
   :widths: auto

   ============================    ==========================    =======================
   enum G4EmProcessSubType         enum G4HadronicProcessType    enum G4DecayProcessType
   ============================    ==========================    =======================
   fCoulombScattering      =  1    fHadronElastic       = 111    DECAY             = 201
   fIonisation             =  2    fHadronInelastic     = 121    DECAY_WithSpin,
   fBremsstrahlung         =  3    fCapture             = 131    DECAY_PionMakeSpin,
   fPairProdByCharged      =  4    fMuAtomicCapture     = 132    DECAY_Unknown     = 211
   fAnnihilation           =  5    fFission             = 141    DECAY_MuAtom      = 221
   fAnnihilationToMuMu     =  6    fHadronAtRest        = 151    DECAY_External    = 231
   fAnnihilationToHadrons  =  7    fLeptonAtRest        = 152    
   fNuclearStopping        =  8    fChargeExchange      = 161
   fElectronGeneralProcess =  9    fRadioactiveDecay    = 210
   fMultipleScattering     = 10
   fRayleigh               = 11
   fPhotoElectricEffect    = 12
   fComptonScattering      = 13
   fGammaConversion        = 14
   fGammaConversionToMuMu  = 15
   fGammaGeneralProcess    = 16
   fCerenkov               = 21
   fScintillation          = 22
   fSynchrotronRadiation   = 23
   fTransitionRadiation    = 24
   ============================    ==========================    =======================

Release note (newly included parameters since) :

- G4DecayProcessType    : fElectronGeneralProcess and fGammaGeneralProcess since v10.5
- G4HadronicProcessType : fMuAtomicCapture and fLeptonAtRest since v10.4
- G4DecayProcessType    : DECAY_MuAtom since v10.4

.. note::
        - If the MCParticles entry is a primary particle, "0" is assigned.
        - When the *Geant4* does not give the physics process subtype information, "-1" is assigned. This happens rarely.

.. warning::
        fElectronGeneralProcess and fGammaGeneralprocess are internal Geant4 parameters, which 
        general users are not supposed to see. If you see any of these, please contact the simulation convener 
        or Geant4 team. 
 
