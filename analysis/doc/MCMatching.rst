Truth-matching
==============

.. _mcmatching:

-----------
MC matching
-----------

A general overview of the main MC matching algorithm and its user interface can be found in the
proceeding `Monte Carlo matching in the Belle II software <https://doi.org/10.1051/epjconf/202125103021>`_
for the CHEP 2021 conference.

~~~~~~~~~~~~~~~~~~~~~~
First, you must run it
~~~~~~~~~~~~~~~~~~~~~~

MCMatching relates ``Particle`` and ``MCParticle`` objects.

.. important::
        Most MC matching variables will have non-trivial values only if the :b2:mod:`MCMatcherParticles` module is actually executed.
        It can be executed by adding the module to your path, there is a `modularAnalysis.matchMCTruth` convenience function to do this.

~~~~
Core
~~~~

MC matching at Belle II returns two important pieces of information:
the true PDG id of the particle :b2:var:`mcPDG`,
and an error flag :b2:var:`mcErrors`.

Both variables will have non-trivial values only if the MCMatching module,
which relates composite ``Particle`` (s) and ``MCParticle`` (s), is executed.
:b2:var:`mcPDG` is set to the PDG code of the first common mother ``MCParticle`` of the daughters of this ``Particle``.


.. b2-variables::
        :variables: mcPDG,mcErrors
        :noindex:

~~~~~~~~~~~~~~~
Extra variables
~~~~~~~~~~~~~~~

There are several extra variables relating to MCMatching.
Many are defined for convenience and can be recreated logically from :b2:var:`mcPDG` and :b2:var:`mcErrors`.
Some extra variables are provided externally, for example :b2:var:`isCloneTrack` from the tracking-level MC matching.

.. b2-variables::
        :variables: isSignal,isSignalAcceptWrongFSPs,isSignalAcceptMissingNeutrino,isSignalAcceptMissingMassive,isSignalAcceptMissingGamma,isSignalAcceptMissing,isWrongCharge,isMisidentified,isCloneTrack,isOrHasCloneTrack,genNStepsToDaughter(i),genNMissingDaughter(PDG)
        :noindex:


.. _Error_flags:

~~~~~~~~~~~
Error flags
~~~~~~~~~~~

The error flag :b2:var:`mcErrors` is a bit set where each bit flag describes
a different kind of discrepancy between reconstruction and ``MCParticle``.
The individual flags are described by the ``MCMatching::MCErrorFlags`` enum.
A value of mcErrors equal to 0 indicates perfect reconstruction (signal).
Usually candidates with only FSR photons missing are also considered as signal,
so you might want to ignore the corresponding ``c_MissFSR`` flag.
The same is true for ``c_MissingResonance``, which is set for any missing composite particle (e.g. :math:`K_1`, but also :math:`D^{*0}`).

The behavior of :b2:var:`mcErrors` can be configured by decay string grammar with :func:`modularAnalysis.reconstructDecay`.
For more information and examples how to use the decay strings correctly, please see :ref:`DecayString` and :ref:`Grammar_for_custom_MCMatching`.

Full documentation of the MCMatching algorithm is described in `BELLE2-CONF-PROC-2022-004 <https://docs.belle2.org/record/2868/files/BELLE2-CONF-PROC-2022-004.pdf>`_.

==============================  ================================================================================================
Flag                            Explanation
==============================  ================================================================================================
 c_Correct       = 0             This Particle and all its daughters are perfectly reconstructed.
 c_MissFSR       = 1             A Final State Radiation (FSR) photon is not reconstructed (based on MCParticle: :c_IsFSRPhoton).
 c_MissingResonance = 2          The associated MCParticle decay contained additional non-final-state particles (e.g. a rho)
                                 that weren't reconstructed. This is probably O.K. in most cases.
 c_DecayInFlight = 4             A Particle was reconstructed from the secondary decay product of the actual particle.
                                 This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might mean
                                 a pion hypothesis was used for a secondary electron.
 c_MissNeutrino  = 8             A neutrino is missing (not reconstructed).
 c_MissGamma     = 16            A photon (not FSR) is missing (not reconstructed).
 c_MissMassiveParticle = 32      A generated massive FSP is missing (not reconstructed).
 c_MissKlong     = 64            A Klong is missing (not reconstructed).
 c_MisID = 128                   One of the charged final state particles is mis-identified (wrong signed PDG code).
 c_AddedWrongParticle = 256      A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters)
                                 belongs to another Particle.
 c_InternalError = 512           No valid match was found. For tracks, it indicates that there
                                 is not a true track related to the reconstructed one. Might indicate fake or background
                                 track or cluster.
 c_MissPHOTOS    = 1024          A photon created by PHOTOS was not reconstructed (based on MCParticle: :c_IsPHOTOSPhoton).
 c_AddedRecoBremsPhoton = 2048   A photon added with the bremsstrahlung recovery tools (correctBrems or correctBremsBelle) has
                                 no MC particle assigned, or it doesn't belong to the decay chain of the corrected lepton mother
==============================  ================================================================================================


~~~~~~~~~~~~~~
Example of use
~~~~~~~~~~~~~~

The two variables together allow the user not only to distinguish signal (correctly reconstructed)
and background (incorrectly reconstructed) candidates, but also to study and identify various types of physics background
(e.g. mis-ID, partly reconstructed decays, ...).
To select candidates that have a certain flag set, you can use bitwise and to select only this flag from :b2:var:`mcErrors`
and check if this value is non-zero: ``(mcErrors & MCMatching::c_MisID) != 0`` .
For use in a ``TTree`` selector, you'll need to use the integer value of the flag instead:

.. code-block:: cpp

        ntuple->Draw("M", "(mcErrors & 128) != 0")

You can also make use of ``MCMatching::explainFlags()`` which prints a human-readable
list of flags present in a given bitset. Can also be used in both C++ and python:

.. code-block:: python

        import basf2
        from ROOT import Belle2, gInterpreter
        gInterpreter.ProcessLine('#include "analysis/utility/MCMatching.h"')
        print(Belle2.MCMatching.explainFlags(a_weird_mcError_number))


If instead only binary decision (1 = signal, 0 = background) is needed,
then for convenience one can use :b2:var:`isSignal` (or :b2:var:`isSignalAcceptMissingNeutrino` for semileptonic decays).

.. code-block:: python
        
        from modularAnalysis import variablesToNtuple
        variablesToNtuple("X:mycandidates -> Y Z", variables = ["isSignal"] + other_interesting_variables)
        
assuming you have reconstructed :code:`X -> Y Z` :

.. code-block:: python

        from modularAnalysis import applyCuts
        applyCuts('X:myCandidates', 'isSignal==1')

-------------------------------------------------------
MC decay finder module :b2:mod:`ParticleCombinerFromMC`
-------------------------------------------------------

Analysis module to reconstruct a given decay using the list of generated particles ``MCParticle``. Only signal particles with `isSignal` equal to 1 are stored.

The module can be used for:

* Determination of the number of generated decays for efficiency studies, especially in the case of inclusive decays (e.g.: What's the generated number of :math:`B \to D^0 X` decays?).
* Matched MC decays as input for a truth matching module.
 
.. code-block:: python

  import basf2
  
  # Create main path
  main = basf2.create_path()
  
  # Modules to generate events, etc.
  ...

  import modularAnalysis as ma

  # Load particles from MCParticle at first
  ma.fillParticleListFromMC('K+:MC',    '', path=main)
  ma.fillParticleListFromMC('pi+:MC',   '', path=main)
  ma.fillParticleListFromMC('e+:MC',    '', path=main)
  ma.fillParticleListFromMC('nu_e:MC',  '', path=main)
  ma.fillParticleListFromMC('gamma:MC', '', path=main)

  """
  Example 1
  Search for B+ decaying to anti-D0* e+ nu_e, where anti-D0* decays to [anti-D0 -> K+ pi- pi0] and pi0.
  Additional photons emitted are ignored. Charge conjugated decays are matched, too.
  """
  # Reconstruct pi0 from gamma gamma at fist for convenience. Then reconstruct B+ with pi0:gg.
  ma.reconstructMCDecay('pi0:gg =direct=> gamma:MC gamma:MC', '', path=main)
  ma.reconstructMCDecay(
    'B+:DstENu =direct=> [anti-D*0:D0pi0 =direct=> [anti-D0:Kpipi0 =direct=> K+:MC pi-:MC pi0:gg] pi0:gg ] e+:MC nu_e:MC ',
    '',
    path=main)

  # One can directly reconstruct pi0:gg in same decay string.
  # But in this case, one have to write sub-decay of pi0:gg only once. Otherwise same particles are registered twice.
  # ma.reconstructMCDecay(
  #     'B+:DstENu =direct=>\
  #      [anti-D*0:D0pi0 =direct=> [anti-D0:Kpipi0 =direct=> K+:MC pi-:MC [pi0:gg =direct=> gamma:MC gamma:MC]] pi0:gg ]\
  #      e+:MC nu_e:MC ',
  #     '',
  #     path=main)


  """
  Example 2
  Search for B+ decaying to anti-D0 + anything, where the anti-D0 decays to K+ pi-.
  Ignore additional photons emitted in the anti-D0 decay. Charge conjugated decays
  are matched, too. If there is a match found, save to ParticleList 'B+:testB'
  """
  # Reconstruct B+ from [anti-D0 =direct=> K+ pi-] accepting missing daughters
  ma.reconstructMCDecay('B+:D0Kpi =direct=> [anti-D0:Kpi =direct=> K+:MC pi-:MC] ... ?gamma ?nu', '', path=main)
  

  ...
 

For more information and examples how to use the decay strings correctly, please see :ref:`DecayString` and :ref:`Grammar_for_custom_MCMatching`.


----------------------------------------------
MC decay finder module :b2:mod:`MCDecayFinder`
----------------------------------------------

Analysis module to search for a given decay in the list of generated particles ``MCParticle``.

The module can be used for:

* Determination of the number of generated decays for efficiency studies, especially in the case of inclusive decays (e.g.: What's the generated number of :math:`B \to D^0 X` decays?).
* Matched MC decays as input for a truth matching module.
 
.. code-block:: python

  import basf2
  
  # Create main path
  main = basf2.create_path()
  
  # Modules to generate events, etc.
  ...

  import modularAnalysis as ma
  # Search for B+ decaying to anti-D0 + anything, where the anti-D0 decays to K+ pi-.
  # Ignore additional photons emitted in the anti-D0 decay. Charge conjugated decays
  # are matched, too. If there is a match found, save to ParticleList 'B+:testB'
  ma.findMCDecay('B+:testB', 'B+ =direct=> [anti-D0 =direct=> K+ pi-] ... ?gamma ?nu', path=main)
  
  # Modules which can use the matched decays saved as Particle in the ParticleList 'B+:testB'
  ...
 

For more information and examples how to use the decay strings correctly, please see :ref:`DecayString` and :ref:`Grammar_for_custom_MCMatching`.

.. _MCDecayString:

---------------
MC decay string
---------------

Analysis module to search for a generator-level decay string for given particle.

~~~~~~~~~~~~~~~~~~
Using decay hashes
~~~~~~~~~~~~~~~~~~

The use of decay hashes is demonstrated in :code:`B2A502-WriteOutDecayHash.py` and :code:`B2A503-ReadDecayHash.py`.

:code:`B2A502-WriteOutDecayHash.py` creates one ROOT file, via `modularAnalysis.variablesToNtuple`
containing the requested variables including the two decay hashes, and a second root file containing the two decay hashes
and the full decay string.
The decay strings can be related to the candidates that they are associated with by matching up the decay hashes.
An example of this using python is shown in :code:`B2A503-ReadDecayHash.py`.

.. code-block:: python

  path.add_module('ParticleMCDecayString', listName='my_particle_list', fileName='my_hashmap.root')

This will produce a file with all of the decay strings in it, along with the decayHash
(hashes the MC decay string of the mother particle) and decayHashExtended
(hashes the decay string of the mother and daughter particles).
The mapping of hashes to full MC decay strings is stored in a ROOT file determined by the fileName parameter.

Then the variables ``extraInfo(decayHash)`` and ``extraInfo(decayHashExtended)`` are available in the `VariableManager`.

.. _GeneratedDecayModes:

---------------------
Generated decay modes
---------------------

A tool has been developed which analyzes the array of MC particles and determines the generated decay mode of the event.

A total of 825 :math:`B^+` decay modes have been defined:

  * The first 80 numbers are semi-leptonic decays (:math:`B^+ \to h l^+ \nu_l`), e.g.
  
     * 1001: :math:`B^+ \to \bar{D^{*0}} e^+ \nu_e`
     * 1002: :math:`B^+ \to \bar{D^0} e^+ \nu_e`
     * 1003: :math:`B^+ \to \bar{D_1^0} e^+ \nu_e`
  * The numbers 1085 to 1092 are radiative decays.
  * There are 17 :math:`B^+ \to X l^+ l^-` decay modes.
  * The numbers 1113 to 1608 are hadronic charmless decays.
  * Then there are 70 charmonium decays with :math:`J/\psi`, :math:`\Psi(2S)`, :math:`\eta_c`, etc.
  * This is followed by decays involving two or one charm meson.
  * The last 17 decay modes involve baryons.

The same number of :math:`B^-` decay modes have been implemented. They have negative tags.

.. b2-variables::
   :variables: BplusMode,BminusMode

The same general structure is used for the 1000 :math:`B^0` and :math:`\bar{B^0}` modes accessible via the variables

.. b2-variables::
   :variables: B0Mode,Bbar0Mode

For :math:`B_s^0` and :math:`\bar{B_s^0}` a total of 264 decay have been defined.

.. b2-variables::
   :variables: Bs0Mode,Bsbar0Mode

Besides the B meson decays three :math:`D^{*\pm}`, 84 :math:`D_s^\pm`, 84 :math:`D^\pm`, and 136 :math:`D^0` charm meson decays are implemented.
For each category the tag number starts again at 1001.

.. b2-variables::
   :variables: DstplusMode,DstminusMode,DsplusMode,DsminusMode,DplusMode,DminusMode,D0Mode,Dbar0Mode

The full list of all decay modes can be found in the technical Belle note `BELLE2-NOTE-TE-2021-002 <https://docs.belle2.org/record/2215/files/BELLE2-NOTE-TE-2021-002.pdf>`_.

.. _TauDecayMCModes:

------------------
Tau decay MC modes
------------------

A special case is the decay of generated tau lepton pairs. For their study, it is useful to call the function ``labelTauPairMC`` in the steering file.

.. code-block:: python

        from modularAnalysis import labelTauPairMC
        labelTauPairMC()

.. b2-variables::
        :variables: tauPlusMCMode,tauMinusMCMode,tauPlusMCProng,tauMinusMCProng

Using MC information, `labelTauPairMC` identifies if the generated event is a tau pair decay.

The variables `tauPlusMCProng` and `tauMinusMCProng` store the prong (number of final state charged particles) coming from each one of the generated tau leptons. If the event is not a tau pair decay, the value in each one of these variables will be 0.

The channel number will be stored in the variables `tauPlusMCMode`, and `tauMinusMCMode` (one for the positive and the other for the negative) according to the following table:

============  ===============================================  ============  ==================================================
MC mode       Decay channel                                    MC mode       Decay channel
============  ===============================================  ============  ==================================================
 -1           Not a tau pair event                             24            :math:`\tau^- \to \pi^- \omega \pi^0 \nu`
 1            :math:`\tau^- \to e^- \nu \bar{\nu}`             25            :math:`\tau^- \to \pi^- \pi^+ \pi^- \eta \nu`
 2            :math:`\tau^- \to \mu^- \nu \bar{\nu}`           26            :math:`\tau^- \to \pi^- \pi^0 \pi^0 \eta \nu`
 3            :math:`\tau^- \to \pi^- \nu`                     27            :math:`\tau^- \to K^- \eta \nu`
 4            :math:`\tau^- \to \rho^- \nu`                    28            :math:`\tau^- \to K^{*-} \eta \nu`
 5            :math:`\tau^- \to a_1^- \nu`                     29            :math:`\tau^- \to K^- \pi^+ \pi^- \pi^0 \nu`
 6            :math:`\tau^- \to K^- \nu`                       30            :math:`\tau^- \to K^- \pi^0 \pi^0 \pi^0 \nu`
 7            :math:`\tau^- \to K^{*-} \nu`                    31            :math:`\tau^- \to K^0 \pi^- \pi^+ \pi^- \nu`
 8            :math:`\tau^- \to \pi^- \pi^+ \pi^- \pi^0 \nu`   32            :math:`\tau^- \to \pi^- \bar{K}^0 \pi^0 \pi^0 \nu`
 9            :math:`\tau^- \to \pi^- \pi^0 \pi^0 \pi^0 \nu`   33            :math:`\tau^- \to \pi^- K^+ K^- \pi^0 \nu`
 10           :math:`\tau^- \to 2\pi^- \pi^+ 2\pi^0 \nu`       34            :math:`\tau^- \to \pi^- K^0 \bar{K}^0 \pi^0 \nu`
 11           :math:`\tau^- \to 3\pi^- 2\pi^+ \nu`             35            :math:`\tau^- \to \pi^- \omega \pi^+ \pi^- \nu`
 12           :math:`\tau^- \to 3\pi^- 2\pi^+ \pi^0 \nu`       36            :math:`\tau^- \to \pi^- \omega \pi^0 \pi^0 \nu`
 13           :math:`\tau^- \to 2\pi^- \pi^+ 3\pi^0 \nu`       37            :math:`\tau^- \to e^- e^- e^+ \nu \bar{\nu}`
 14           :math:`\tau^- \to K^- \pi^- K^+ \nu`             38            :math:`\tau^- \to f_1 \pi^- \nu`
 15           :math:`\tau^- \to K^0 \pi^- K^0bar \nu`          39            :math:`\tau^- \to K^- \omega \nu`
 16           :math:`\tau^- \to K^- K^0 \pi^0 \nu`             40            :math:`\tau^- \to K^- K^0 \pi^+ \pi^- \nu`
 17           :math:`\tau^- \to K^- \pi^0 \pi^0 \nu`           41            :math:`\tau^- \to K^- K^0 \pi^0 \pi^0 \nu`
 18           :math:`\tau^- \to K^- \pi^- \pi^+ \nu`           42            :math:`\tau^- \to \pi^- K^+ \bar{K}^0 \pi^- \nu`
 19           :math:`\tau^- \to \pi^- \bar{K}^0 \pi^0 \nu`
 20           :math:`\tau^- \to \eta \pi^- \pi^0 \nu`
 21           :math:`\tau^- \to \pi^- \pi^0 \gamma \nu`
 22           :math:`\tau^- \to K^- K^0 \nu`
 23           :math:`\tau^- \to \pi^- 4\pi^0 \nu`
============  ===============================================  ============  ==================================================

--------------
Track matching
--------------

A reconstructed track can be:

1) **matched**, the reconstructed track is matched to a true track and it is its best description.
2) **clone**, the reconstructed track is matched to a true track, but there is another reconstructed track that better describes the true track (this second reconstructed track will therefore be matched)
3) **fake**, the reconstructed track is not matched to any true track. It can be a beam-background track or a track built out of noise hits in the detector, or a mixture of these two.

.. note::
        In case of matched or clone tracks, the charge of the reconstructed track is **not checked** against the charge of the true track.
        The charge check is anyway included in the MCMatching that sets the :ref:`Error_flags`.

More details on the track matching can be found in the :ref:`trk_matching` section of the Tracking chapter, in particular :ref:`trk_matching_analysis`.
Here is a table to translate the matching status at tracking level with the one at analysis level:

=================  ===============
tracking-level     analysis-level
=================  ===============
 matched           matched
 wrongCharge       matched
 clone             clone
 cloneWrongCharge  clone
 background        fake
 ghost             fake
=================  ===============

---------------
Photon matching
---------------

To understand the method of photon matching, a basic introduction to the ECL objects used during the reconstruction of simulated data is required.

Starting with ``ECLSimHits`` from the GEANT4 simulation, ``ECLDigits`` are created and then calibrated to make ``ECLCalDigit`` objects which store the energy and
time of a single ECL crystal. The ``ECLCalDigits`` are then grouped to make ``ECLShower`` objects. The shower objects are corrected and calibrated, and used to
calculate shower-shape quantities and certain particle likelihoods (these calculations are derived using information stored in subsets of the ``ECLCalDigits`` that form the
shower). Following this, track matching is performed between reconstructed tracks and shower objects. The last step is the conversion of the ``ECLShower`` object into a mdst
``ECLCluster`` object which is the highest level ECL reconstruction object.

Each ``ECLShower`` object (and by extension each ``ECLCluster`` object) holds weighted **relations** to a maximum of twenty-one ``ECLCalDigits``, with the weights
calculated using the fraction of energy each ``ECLCalDigit`` contributes to each shower. In addition to this, the ``ECLCalDigit`` can itself have a weighted relation to none, one or many ``MCParticles``. This is calculated
using the total energy deposited by the ``MCParticle`` in each ``ECLCalDigit``. A diagram that visualises these relations is given in :numref:`photon_matching`.

.. _photon_matching:

.. figure:: figs/photon_matching.png
   :width: 45em
   :align: center

   Schematic diagram showing the weighted relations between ECL reconstruction objects and simulated particles.

The overall weight for the relation between an ``ECLCluster`` object and a ``MCParticle`` is then given by the product of the weight between the corresponding ``ECLShower`` and ``ECLCalDigit`` and the weight
between the ``ECLCalDigit`` and ``MCParticle``. For example, the weight of the relation between the first ``ECLCluster`` in :numref:`photon_matching` and MCParticle :math:`\gamma_2` is given by :math:`1.0\times 0.8=0.8` GeV.

An ``ECLCluster`` that is not matched to any track is reconstructed as a photon ``Particle``, and relations between the photon ``Particle`` and ``MCParticles`` are only set at the user-analysis level if the following conditions
are met:

1) :math:`\mathrm{weight}/{E_\mathrm{rec}} > 0.2` GeV
2) :math:`\mathrm{weight}/{E_\mathrm{true}} > 0.3` GeV

where the *weight* here refers to the relation with the largest weight. This means that if multiple relations between a given ``Particle`` and ``MCParticles`` exist, only the relation with the largest weight will be used, and the
corresponding ``MCParticle`` with this relation will be used to decide the photon matching.

A photon match is made if `mcErrors` == 0 and the ``MCParticle`` has a `mcPDG` == 22. If the chosen ``MCParticle`` does not correspond to a true photon, then the `mcErrors` :math:`\neq` 0 and no correct match will be made (even if
another one of the smaller-weighted relations for the particle is correct).

Information regarding these weights can be accessed on a user-analysis level using the following variables:

* `mcMatchWeight`
* `clusterTotalMCMatchWeight`
* `clusterBestMCMatchWeight`
* `clusterMCMatchWeight`

.. note::
        These weight variables can be used to help isolate photons that originate from **beam background processes**. Such photons typically have a very low total weight.

This information has been extracted from `this talk <https://confluence.desy.de/download/attachments/53768739/2017_12_mcmatching_ferber.pdf>`_.
More details about MC matching for photons can be found there and from the references therein.

.. _TopologyAnalysis:

.. include:: TopoAna.rst
