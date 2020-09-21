.. _onlinebook_first_steering_file:

First steering file
===================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 30 min

    **Exercises**: 60 min

    **Prerequisites**: 
    	
    	* Creating and running scripts in the terminal
    	* Basic python

    **Questions**:

        * How can I load data?
        * How can I reconstruct a decay?
        * How can I match MC?
        * Can I see some pretty pictures?

    **Objectives**:

        * Reconstruct :math:`B \to J/\Psi(\to e^+e^-)K_s(\to \pi^+\pi^+)`

In this hands-on tutorial you'll be writing your first steering file.
But before we can start, you have to set up the necessary environment.

.. admonition:: Task
    :class: exercise stacked

    Set up the basf2 environment using the currently recommended software version.

.. admonition:: Hint
    :class: toggle xhint stacked

    Go back to :ref:`onlinebook_basf2_introduction` to see the step-by-step instructions.

.. admonition:: Solution
    :class: toggle solution

    .. code-block:: bash

        source /cvmfs/belle.cern.ch/tools/b2setup
        b2help-releases
        b2setup <release-shown-at-prompt-by-previous-script>

There are three lines of code that are part of each steering script:

   * Importing the ``basf2`` python library
   * Creating a `basf2.Path`
   * Processing the path

.. admonition:: Task
    :class: exercise stacked

    Open an empty file with an editor of your choice. Add those three lines.
    Run the script. Don't worry, if you've done everything correct, you should
    see some error messages. Read them carefully!

.. admonition:: Hint
    :class: toggle xhint stacked

    Have a look at `basf2` and `basf2.process`

.. admonition:: Solution
    :class: toggle solution

    `010_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/010_first_steering_file.py>`_

Of course, no events could be processed because no data has been loaded yet.
Let's do it. As already described in the previous lesson almost all
convenience functions that are needed can be found in `modularAnalysis`.

It is recommended to use `inputMdstList` or `inputMdst`. If you look at the
source code, you'll notice that the latter actually calls the more general
former.

.. admonition:: Exercise
    :class: exercise stacked

    How many arguments are required for the `inputMdstList` function?
    Which value has to be set for the environment type?

.. admonition:: Solution
    :class: toggle solution

    Three parameters have no default value and are therefore required:

    * the environment type
    * a list of root input files
    * the path

    In 99 % ``default`` should be set as ``environmentType``.

In a later lesson you'll learn how and where to find input files for your
analysis. For the purpose of this tutorial we have prepared some local input
files. They are located on kekcc at
``/group/belle2/users/tenchini/prerelease-05-00-00a/1111540100/``.

.. admonition:: Exercise
    :class: exercise stacked

    Check out the location of the files mentioned above. Which two settings of
    MC are provided?

.. admonition:: Solution
    :class: toggle solution

    There are each 100 files with and without beam background (BGx1 and BGx0).
    Their names only differ by the final suffix, which is an integer between 0
    and 99.

.. admonition:: Task
    :class: exercise stacked

    Extend your steering file by loading the data of one of the local input
    files.

.. admonition:: Hint
    :class: toggle xhint stacked

    First, you have to import modularAnalysis. It's better to load the whole
    file, since you will need more functions of it later on. It might be
    convenient to set an abbreviation, e.g. ``ma``. In principle, for one
    input file `inputMdst` is sufficient.

.. admonition:: Solution
    :class: toggle solution

    `011_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/011_first_steering_file.py>`_

The mdst data objects (Tracks, ECLCluster, KLMCluster, V0s) of the input file
have to be transferred into `Particle` data objects. This is done via the
`ParticleLoader` module and its wrapper function `fillParticleList`.

.. admonition:: Exercise
    :class: exercise stacked

    Read the documentation of `fillParticleList` to familiarize yourself with the required arguments.

    Which six final state particles can be created from Tracks?

.. admonition:: Solution
    :class: toggle solution

    Electrons, muons, pions, kaons, protons, and deuterons.

Internally, the anti-particle lists are always filled as well, so it is not
necessary to call `fillParticleList` for ``e+`` and ``e-``. In fact, you will
see a warning message for the second call telling you that the corresponding
particle list already exists.

Each particle used in the ``decayString`` argument of the `fillParticleList`
function can be extended with a label. This is useful to distinguish between
multiple lists of the same particle type with different selection criteria,
e.g. soft and hard photons.

.. code-block:: python

    ma.fillParticleList("e-:soft", "E < 1", path=main)
    ma.fillParticleList("e-:hard", "E > 3", path=main)

.. warning:: If the provided cut string is not empty you can not use the label ``all``.

There are standard particle lists with predefined selection criteria. While
those for charged final state particles should only be used in early stages of
your analysis and be replaced with dedicated selections adjusted to the needs
of the decay mode you are studying, it is recommended to use them for V0s
(:math:`K_S^0`, :math:`\Lambda^0`). They are part of the library `stdV0s`.

.. admonition:: Exercise
    :class: exercise stacked

    What is the full name of the standard :math:`K_S^0` particle list?

.. admonition:: Hint
    :class: toggle xhint stacked

    Read the documentation of `stdV0s.stdKshorts`.

.. admonition:: Solution
    :class: toggle solution

    It's ``K_S0:merged`` because it is a combination of Kshorts created
    directly from V0s found in the tracking and combinations of two charged
    pions.

.. admonition:: Task
    :class: exercise stacked

    Our ultimate goal is to reconstruct :math:`B \to J/\Psi(\to e^+e^-)K_s(\to
    \pi^+\pi^+)`. Extend your steering file by loading electrons, positrons,
    and Kshorts. At the very end of your script you should also print a
    summary table of all modules added to your path using the function `statistics`.

.. admonition:: Hint
    :class: toggle xhint stacked

    All you need is `fillParticleList`, `stdKshorts`, and `statistics`.
    Remember that charge-conjugated particles are automatically created.

.. admonition:: Solution
    :class: toggle solution

    `012_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/012_first_steering_file.py>`_

Now we have a steering file in which final state particles are loaded from the
input mdst file to particle lists. One of the most powerful modules of the
analysis software is the `ParticleCombiner`. It takes those particle lists and
finds all **unique** combinations. The same particle can of course not be used
twice, e.g. the two positive pions in :math:`D^0 \to K^- \pi^+ \pi^+ \pi^-`
have to be different mdst track objects. However, all of this is taken care of
internally. For multi-body decays like the one described above there can
easily be many multiple candidates, which share some particles but differ by
at least one final state particle.

The wrapper function for the ParticleCombiner is called `reconstructDecay`.
Its first argument is a `DecayString`, which is a combination of a mother
particle (list), an arrow, and daughter particles. The `DecayString` has its
own grammar with several markers, keywords, and arrow types.

.. admonition:: Task
    :class: exercise stacked

    Extend the steering file by first forming J/psi candidates from
    electron-positron combinations and then combining them with Kshorts to
    form B0 candidates.

.. admonition:: Hint
    :class: toggle xhint stacked

    All you need is to call `reconstructDecay` twice.

.. admonition:: Solution
    :class: toggle solution

    `013_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/013_first_steering_file.py>`_

.. admonition:: Task
    :class: exercise stacked

    Save the beam-constrained B mass and the difference between half the
    centre-of-mass energy and the reconstructed B energy into an output
    ntuple.

.. admonition:: Hint
    :class: toggle xhint stacked

    The variables are called `Mbc` and `deltaE`.

.. admonition:: Solution
    :class: toggle solution

    `014_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/014_first_steering_file.py>`_

.. admonition:: Task
    :class: exercise stacked

    Run the MC matching for all particles of the decay chain and add the
    information whether the reconstructed B meson is a signal candidate to the
    ntuple.

.. admonition:: Hint
    :class: toggle xhint stacked

    Only one call of `matchMCTruth` is needed. The necessary variable is
    called `isSignal`.

.. admonition:: Solution
    :class: toggle solution

    `015_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/015_first_steering_file.py>`_

.. admonition:: Task
    :class: exercise stacked

    Save all kinematics information, both the truth and the reconstructed values, of the B meson to the ntuple.

.. admonition:: Hint
    :class: toggle xhint stacked

    The variable collections ``kinematics``, ``mc_kinematics``,
    ``deltae_mbc``, and ``mc_truth`` make your life a lot easier.

.. admonition:: Solution
    :class: toggle solution

    `016_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/016_first_steering_file.py>`_

.. admonition:: Task
    :class: exercise stacked

    Add PID and track variables for all charged final state particles and the
    invariant mass of the intermediate resonances to the ntuple. Also add the
    standard variables from before for all particles in the decay chain, the
    kinematics both in the lab and the CMS frame.

.. admonition:: Hint
    :class: toggle xhint stacked

    Variable collections and alias functions are your friend!

.. admonition:: Solution
    :class: toggle solution

    `019_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/019_first_steering_file.py>`_

.. admonition:: Key points
    :class: key-points

    * The ``modularAnalysis`` module contains most of what you'll need for now
    * ``inputMdstList`` is used to load data
    * ``fillParticleList`` adds particles into a list
    * ``reconstructDecay`` combined FSPs from different lists to "reconstruct" particles
    * ``matchMCTruth`` matches MC
    * ``variablesToNtuple`` saves an output file
    * Don't forget ``process(path)`` or nothing happens

