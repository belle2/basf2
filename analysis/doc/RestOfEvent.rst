Rest of event
=============

Why?
----

In general, events in Belle II environment are more "polluted" compared to
Belle. This means that there are more particles in the event that do not
originate from collisions or additional ECLClusters produced in single particle
hits and additional tracks from decays of particles in flight, backscatter and
so on. Such Tracks and ECLClusters are regarded as **extra**. They spoil the purity
of the event and should be removed from our selection which we are interested
in.

The purpose of this page is to show in detail how to properly use the
RestOfEvent (``ROE``) object. This tutorial is needed, since the current version is
quite more advanced than the initial one.

Overview
--------

The ROE can be used in multiple cases. For example, two of the most common ones
are in **tagged** and **untagged** analyses of BB meson decays.

  - In tagged analyses, where one reconstructs both BB mesons, the ROE object
    should be empty in a perfect situation. However, it often contains tracks
    and/or clusters from either the collision event or the background. Cleaning up
    the ROE in this case can help improve variables such as :math:`E_{extra}`, which
    should peak at 0.
  - In untagged analyses one reconstruct only the signal *B* meson, everything
    else in the event is considered to come from the companion *B* meson. In
    addition to tracks and clusters from the companion *B* meson, the ROE contains
    also the extra tracks and clusters, as in the case of the tagged analysis.
    Cleaning up the ROE in this case can help improve variables such as :math:`M_{BC}` or
    :math:`\Delta E`.

It is therefore a generally bad idea to take everything into account when dealing with the ROE.

Cleaning Up
-----------

There are two ways of cleaning up the ROE. A quick and basic way and a more
advanced way. Both of them use the so called ROE masks, which are maps of
boolean values for specific track and cluster candidates. Boolean values then
tell if the object should be taken into account or not, based on some sort of
user-defined selection. This makes more sense than just throwing things out of
the ROE and it offers a larger flexibility, ability to compare different
selections at once and so on.

Below you will find more details for both options described above. They are
demonstrated as a reconstruction of the :math:`B\to\pi\ell\nu` decay
channel with the untagged method. The parts of code bellow are snippets from
basf2 python steering files.

**The basic way**

In short, you just have to provide two cut strings with track- and
cluster-based variables. The software then creates temporary (throw-away)
particles from the corresponding reco objects and checks if they pass the cut.

For calculation of energy in ROE, mass has to be assigned to each track. All
tracks are reconstructed as pions by default, but it is possible to set
different ones. It seems that the fractions of particles in generic
:math:`B\bar{B}` events are :math:`(f_e,f_{\mu},f_{\pi},f_K,f_p,f_d)=(0.09,0.11,0.62,0.14,0.048,0)`

::

  # signal B meson reconstructed as a particle list 'B0:rec'
   
  # build ROE object
  buildRestOfEvent('B0:rec')
   
  # create a mask with a name 'basic', use some simple cuts
  track_cuts = 'abs(d0) < 0.2 and abs(z0) < 0.2'
  good_photons = 'Theta > 0.296706 and Theta < 2.61799 and clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]'
  appendROEMask('B0:rec', 'basic', track_cuts, good_photons)
   
  # or
   
  # set some a-priori fractions for (e,mu,pi,K,p,d) (or [-1] to use true mass hypothesis)
  # appendROEMask('B0:rec', 'simple', 'abs(d0) < 0.05 and abs(z0) < 0.1', 'goodGamma == 1', [0.09,0.11,0.62,0.14,0.048,0])

Thats it! It is also possible to create masks with no cuts and update them
later with :func:`modularAnalysis.updateROEMask`,
:func:`modularAnalysis.updateROEMasks` or
:func:`modularAnalysis.updateROEFractions`. More info on this in
:doc:`modular-analysis-convenience-functions`.
More info regarding ECLCluster cuts in `clusters cleaning page <https://confluence.desy.de/display/BI/Physics+Pi0+and+extra+clusters+cleaning>`_ .

Note:
  Cuts on tracks (at least on d0 and z0) are not recommended (unless you know
  what you are doing) since you throw away tracks from long lived decaying
  particles.

**The advanced way**

The first main difference here is that the software doesn't create temporary
particles. In this case **you** provide the particle lists of charged pions
(reconstructed from tracks in ROE) and neutral photons (reconstructed from
clusters in ROE). You can then apply and kind of additional info to the
particles in these particle lists, even training from MVA's, which was the
original initiative for this procedure. Here the provided cut strings are
applied to the particles in particle lists and then you specify if you want to
keep or discard the objects used by the particles.

The second main difference is that this only works with the ``for_each`` path,
where this part of the path is executed for each ROE object in event (usually
there is more than one, the same as number of signal B meson candidates)

::

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
   
  # let's assume that training INFO is available for tracks and clusters, apply training (should be switched to new MVA in near future)
  applyTMVAMethod('gamma:roe', prefix=pathToClusterTraining, method="FastBDT", expertOutputName='SignalProbability',
                  workingDirectory=tmvaWorkDir, path=roe_main)
  applyTMVAMethod('pi+:roe', prefix=pathToTrackTraining, method="FastBDT", expertOutputName='SignalProbability',
                  workingDirectory=tmvaWorkDir, path=roe_main)
   
  # keep the selection based on some optimized cut
  keepInROEMasks('gamma:roe', 'advanced', 'sigProb > 0.5', path=roe_main)
  keepInROEMasks('pi+:roe', 'advanced', 'sigProb > 0.5', path=roe_main)
   
  # execute roe_main
  analysis_main.for_each('RestOfEvent', 'RestOfEvents', roe_main)
   
  ######################
  # exit for_each path #
  ######################

See also :func:`modularAnalysis.discardFromROEMasks`

Note: this is the recommended way of dealing with ROE.

ROE variables
-------------

A cleaned up ROE has an effect on variables which depend on ROE. Such variables
can be found at
``analysis/variables/src/ROEVariables.cc``.
Most of them are ROE dependant and accept the name of the ROE mask as a
parameter. Others are more complicated, but the documentations should provide
the needed information. An example on how to save a ROE dependant variable is
shown below.

::

  # set ntuple tools
  tools  = ['EventMetaData', '^B0']
  .
  .
  .
  tools += ['CustomFloats[ROE_neextra(advanced)]', '^B0']
   
  # you can also use an alias
  from variables import variables
  variables.addAlias('ROECharge', 'ROE_charge(advanced)')
  tools += ['CustomFloats[ROECharge]', '^B0']
   
  # save to tree
  ntupleTree('treeName',   'B0:rec', tools)

Further optimization of advanced way
------------------------------------

Since the advanced way is quite flexible, a lot of things can be done to
optimize it even further. Some of the possibilities are listed below.

**Tracks**

Decay in flight optimization

  Some energetic charged particles like :math:`\Delta` baryons, kaons or even
  pions can decay in flight, which produces additional tracks from the same
  source of momentum. Counting both or all such tracks leads to double counting
  of momentum and wrong determination of charge in the event. It is possible to
  search for track pairs from the same source of momentum and train to search for
  them. However, they are not easily recognized, so a decision was made to train
  for such a cases and assign an extraInfo probability to the tracks.

:math:`V_0` optimization

  By default, MVA is trained to recognize primary tracks
  (:b2:var:`mcPrimary` ==1) from charged :math:`e,\mu,\pi,K~or~p`. Some of these
  can also come from the so-called :math:`V_0` particles
  (:math:`K_S,\Lambda_0,K_S,\Lambda_0`, converted :math:`\gamma`),
  which decay away from the IP region.
  Belle II has software dedicated to find these :math:`V_0` candidates. It is
  possible then to train the MVA to recognize the signal and apply vertex fits.
  After the fit, the :math:`V_0` momentum is better defined, so in principle it
  would be better to use the :math:`V_0` momentum in ROE calculations instead of
  daughter tracks momenta.
  If this procedure is chosen, the initial MVA training should focus on primary
  tracks not coming from :math:`V_0` candidates, since their point of origin can
  confuse the MVA training.

Track duplicates

  Track duplicates are track fits made on hits from the same particle trajectory,
  so only one track should be taken into account in such cases. In MC5 the
  tracking software still used MC based information and there were no track
  duplicates. However, this was later-on switched off and track duplicates are
  present in newer MC productions. *This still needs to be tested on newer
  samples and implemented in the ROE selection*. 

**Clusters**

:math:`\pi_0` optimization

  By default, MVA is trained to recognize primay photons (:b2:var:`mcPrimary` ==1 &&
  :b2:var:`mcPDG` ==22). The majority of photons at Belle II are expected to come
  from :math:`\pi_0` particles.
  A good way to optimize the ROE is to train MVA to recognize photons
  coming from :math:`\pi_0` candidates.
  Information from :math:`\pi_0` training is then applied to pairs of photons and this is
  then used in the cluster MVA training.

  Note:
    You can help and contribute by adding new ways of optimization or other ways of improving!

Available ROE training
----------------------

This is probably the most relevant part of the page for you, but the
information above was also needed if you want to perform your own optimization.
Here you can find some existing training info that you can apply to your case.

Warning:
  This MVA was trained to recognize good tracks and clusters from extra ones.
  The trainings were performed on generic samples and can be used for any case of
  ROE cleaning. In principle, this means that it doesn't matter what your signal
  side is or if you're doing tagged or untagged analyses. Although this has been
  the desired goal, it has not been tested on other analyses yet. Feedback is
  most welcome.

Below you can find the relevant part of a steering file that creates an
optimally cleaned ROE in the form of a single ROE mask. The location of the
trainings can be found at
``/home/belle/lubej/belle2/projects/pilnu/bkgx1/1_ROETraining/training/TMVA//home/belle/lubej/belle2/projects/pilnu/bkgx1/1_ROETraining/training/TMVA/``.
This is an example which uses all of the ROE optimizations which are available
at the moment.

All the optimization builds up to a quite complicated code. For the time being
there are some personal python functions available and they should be
understood and not be used as a black box.

::

  # signal B meson reconstructed as a particle list 'B0:rec'
   
  # build ROE object
  buildRestOfEvent('B0:rec')
   
  # create a default mask with fractions (everything into account)
  appendROEMask('B0:rec', 'opt', '', '', [0.09,0.11,0.62,0.14,0.048,0])
   
  # set up path to python functions, import scripts
  sys.path.append('/home/belle/lubej/belle2/projects/pilnu/bkgx1/scripts')
  from applyInfo import *
   
  ###########################################
  # enter the for_each path called roe_main #
  ###########################################
   
  # create for_each path
  roe_main = create_path()
   
  # load tracks and clusters, apply MVA info, use all optimizations
  applyPrimaryInfo('roe', roe_main)
   
  # optimize ROE with V0, (good separation but loose cut, because nothing wrong if accidental fake V0)
  optimizeROEWithV0('K_S0:roe', 'opt', 'sigProb > 0.4', path=roe_main)
   
  # optimize ROE with track/cluster selection
  keepInROEMasks('gamma:roe', 'opt', 'sigProb > 0.67', path=roe_main)
  keepInROEMasks('pi+:roe', 'opt', 'sigProb > 0.04', path=roe_main)
   
  # execute roe_main
  analysis_main.for_each('RestOfEvent', 'RestOfEvents', roe_main)
   
  ######################
  # exit for_each path #
  ######################

The cuts used for tracks and clusters above are optimal cuts. They have been
determined by maximizing the height of the :math:`\Delta{E}` distribution of
the signal *B* for the case of the untagged analysis. In principle this only
served as a reference point and should not affect other analyses, but, again,
this has not been tested.

The more optimal the ROE selection is, the more entries fall in the
:math:`\Delta{E}=0` region of the distribution, so the ROE selection is
regarded as optimal when the peak of the :math:`\Delta{E}` distribution is maximal.

Below you can see two plots:

  - A fitted :math:`\Delta{E}=0` distribution with a selected number of bins.
    Using a fit to determine the maximum is more stable and non-number-of-bins
    dependant.

  - A 2D plot of the :math:`\Delta{E}=0` distribution height (z-axis) with
    respect to the changing of the cluster cut (x-axis) and track cut (y-axis).

.. figure::  single_fit.png
  :width: 40em
  :align: left

.. figure:: 2Dopt.png
  :width: 40em
  :align: left
