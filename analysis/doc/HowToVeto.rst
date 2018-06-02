How to veto
===========

Introduction
------------

In many physics analysis one of the (final state) particles originates from a specific decay and is not part of the signal decay chain. Such candidates of course contribute to the background and need to rejected as effectively as possible. Few examples:

* in studies of radiative B meson or charm decays, such as B0 -> rho0 gamma, the photon candidate, which is used in the reconstruction of the B0 meson, most often originates in reality from the decay of a pi0 or eta mesons.
* in studies of semileptonic B meson or charm decays the lepton sometimes originates from the decay of charmonium, J/psi for example.
* ...

In cases like these, we usually construct a veto in order to suppress the background sources. By veto we usually mean, that we

#. calculate or construct some sort of a probability or likelihood that particle :math:`X` (used in the reconstruction of the decay of interest, aka signal side) in reality originates from some other decay, :math:`M \to X Y`.
#. if the calculated probability is larger then some threshold value then we reject X and with it the signal candidate. We veto :math:`M`.

Overview of the workflow
------------------------

Any veto (:math:`\pi^0,\ \eta,\ J/\psi,\ ...`) can be fully constructed and configured within the analysis python steering file. This section overviews steps that are necessary in the construction of each veto. Next subsection shows how each of the mentioned steps are practically implemented.

The analysis of some specific signal side decay with veto has the following steps (here it is assumed for the sake of clarity the pi0 veto is constructed):

#. Reconstruct signal decay candidates
    * this includes filling of FS particle lists, making combinations, performing vertex fits, applying cuts, etc...
#. Create RestOfEvent (ROE) object for each signal candidate
#. Create new path to be executed for each ROE object in the event
    #. before doing anything check if the current ROE object is related to a Particle from our signal ParticleList. If it is not then there's no point of doing anything, so we can finish the execution of current iteration of for_each ROE path.
    #. fill ParticleList with all (or selected) photons from ROE
    #. fill ParticleList with signal photon candidate which is used in the reconstruction of the current signal side candidate (related to current ROE)
    #. combine the two lists to form pi0 candidates
    #. use TMVA or any other method to determine the probability or likelihood that signal photon originates from :math:`\pi^0` decay
    #. select the best :math:`\pi^0` candidate
    #. write the probability or likelihood value calculated in step iv) for the best pi0 candidate as extraInfo to the current :math:`B^0` candidate
#. Continue with signal side reconstruction
    * continuum suppression, flavour tagging, tag vertex, etc...
#. Fill flat ntuple for offline analysis

All analysis actions coloured in blue are executed once per event and all coloured in green are executed once per each ROE object in the event. The analysis actions coloured with green together construct a veto.


Examples
--------

:math:`\pi^0` veto in :math:`B^0 \to \rho^0 \gamma` decays
The code is taken from an existing tutorial: :code:`B2A306-B02RhoGamma-withPi0Veto.py`

1. The B0 -> rho0 gamma candidates are reconstructed and collected in the ParticleList with name B0.
2. Create ROE objects

.. code-block:: python

    buildRestOfEvent('B0')   
 
3. Create roe_path in which the veto will be constructed. In addition another dead-end path needs to be created, which will be used in step o)

.. code-block:: python

    roe_path = create_path() 
    deadEndPath = create_path()    

In next steps the veto is constructed. In this example the veto works in the following way:

* combine photon (gamma) used in the reconstruction of the B0 candidate with all other photons found in the event with energy above 50 MeV to form :math:`\pi^0` candidates
* find best pi0 candidate with invariant mass closest to :math:`\pi^0`'s nominal mass
* write value of invariant mass of the best :math:`\pi^0` as 'pi0veto' extraInfo

o. check if ROE is related to any Particle from :code:`B0` list

.. code-block:: python

    signalSideParticleFilter('B0', '', roe_path, deadEndPath)  
 

i. fill ParticleList with all photons that have 'E>0.050' from ROE (using isInRestOfEvent variable)

.. code-block:: python

    fillParticleList('gamma:roe', 'isInRestOfEvent == 1 and E > 0.050', path=roe_path)
 

ii. fill ParticleList with signal photon candidate which is used in the reconstruction of the current signal side candidate (related to current ROE)

.. code-block:: python

    fillSignalSideParticleList('gamma:sig', 'B0 -> rho0 ^gamma', roe_path)
 

iii. combine the two lists to form pi0 candidates

.. code-block:: python

    reconstructDecay('pi0:veto -> gamma:sig gamma:roe', '0.080 < M < 0.200', path=roe_path)
 

v. select the best pi0 candidate

.. code-block:: python

    rankByLowest('pi0:veto', 'abs(dM)', 1, path=roe_path)
 

vi. write the probability or likelihood value calculated in step iv) for the best pi0 candidate as extraInfo to the current B0 candidate

.. code-block:: python

    variableToSignalSideExtraInfo('pi0:veto', {'M': 'pi0veto'}, path=roe_path)
 

vii. Connect the roe_path with the main path

.. code-block:: python

    analysis_main.for_each('RestOfEvent', 'RestOfEvents', roe_path)
 

4. Continue with signal side reconstruction. At this point the B0 candidates have extraInfo(pi0veto) attached.

.. code-block:: python

    printVariableValues('B0', ['pi0veto'])
 

If the signal photon candidate could not be paired with any other photon candidate from ROE to form a :math:`\pi^0` candidate, then extraInfo(pi0veto)=-999.

