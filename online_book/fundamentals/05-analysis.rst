.. _onlinebook_fundamentals_analysis:

Analysis
========

.. sidebar:: Overview
    :class: overview

    **Teaching**: 30 minutes

    **Prerequisites**:

        * :ref:`Fundamentals Introduction <onlinebook_fundamentals_introduction>`
        * :ref:`Fundamentals of Data Processing <onlinebook_fundamentals_datataking>`

    **Objectives**:

        * The concept of Skimming
        * Reconstructed and Combined particles


The Skimming
------------

After data processing, the mDST files are finally available for analysis. While it's possible
to run directly on them, it's however quite inconvenient. The mDST contain all the events that
have been acquired, while usually an analysis needs only a very small fraction of them, not to
mention that several analyses may share the need to reconstruct the same objects (countless
analyses include a :math:`D^{\star}` or a :math:`J/\psi`, for example).

The goal of the analysis skims (often just called skims) is to produce smaller datasets,
each amounting to few percents of the total dataset,  that can be shared among several analyses.

To produce a skim one has do define a rather simple selection, which is as inclusive as possible
(or, in other words, uses cuts as loose as possible), while keeping the retention rate within
reasonable boundaries.

.. note::
   *When you run your analysis, you should first check if there is a skim that suits it*. Running
   on skimmed files is much faster and safer than running on the full dataset. If you cannot
   find a skims that suits your needs, talk to your working group convener first to figure
   out the best strategy for you.




Reconstructed and Combined particles
------------------------------------

The Belle II detectors can provide three kinds of information:

* Momentum
* Energy
* PID probability.

Of course not all of them are available for every particle, in fact in most cases
only two of them are, and however only for a very limited number of particles.  Most of the particles
in the :math:`e^+e^-` collision do not live long enough to leave any signal inside the detector, and must
be reconstructed measuring their decay products. This introduces the very important distinction
between measured particles, who leave a signal inside the detector and are reconstructed directly
starting from those signals and combined particles that are reconstructed as sum of measure particles.

.. admonition:: Question
    :class: exercise stacked

    What is the average flight length of a 1 GeV muon and a 1 GeV :math:`D^+` meson?
    Do they both reach the tracking system?

.. admonition:: Hint
    :class: toggle xhint stacked

    Go to https://pdglive.lbl.gov/Viewer.action and look up at the muon and :math:`D^+` lifetime.
    Remember that the beampipe radius is 1 cm.

.. admonition:: Solution
    :class: toggle solution

    The average flight length of a particle of speed :math:`\beta` and lifetime
    :math:`\tau` is
    :math:`L = \gamma\beta\tau c`. The Lorentz factor is :math:`\gamma = \frac{E}{Mc^2}` while :math:`\beta = pc/E`,
    therefore :math:`L = \tau p/M`. From this:
    :math:`L_{\mu} \approx 63` m and :math:`L_{D^0} \approx 170` μm. Only the muon reaches
    the tracking system.


.. admonition:: Question
    :class: exercise stacked

    Keeping in mind the result of the previous exercise, which particles do you think survive long
    enough to reach the active volume of the detector and leave signal there?

.. admonition:: Hint
    :class: toggle xhint stacked

    Think about the truly stable and very long lived particles you know, but remember that not all
    neutral particles are well reconstructed by the ECL or KLM!

.. admonition:: Solution
    :class: toggle solution


    Let's start from the **charged particles**.

    * All the strongly or electromagnetically decaying resonances, both charged and neutral
      (:math:`\rho`, :math:`K^\star`, :math:`\pi^0`, ...) do not
      live long enough to significantly move away from the :math:`e^+e^-` interaction point, so they can
      only be reconstructed by detecting their decay products.
    * Proton, electrons and their anti-particles are stable and definitively leave ionization signals
      in the tracking system, so they should be in the list.
    * Other particles that could leave ionization signals are the long-lived, weakly decaying particles.
      Charged pions, charged kaons and muons are not stable, but we saw already that the latter has a
      very long flight length, usually exceeding the scale of the detector. Pions and kaons decay much
      faster than a muon, but still have a :math:`c\tau` of approximately 8 and 4 meters respectively,
      which makes them likely to leave a detectable track before decaying. They are both on the list.
      :math:`D` and :math:`B` mesons fly much less than a mm before decaying, so they cannot leave any
      detectable track.
    * Some hyperons, strange baryons, are charged and have a sizable lifetime. The longest-living one
      is the :math:`\Xi^+` with a lifetime of 1.7 ns corresponding to :math:`c\tau \approx 5` cm. Such a particle,
      especially if it has some few GeV of momentum, can cross the PXD and even the inner layers of the SVD,
      leaving a signal. However, such a short track would be very difficult to reconstruct, and it's
      much more convenient to reconstruct these hyperons by looking at their (almost) stable decay products.
    * Finally, there's one last category of stable charged particles we can detect: light (anti-)nuclei such as the
      deuteron, tritium or helium. These can be produced ether in the :math:`e^+e^-` collision or, much more
      easily, by spallation processes on the inner detector materials.


    Let's look now at the **neutral particles**.

    * The photon is stable, and the ECL is designed exactly
      to measure photon energies.
    * The (anti-)neutron is basically stable for our purposes, but it
      leaves no signal in the tracking system and the ECL is not designed as an hadronic calorimeter.
      There are studies focused on reconstructing this particle in the ECL, but let's leave it out
      of the list for the moment.
    * The :math:`K_L` has :math:`c\tau \approx 15 m`, so it's definitely to be considered stable
      in the scale of the experiment. Like the neutron, it leaves no ionization, but the KLM is designed
      to detect its interaction in the iron layers of the solenoid's return yoke. Let's count it as a
      reconstructed particle.
    * :math:`\Lambda` and :math:`K_s` behave similarly to the :math:`K_L`, but their lifetime is much shorter
      and, at the Belle II energies, they mostly decay inside the tracking volume. The most convenient way
      to reconstruct them is to reconstruct and combine their decay products, pions and proton.


Let's see now how reconstructed and combined particles are handled, and what are the special cases.
Reconstructed particles are also referred to as *final state particles (FSP)*, as they are the very final
products of any decay chain we may be interested in reconstructing.
In making an analysis, one has three building blocks:

Reconstructed particle
    Reconstructed particles are the basic building block for any analysis. The originate from two
    different reconstruction objects: charged particles are reconstructed from tracks, photons and
    :math:`K_L` from ECL or KLM clusters. Of course a charged track entering the ECL will leave a signal,
    so one can have a cluster attached to a charged tracks.
    The tracking can only measure ether 3-momentum of a particle, so to calculate its 4-momentum one
    has to make an assumption on the mass. This is usually based on the response of the PID system.
    On the other hand clusters provide a measurement of the energy, but not of the momentum. To get it, we
    make both an assumption on the particle mass, and on its production point (all photons and :math:`K_L` are
    assumed to originate in the primary interaction point).
    Tracks and clusters are produced during the reconstruction step. The only operation that is left to the
    final user at the analysis level is the mass assignment.

Combined particles
   Summing the 4-momenta of reconstructed particles one can reconstruct any resonance, at least as
   long as all its decay products are measured. One can then proceed further and combine combined particles
   to move upwards in a decay tree, until the desired step is reached.
   The creation of combined particles is done at the analysis level by the final user, using the tools
   provided by the basf2 analysis package.

V0
   Finally, there's a class of combined particle that require a special treatment, and are therefore provided
   to the user by the reconstruction procedure. So-called V0 are neutral particles decaying into two charged
   particles far from the interaction point, leaving a typical V-shaped signature. These particles can of course be
   reconstructed combining the four-momenta of their daughters, but if the decay has happened outside of the beam pipe
   it is better to re-run the tracking before doing it, since if a track originates not in the IP, it will cross less
   material than expected and the multiple scattering corrections must be updated.

.. include:: ../lesson_footer.rstinclude

.. topic:: Author(s) of this lesson

    Umberto Tamponi,
    Martin Ritter,
    Oskar Hartbrich,
    Michael Eliachevitch,
    Sam Cunliffe
