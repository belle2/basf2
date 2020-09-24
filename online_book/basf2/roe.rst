.. _onlinebook_roe:

The Rest of Event (ROE)
=======================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 10-20 min

    **Exercises**: 5-10 min

    **Prerequisites**:

    	* The previous lesson

    **Questions**:

        * I combined several particles into ``X``. How do I select everything
          that is not "in" ``X``?
        * How to exclude some particles from this Rest of Event / what is an ROE mask?

    **Objectives**:

        * Reconstruct the ROE of a B meson


After the reconstruction of the signal particle list it is very useful
to look into the the particles that are not associated to the signal particle list.
In `basf2` these particles are called "Rest of Event" and this is the main topic of the chapter.

The Rest of Event (ROE) can contain a lot of information: in case of B-physics, the ROE of one B-meson includes
particles from the partner B-meson and in case of charm and tau analysis, the ROE of the lepton has the partner
lepton.

.. admonition:: Exercise
     :class: exercise stacked clear

      Find the documentation of the ROE module. What are its use cases for tagged and untagged analyses?

.. admonition:: Hint
     :class: toggle xhint stacked

     It's included in the "Advanced Topics" section of the analysis module

.. admonition:: Solution
     :class: toggle solution

     Documentation is here: `RestOfEvent`.

Basics
------

In this chapter we will continue our work on the steering file from the last lesson.
Remember that you have reconstructed a ``B0`` particle list.
We now want to reconstruct the Rest of Event of the ``B0``.

.. admonition:: Exercise
     :class: exercise stacked

      Look up how to create a Rest Of Event for your particle list
      (it's a single line of code).
      Add the ``fillWithMostLikely=True`` option.

.. admonition:: Hint
     :class: toggle xhint stacked

     You have already found the `RestOfEvent` chapter in the last exercise.
     Take a look at the basic usage example.

.. admonition:: Solution
     :class: toggle solution

     .. literalinclude:: steering_files/029_roe.py
         :lines: 47-48
         :emphasize-lines: 48
         :lineno-start: 47

That's it, the ROE has been reconstructed!
Behind these python curtains, a ``RestOfEvent`` object is created for each particle in ``B0``
particle list and it includes all other charged or neutral particles, that have not been
associated to the corresponding signal candidate. By default, the charged particles assumed to be pions,
and the neutral particles have photon or :math:`K_L^0` hypothesis.

ROE variables
~~~~~~~~~~~~~

In principle, one can already try to use some of the Rest of Event variables.

.. admonition:: Exercise
     :class: exercise stacked

     Find documentation for Rest Of Event variables

.. admonition:: Hint
     :class: toggle xhint stacked

     One can use search feature in the basf2 documentation, or use offline help by typing ``basf2 variables.py``
     in bash terminal (for example ``basf2 variables.py | grep -i roe``).

.. admonition:: Solution
     :class: toggle solution

     The Rest Of Event variables are in Rest Of Event secection of `VariableManager` page,
     which starts from :b2:var:`bssMassDifference` variable.


Among the most universal and useful are ROE invariant mass :b2:var:`roeM` or ROE energy :b2:var:`roeE`. Also, one can call
:b2:var:`nROE_Charged` or :b2:var:`nROE_Photons` to know how many charged particles or
photons entered the ROE.

Remember that we were collecting all variables in the ``b_vars`` list.
Let's include the following lines to have a useful selection of them:

.. literalinclude:: steering_files/029_roe.py
     :lines: 61-68
     :lineno-start: 61

.. admonition:: Exercise
     :class: exercise

     Run your steering file and check that it completes without error.

In principle we could already start to do an analysis.
However, the ROE variables that we have just defined are not quite useful yet:
we first need to "clean up" the ROE.
For this, we define ROE masks.

ROE masks
~~~~~~~~~

The main philosophy of the ROE is to include *every* particle in the event,
that has not been associated to the signal candidate.
That is why a typical ROE contains not only the partner particle (e.g. the tag or signal B),
but also all other particles, like
hadron split-off particles, :math:`\delta`-rays, unused radiative photons, beam-induced background particles or products of kaon or pion decays.
It is up to the analyst to decide what particles actually matter for the analysis.
To "clean up" the ROE particles, *ROE masks* are used.

ROE masks are just sets of selection cuts
to be applied on the ROE particles.

For our example, let's start by defining the following selection cut strings:

.. literalinclude:: steering_files/029_roe.py
     :lines: 47-50
     :emphasize-lines: 50-51
     :lineno-start: 47


Here we create different cuts for charged particles, like electrons or charged pions, and for photons,
because of different methods of measurement used to detect these particles.

.. tip::

    These are example cuts, please use official guidelines from
    Charged or Neutral Performance groups to set up your own selection in a "real" analysis.


.. admonition:: Exercise
     :class: exercise stacked

     Create a ROE mask using the `charged_cuts` and `photon_cuts` strings with the
     `appendROEMask` or `appendROEMasks` function.

.. admonition:: Hint
     :class: toggle xhint stacked

     A mask is defined as a tuple with three values. Use `appendROEMasks` to
     "activate" it.

.. admonition:: Solution
     :class: toggle solution


     .. literalinclude:: steering_files/029_roe.py
         :lines: 47-52
         :emphasize-lines: 51-52
         :lineno-start: 47

Now we have created a mask with a name ``my_mask``, that will only allow track-based
particles that pass selection cuts ``track_based_cuts`` and ECL-based particles, that pass
``ecl_based_cuts``.

The analyst can create as many ROE masks as needed and use them in different ROE-dependent
algorithms or ROE variables.
For ROE variables, the mask is specified as an argument, like ``roeM(my_mask)``
or ``roeE(my_mask)``.

In the last section, we defined two lists of ROE variables (``roe_kinematics`` and ``roe_multiplicities``).
Now we want to have
the same variables but with the ``my_mask`` argument. Since we're lazy, we use a python
loop to insert this argument.

.. admonition:: Exercise
     :class: exercise stacked

     Write a ``for`` loop that runs over ``roe_kinematics + roe_multiplicities`` and
     replaces the ``()`` of each variable with ``(my_mask)``. Add these new
     variables to the ``b_vars`` list.

.. admonition:: Hint
     :class: xhint stacked toggle

     .. code-block:: python

        The variables are nothing more than a string, which has a ``replace``
        method:

        >>> "roeE()".replace("()", "(my_mask)")
        'roeE(my_mask)'

.. admonition:: Hint
     :class: xhint stacked toggle

     .. code-block:: python

        for roe_variable in roe_kinematics + roe_multiplicities:
            roe_variable_with_mask = <your code here>
            b_vars.append(roe_variable_with_mask)


.. admonition:: Solution
     :class: solution toggle

     .. literalinclude:: steering_files/029_roe.py
         :lines: 62-73
         :lineno-start: 62
         :emphasize-lines: 70-73


.. tip::

    There are also KLM-based hadrons in ROE, like :math:`K_L^0` or neutrons, but they are
    not participating in ROE 4-momentum computation, because of various temporary
    difficulties in KLM reconstruction. Nevertheless, one can count them using
    :b2:var:`nROE_NeutralHadrons` variable.

.. admonition:: Exercise
     :class: exercise stacked

     Your steering file is now complete. Run it!

.. admonition:: Solution
     :class: solution toggle

     Your steering file should look like this:

     .. literalinclude:: steering_files/029_roe.py
         :emphasize-lines: 47-52,61-73
         :linenos:


Quick plots
~~~~~~~~~~~



This concludes the Rest of Event setup as a middle stage algorithm to run :ref:`onlinebook_cs`,
:ref:`onlinebook_flavor_tagging` or tag :ref:`onlinebook_vertex_fitting`.

.. admonition:: Key points
    :class: key-points

    * The ROE of a selection is build with `buildRestOfEvent`
    * ROE masks are added with `appendROEMask` or `appendROEMasks`.
      Use them to clean up beam-induced or other background particles.
    * For many analyses ROE is used as middleware to get tag vertex fit,
      continuum suppression or flavor tag.
    * Usage of ROE without a mask is **not** recommended.


.. topic:: Authors of this lesson

    Sviatoslav Bilokin,
    Kilian Lieret
