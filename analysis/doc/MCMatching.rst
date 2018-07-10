.. _mcmatching:

-----------
MC matching
-----------

MC matching at in Belle II returns two important pieces of information: the true PDG id of the particle (``mcPDG``), and an error flag (``mcErrors``). 
There are several extra variables defined *for your convenience*, however all information is contained in the first two.

~~~~
Core
~~~~

.. b2-variables::
        :variables: mcPDG,mcErrors

~~~~~~~~~~~
Convenience
~~~~~~~~~~~

.. b2-variables::
        :variables: isSignal,isExtendedSignal,isSignalAcceptMissingNeutrino,isWrongCharge,isMisidentified,isCloneTrack,isOrHasCloneTrack

-----------
More detais
-----------

Both variables will have non-trivial values only if the MCMatching module, which relates composite Particle(s) and MCParticle(s), is executed. mcPDG is set to the PDG code of the first common mother MCParticle of the daughters of this Particle.

More details about MCMatching for tracks is [here](https://confluence.desy.de/display/BI/Software+TrackMatchingStatus), and for photons is [here]( https://confluence.desy.de/download/attachments/53768739/2017_12_mcmatching_ferber.pdf).

.. TODO: amalgamate this information better and link to the tracking/neutrals sphinx doc when it exists.

The error flag (mcErrors) is a bit set where each bit flag describes a different kind of discrepancy between reconstruction and MCParticle. The individual flags are described by the MCMatching::MCErrorFlags enum. A value of mcErrors equal to 0 indicates perfect reconstruction (signal). Usually candidates with only FSR photons missing are also considered as signal, so you might want to ignore the corresponding c_MissFSR flag. The same is true for c_MissingResonance, which is set for any missing composite particle (e.g. K_1, but also D*0).

~~~~~~~~~~~~~~~
The error flags
~~~~~~~~~~~~~~~

=============================  ================================================================================================
Flag                           Explanation  
=============================  ================================================================================================  
 c_Correct       = 0           This Particle and all its daughters are perfectly reconstructed. 
 c_MissFSR       = 1           A Final State Radiation (FSR) photon is not reconstructed (based on MCParticle: :c_IsFSRPhoton). 
 c_MissingResonance = 2        The associated MCParticle decay contained additional non-final-state particles (e.g. a rho)
                               that weren't reconstructed. This is probably O.K. in most cases. 
 c_DecayInFlight = 4           A Particle was reconstructed from the secondary decay product of the actual particle. 
                               This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might mean
                               a pion hypothesis was used for a secondary electron. 
 c_MissNeutrino  = 8           A neutrino is missing (not reconstructed). 
 c_MissGamma     = 16          A photon (not FSR) is missing (not reconstructed). 
 c_MissMassiveParticle = 32    A generated massive FSP is missing (not reconstructed). 
 c_MissKlong     = 64          A Klong is missing (not reconstructed).  
 c_MisID = 128                 One of the charged final state particles is mis-identified (wrong signed PDG code).
 c_AddedWrongParticle = 256    A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters)
                               belongs to another Particle. 
 c_InternalError = 512         There was an error in MC matching. Not a valid match. Might indicate fake/background 
                               track or cluster. 
 c_MissPHOTOS    = 1024        A photon created by PHOTOS was not reconstructed (based on MCParticle: :c_IsPHOTOSPhoton). 
=============================  ================================================================================================

--------------
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
        
assuming you have reconstructed :code:`X -> Y Z` :

.. code-block:: python

        from modularAnalysis import applyCuts
        applyCuts('X:myCandidates', 'isSignal==1')

--------------------------------------
MC decay finder module `MCDecayFinder`
--------------------------------------

Analysis module to search for a given decay in the list of generated particles (MCParticle).
See more at `confluence page <https://confluence.desy.de/display/BI/Physics+MCDecayFinder>`_.

The module can be used for:

* Determination of the number of generated decays for efficiency studies, especially in the case of inclusive decays (e.g.: What's the generated number of B -> D^0 X decays?).
* Matched MC decays as input for a truth matching module.

~~~~~~~~~~~~~~~~~~~~~
Steering file snippet
~~~~~~~~~~~~~~~~~~~~~
 
.. code-block:: python

  from basf2 import *
  
  # Create main path
  main = create_path()
  
  # Modules to generate events, etc.
  ...
  
  mcfinder = register_module('MCDecayFinder')
  # Search for B+ decaying to anti-D0 + anything, where the anti-D0 decays to K+ pi-.
  # Ignore additional photons emitted in the anti-D0 decay. Charge conjugated decays
  # are matched, too. If there is a match found, save to ParticleList 'testB'
  mcfinder.param('strDecayString', 'B+ -> [anti-D0 => K+ pi-] ...')
  mcfinder.param('strListName', 'testB')
  main.add_module(mcfinder)
  
  # Modules which can use the matched decays saved as Particle in the ParticleList 'testB'
  ...
 

~~~~~~
Status
~~~~~~

Skipping of intermediate states in decay chain not supported yet, e.g. $B \to \pi \pi K$.

---------------
MC decay string
---------------

See more at `confluence page <https://confluence.desy.de/display/BI/Physics+MCDecayString#PhysicsMCDecayString-Status>`

Analysis module to search for a generator-level decay string for given particle.

~~~~~~
Status
~~~~~~

Prior to release-01-00-00 the MCDecayString could only be used with NtupleTools via a hashed version, and a separate output file containing the hashes and the full decay strings, by matching the hashes between the two files.  See the section below for how to include this information in pre release-01-00-00 NtupleFiles.

~~~~~~~~~~~~~~~~~~
Using decay hashes
~~~~~~~~~~~~~~~~~~

The use of decay hashes is demonstrated in :code:`B2A502-WriteOutDecayHash.py` and :code:`B2A503-ReadDecayHash.py`.

B2A502-WriteOutDecayHash.py creates one ROOT file, via variablesToNtuple containing the requested variables including the two decay hashes, and a second root file containing the two decay hashes, and the full decay string.  The decay strings can be related to the candidates that they are associated with by matching up the decay hashes.  An example of this using python is shown in B2A503-ReadDecayHash.py.

~~~~~~~~~~~~~~~~~~~~~~~~
Including the NtupleTool
~~~~~~~~~~~~~~~~~~~~~~~~

To use the MCDecayString as an NtupleTool, it is necessary to include the module ParticleMCDecayStringModule, for example:

.. code-block:: python

  analysis_main.add_module('ParticleMCDecayString', listName='D*+')

The NtupleTool can then be added, as follows:

.. code-block:: python

  toolsDST += ['MCDecayString', '^D*+']


This can be seen in the tutorial: :code:`analysis/examples/tutorials/B2A504-MCDecayStringNtupleTool.py`

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Understanding the decay string
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following is an example of a decay string:

:code:`' -413 (--> -421 (--> 321 -211) -211) |  10022 (--> 413 (--> 421 (--> -321 211) 211) 111 (--> 22 22) 111 (--> 22 22) ^-413 (--> -421 (--> 321 -211) -211)) |  10022 (--> 413 (--> 421 (--> -321 211) 211) 111 (--> 22 22) 111 (--> 22 22) -413 (--> ^-421 (--> 321 -211) -211)) |  10022 (--> 413 (--> 421 (--> -321 211) 211) 111 (--> 22 22) 111 (--> 22 22) -413 (--> -421 (--> ^321 -211) -211)) |  10022 (--> 413 (--> 421 (--> -321 211) 211) 111 (--> 22 22) 111 (--> 22 22) -413 (--> -421 (--> 321 ^-211) -211)) |  10022 (--> 413 (--> 421 (--> -321 211) 211) 111 (--> 22 22) 111 (--> 22 22) -413 (--> -421 (--> 321 -211) ^-211))'`

The string consists of several parts, separated by pipes :code:`|`.

In each of the strings particles are identified via their PDG number; see for example: http://pdg.lbl.gov/2017/reviews/rpp2016-rev-monte-carlo-numbering.pdf

The first part is the desired decay that is being searched for.

This is followed by a number of strings equal to the number of particles in the desired decay (five in the example above: the D* (-413), the D (-421), the kaon (321), the first pion (-211), and the second pion (-211)).  For each of these particles the full string of the actual MC decay is given if the particle has a match, or "(No match)" if the particle does not have a match.  For example, the first particle is a D*(-413), and the associated string shows it matching with a D*(-413) indicated by a caret, ^, placed before the matched particle in the string.  In the string above all particles are corrected matched.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Using the decay string with ROOT
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The decay string is stored as a :code:`std: :string` in the Ntuple tools; these are handled well by all recent versions of ROOT (including the version included in externals of basf2), but there may be some issues reading this if you are using a really old version of ROOT.

The string will even plot directly onto a TCanvas if you click on the :code:`c_str()` function of the string, though this is unlikely to be very useful unless you have only a few events and a customised axis layout on the canvas.

The strings can be drawn to the terminal, subject to any cuts you with to apply to, for example, help out with identifying the source of events that pass a particular set of cuts via:

.. code-block:: bash

  root [3] dsttree->Scan("DST_mcDecayString", "iCand==0 && evt_no == 42", "colsize=300")

  # or

  root [4] Bplus->Scan("B_mcDecayString", "B_mbc > 5.26 && abs(B_deltae) < 0.05", "colsize=300")

It is necessary to specify the colsize variable in order to see the full string (if omitted only the first 8 characters are displayed), and the value should be set appropriately to see the full string for your decay.

~~~~~~~~~~~~~~
Concise format
~~~~~~~~~~~~~~

The decay string format is rather long, and it is possible to use a shorter format, by passing the option :code:`conciseString` to the module as follows:

.. code-block:: python

  analysis_main.add_module('ParticleMCDecayString', listName='D*+', conciseString = True)

The concise string has the following format:

:code:`521 (--> 310 211 111 (--> 22 22)) | 300553 (--> a521 (--> b310 c211 d111 (--> e22 f22)) -521 (--> 421 (--> 223 (--> -211 211 111 (--> 22 22)) 130) -213 (--> -211 111 (--> 22 22)) -311 (--> 310) 321 -211))`

In this example each of the six particles in the decay that is searched for are given an identifier (by default the minuscule Roman alphabet / Romaji, i.e. "a", "b", "c", etc, incrementing alphabetically).  There is only one string giving the actual MC decay, and it contains the identifiers with the particle to which they are matched.

Multiple identifiers could match up to a single particle, commonly this might be an Y(4S) or a virtual photon:

:code:`521 (--> 310 211 111 (--> 22 22)) |  ab300553 (--> 521 (--> 310 c211 111 (--> 22 22)) -521 (--> 413 (--> 421 (--> 310 310 211 -211) 211) 313 (--> 311 (--> 310) 111 (--> 22 22)) -321 -213 (--> -211 d111 (--> f22 e22))))`

It there were unmatched particles it would look something like this:

:code:`521 (--> 310 211 111 (--> 22 22)) | 300553 (--> 521 (--> b310 211 111 (--> 22 f22)) -521 (--> 421 (--> 223 (--> -211 c211 111 (--> 22 22)) 130) -213 (--> -211 111 (--> 22 22)) -311 (--> 310) 321 -211)) | No match: ade`


If it is not possible to convert the string to the concise format then the standard string format is returned instead. 

This will happen for instance if your decay has more than particles than identifiers (26 by default).  It is possible to alter the list of identifiers or add more by setting the option "identifiers", which has a default of :code:`std::string("abcdefghijklmnopqrstuvwxyz")`.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Pre release-01-00-00 inclusion in NtupleTools
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To run ParticleMCDecayString and include information in the NtupleFile created from NtupleTools it is possible to do the following:

.. code-block:: python

  analysis_main.add_module('ParticleMCDecayString', listName='my_particle_list', fileName='my_hashmap.root')

This will produce a file with all of the decay strings in it, along with the decayHash (hashes the MC decay string of the mother particle) and decayHashExtended (hashes the decay string of the mother and daughter particles).  The mapping of hashes to full MC decay strings is stored in a ROOT file determined by the fileName parameter.

Then the decayHash and decayHashExtended can be included in NtupleTools by including them as extrainfo as a custom float:

.. code-block:: python

  tools += ['CustomFloats[extraInfo(DecayHash)', my_decay]
  tools += ['CustomFloats[extraInfo(DecayHashExtended)', my_decay]

or (recommended) via an alias:

.. code-block:: python

  from variables import variables
  variables.addAlias('decayHash', 'extraInfo(DecayHash)')
  variables.addAlias('decayHashExtended', 'extraInfo(DecayHashExtended)')
  ...
  tools += ['CustomFloats[decayHash:decayHashExtended]', my_decay]

The analyst can then compare the hashes in the nTupleFile with the hashes in the root file produced by the ParticleMCDecayString module to retrieve the decay strings.

----------------
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
