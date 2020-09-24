.. _onlinebook_fei:

Full Event Interpretation
=========================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 10 min

    **Exercises**: 30 min (including processing)

    **Prerequisites**: 
    	
    	* Lessons 1-2

    **Questions**:

        * What are tagged analyses?
        * How can I reconstruct a generic B meson?
        * How can I use information from **both** B mesons for my analysis?

    **Objectives**:

        * Learn how to use the FEI in your steering file
        * Understand the output from the FEI


Introduction
------------

In each :math:`\Upsilon(4S)` decay at Belle II, two B mesons are produced. If one of the B mesons decays to a final
state involving neutrinos, this B meson cannot be reconstructed completely as the neutrinos are invisible to the detector.
The information can, however, be recovered from the second B meson. This is called **tagging**,
the second B meson is referred to as B\ :sub:`tag`. The Full Event Interpretation (FEI; pronounced like phi)
is an algorithm which reconstructs this B\ :sub:`tag` in roughly 10,000 channels.
It uses a hierarchical approach to do so, first reconstructing low-level particles such as Kaons and Pions,
then intermediate D mesons and finally B mesons. At each stage, the most-likely particle candidates are selected using
multivariate classifiers and then used for the next stage. Usually, we do not have to train these classifiers ourselves as
this requires large computing resources. Instead, the classifiers are pre-trained and can be loaded from a central database.

Fortunately, for you as a user this is all contained within a few lines of code, allowing you to easily include the FEI
in your steering file.
In the first part of this exercise, we will use the FEI to reconstruct a  B\ :sup:`+`\ :sub:`tag` and have a look at its
properties. In the second part we will combine the B\ :sub:`tag` with a B meson reconstructed in a decay channel
involving neutrinos, referred to as B\ :sub:`sig`.


.. admonition:: Exercise
     :class: exercise stacked

      Find the documentation of the FEI and look up "hadronic tagging" and
      "semileptonic tagging" in the FEI's documentation.
      What is the difference between both analysis strategies? What are advantages and disadvantages?

.. admonition:: Hint
     :class: toggle xhint stacked

     The documentation can be found in the :ref:`analysis/doc/index-01-analysis:Advanced Topics` section of the analysis module.
     The definitions can be found in the section titled ``Hadronic, Semileptonic and Inclusive Tagging``.

.. admonition:: Solution
     :class: toggle solution

     The documentation is here: `FullEventInterpretation`

     Hadronic tagging uses only B meson decays without neutrinos and is therefore more precise
     (as the exact momentum of the B meson can be known).

     Semileptonic tagging uses decays that include one neutrino,
     these have higher branching fractions but are less precise as the kinematic information is incomplete.


In this exercise we will use hadronic tagging, the procedure for semileptonic tagging is analogous but as it is less
commonly used it is not discussed in this exercise.


Reconstructing a B\ :sup:`+`\ :sub:`tag`
----------------------------------------

Prerequisites
*************

As usual, we start our steering file by importing the  necessary python packages,
creating a basf2 path and loading input data.
In addition to the usual python packages (``basf2`` and `modularAnalysis`) we also import the ``fei`` package.

.. admonition:: Exercise
    :class: exercise stacked

    Import ``basf2``, ``modularAnalysis`` and ``fei``. Then create a path and
    load input data from an ``mdst`` file.

    You can again use mdst files from ``/group/belle2/users/tenchini/prerelease-05-00-00a/charged/`` on kekcc.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/070_fei.py
        :language: python
        :lines: -21

Now we need the Global Tag in which the weight files for the FEI can be found. This can change once a new central
training of the FEI is released so it is best to use the `b2conditionsdb-recommend<b2conditionsdb-recommend>` tool
with the mdst file as argument.

The correct Global Tag must then be used in your steering file by assigning it
to the `conditions.globaltags <ConditionsConfiguration.globaltags>` list in the ``basf2`` namespace.

.. admonition:: Exercise
    :class: exercise stacked

    Look up the correct Global Tag for our mdst file using `b2conditionsdb-recommend<b2conditionsdb-recommend>`.
    The command will return multiple Global Tags, choose the one starting with ``analysis_tools``
    as this one contains the FEI's weight files.

    Include the Global Tag in your steering file.

.. admonition:: Hint
    :class: toggle xhint stacked

    Execute

    .. code-block:: bash

        b2conditionsdb-recommend /group/belle2/users/tenchini/prerelease-05-00-00a/charged/charged_eph3_BGx0_0.root

    The results are presented in one line separated by spaces. Pick the tag starting with ``analysis_tools`` and assign
    it to ``b2.conditions.globaltags``.

    Note: This variable always takes a **list** of tags.

.. admonition:: Solution
    :class: toggle solution

    The correct global tag is ``analysis_tools_release-04-02``

    Include it in the steering file like this:

    .. literalinclude:: steering_files/070_fei.py
        :language: python
        :lines: 25


Configuring the FEI
*******************

Now that all the prerequisites are there, we have to configure the FEI for our purposes. To do this,
we need to configure two objects:
The `fei.get_default_channels` function and the `fei.FeiConfiguration` object.

The `fei.get_default_channels` function controls which channels the FEI reconstructs. Disabling channels speeds up the FEI
so it makes sense to only select what you need by specifying the appropriate arguments.
As only charged B mesons are reconstructed in this exercise, the ``chargedB`` argument has to be set to True (default)
and the ``neutralB`` argument to False.
The same applies to the ``hadronic`` and ``semileptonic`` arguments, set ``hadronic=True`` and ``semileptonic=False``
as you will only reconstruct B mesons in hadronic decay channels.

.. admonition:: Exercise
    :class: exercise stacked

    Configure `fei.get_default_channels` as described above and assign it to a variable.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/070_fei.py
        :language: python
        :lines: 29-31


The `fei.FeiConfiguration` class controls the FEI's other configuration.
Here, the FEI monitoring should be disabled with the appropriate argument (``monitor=False``) as we are not interested in
the internal performance characteristics of the FEI's stages.
We also have to specify the FEI prefix argument here. This prefix allows distinguishing between different trainings
in a single Global Tag and is ``prefix=FEIv4_2020_MC13_release_04_01_01`` for the current central training.


.. admonition:: Exercise
    :class: exercise stacked

    Configure `fei.FeiConfiguration` as described above and assign it to a variable.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/070_fei.py
        :language: python
        :lines: 34-36

The configuration created above must now be turned into a ``basf2`` path which can be appended to the main path.
This is done with the `fei.get_path` function which takes the channel configuration
and the general FEI configuration as arguments and returns a `FeiState` object.
The ``path`` attribute of this newly-created `FeiState` (e.g. ``feistate.path``) is then appended
to the main path with the `basf2.Path.add_path` method.

.. admonition:: Exercise
    :class: exercise stacked

    Create the FEI path with `fei.get_path()` and use its `basf2.Path.add_path`
    method to add it to your main path.

.. admonition:: Hint
    :class: toggle xhint stacked

    The syntax is ``mainpath.add_path(feipath)``.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/070_fei.py
        :language: python
        :lines: 38-42


You have now successfully added the FEI to the main path. The FEI will add a particle list
to the datastore called ``B+:generic``. In addition to the usual variables, the B meson candidates in the particle list
will also have two ``extraInfo`` variables set:
The ``extraInfo(decayModeID)`` and the ``extraInfo(SignalProbability)`` variables. The first one specifies the decay
channel in which the B meson has been reconstructed, the second one is the output of the B meson classifier. This value
can be used to select B meson candidates to which the FEI assigns a higher probability.

Final Steps
***********

What remains now is adding Monte Carlo matching and writing the ``B+:generic`` list to a ``.root`` file.
You should already be familiar with both topics from the previous exercises.

.. admonition:: Exercise
    :class: exercise stacked

    Add MC matching to the ``B+:generic`` particle list.

    Then, write out the ``B+:generic`` particle list to a ``root`` file.
    Interesting variables are `Mbc`, `deltaE`,
    ``extraInfo(decayModeID)``, ``extraInfo(SignalProbability)`` and `isSignal`.

    Finally, start the event loop with a call to `basf2.process`.

.. admonition:: Hint
    :class: toggle xhint stacked

    Go back to :ref:`onlinebook_first_steering_file` to see the step-by-step instructions on how to add MC matching,
    write the ntuple and start the event loop.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/070_fei.py
        :language: python
        :lines: 45-46,56-64,66-73

The FEI returns not only one B meson candidate for each event but up to 20. Using the `modularAnalysis.rankByHighest`
function, it is possible to rank the candidates by the B meson classifier output in the
``extraInfo(SignalProbability)`` variable. This is optional but often useful to select the best, i.e. most likely correct,
candidate.

.. admonition:: Exercise (optional)
    :class: exercise stacked

    Use ``rankByHighest`` to rank the B meson candidates in the ``B+:generic`` list by the
    ``extraInfo(SignalProbability)`` variable. Write the rank to a new variable called ``FEIProbabilityRank``.

    Don't forget to write this variable to your ntuple (within an ``extraInfo`` metavariable).

.. admonition:: Hint
    :class: toggle xhint stacked

    You should already be familiar with Best Candidate Selection from the :ref:`onlinebook_various_additions` lesson.
    The documentation on ``rankByHighest`` can be found here: `modularAnalysis.rankByHighest`.

    The variable in `variablesToNtuple` should be called ``extraInfo(FEIProbabilityRank)``.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/070_fei.py
        :language: python
        :lines: 48-73

You can now execute you steering file which should look somewhat like this:

.. admonition:: Final steering file
    :class: toggle solution

    .. literalinclude:: steering_files/070_fei.py
        :language: python



Offline analysis
****************

Now that you have created your ntuple, we can have a look at the properties of the B mesons we have created.

The beam-constrained mass `Mbc` which we have written to our ntuple is defined as

.. math::

    M_{bc} = \sqrt{E_{\text{beam}}^2 - \mathbf{p}_{B}^2}

For correctly reconstructed B mesons, this variable should peak at the B meson mass.


.. admonition:: Exercise
    :class: exercise stacked

    Load your ntuple file into python, either using ``root_pandas`` or ``uproot``.
    Then, plot the distribution of `Mbc` from 5.15 - 5.3 GeV.

    You should see broad peak with a sharp drop-off below 5.2 GeV.
    This drop-off is caused by a fixed pre-cut in the FEI. Candidates below this threshold are rejected before
    classification as they are always incorrect.

.. admonition:: Solution
    :class: toggle solution

    .. code-block:: python

        # Include this only if running in a Jupyter notebook
        %matplotlib inline

        from root_pandas import read_root

        df = read_root('B_charged_hadronic.root')

        ax = df.hist(df['Mbc'], 30, range=(5.15, 5.3))
        ax.set_xlabel(r'$\mathrm{M}_{\mathrm{bc}}$ / GeV')
        ax.set_ylabel('Number of candidates')

        ax.figure.savefig('m_bc.pdf')


.. admonition:: Question
    :class: exercise stacked

    The distribution of `Mbc` you just plotted doesn't peak at the B meson mass of 5.28 GeV. Why is that?

.. admonition:: Solution
    :class: toggle solution

    We haven't really used the classifier output of the FEI yet. The up to 20 candidates in each event are selected by
    FEI Signal Probability but many still have low absolute classifier values and by definition almost all of them are
    misreconstructed.


.. admonition:: Exercise
    :class: exercise stacked

    Increase the signal purity by requiring minimum values of ``extraInfo__boSignalProbability__bc``. Try cuts at 0.001,
    0.01 and 0.1!

    You can also try selecting the candidate with the highest FEI Probability in each event by using the
    ``extraInfo__boFEIProbabilityRank__bc`` variable created with `modularAnalysis.rankByHighest`
    (if you have done this).

    These selections should increase the purity of the B meson candidates under consideration and lead to a sharper
    peak at the B mass.

    You can also have a look at the correctly reconstructed B mesons by requiring ``isSignal == 1``. By comparing this
    to the cuts on the FEI classifier you can see how well the FEI identifies correctly reconstructed B mesons.


.. admonition:: Solution
    :class: toggle solution

    .. code-block:: python

        fig, ax = plt.subplots()

        # If you didn't do the optional exercise, the extraInfo__boFEIProbabilityRank__bc column won't be there.
        # Just remove this part of the query in this case.
        sig_prob_cut_df = df.query('(extraInfo__boSignalProbability__bc > 0.01)
                                    & (extraInfo__boFEIProbabilityRank__bc == 1)')

        n, bins, patches = ax.hist(sig_prob_cut_df['Mbc'], 30, range=(5.15, 5.3))
        ax.set_xlabel(r'$\mathrm{M}_{\mathrm{bc}}$ / GeV')
        ax.set_ylabel('Total number of candidates')
        ax.set_title('SigProb > 0.01')

        plt.show()

Congratulations, you have now discovered the B meson in Monte Carlo data!
This concludes the first part of this lesson. The second part of this lesson will show you now how to use
the reconstructed B\ :sub:`tag` in you own analysis.


Reconstructing the full :math:`\Upsilon(4S)` event
--------------------------------------------------


The FEI skim
************

You might have noticed that applying the FEI takes some time, even for the small file we have just processed.
For this reason and to save computing resources, datasets with pre-applied FEI tagging exist. These
preprocessed datasets are called *skims*. We will be using the FEI skim in this exercise.




.. admonition:: Key points
    :class: key-points

    * Get the weight files from the conditions database
    * Add the FEI to your path with `fei.get_default_channels` and `fei.FeiConfiguration`.
    * FEI Purity and efficiency are controlled by a cut on ``extraInfo(SignalProbability)``

.. topic:: Author of this lesson

    Moritz Bauer
