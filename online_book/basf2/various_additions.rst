.. _onlinebook_various_additions:

Various additions
=================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 30min

    **Exercises**: 15min

    **Prerequisites**:

        * :ref:`onlinebook_first_steering_file`

    **Questions**:

        * What is Bremsstrahlung radiation? Why do we care about it?
        * How do we look for Bremsstrahlung photons?
        * Why do we need to perform a best candidate selection?
        * How can I correct for Bremsstrahlung?
        * How can I perform a best candidate selection (BCS)?

    **Objectives**:

        * Use the different Bremsstrahlung recovery strategies to correct the
          momentum of the electrons in the
          :math:`B^0 \to K_S^0 J/\psi\left(\to e^+ e^-\right)` decay
        * Perform a Best Candidate Selection

What is Bremsstrahlung radiation?
---------------------------------

As you may guess from its unfriendly typing, "Bremsstrahlung" is a German word,
literally meaning "braking radiation"; it is used to describe the
electromagnetic radiation that particles emit when decelerating.
You can read more about it in the many, many pages available on the Internet,
starting with `Wikipedia <https://en.wikipedia.org/wiki/Bremsstrahlung>`_
and the `PDG review <https://pdg.lbl.gov/2020/reviews/rpp2020-rev-passage-particles-matter.pdf>`_.

At Belle II, Bremsstrahlung radiation is emitted by particles when traversing the
different detector components. As the largest material load is near the
interaction region, most Bremsstrahlung radiation is expected to originate
there. The total radiated power due to Bremsstrahlung by a particle of charge
:math:`q` moving at speed (in units of :math:`c`) :math:`\beta` is given by

.. math::

        P = \frac{q^2\gamma^6}{6\pi\varepsilon_0 c}
        \left(
            \left(\dot{\vec{\beta}}\right)^2 -
            \left(\vec{\beta} \times \dot{\vec{\beta}}\right)^2
        \right)

where :math:`\gamma = \frac{1}{\sqrt{1-\beta^2}}`.

For cases where the acceleration is parallel to the velocity, the radiated power
is proportional to :math:`\gamma^6`, whereas for perpendicular acceleration it
scales with :math:`\gamma^4`.
As :math:`\gamma = E/mc^2`, lighter particles will lose more energy through
Bremsstrahlung than heavier particles with the same energy.
At Belle II, we usually only consider Bremsstrahlung loses for electrons and
positrons.

.. admonition:: Exercise (optional)
   :class: exercise stacked

   From the general equation for radiated power, derive the explicit form for
   the limit cases of perpendicular and parallel acceleration (if you are
   attending the Starter Kit, you may want to try this later, so it doesn't
   interfere with the flow of the lesson).

.. admonition:: Hint
   :class: toggle xhint stacked

   The case of parallel acceleration and velocity should be straightforward. For
   the perpendicular case, the next identity may be useful:

   .. math::

        \left(\vec{\beta}\cdot \dot{\vec{\beta}}\right)^2 =
        \dot{\beta}^2\beta^2 - \left(\vec{\beta} \times \dot{\vec{\beta}}\right)^2

.. admonition:: Solution
   :class: toggle solution

   .. math::

        P_{a\parallel v} =
        \frac{q^2a^2\gamma^6}{6\pi\varepsilon_0c^3} \hspace{1cm} P_{a\bot v} =
        \frac{q^2a^2\gamma^4}{6\pi\varepsilon_0c^3}

A proper method that accounts for Bremsstrahlung loses is of utmost importance
at B factories; at the end of this section, you will be able to obtain the
invariant mass distribution for the :math:`J/\psi \to e^+e^-` meson decay after
correcting for the Bremsstrahlung radiation, and compare it with the
distribution you obtained in the previous lesson.

How do we look for Bremsstrahlung photons
-----------------------------------------

Though we will not discuss it here (but, if you are interested, you can consult
`this document <http://kirkmcd.princeton.edu/examples/moving_far.pdf>`_), the
radiated power for relativistic particles is maximum around the particle's
direction of motion; we thus expect Bremsstrahlung photons to be mostly emitted
in a cone around the momentum vector of the electrons (and positrons).
The procedures we use to perform Bremsstrahlung recovery are based on this
assumption.

The Belle like recovery looks for photons on a single cone around the initial
momentum of the particle; on the other side, the Belle II method uses multiple
cones, centered around the momentum of the particle at the points along its path
where it was more likely to emit Bremsstrahlung radiation.
The Belle II method also performs a pre-processing of the data, and applies some
initial cuts on the Bremsstrahlung photons and on the electrons which the user
cannot undo.
Although we recommend using the Belle II method, you should check which procedure
works best for your analysis.

In order to perform Bremsstrahlung recovery (either with the Belle or the Belle
II methods), you need first to construct two particle lists: the first one will
have the particles whose energies you want to recover, and the second one will
contain the Bremsstrahlung photons you will use to recover said energies.
Making use of the steering file developed in the previous sections, we already
have our first particle list ready: ``e+:uncorrected`` (the reason why this
particle list was given this name is, well, because these positrons haven't been
Bremsstrahlung corrected yet!).

Next we will build up the list of possible Bremsstrahlung photons.
In order to reduce the number of background clusters included, we first define a
minimum cluster energy according to the region in the ECL the cluster is found.

Because this cut will be a bit complicated, we will define aliases for cuts.
This actually works with the `addAlias` function as well, if we combine it
with the `passesCut` function.

.. admonition:: Exercise
    :class: exercise stacked

    How would you define the alias ``myCut`` for the cut ``E > 1 and p > 1``?

.. admonition:: Solution
    :class: solution toggle

    You can use the ``passesCut`` function to turn a cut into a variable and
    assign an alias for it.

    .. code-block:: python3

        from variables import variables as vm
        vm.addAlias("myCut", "passesCut(E > 1 and p > 1")

.. admonition:: Exercise
   :class: exercise stacked

   Create a particle list, called ``gamma:brems``, with photons following the next cuts:

         1. If the photons are in the forward endcap of the ECL, their energy should be at least 75 MeV
         2. If they are in the barrel region, their energy should be larger than 50 MeV
         3. Finally, if they are in the backward endcap, their energy should be larger than 100 MeV

   To do this, you need the `clusterReg` and `clusterE` variable.
   To keep everything neat and
   tidy, we recommend that you define the aliases ``goodFWDGamma``,
   ``goodBRLGamma`` and ``goodBWDGamma`` for the three cuts. Finally you can
   combine them to a ``goodGamma`` cut and use this to fill the particle list.

.. admonition:: Hint
   :class: toggle xhint stacked

   The cuts will look like this:

   .. code-block:: python3

       vm.addAlias(
           "goodXXXGamma", "passesCut(clusterReg == XXX and clusterE > XXX)"
       )

   where the ``XXX`` should be filled by you.

.. admonition:: Another hint
   :class: toggle xhint stacked

   This is the first one:

   .. literalinclude:: steering_files/039_various_additions.py
           :start-at: S10
           :end-at: E10

.. admonition:: Solution
   :class: toggle solution

        .. literalinclude:: steering_files/039_various_additions.py
           :start-at: S20
           :end-at: E20

Next, we perform the actual recovery, using the `correctBrems` function in the
Modular Analysis package.

This step will create a new particle list; each particle in this list will have
momentum given by the sum of the original, uncorrected particle momentum, and
the momenta of all the Bremsstrahlung photons in the ``gamma:brems`` list that
fall inside the cone(s) we mentioned previously. Each new particle will also
have as daughters the original particle and its Bremsstrahlung photons (if any),
and an `extraInfo` field named ``bremsCorrected`` that will indicate if at least
one Bremsstrahlung photon was added to this particle.

.. admonition:: Exercise
   :class: exercise stacked

   Perform Bremsstrahlung recovery on the ``e+:uncorrected`` list,  using the
   `correctBrems` function and the ``gamma:brems`` photons. Create a new
   variable, called ``isBremsCorrected``, that tells us if a particle has been
   Bremsstrahlung corrected

.. admonition:: Solution
   :class: toggle solution

        .. literalinclude:: steering_files/039_various_additions.py
           :start-at: S30
           :end-at: E30

.. admonition:: Question
   :class: exercise stacked

   Assume that one particle in the ``e+:corrected`` particle list has
   ``isBremsCorrected`` equal to ``False``.
   How many daughters does this particle have? What is the relation between the
   daughter(s) momenta and this particle momentum?

.. admonition:: Solution
   :class: toggle solution

   No Bremsstrahlung photons were found for this particle, so it only has *one*
   daughter, the original uncorrected one.
   Since there was no correction performed, the momentum of this particle will
   simply be the same as the momentum of its daughter.

.. admonition:: Exercise
   :class: exercise stacked

   How would you use the Belle method for Bremsstrahlung recovery, instead of the
   Belle II one?

.. admonition:: Hint
   :class: toggle xhint stacked

   Take a look at the documentation: `correctBremsBelle`

.. admonition:: Solution
   :class: toggle solution

   .. code-block:: python3

      ma.correctBremsBelle('e+:corrected', 'e+:uncorrected', 'gamma:brems', path=main)

Note that the Bremsstrahlung correction methods have multiple optional
parameters.
Make sure to read their documentation in order to be able to make the best use
of these tools.

When working on MC data, a special note of caution is at place.
In the simulation, Bremsstrahlung photons do not have an ``mcParticle``
associated to them; because of this, the usual `MCMatching` procedure will give
faulty results.
In order to avoid this, when checking the MC truth of decays containing
Bremsstrahlung corrected particles,  you can either replace the ``isSignal``
variable by the `isSignalAcceptBremsPhotons` one, or add the ``?addbrems``
marker to the decay string:

.. literalinclude:: steering_files/039_various_additions.py
   :start-at: S40
   :end-at: E40

Finally, let's add the invariant mass of the :math:`J/\psi` meson without any
Bremsstrahlung recovery applied. Then, after running your steering file, compare
this invariant mass with the one obtained after the recovery, by selecting only
the correctly reconstructed :math:`J/\psi`. Can you see the effect of the
Bremsstrahlung recovery?

.. admonition:: Exercise
    :class: exercise stacked

    Create a variable to calculate the invariant mass of the
    :math:`J/\psi` meson using the *uncorrected* momenta of the leptons. Call it
    ``M_uncorrected``.
    You may find the meta-variable `daughterCombination` useful.

.. admonition:: Hint
    :class: toggle xhint stacked

    ``daughterCombination(M,0:0,1:0)`` will give us the invariant mass of the first
    daughter of the first daughter, and the first daughter of the second daughter.
    Since all particles in the ``e+:corrected`` particle list have as first daughter
    the uncorrected particle, we just need to calculate this daughter combination for
    the :math:`J/\psi` meson.

.. admonition:: Hint
    :class: toggle xhint stacked

    We can do this by directly appending the expression to
    the list of :math:`J/\psi` variables we want to store, or we can rather make it a
    variable of the B mesons, by using the `daughter` meta-variable.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/039_various_additions.py
      :start-at: S50
      :end-at: E50

.. admonition:: Exercise
    :class: exercise stacked

    Your steering file should now be complete. Please run it or compare it with the solution.

.. admonition:: Solution
    :class: solution toggle

    Your steering file should look like this:

    .. literalinclude:: steering_files/035_various_additions.py
        :linenos:

.. admonition:: Exercise
    :class: exercise stacked

    Plot a histogram of `M` and ``M_uncorrected`` for the correctly reconstructed
    :math:`J/\psi` mesons

.. admonition:: Solution
   :class: toggle solution

    .. literalinclude:: roe/invariant_mass_plot.py
      :linenos:

    The results should look similar to :numref:`jpsi_brems_validation_plot` (this was obtained with a
    different steering file, so do not mind if your plot is not exactly the same).

    .. _jpsi_brems_validation_plot:

    .. figure:: roe/brems_corr_invariant_mass.png
       :width: 40em
       :align: center

       Invariant mass distributions for the reconstructed decay, :math:`J/\psi \to e^+e^-`,
       with and without Bremsstrahlung correction

.. admonition:: Extra exercises
   :class: exercise

   * Store the ``isBremsCorrected`` information of the positrons and electrons
     used in the :math:`J/\psi` reconstruction
   * Create a variable  named ``withBremsCorrection`` that indicates if any of
     the leptons used in the reconstruction of the B meson was Bremsstrahlung recovered

.. admonition:: Key points
    :class: key-points

    * There are two ways of performing Bremsstrahlung correction: `correctBrems` and
      `correctBremsBelle`
    * Both of them create new particle lists
    * The members of the new particle list will have as daughter the original
      uncorrected particle and, if a correction was performed, the
      Bremsstrahlung photons used
    * MC matching with Bremsstrahlung corrected particles requires a special
      treatment: use the `isSignalAcceptBremsPhotons` variable, or add the
      ``?addbrems`` marker in the decay string

Best Candidate Selection
------------------------

Sometimes, even after multiple selection criteria have been applied, a single
event may contain more than one candidate for the reconstructed decay.
In those cases, it is necessary to use some indicator that measures the quality
of the multiple reconstructions, and that allow us to select the best one (or,
in certain studies, select one candidate at random). Which variable to use as
indicator depends on the study, and even on the analyst. Our intention here is
not to tell you how to select the best quality indicator, but rather to show yo
how to use it in order to select the best candidate.

The Modular Analysis package has two very useful functions, `rankByHighest` and
`rankByLowest`.
Each one does exactly as its name indicates: they rank particles in descending
(`rankByHighest`) or ascending (`rankByLowest`) order, using the value of the
variable provided as a parameter.
They append to each particle an `extraInfo` field with the name
``${variable}_rank``, with the best candidate having the value one (1).
Notice that *each particle/anti-particle list is sorted separately*, i.e., if a
certain event has multiple :math:`B^+` and :math:`B^-` candidates, and you apply
the ranking function to any of the particle lists, each list will be ranked
separately.

Best candidate selection can then be performed by simply selecting the particle
with the lowest rank.
You can do that by either applying a cut on the particle list, or directly
through the `rankByHighest` and `rankByLowest` functions, by specifying a
non-zero value for the ``numBest`` parameter.
Make sure to check the documentation of these functions.

Continuing with our example, we will make a best candidate selection using the
:b2:var:`random` variable, which returns a random number between 0 and 1 for
each candidate.
We will select candidates with the largest value of `random`.
In order to have uniform results across different sessions, we manually set the
random seed.

.. admonition:: Exercise
   :class: exercise stacked

   Set the basf2 random seed to ``"Belle II StarterKit"``.
   Then, rank your B mesons using the `random` variable, with the one with the
   highest value first.
   Keep only the best candidate.

.. admonition:: Hint
   :class: toggle xhint stacked

   You may want to check the documentation for the `rankByHighest` and `set_random_seed` functions.

.. admonition:: Solution
   :class: toggle solution

    .. literalinclude:: steering_files/039_various_additions.py
       :start-at: S60
       :end-at: E60

.. warning::

   Best candidate selection is used to pick the most adequately reconstructed
   decay, after all other selection cuts have been applied.
   As so, make sure to include it **after** you have performed all the other
   cuts in your analysis.

.. admonition:: Exercise
    :class: exercise stacked

    Your steering file should now be complete. Run it or compare it with the
    solution.

.. admonition:: Solution
     :class: solution toggle

     .. literalinclude:: steering_files/039_various_additions.py
       :linenos:

.. admonition:: Extra exercises
   :class: exercise

   * Remove the ``numBest`` parameter from the `rankByHighest` function, and
     store both the ``random`` and  the ``extraInfo(random_rank)`` variables.
     You can, and probably should, use aliases for these variables.
     Make sure that the ranking is working properly by plotting one variable
     against the other for events with more than one candidate (the number of
     candidates for a certain event is stored automatically when performing a
     reconstruction.
     Take a look at the output root file in order to find how is this variable named).
   * Can you think of a good variable to rank our B mesons? Try to select
     candidates based on this new variable, and compare how much do your results
     improve by, i.e., comparing the number of true positives, false negatives,
     or the distributions of fitting variables such as the beam constrained mass.

.. note::

   From light release ``light-2008-kronos``, the Modular Analysis package
   introduces the convenience function `applyRandomCandidateSelection`, which is
   equivalent to using `rankByHighest` or `rankByLowest` with the `random`
   variable, and with ``numBest`` equal to 1.


.. admonition:: Key points
    :class: key-points

    * Best candidate selection can be performed with the `rankByHighest` and
      `rankByLowest` functions
    * These functions sort particles and antiparticles separately
    * From light release ``light-2008-kronos``, a new helper function can be
      used to perform random candidate selection: `applyRandomCandidateSelection`

.. include:: ../lesson_footer.rstinclude

.. topic:: Authors of this lesson

     Alejandro Mora,
     Kilian Lieret
