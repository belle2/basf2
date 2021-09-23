.. _restOfEvent:

Rest Of Event
=============

Heavy quarks, like :math:`b` and :math:`c` quarks, are produced in pairs in electron-positron collisions
and, typically, they form two heavy hadrons as an intermediate-state particles. 
To study these kinds of events we have to reconstruct at least one hadron decay and 
look for another hadron in the rest of the event. Such use case is one of the possible 
application of the Rest Of Event (ROE) modules in the basf2 analysis framework.

.. tip:: For an introductory hands-on lesson, take a look at :numref:`onlinebook_roe`.

Overview
--------
The Rest Of Event is built around a reconstructed target particle and it is 
a collection of all other particles in the event. 
The ROE can be used in multiple cases. For example, two of the most common ones
are in **tagged** and **untagged** analyses of :math:`Y(4S)` decays:

  - In tagged analyses, where one reconstructs both *B* mesons, the ROE object
    should be empty in a perfect situation. However, it often contains particles
    from either the collision event or the background. Cleaning up
    the ROE in this case can help improve variables such as :math:`E_{extra}`, which
    should be close to 0.
  - In untagged analyses one reconstruct only the signal *B* meson, everything
    else in the event is considered to come from the companion *B* meson. In
    addition to tracks and clusters from the companion *B* meson, the ROE contains
    also the extra tracks and clusters, as in the case of the tagged analysis.
    Cleaning up the ROE in this case can help improve variables such as :math:`M_{bc}` or
    :math:`\Delta E`.

It is therefore a generally bad idea to take everything into account when dealing with the ROE.

Basic usage
-----------

The ROE is formed by calling :func:`modularAnalysis.buildRestOfEvent` command:

.. code-block:: python

  import basf2 as b2
  import modularAnalysis as ma
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # Build the ROE object:
  ma.buildRestOfEvent('B0:rec', path = main_path)
   
This code simply adds ``RestOfEvent`` objects, which are related to each ``B0:rec`` candidate.
Now, it is possible to calculate simple ROE-based variables using target particle candidate,
like the ROE momentum :b2:var:`roeP`, the ROE mass :b2:var:`roeM` or others. The full list of the ROE variables can be found
by using command ``b2help-variables`` under the **Rest of Event** section.

.. warning:: 
  Names or behavior of the ROE variables may vary from release to release. 
  Please recheck list of variables ``b2help-variables`` when switching between the releases.

After reconstructing the ROE, one can use ROE-dependent modules, like :doc:`FlavorTagger`, `ContinuumSuppression`, :doc:`FullEventInterpretation` and other algorithms.

ROE particle type hypothesis
----------------------------


By default, the ROE object is filled by pions, photons and :math:`K_L^0`'s.
This can be changed by passing an additional argument to the builder method:


.. code-block:: python

  import modularAnalysis as ma
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # Build the ROE object:
  ma.buildRestOfEvent('B0:rec', fillWithMostLikely=True, path = main_path)

This will automatically provide to the ROE the charged particles, which have the most probable mass hypothesis, according to their PID information.
Also, the neutral particles, photons and :math:`K_L^0`'s will be supplied to the ROE.

.. hint ::
  This option is also available for the Event Shape and the Event Kinematics computation.

Charged PID priors
------------------

User can provide prior expectations for the most probable mass hypothesis mentioned above. This is useful to suppress harmful mis-IDs, like charged pion 
being reconstructed as a muon because of similarity of their PID likelihoods. 
The priors are provided in the form of a list containing 6 float numbers, which correspond to 
:math:`[ e^\mp, \mu^\mp, \pi^\mp K^\pm, p^\pm, d^\pm]`.

Here is an example of the prior usage:

.. code-block:: python

  import modularAnalysis as ma
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # Build the ROE object:
  ma.buildRestOfEvent('B0:rec', fillWithMostLikely=True, 
        chargedPIDPriors=[0.0, 0.0, 1.0, 1.0, 0.0, 0.0], path = main_path)

In this example, only kaons and pions will enter ROE. The same functionality is enabled for Event Shape and the Event Kinematics algorithms. 

.. note::
  An additional study is needed to fully understand the prior behavior.
  Please share your experience.

Selection cut based method
--------------------------

Nevertheless, there is an option to add particle lists manually:

.. code-block:: python

  import basf2 as b2
  import modularAnalysis as ma
  import stdCharged
  stdCharged.stdE('good')
  stdCharged.stdK('good')
  stdCharged.stdMu('good')
  stdCharged.stdPr('good')
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # Build the ROE object:
  ma.buildRestOfEvent('B0:rec',['K+:good', 'p+:good','e+:good','mu+:good'], path = main_path)

Particles from these lists will be added to the ROE first, and then the ROE object will be completed 
by adding the default particle types, pions, photons and :math:`K_L^0`'s. 
Therefore, it is not necessary to provide here a pion particle list with any PID cuts.
The order of provided particle lists matters, so preferably, the particle lists which contain rare particles should enter the ROE first.

Changing charged particle hypothesis is important, as the charged particles have different mass, and it will 
affect the computation of the ROE mass or energy.
Also, this method allows to study the ROE particle composition by using :b2:var:`nROE_Charged` metavariable, 
as well as :b2:var:`nROE_Photons` and :b2:var:`nROE_NeutralHadrons` metavariables, see output of ``b2help-variables`` command.


Accessing ROE particles
-----------------------

Often, we need to reconstruct the companion hadron in the event or just access the ROE particles.
In basf2 this is realized by ``path.for_each`` functionality:

.. code-block:: python

  import basf2 as b2
  import modularAnalysis as ma
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # build the ROE object
  ma.buildRestOfEvent('B0:rec', path = main_path)
  # Create a path for ROE logic
  roe_path = b2.create_path()
  # Associate a module to be executed for each ROE candidate:
  ma.fillParticleList('gamma:roe', 'isInRestOfEvent == 1', path = roe_path)
  # Execute loop for each ROE:
  main_path.for_each('RestOfEvent', 'RestOfEvents', path = roe_path)

In this example we create another path ``roe_path``, which is used to loop over the created ROE objects.
By calling ``modularAnalysis`` methods with ``path = roe_path`` we create basf2 modules, which will be executed for each ROE candidate.
Here we fill ``gamma:roe`` particle list with ROE photons by using 
a cut ``isInRestOfEvent == 1``. One can proceed to fill other types ROE particles, 
like pions, kaons etc, in the same way. 
It is possible to execute other modules in the ROE loop, like :func:`modularAnalysis.reconstructDecay` for example,
however, it is **important** to use a proper ROE path when calling any basf2 functionality in the ROE loop.
One can also move information calculated in the ROE to the signal side.
The most common use-case is probably a veto as described in detail in :ref:`HowToVeto` which uses

.. code-block:: python

    ma.variableToSignalSideExtraInfo('pi0:veto', {'M': 'pi0veto'}, path=roe_path)

to add an ExtraInfo to the signal particle that defines the ROE. Via the
module `SignalSideVariablesToDaughterExtraInfo` this functionality is
extended to any particle on the signal side, for example:

.. code-block:: python

  roe_path.add_module('SignalSideVariablesToDaughterExtraInfo', 
        particleListName='D0:all', variablesToExtraInfo={'x' : 'VtxX'})

If your script creates ROE objects from multiple signal side particle lists, 
one can use `SignalSideParticleFilter` module in the ROE loop to iterate only 
over ROE objects related to a certain particle list:

.. code-block:: python
  
  import basf2 as b2
  import modularAnalysis as ma
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # build the ROE object
  ma.buildRestOfEvent('B0:rec', path = main_path)
  # Create a path for ROE logic
  roe_path = b2.create_path()
  # Create an empty path to interrupt the execution in case of a different particle list
  deadEndPath = b2.create_path()
  # Execute the filter module:
  ma.signalSideParticleFilter('B0:rec', '', roe_path, deadEndPath)
  # Associate a module to be executed for each ROE candidate:
  ma.fillParticleList('gamma:roe', 'isInRestOfEvent == 1', path = roe_path)
  # Execute loop for each ROE:
  main_path.for_each('RestOfEvent', 'RestOfEvents', path = roe_path)


ROE masks
---------

The ROE object contains **every** particle in the event, which has not been associated to the target particle candidate. 
Therefore, a clean up procedure is **necessary** to filter out beam-induced energy depositions from the ROE.
In basf2 framework it is done using a concept of ROE mask. The ROE mask is a simple container of particles, 
which have passed a selection criteria:

.. code-block:: python

  import basf2 as b2
  import modularAnalysis as ma
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # Build the ROE object:
  ma.buildRestOfEvent('B0:rec', path = main_path)
  # Create a mask tuple:
  cleanMask = ('cleanMask', 'abs(d0) < 10.0 and abs(z0) < 20.0',
                            'E > 0.06 and abs(clusterTiming) < 20')
  # append masks to existing ROE object
  ma.appendROEMasks('B0:rec', [cleanMask], path = main_path)
  
The mask tuples should contain a mask name and cuts for charged particles, for photons and for :math:`K_L^0` or hadrons.
In the example above a cut is not set, therefore, all hadrons will pass the mask.

.. warning::
  Mask name ``all`` is reserved for no ROE mask case, users cannot create a ROE mask wth this name. 
  The mask name ``all`` can be provided to the ROE metavariables and ROE-dependent modules
  right after creation of the ROE to use all particles in the ROE with no selection cuts, although it is not advised. 
  Moreover, mask names can only contain alphanumeric or underscore characters.

Most of ROE variables accept mask name as an argument, which allows user to compare 
variable distributions from different ROE masks. 
For example, the :b2:var:`roeE` variable will be computed using only ROE particles from a corresponding mask.

.. note::
  Hard cuts on track impact parameters :math:`d_0` and :math:`z_0` are not recommended since one can throw away tracks from long lived decaying
  particles.

After appending masks to ROE, one can perform different manipulations with masked particles.
The methods :func:`modularAnalysis.keepInROEMasks` and :func:`modularAnalysis.discardFromROEMasks` 
allow to keep or discard particles from an ROE mask if they are present in an input particle list. 
Also, one can replace masked particles by their parent :math:`V_0` objects, by using :func:`modularAnalysis.optimizeROEWithV0`.
These methods should be executed inside the ROE loop:

.. code-block:: python

  import basf2 as b2
  import modularAnalysis as ma
  import vertex as vtx
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # build the ROE object
  ma.buildRestOfEvent('B0:rec', path = main_path)
  # Create a mask tuple:
  cleanMask = ('cleanMask', 'abs(d0) < 10.0 and abs(z0) < 20.0',
                            'E > 0.06 and abs(clusterTiming) < 20')
  # append masks to the existing ROE object
  ma.appendROEMasks('B0:rec', [cleanMask], path = main_path)
  
  # Create a path for ROE logic
  roe_path = b2.create_path()
  
  # Filling example ROE photons:
  ma.fillParticleList('gamma:bad', 'isInRestOfEvent == 1 and E < 0.05', path = roe_path)
  ma.fillParticleList('gamma:good', 'isInRestOfEvent == 1 and E > 0.1', path = roe_path)
  # Discard or keep ROE photons, other types of particles are not modified:
  ma.discardFromROEMasks('gamma:bad',['cleanMask'],'', path = roe_path)
  ma.keepInROEMasks('gamma:good',['cleanMask'],'',path = roe_path)
  # Fill ROE pion particle list:
  ma.fillParticleList('pi+:roe', 'isInRestOfEvent == 1', path = roe_path)
  # Reconstruct a K_S0 candidate using ROE pions:
  ma.reconstructDecay('K_S0:roe -> pi+:roe pi-:roe', '0.45 < M < 0.55', path = roe_path)
  # Perform vertex fitting:
  vtx.kFit('K_S0:roe',0.001, path=roe_path)
  # Insert a K_S0 candidate into the ROE mask:
  ma.optimizeROEWithV0('K_S0:roe',['cleanMask'],'', path=roe_path)
  # Execute loop for each ROE:
  main_path.for_each('RestOfEvent', 'RestOfEvents', path = roe_path)

These advanced ROE methods can be used for further clean up from beam-induced pollution and for applications of MVA training.

Nested ROE
----------

To analyze some decay channels, particularly in charm physics, it is necessary to reconstruct a nested ROE object around a target particle and using
particles from host ROE object:

.. code-block:: python

  import basf2 as b2
  import modularAnalysis as ma
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # build the ROE object
  ma.buildRestOfEvent('B0:rec', path = main_path)
  # Create a mask tuple:
  cleanMask = ('cleanMask', 'abs(d0) < 10.0 and abs(z0) < 20.0',
                            'E > 0.06 and abs(clusterTiming) < 20')
  # append masks to existing ROE object
  ma.appendROEMasks('B0:rec', [cleanMask], path = main_path)
  # Create a path for ROE logic
  roe_path = b2.create_path()
  # Associate a module to be executed for each ROE candidate:
  ma.fillParticleList('gamma:roe', 'isInRestOfEvent == 1', path = roe_path)
  # reconstructing an energetic pi0 inside host ROE:
  ma.reconstructDecay('pi0:roe -> gamma:roe gamma:roe', 'p > 0.5', path = roe_path)
  # build a nested ROE using a mask
  ma.buildNestedRestOfEvent('pi0:roe', maskName = 'cleanMask', path = roe_path)
  nestedroe_path = b2.create_path()
  # fill a pion list in nested ROE, please notice the change of path
  ma.fillParticleList('pi+:nestedroe', 'isInRestOfEvent == 1', path = nestedroe_path)
  # reconstructing a K_S0 inside nested ROE:
  ma.reconstructDecay('K_S0:nestedroe -> pi+:nestedroe pi-:nestedroe', 'p > 0.5',
                      path = nestedroe_path)
  # Execute loop for each nested ROE:
  roe_path.for_each('RestOfEvent', 'NestedRestOfEvents', path = nestedroe_path)
  # Execute loop for each host ROE:
  main_path.for_each('RestOfEvent', 'RestOfEvents', path = roe_path)

In this piece of code, we first reconstruct a host ROE object with a mask *cleanMask*, we create ``roe_path`` path for it, 
and we reconstruct a :math:`\pi_0` object inside the host ROE, similarly to the previous code snippets.
Then we create a nested ROE using :func:`modularAnalysis.buildNestedRestOfEvent`, which
is going to be reconstructed using particles from *cleanMask* of the host ROE.
This is needed to clean up the nested ROE from the beam-background energy depositions. 
Then we create ``nestedroe_path`` path for the nested ROE modules and finally we reconstruct a :math:`K_S^0` inside the nested ROE.
One can execute all possible ROE-related methods using nested ROE objects or loops. 


Load ROE as a particle
----------------------

It is possible to load ROE as a particle, which can be manipulated as any other particle in ``basf2``:

.. code-block:: python
  
  import basf2 as b2
  import modularAnalysis as ma
  main_path = b2.create_path()
  # Suppose we have a signal B meson stored in a particle list 'B0:rec'
  # build the ROE object
  ma.buildRestOfEvent('B0:rec', path = main_path)
  # Create a mask tuple:
  cleanMask = ('cleanMask', 'abs(d0) < 10.0 and abs(z0) < 20.0',
                            'E > 0.06 and abs(clusterTiming) < 20')
  # append masks to existing ROE object
  ma.appendROEMasks('B0:rec', [cleanMask], path = main_path)
  # Load ROE as a particle and use a mask 'cleanMask':
  ma.fillParticleListFromROE('B0:tagFromROE', '', maskName='cleanMask', 
    sourceParticleListName='B0:rec', path=main_path)
  
  # A shorter option:
  # ma.fillParticleListFromROE('B0:tagFromROE -> B0:rec', '', 'cleanMask', path=main_path)

The resulting particle list can be combined with other particles, like
``Upsilon(4S) -> B0:tagFromROE B0:rec`` in this example.
Also, any variable should be valid for the ROE particle, however, one should be
aware that these particles typically have a very large amount of daughter
particles.


Another option is to load a particle, which represents missing momentum in the
event:

.. code-block:: python
  
  ma.fillParticleListFromROE('nu:missing', '', maskName='cleanMask', 
    sourceParticleListName='B0:rec', useMissing = True, path=main_path)

These reconstructed neutrino particles have no daughters, and they can be
useful in combination with the visible signal side, for example in semileptonic
:math:`B`-meson decays, where tag side has been reconstructed using :doc:`FullEventInterpretation`.

.. hint::
  It is recommended to try to use ROE variables first, unless it is *absolutely* necessary to reconstruct ROE as a particle in your analysis.
  The decay vertex of the resulting particles can be fitted by `kFit`.
  Also MC truth-matching works, but after removing all neutral hadrons matched to tracks. 
  More improvements will come soon.


Create ROE using MCParticles
----------------------------

It is possible to create ROE using MCParticles:

.. code-block:: python

  ma.fillParticleListFromMC("B0:gen", signal_selection, 
        addDaughters=True, skipNonPrimaryDaughters=True, path=main_path)
  ma.buildRestOfEventFromMC("B0:gen",path=main_path)

It is important to add primary daughters to the signal side particle and not to forget to provide a selection cut. 

.. note::
  ROE masks and many of the ROE variables are working only with reconstructed particles.
  As a workaround one can reconstruct ROE as a particle.

MVA based cleaning
------------------

One can then apply and kind of additional info to the
particles in these particle lists, even training from MVA's, which was the
original initiative for this procedure. 
Here the provided cut strings are applied to the particles in particle lists and then you specify if you want to
keep or discard the objects used by the particles.

.. code-block:: python

  # signal B meson reconstructed as a particle list 'B0:rec'
   
  # build ROE object
  buildRestOfEvent('B0:rec')
   
  # create a default mask with fractions (everything into account)
  appendROEMask('B0:rec', 'advanced', '', '', [0.09,0.11,0.62,0.14,0.048,0])
   
  ###########################################
  # enter the for_each path called roe_main #
  ###########################################
   
  # create for_each path
  roe_main = create_path()
   
  # load tracks and clusters from ROE as pi+ and gamma
  fillParticleList('gamma:roe', 'isInRestOfEvent == 1', path=roe_main)
  fillParticleList('pi+:roe', 'isInRestOfEvent == 1', path=roe_main)
   
  # let's assume that training INFO is available for tracks and clusters, 
  # apply training (should be switched to new MVA in near future)
  applyTMVAMethod('gamma:roe', prefix=pathToClusterTraining, 
                  method="FastBDT", expertOutputName='SignalProbability',
                  workingDirectory=tmvaWorkDir, path=roe_main)
  applyTMVAMethod('pi+:roe', prefix=pathToTrackTraining, 
                  method="FastBDT", expertOutputName='SignalProbability',
                  workingDirectory=tmvaWorkDir, path=roe_main)
   
  # keep the selection based on some optimized cut
  keepInROEMasks('gamma:roe', 'advanced', 'sigProb > 0.5', path=roe_main)
  keepInROEMasks('pi+:roe', 'advanced', 'sigProb > 0.5', path=roe_main)
   
  # execute roe_main
  main_path.for_each('RestOfEvent', 'RestOfEvents', roe_main)
   
  ######################
  # exit for_each path #
  ######################

