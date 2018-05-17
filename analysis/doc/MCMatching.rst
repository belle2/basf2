.. _mcmatching:

MC matching
===========

MC matching at in Belle II returns two important pieces of information: the true PDG id of the particle (``mcPDG``), and an error flag (``mcErrors``). 
There are several extra variables defined *for your convenience*, however all information is contained in the first two.

Core
~~~~

.. b2-variables::
        :variables: mcPDG,mcErrors

Convenience
~~~~~~~~~~~

.. b2-variables::
        :variables: isSignal,isSignalExtended,isSignalAcceptMissingNeutrino,isWrongCharge,isMisidentified

More detais
-----------

Both variables will have non-trivial values only if the MCMatching module, which relates composite Particle(s) and MCParticle(s), is executed. mcPDG is set to the PDG code of the first common mother MCParticle of the daughters of this Particle.

More details about MCMatching for tracks is [here](https://confluence.desy.de/display/BI/Software+TrackMatchingStatus), and for photons is [here]( https://confluence.desy.de/download/attachments/53768739/2017_12_mcmatching_ferber.pdf).

.. TODO: amalgamate this information better and link to the tracking/neutrals sphinx doc when it exists.

The error flag (mcErrors) is a bit set where each bit flag describes a different kind of discrepancy between reconstruction and MCParticle. The individual flags are described by the MCMatching::MCErrorFlags enum. A value of mcErrors equal to 0 indicates perfect reconstruction (signal). Usually candidates with only FSR photons missing are also considered as signal, so you might want to ignore the corresponding c_MissFSR flag. The same is true for c_MissingResonance, which is set for any missing composite particle (e.g. K_1, but also D*0).

The error flags
~~~~~~~~~~~~~~~

=============================  ===========   
Flag                           Explanation  
=============================  ===========   
 c_Correct       = 0           This Particle and all its daughters are perfectly reconstructed. 
 c_MissFSR       = 1           A Final State Radiation (FSR) photon is not reconstructed (based on MCParticle::c_IsFSRPhoton). 
 c_MissingResonance = 2        The associated MCParticle decay contained additional non-final-state particles (e.g. a rho) that weren't reconstructed. This is probably O.K. in most cases. 
 c_DecayInFlight = 4           A Particle was reconstructed from the secondary decay product of the actual particle. This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might mean a pion hypothesis was used for a secondary electron. 
 c_MissNeutrino  = 8           A neutrino is missing (not reconstructed). 
 c_MissGamma     = 16          A photon (not FSR) is missing (not reconstructed). 
 c_MissMassiveParticle = 32    A generated massive FSP is missing (not reconstructed). 
 c_MissKlong     = 64          A Klong is missing (not reconstructed).  
 c_MisID = 128                 One of the charged final state particles is mis-identified (wrong signed PDG code).
 c_AddedWrongParticle = 256    A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters) belongs to another Particle. 
 c_InternalError = 512         There was an error in MC matching. Not a valid match. Might indicate fake/background track or cluster. 
 c_MissPHOTOS    = 1024        A photon created by PHOTOS was not reconstructed (based on MCParticle::c_IsPHOTOSPhoton). 
=============================  ============   


Example of use
--------------

The two variables together allow the user not only to distinguish signal (correctly reconstructed) and background (incorrectly reconstructed) candidates, but also to study and identify various types of physics background (e.g. mis-ID, partly reconstructed decays, ...). To select candidates that have a certain flag set, you can use bitwise and to select only this flag from mcErrors and check if this value is non-zero: ``(mcErrors & MCMatching::c_MisID) != 0``.
For use in a TTree selector, you'll need to use the integer value of the flag instead:

.. code-block:: cpp

        ntuple->Draw("M", "(mcErrors & 128) != 0")

You can also make use of ``MCMatching::explainFlags()``` which prints a human-readable list of flags present in a given bitset. Can also be used in both C++ and python:

.. code-block:: python

        import basf2
        from ROOT import Belle2, gInterpreter
        gInterpreter.ProcessLine('#include "analysis/utility/MCMatching.h"')
        print(Belle2.MCMatching.explainFlags(a_weird_mcError_number)) 


If instead only binary decision (1 = signal, 0 = background) is needed, then it for convenience one can use ``isSignal`` (or ``isSignalAcceptMissingNeutrino`` for semileptonic decays).

.. code-block:: python

        ntupleTools = ['CustomFloats[isSignal]', '^X -> ^Y Z']
        
assuming you have reconstructed `X -> Y Z` :

.. code-block:: python

        from modularAnalysis import applyCuts
        applyCuts('X:myCandidates', 'isSignal==1')

Tau decay McMode
----------------

An special case is the tau decay McModes. They were designed to study generated tau pair events.
Consist of two variables ``tauPlusMcMode``, and ``tauMinusMcMode``. To use them, is required to call first ``labelTauDecays`` in the steering file.

.. code-block:: python

        from modularAnalysis import labelTauDecays
        labelTauDecays()


.. b2-variables::
        :variables: tauPlusMcMode,tauMinusMcMode

        
The variables store an integer MC mode, which corresponds to one decay channel of the tau lepton (one for the positive and the other for the negative).

============  ==============================  ============  ==============================
MC mode       Decay channel                   MC mode       Decay channel
============  ==============================  ============  ==============================
 -1           Not a tau pair event             24           tau- -> pi- omega pi0 nu
 1            tau- -> e- nu anti_nu            25           tau- -> pi- pi+ pi- eta nu
 2            tau- -> mu- nu anti_nu           26           tau- -> pi- pi0 pi0 eta nu
 3            tau- -> pi- nu                   27           tau- -> K- eta nu
 4            tau- -> rho- nu                  28           tau- -> K*- eta nu
 5            tau- -> a1- nu                   29           tau- -> K- pi+ pi- pi0 nu
 6            tau- -> K- nu                    30           tau- -> K- pi0 pi0 pi0 nu
 7            tau- -> K*- nu                   31           tau- -> K0 pi- pi+ pi- nu
 8            tau- -> pi- pi+ pi- pi0 nu       32           tau- -> pi- K0bar pi0 pi0 nu
 9            tau- -> pi- pi0 pi0 pi0 nu       33           tau- -> pi- K+ K- pi0 nu
 10           tau- -> 2pi- pi+ 2pi0 nu         34           tau- -> pi- K0 K0bar pi0 nu
 11           tau- -> 3pi- 2pi+ nu             35           tau- -> pi- omega pi+ pi- nu
 12           tau- -> 3pi- 2pi+ pi0 nu         36           tau- -> pi- omega pi0 pi0 nu
 13           tau- -> 2pi- pi+ 3pi0 nu         37           tau- -> e- e- e+ nu anti_nu
 14           tau- -> K- pi- K+ nu             38           tau- -> f1 pi- nu
 15           tau- -> K0 pi- K0bar nu          39           tau- -> K- omega nu
 16           tau- -> K- K0 pi0 nu             40           tau- -> K- K0 pi+ pi- nu
 17           tau- -> K- pi0 pi0 nu            41           tau- -> K- K0 pi0 pi0 nu
 18           tau- -> K- pi- pi+ nu            42           tau- -> pi- K+ K0bar pi- nu
 19           tau- -> pi- K0bar pi0 nu
 20           tau- -> eta pi- pi0 nu
 21           tau- -> pi- pi0 gamma nu
 22           tau- -> K- K0 nu
 23           tau- -> pi- 4pi0 nu
============  ==============================  ============  ==============================