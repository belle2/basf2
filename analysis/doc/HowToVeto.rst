.. _HowToVeto:

How to Veto
===========

Introduction
------------

In many physics analyses one of the (final state) particles originates from a specific decay and is not part of the signal decay chain. 
Such candidates contribute to the background and need to rejected as effectively as possible. 
Examples:

* in studies of radiative B meson or charm decays, such as :math:`B^0\to\rho^0\gamma`, the photon candidate, 
  which is used in the reconstruction of the :math:`B^0` meson may, in reality, originate from the decay of either 
  :math:`\pi^0\to\gamma\gamma` or :math:`\eta\to\gamma\gamma`.

* in studies of semileptonic B meson or charm decays the lepton sometimes originates from the decay of charmonium, :math:`J/\psi` for example.

To deal with these backgrounds, we construct a veto in order to suppress the background sources. 
Vetoing comes in two steps:

#. calculate or construct some sort of a probability or likelihood that particle :math:`X` (used in the reconstruction of the
   signal decay of interest) in reality originates from some other decay, :math:`M \to X Y`.
#. if the calculated probability is larger then some threshold value then we reject X and with it the signal candidate. We veto :math:`M`.

Any veto (:math:`\pi^0,\ \eta,\ J/\psi,\ ...`) can be fully constructed and configured within the python steering file. 

Overview of the workflow
------------------------

The analysis of some specific signal side decay with a veto has the following steps.
Here we use the worked example of a veto for background photons from a :math:`\pi^0\to\gamma\gamma` decay but the procedure generalises.

#. Reconstruct signal decay candidates
    * this includes filling of final-state particle lists, making combinations, performing vertex fits, applying cuts, etc...
#. Create a :doc:`RestOfEvent` object for each signal candidate
#. Create new `basf2.Path` to be executed for each :doc:`RestOfEvent` object in the event (one for each signal candidate)

   .. warning:: 
        It is important to check if the current :doc:`RestOfEvent` object is related to a Particle from our signal ParticleList. 
        This is a subtlety, but you may find unrelated :doc:`RestOfEvent` objects in the DataStore (from some other decay mode).
        In this case we simply skip the :doc:`RestOfEvent` by executing a *so called* "dead end" path.
        
     
#. Fill a `ParticleList`_ with all (or selected) photons from the :doc:`RestOfEvent`
#. Fill a `ParticleList`_ with signal photon candidates which is used in the reconstruction of the current signal side candidate (related to current :doc:`RestOfEvent`)
#. Combine the two lists to form pi0 candidates
#. Use TMVA or any other method to determine the probability or likelihood that signal photon originates from :math:`\pi^0` decay
#. Select the best :math:`\pi^0` candidate
#. Write the probability (or likelihood value) calculated in step iv for the best :math:`\pi^0` candidate as extraInfo to the current :math:`B^0` candidate
#. Continue with signal side reconstruction
    * continuum suppression, flavour tagging, tag vertex, etc...
#. Fill flat `ntuple for offline analysis <v2nt>`.

All analysis actions coloured in blue are executed once per event and all coloured in green are executed once per each :doc:`RestOfEvent` object in the event. The analysis actions coloured with green together construct a veto.

.. seealso:: This is an important usecase of the `basf2.Path.for_each` functionality.

.. _ParticleList: https://software.belle2.org/development/classBelle2_1_1ParticleList.html

Examples
--------

:math:`\pi^0` veto in :math:`B^0 \to \rho^0 \gamma` decays
The code is taken from an existing tutorial: :code:`B2A306-B02RhoGamma-withPi0Veto.py`

1. The B0 -> rho0 gamma candidates are reconstructed and collected in the ParticleList with name B0.
2. Create :doc:`RestOfEvent` objects

.. code-block:: python

    import basf2
    from modularAnalysis import buildRestOfEvent
   
    mymainpath = basf2.Path()
    buildRestOfEvent('B0', path=mymainpath)   
 
3. Create roe_path in which the veto will be constructed. In addition another dead-end path needs to be created, which will be used in step o)

.. code-block:: python

    roe_path = basf2.Path() 
    deadEndPath = basf2.Path()    

In next steps the veto is constructed. In this example the veto works in the following way:

* combine photon (gamma) used in the reconstruction of the B0 candidate with all other photons found in the event with energy above 50 MeV to form :math:`\pi^0` candidates
* find best pi0 candidate with invariant mass closest to :math:`\pi^0`'s nominal mass
* write value of invariant mass of the best :math:`\pi^0` as 'pi0veto' extraInfo

o. check if :doc:`RestOfEvent` is related to any Particle from :code:`B0` list

.. code-block:: python

    signalSideParticleFilter('B0', '', roe_path, deadEndPath)  
 

4. fill ParticleList with all photons that have 'E>0.050' from :doc:`RestOfEvent` (using isInRestOfEvent variable)

.. code-block:: python

    fillParticleList('gamma:roe', 'isInRestOfEvent == 1 and E > 0.050', path=roe_path)
 

5. fill ParticleList with signal photon candidate which is used in the reconstruction of the current signal side candidate (related to current :doc:`RestOfEvent`)

.. code-block:: python

    fillSignalSideParticleList('gamma:sig', 'B0 -> rho0 ^gamma', roe_path)
 

6. combine the two lists to form :math:`\pi^0\to\gamma\gamma` candidates

.. code-block:: python

    reconstructDecay('pi0:veto -> gamma:sig gamma:roe', '0.080 < M < 0.200', path=roe_path)
 

7. select the best :math:`\pi^0` candidate

.. code-block:: python

    rankByLowest('pi0:veto', 'abs(dM)', 1, path=roe_path)
 

8. write the probability or likelihood value calculated in step iv) for the best pi0 candidate as extraInfo to the current B0 candidate

.. code-block:: python

    variableToSignalSideExtraInfo('pi0:veto', {'M': 'pi0veto'}, path=roe_path)
 

9. Connect the roe_path with the main path

.. code-block:: python

    mymainpath.for_each('RestOfEvent', 'RestOfEvents', roe_path)
 

10. Continue with signal side reconstruction. At this point the B0 candidates have extraInfo(pi0veto) attached.

.. code-block:: python

    printVariableValues('B0', ['pi0veto'], path=mymainpath)

If the signal photon candidate could not be paired with any other photon candidate from the :doc:`RestOfEvent` to form a :math:`\pi^0` candidate, then ``extraInfo(pi0veto) = NaN``.

