.. _onlinebook_cs:

Continuum Suppression (CS)
==========================

.. sidebar:: Overview
    :class: overview

    **Teaching**:

    **Exercises**:

    **Prerequisites**:

    	* None

    **Questions**:

        * What is continuum?
        * How can I separate it from signal events?

    **Objectives**:

        * Suppress continuum


Introduction
------------------------------------------------------------------

Most e\ :sup:`+` e\ :sup:`-` interactions at Belle II do not result in a ϒ(4S) resonance
which then decays to two B mesons.

Of these non-ϒ(4S) events, those resulting in some state without hadrons are usually not problematic in analyses
looking for B decays as they are already rejected by the trigger.

Continuum events are more problematic. B meson candidates reconstructed from these decays show a broad distribution
in variables such as the beam-constrained mass which makes them difficult to separate and suppress
when extracting a signal component.

.. admonition:: Question
    :class: exercise stacked

    Do you still remember what continuum is?

.. admonition:: Hint
    :class: toggle xhint stacked

    Have a look back in :ref:`backgrounds` where this is introduced.

.. admonition:: Solution
    :class: toggle solution

    When we talk about continuum, we mean events with the process e\ :sup:`+` e\ :sup:`-` → qq,
    i.e. directly to some lighter hadrons without creating a ϒ(4S) resonance.

    In Belle II Monte Carlo, the centrally produced continuum samples are separated by their quark content and are
    called 'uubar', 'ddbar', 'ssbar' and 'ccbar'.

If variables which you already know from the previous exercises are bad at separating continuum and
BB events, which other properties of the events can we use?
The answer is the overall **shape** of the events, i.e. the momentum-weighted distribution of all particles in the
detector.

.. admonition:: Question
    :class: exercise stacked

    Which of these two pictures better represents the distribution (shape) of particles you would expect in a BB event?
    Which represents a continuum event?

    .. figure:: figs/continuum_without_labels.png
        :width: 40em
        :align: center

.. admonition:: Hint
    :class: toggle xhint stacked

    Think about the different masses of the Continuum hadrons compared to B mesons. How does this reflect in the
    momentum?

.. admonition:: Solution
    :class: toggle solution

    The continuum particles are strongly collimated due to the large available momentum for the decay to light hadrons.
    In contrast, the particles from the BB event are uniformly distributed.

    .. figure:: figs/continuum_with_labels.png
        :width: 40em

        (Credit: Markus Röhrken)

So how do we get access to the event shape? We construct B candidates and then create a Rest of Event for them.
This allows us to study the entire event and compute shape properties,
while taking into account which particles belong to our signal reconstruction.

.. warning::

    In addition to the :ref:`analysis_continuumsuppression` tools that
    we will be using in this exercise, there is also the :ref:`analysis_eventshape` framework in
    basf2 which calculates similar properties to the Continuum Suppression module.
    However, this does not use candidate-based analysis and is not designed for Continuum Suppression.

    Always make sure the variables you're using in the exercise are from the Continuum Suppression module and not the
    similarly-named ones from the Event Shape Framework.

Which properties can we use?
A popular one is the ratio of the second and zeroth Fox-Wolfram moment:

.. math::

    R_2 = \frac{H_2}{H_0}

This variable is called `R2` in basf2 (not to be confused with `foxWolframR2` which is the same property
but from the Event Shape Framework).

Fox-Wolfram moments are rotationally-invariant parametrisations of the distribution of particles in an event.
They are defined by

.. math::

    H_l = \sum_{i,j} \frac{\lvert p_i \rvert \lvert p_j \rvert }{E^2_{\text{event}}} P_l(\cos{\theta_{i, j}})

with  the momenta p :sub:`i,j`, the angle θ :sub:`i,j` between them, the total energy in the event
E :sub:`event` and the Legendre Polynomials P :sub:`l`.

Other powerful properties are those based on the thrust vector. This is the vector along which the total projection
of a collection of momenta is maximised. This collection of momenta can be the B candidate or the rest of event.

The cosine of the angle between both thrust vectors, `cosTBTO` in basf2, is a thrust-based discriminating variable.
In BB events, the particles are almost at rest and so the thrust vectors are uniformly distributed. Therefore,
`cosTBTO` will also be uniformly distributed between 0 and 1.
In qq events, the particles are collimated and the thrust axes point back-to-back, leading to a peak at high values of
`cosTBTO`.
A similar argument can be made for the angle of the thrust axis with the beam axis which is `cosTBz` in basf2.

In addition to the angular quantities, basf2 also provides the total thrust magnitude of both the B candidate `thrustBm`
and the ROE `thrustOm`. Depending on the signal process, these can also provide some discriminating power.

If you would like to know more, Chapter 9 of `The Physics of the B Factories book <https://arxiv.org/abs/1406.6311>`_
has an extensive overview over these quantities.


.. admonition:: Question
    :class: exercise stacked

    Can you find out which other variables are provided by basf2 for continuum suppression?

.. admonition:: Hint
    :class: toggle xhint stacked

    Check the Continuum Suppression variable group in :ref:`analysis/doc/index-01-analysis:Variables`.

.. admonition:: Solution
    :class: toggle solution

    In addition to the five variables

    * `R2`
    * `cosTBTO`
    * `cosTBz`
    * `thrustBm`
    * `thrustOm`

    mentioned above, basf2 also provides "CLEO cones" (`CleoConeCS`) and
    "Kakuno-Super-Fox-Wolfram" variables (`KSFWVariables`). These are more complex engineered variables and
    are mostly used with machine learning methods.

First Continuum Suppression steps in basf2
------------------------------------------

Now, how do we access the shape of events in basf2?

First we need some data. In this exercise we will use two samples, one with "uubar" continuum background and one
with B → K :sub:`S` :sup:`0` 𝜋 :sup:`0` decays. These samples are called ``uubar_sample.root`` and
``B02ks0pi0_sample.root`` and can be used with the `basf2.find_file` function
(you need the ``data_type='examples'`` switch and also have to prepend ``starterkit/2021/`` to the filename).
If this doesn't work you can find the files in ``/sw/belle2/examples-data/starterkit/2021`` on KEKCC.


.. admonition:: Exercise
    :class: exercise stacked

    Load the mdst files mentioned above, then reconstruct Kshort candidates from two charged pions.
    Load the charged pions with the cut ``'chiProb > 0.001 and pionID > 0.5'`` and combine only pions whose combined
    invariant mass is within 36 MeV of the neutral kaon mass (498 MeV).
    We won't be using the Kshorts from the `stdV0s` package as these are always vertex fit which we don't need.

    Then, load some neutral pion candidates from `stdPi0s` and combine them with the Kshort candidates to
    B0 candidates. Only create B0 candidates with `Mbc` between 5.1 GeV and 5.3 GeV and `deltaE`
    between -2 GeV and 2 GeV.

    These cuts are quite loose but this way you will be able to reconstruct B0 candidates from continuum events without
    processing large amounts of continuum Monte Carlo.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/090_cs.py
        :language: python
        :lines: -34


.. admonition:: Exercise
    :class: exercise stacked

    Now, create a Rest of Event for the B0 candidates and append a mask with the track cuts
    ``'nCDCHits > 0 and useCMSFrame(p)<=3.2'`` and the cluster cuts ``'p >= 0.05 and useCMSFrame(p)<=3.2'`` to it.
    These cuts are common choices for continuum suppression, however they might not be the best ones for your analysis
    later on!

    Then, adding the continuum suppression module is just a single call to the
    `modularAnalysis.buildContinuumSuppression` function. You have to pass the name of the ROE mask you've just created
    to the function.

.. admonition:: Hint
    :class: toggle xhint stacked

    You can use `modularAnalysis.appendROEMasks` to add the mask.

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/090_cs.py
        :language: python
        :lines: 35-44


.. admonition:: Exercise
    :class: exercise stacked

    Now you can write out a few event shape properties. Use the five properties mentioned above. To evaluate the
    performance of these variables, add the truth-variable `isContinuumEvent`.

    You can also add the beam-constrained mass `Mbc` which you should know from previous exercises to see the uniform
    background component in this variable.

    Then, process the path and run the steering file!

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/090_cs.py
        :language: python
        :lines: 46-62

Now that we have created our ntuple, we can look at the data and see how well the variables suppress continuum.

.. admonition:: Exercise
    :class: exercise stacked

    Plot the distributions of R2 from 0 to 1 for both continuum and signal components as
    determined by `isContinuumEvent`.

    Where would you put the cut when trying to retain as much signal as possible?

    If you want you can also plot the other four variables and see how their performance compares.


.. admonition:: Hint
    :class: toggle xhint stacked

    Use ``histtype='step'`` when plotting with matplotlib, this makes it easier to see the difference between the two
    distributions.

.. admonition:: Solution
    :class: toggle solution

    .. code-block:: python

        # Include this only if running in a Jupyter notebook
        %matplotlib inline

        import matplotlib.pyplot as plt
        from root_pandas import read_root

        df = read_root('ContinuumSuppression.root')

        fig, ax = plt.subplots()

        signal_df = df.query('(isContinuumEvent == 0.0)')
        continuum_df = df.query('(isContinuumEvent == 1.0)')

        n, bins, patches = ax.hist(signal_df['R2'], bins=30, range=(0, 1), label='Not Continuum', histtype='step')
        n, bins, patches = ax.hist(continuum_df['R2'], bins=30, range=(0, 1), label='Continuum', histtype='step')
        ax.set_xlabel('R2')
        ax.set_ylabel('Total number of candidates')
        ax.legend()
        fig.savefig('R2.pdf')


    Your plot should look similar to this:

    .. figure:: figs/R2_uubar.png
        :width: 40em
        :align: center

    Judging by this plot, a cut at R2 = 0.4 would provide good separation. Of course, this can change if you employ cuts
    on other CS variables too!

.. admonition:: Exercise
    :class: exercise stacked

    In the previous exercise we have used a ``uubar`` sample as our continuum sample. How would you expect the
    distribution in `R2` to change when we switch this out with a ``ccbar`` sample? Think about this for a bit,
    then try it! You can use the file ``ccbar_sample.root`` in the starterkit folder.

.. admonition:: Solution
    :class: toggle solution

    The separation becomes worse as the charmed hadrons are heavier and have less momentum:

    .. figure:: figs/R2_ccbar.png
        :width: 40em
        :align: center

So how do we separate our signal component from continuum background in the presence of all types of continuum? As you
have seen with the five variables we have introduced so far, none of them can provide perfect separation.
Fortunately, there is a solution to this: Boosted Decision Trees!


Continuum suppression using Boosted Decision Trees
__________________________________________________

Boosted Decision Trees (DT) are a specific type of a machine learning model used for classification tasks. Its goal is to predict the value of a target variable based on the values of the input variables.

"Decision tree" refers to the general structure: the classification is done with a series of "decisions". "Decisions" are logical operations (like ">", "<", "=", etc.) on the features of each data point, where the data points are separated into groups judging by the outcome. Each outcome has a separate line of "decisions" following it. Maximum number of such "decisions" is called the "tree depth".

"Boosted" refers to the specific way the tree is formed: gradient boosting. Gradient boosting means, that a final tree is made by combining a series of smaller trees of a fixed depth. 

In our case BDT is used directly to distinguish between continuum and non-continuum events. The target variable is the "Continuum probability" - the probability of an event being a continuum event. The input variables can be in principle any varible that distinguishes between continuum and non-continuum events. The recommended variables and the most used are the ones introduced in the previous lesson as well as others from the "Continuum Suppression" section of the Variables. As BDT is a supervised machine learning method, there are three steps needed to put it to use:

1. Create learning dataset.
2. Make the algorithm "learn" and output a decision tree, that we can use.
3. Run the raw data over a trained decision tree.

When passing data over the last step, the BDT will write out a continuum probability, which then can be stored in the Ntuples. The actual cutting out of the continuum at the end narrows down to putting a selection criteria on the continuum probability. 

.. include:: ../lesson_footer.rstinclude

.. topic:: Authors of this lesson

    Moritz Bauer
