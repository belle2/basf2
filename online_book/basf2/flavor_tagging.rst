.. _onlinebook_flavor_tagging:

Flavor tagging
==============

.. sidebar:: Overview
    :class: overview

    **Teaching**: 10 min

    **Exercises**: 15 min

    **Prerequisites**:

    	* The previous lesson

    **Questions**:

        * How can I distinguish between a :math:`B_\text{tag}^0` and a :math:`\bar B_\text{tag}^0`

    **Objectives**:

        * Perform flavor tagging

How can we distinguish between a :math:`B^0` and a :math:`\bar B^0`? This is not
as simple as the case where we have a :math:`B^+` vs a :math:`B^-` and can just
consider the charge of the particles. Yet determining this "flavor" of the
:math:`B` meson is crucial to look into CP violation.

This is where the `FlavorTagger` comes in. Used in an analysis, where we have
reconstructed a signal :math:`B_\text{sig}`, the `FlavorTagger` looks at the ROE
of the :math:`B_\text{sig}` (i.e. at the :math:`B_\text{tag}`) and applies
machine learning techniques (also called multivariate analysis) to determine the
flavor of the :math:`B_\text{tag}`.
In this lesson we will not train this model ourselves, but rather use
pre-computed weights. So where do we get them?

Luckily, such sets of weights are contained in the conditions database.
That means that we only need to use a specific global tag (if you forgot what
that is, consult this page again: :ref:`conditionsdb_overview`).

.. admonition:: Exercise
    :class: stacked exercise

    Add a line to your previous steering file that adds the
    ``analysis_tools_release-04-02`` global tag. For this you need to call the
    `prepend_globaltag` method of the `basf2.conditions` object (which is an
    instance of the `ConditionsConfiguration` class).

.. admonition:: Solution
    :class: solution toggle

    Simply add this to the top of your steering file:

    .. literalinclude:: steering_files/049_flavor_tagging.py
        :lines: 15-16
        :lineno-start: 15

Good! Now we have the weights and we have already created the ROE, so we can
start right away!

.. admonition:: Exercise
    :class: stacked exercise

    Call the `flavorTagger.flavorTagger` function (you need to specify your
    path and your particle list).

.. admonition:: Solution
    :class: toggle solution

    Remember to import the module:

    .. literalinclude:: steering_files/049_flavor_tagging.py
        :lines: 7
        :lineno-start: 7

    and call the function:

    .. literalinclude:: steering_files/049_flavor_tagging.py
        :lines: 77-78
        :lineno-start: 77

.. admonition:: Exercise
    :class: stacked exercise

    Add the the
    ``flavorTagger.flavor_tagging`` variable collection to your output
    variables

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: steering_files/049_flavor_tagging.py
        :lines: 105
        :lineno-start: 105

.. admonition:: Exercise
    :class: stacked exercise

    Run your steering file!

.. admonition:: Solution
    :class: toggle solution

    This is the full steering file at this point:

    .. literalinclude:: steering_files/049_flavor_tagging.py
        :linenos:

Good!
Now let's talk about the output of the flavor tagger. This is the value
:math:`q\cdot r`, where :math:`q=-1` corresponds to a :math:`\bar B^0` and
:math:`q=+1` to :math:`B^0`. :math:`r` is called the dilution factor. It's 0
if the algorighm can't decide between both options for :math:`q` and 1 if the
algorithm is certain about it's decision.

The variable ``FBDT_qrCombined`` is the :math:`q\cdot r` result of one of the
models of the `FlavorTagger` (a *fast boosted decision tree*).
It can also be ``NaN`` to signal that not a single charged
track in the ROE was found, so that the algorithm can't work.

.. note::

    In releases before release-05, a value of :math:`\pm 2` was used instead of
    ``NaN``.

Part of the variables you just added was also ``qrMC`` for the "true" (MC level) flavor of the :math:`B_\text{tag}`.
It can take the numbers
:math:`\pm 1`, as well as 0 (no flavor defined in the MC) and :math:`\pm 2`
(some problems with MC matching in the ROE).

That means that we can check how well our flavor tagger performed by comparing
it to ``FBDT_qrCombined``!

.. admonition:: Exercise (optional)
    :class: exercise

    Plot a histogram of ``FBDT_qrCombined`` and ``qrMC``.

.. admonition:: Exercise
    :class: exercise stacked

    Only consider candidates with clearly defined MC level flavor tag.
    Compare the output of ``qrMC`` to that of ``FBDT_qrCombined``.

.. admonition:: Hint
    :class: xhint stacked toggle

    For clearly defined MC flavor tag, you only have to distinguish between
    ``qrMC == 0`` and ``qrMC == 1``. For these two cases you can then plot
    the distribution of ``FBDT_qrCombined``.

.. admonition:: Solution
    :class: solution toggle

    .. literalinclude:: flavor_tagging/plot_flavor_tags.py
        :linenos:

    .. _flavor_plots:
    .. figure:: flavor_tagging/flavor_tags.svg
        :width: 400px
        :align: center
        :alt: MC vs reconstructed flavor tag

        MC flavor tag vs flavor tag from FBDT

    You can clearly see that the flavor tagger is by no means perfect, but
    definitely allows to do better than just guessing!

.. admonition:: Key points
    :class: key-points

    * The flavor tagger is used to discriminate between :math:`B_\text{tag}^0` and
      :math:`\bar B_\text{tag}^0`
    * The output is of the form :math:`\pm 1` times the confidence between 0 and
      1.

.. include:: ../lesson_footer.rstinclude

.. topic:: Authors of this lesson

    Kilian Lieret
