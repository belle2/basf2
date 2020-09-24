.. _onlinebook_first_steering_file:

First steering file
===================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 30 min

    **Exercises**: 90 min

    **Prerequisites**: 
    	
    	* Creating and running scripts in the terminal
    	* Basic python

    **Questions**:

        * How can I load data?
        * How can I reconstruct a decay?
        * How can I match MC?
        * How can I create an ntuple to store information?

    **Objectives**:

        * Reconstruct :math:`B^0 \to J/\Psi(\to e^+e^-)K_S^0(\to \pi^+\pi^+)`

In this hands-on tutorial you'll be writing your first steering file. Our
ultimate goal is to reconstruct :math:`B^0 \to J/\Psi(\to e^+e^-)K_S^0(\to
\pi^+\pi^+)`. You'll be learning step-by-step what is necessary to achieve
this, and in the end you will produce a plot of the B meson candidates. As you
have already learned in the previous sections, basf2 provides a large variety
of functionality. While the final steering file of this lesson will be working
and producing some reasonable output, there are many possible extensions that
you will learn all about in the succeeding lessons.

Let's get started: The very first step is always to set up the necessary
environment.

.. admonition:: Task
    :class: exercise stacked

    Set up the basf2 environment using the currently recommended software
    version.

.. admonition:: Hint
    :class: toggle xhint stacked

    Go back to :ref:`onlinebook_basf2_introduction` to see the step-by-step
    instructions.

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

    .. literalinclude:: steering_files/010_first_steering_file.py

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

    .. literalinclude:: steering_files/011_first_steering_file.py

The mdst data objects (Tracks, ECLCluster, KLMCluster, V0s) of the input file
have to be transferred into Particle data objects. This is done via the
`ParticleLoader` module and its wrapper function `fillParticleList`.

.. admonition:: Exercise
    :class: exercise stacked

    Read the documentation of `fillParticleList` to familiarize yourself with
    the required arguments.

    Which six final state particles can be created from Tracks?

.. admonition:: Solution
    :class: toggle solution

    Electrons, muons, pions, kaons, protons, and deuterons.

Internally, the anti-particle lists are always filled as well, so it is not
necessary to call `fillParticleList` for ``e+`` and ``e-``. In fact, you will
see a warning message for the second call telling you that the corresponding
particle list already exists.

As long as no selection criteria (cuts) are provided, the only difference
between loading different charged final state particle types is the mass
hypothesis used in the track fit.

Each particle used in the ``decayString`` argument of the `fillParticleList`
function can be extended with a label. This is useful to distinguish between
multiple lists of the same particle type with different selection criteria,
e.g. soft and hard photons.

.. code-block:: python

    ma.fillParticleList("e-:soft", "E < 1", path=main)
    ma.fillParticleList("e-:hard", "E > 3", path=main)

.. warning:: If the provided cut string is not empty you can not use the label
             ``all``.

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

    Extend your steering file by loading electrons, positrons, and Kshorts. At
    the very end of your script you should also print a summary table of all
    modules added to your path using the function `statistics`. Then run the
    script and answer the following questions:

    * Which are the mass window boundaries set for the :math:`K_S^0`?
    * Which module had the longest execution time?

.. admonition:: Hint
    :class: toggle xhint stacked

    All you need is `fillParticleList`, `stdKshorts`, and `statistics`.
    Remember that charge-conjugated particles are automatically created.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/012_first_steering_file.py

    In the solution we gave the electrons the label ``uncorrected``. This is
    already in anticipation of a future extension in which Bremsstrahlung
    recovery will be applied (:ref:`onlinebook_various_additions`).

    In the output there are INFO messages that the Kshort invariant mass has
    to be between 0.45 and 0.55 GeV/c :superscript:`2`.

    The module ``TreeFitter_K_S0:RD`` takes the longest. It's a vertex fit of
    the Kshort candidates. You will learn more about vertex fits in
    :ref:`onlinebook_vertex_fitting`.

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
own grammar with several markers, keywords, and arrow types. It is especially
useful for inclusive reconstructions. Follow the provided link if you want to
learn more about the `DecayString`. For the purpose of this tutorial we do not
need any of those fancy extensions, the default arrow type ``->`` suffices.
However, it is important to know how the particles themselves need to be
written in the decay string.

.. admonition:: Exercise
    :class: exercise stacked

    How do we have to type a :math:`J/\Psi`, and what is its nominal mass?

.. admonition:: Hint
    :class: toggle xhint stacked

    Make use of the tool ``b2help-particles``
    (:ref:`onlinebook_basf2basics_b2help_particles`). As a spin-1
    :math:`c\bar{c}` resonance the PDG code of the :math:`J/\Psi` is ``443``.

.. admonition:: Solution
    :class: toggle solution

    The :math:`J/\Psi` has to be typed ``J/psi``. Whenever you misspell a
    particle name in a decay string, there will be an error message telling
    you that it is unknown.

    The invariant mass of the J/psi is set to be 3.0969 GeV/c :superscript:`2`.

.. admonition:: Task
    :class: exercise stacked

    Extend the steering file by first forming :math:`J/\Psi` candidates from
    electron-positron combinations and then combining them with Kshorts to
    form B0 candidates.

.. admonition:: Hint
    :class: toggle xhint stacked

    All you need is to call `reconstructDecay` twice.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/013_first_steering_file.py
        :lines: 1-41, 51-56

In the solution we introduced a loose selection for the electrons using
particle identification (`electronID`), requiring the tracks to originate from
close to the interaction point (`dr` and `dz`), and having a polar angle in
the acceptance of the CDC (`thetaInCDCAcceptance`).

.. admonition:: Exercise
    :class: exercise stacked

    Find out what's the difference between ``dr`` and ``dz``, e.g. why do we
    not have to explicitly ask for the absolute value of dr, and the angular
    range of the CDC acceptance (as implemented in the software).

.. admonition:: Hint
    :class: toggle xhint stacked

    The documentation of `dr` and `dz` should tell you all about the first
    question. The angular range is a bit trickier. You have to directly
    inspect the source code of the variable defined in the variables folder of
    the analysis package. There has been an exercise on how to find the source
    code in :ref:`onlinebook_basf2basics_gettingstarted`.

.. admonition:: Solution
    :class: toggle solution

    The variable `dr` gives the transverse distance, while `dz` is the
    z-component of the point of closest approach (POCA) with respect to the
    interaction point (IP). Components are signed, while distances are
    magnitudes. 

    The polar range of the CDC acceptance is :math:`17^\circ < \theta <
    150^\circ` as written `here
    <https://stash.desy.de/projects/B2/repos/software/browse/analysis/variables/src/AcceptanceVariables.cc#25>`_

To separate signal from background and extract physics parameter, an offline
analysis has to be performed. The final step of the steering file is to write
out information in a so called ntuple using `variablesToNtuple`. It can
contain one entry per candidate or one entry per event.

.. admonition:: Exercise
    :class: exercise stacked

    How do you switch between the two ntuple modes?

.. admonition:: Hint
    :class: toggle xhint stacked

    Look at the documentation of `variablesToNtuple`.

.. admonition:: Solution
    :class: toggle solution

    When providing an empty decay string, an event-wise ntuple will be created.

.. warning::
    
    Only variables declared as ``Eventbased`` are allowed in the event mode.
    Conversely, both candidate and event-based variables are allowed in the
    candidate mode.

A good variable to start with is the beam-constrained mass, which is defined
as

.. math::

    \text{M}_{\rm bc} = \sqrt{E_{\rm beam}^2 - \mathbf{p}_{B}^2}

For correctly reconstructed B mesons this variable should peak at the B meson
mass.

.. admonition:: Task
    :class: exercise stacked

    Save the beam-constrained B mass and the difference between half the
    center-of-mass energy and the reconstructed B energy into an output
    ntuple. Run your steering file with these additions.

.. admonition:: Hint
    :class: toggle xhint stacked

    The variables are called `Mbc` and `deltaE`.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/013_first_steering_file.py

Although you are analyzing a signal MC sample, the reconstruction will find
many candidates that are actually not signal, but random combinations that
happen to fulfill all your selection criteria.

.. admonition:: Task
    :class: exercise stacked

    Write a short python script, in which you load the root ntuple from the
    previous exercise to a dataframe and then plot the distribution of the
    beam-constrained mass into a histogram with 100 bins in the range 4.3 to
    5.3 GeV/c :superscript:`2`. Can you identify the signal and background
    components?

.. admonition:: Hint
    :class: toggle xhint stacked

    Read in the ntuple using ``read_root`` of ``root_pandas``. 

.. admonition:: Solution
    :class: toggle solution

    .. code-block:: python

        import matplotlib.pyplot as plt
        from root_pandas import read_root

        df = read_root('Bd2JpsiKS.root')

        df.hist('Mbc', bins=100, range=(4.3, 5.3))
        plt.xlabel(r'M$_{\rm bc}$ [GeV/c$^{2}$]')
        plt.ylabel('Number of candidates')
        plt.xlim(4.3, 5.3)
        plt.savefig('Mbc_all.png')

    .. figure:: figs/Mbc_all.png
        :width: 40em
        :align: center

        There is a (signal) peak at the nominal B mass of 5.28 GeV/c
        :superscript:`2` and lots of background candidates as a shoulder left
        of the peak.

For the beam-constrained mass we know pretty well how the signal distribution
should look like. But what's the resolution and how much background actually
extends under the signal peak? On MC we have the advantage that we know what
has been generated. Therefore, we can add a flag to every candidate to
classify it as signal or background. Furthermore, we can study our background
sources if we know what the reconstruction has falsely identified.

There is a long chapter on :ref:`mcmatching` in the documentation. You should
definitely read it to understand at least the basics.

.. admonition:: Exercise
    :class: exercise stacked

    Which module do you have to run to get the relations between the
    reconstructed and the generated particles? How often do you have to call
    the corresponding function?

.. admonition:: Hint
    :class: toggle xhint stacked

    Did you have a look at the documentation of :ref:`mcmatching`?

.. admonition:: Solution
    :class: toggle solution

    You need to run the `MCMatcherParticles` module, most conveniently
    available via the wrapper function `modularAnalysis.matchMCTruth`. If run
    for the head of the decay chain, it only needs to be called once because
    the relations of all (grand)^N-daughter particles are set recursively.

.. admonition:: Task
    :class: exercise stacked

    Add MC matching for all particles of the decay chain and add the
    information whether the reconstructed B meson is a signal candidate to the
    ntuple. Run the steering file and again plot the beam-constrained mass but
    this time use the signal flag to visualize which component is signal and
    which is background.

.. admonition:: Hint
    :class: toggle xhint stacked

    Only one call of `matchMCTruth` is needed. The necessary variable is
    called `isSignal`.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/014_first_steering_file.py

    .. code-block:: python

        import matplotlib.pyplot as plt
        from root_pandas import read_root

        df = read_root('Bd2JpsiKS.root')

        df.hist('Mbc', bins=100, range=(4.3, 5.3), by='isSignal')
        plt.xlabel(r'M$_{\rm bc}$ [GeV/c$^{2}$]')
        plt.ylabel('Number of candidates')
        plt.xlim(4.3, 5.3)
        plt.savefig('Mbc_MCsplit.png')

    .. figure:: figs/Mbc_MCsplit.png
        :width: 40em
        :align: center

        The background peaks around 5 GeV/c :superscript:`2`, but indeed
        extends into the signal peak region.

While the MC matching allows us to separate signal from background and study
their shapes, we need to use other variables to achieve the same on collision
data. Initially, it makes sense to look at many different variables and try to
find those with discriminating power between signal and background. The most
basic information are the kinematic properties like the energy and the
momentum (and its components). In basf2 collections of variables for several
topics are prepared.

.. admonition:: Exercise
    :class: exercise stacked

    Find out to which variable collections the three variables belong that we
    added to the ntuple so far.

.. admonition:: Hint
    :class: toggle xhint stacked

    You can find the information in the
    :ref:`analysis/doc/Variables:Collections and Lists` section of the
    documentation.

.. admonition:: Solution
    :class: toggle solution

    The collection ``deltae_mbc`` contains `Mbc` and `deltaE`. The `isSignal`
    variable along with many other truth match variables is in the collection
    ``mc_truth``.

.. admonition:: Task
    :class: exercise stacked

    Save all kinematics information, both the truth and the reconstructed
    values, of the B meson to the ntuple. Also use the variable collections
    from the last exercise to replace the individual list.

.. admonition:: Hint
    :class: toggle xhint stacked

    The variable collections ``kinematics``, ``mc_kinematics``,
    ``deltae_mbc``, and ``mc_truth`` make your life a lot easier.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/015_first_steering_file.py

Apart from variables for the mother B-meson, we are also interested in
information of the other daughter and granddaughter variables. You can access
them via the `daughter` meta variable, which takes an integer and a variable
name as input arguments. The integer (0-based) counts through the daughter
particles, ``daughter(0, p)`` would for example be the momentum of the first
daughter, in our case of the :math:`J/\Psi`. The function can be used
recursively, so ``daughter(daughter(0, E))`` is the energy of the positive
muon. In principle, one can add these nested variables directly to the ntuple
but the brackets have to be escaped and the resulting variable name in the
ntuple is not very user-friendly or intuitive. Instead, one can define aliases
to translate the variables using `addAlias`.

.. admonition:: Exercise
    :class: exercise stacked

    How can you replace ``daughter(daughter(0, E))`` with ``mup_E``?

.. admonition:: Hint
    :class: toggle xhint stacked

    Check the documentation of `addAlias` for the correct syntax.

.. admonition:: Solution
    :class: toggle solution

    .. code-block:: python

        from variables import variables as vm
        vm.addAlias("mup_E", "daughter(daughter(0, E))")

However, this can quickly fill up many, many lines. Therefore, there are tools
to easily create aliases. The most useful is probably
`create_aliases_for_selected`. It lets you select particles from a decay
string via the ``^`` operator, for which you want to define aliases, and also
set a prefix.

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

    .. literalinclude:: steering_files/019_first_steering_file.py

.. admonition:: Key points
    :class: key-points

    * The ``modularAnalysis`` module contains most of what you'll need for now
    * ``inputMdstList`` is used to load data
    * ``fillParticleList`` adds particles into a list
    * ``reconstructDecay`` combined FSPs from different lists to "reconstruct" particles
    * ``matchMCTruth`` matches MC
    * ``variablesToNtuple`` saves an output file
    * Don't forget ``process(path)`` or nothing happens

.. topic:: Author of this lesson

    Frank Meier
