.. _onlinebook_fundamentals_introduction:

Introduction
============

.. sidebar:: Overview
    :class: overview

    **Teaching**: 15 min

    **Prerequisites**:

        * What is a B factory?

    **Objectives**:

        * Understand what is meant by signal and background
        * What is a cut.
        * What different background types are there.

Almost regardless of the quantity you are going to measure in your analysis, you
will have to face some basic problems:

*  select events you want to study (the **signal**) over similar events that
   mimic them (the **background**, more about this later),
*  estimate efficiency of your selection,
*  (possibly) estimate the intrinsic resolution of the quantities you will measure,
*  and finally you will want to count how many signal events you observe,

   - or measure other quantities like an invariant mass (i.e. the position of a peak)
   - or a polarization from an angular distribution.


Cuts and signal selection
-------------------------

The most basic way to select a signal is to apply what, in jargon, are called
"cuts".
A cut is nothing but selection, usually binary, over one quantity that has some
separation power between signal and background. Of course multiples cuts can be
applied in sequence, leading to quite effective background reductions.
Before deciding on the selection criteria however, one must define the variable
that will be used to count of many signal events are left. A good variable has a
very peaking distribution for signal, and a smooth, uniform distribution for the
background.

.. admonition:: Example

    In many b-physics analyses you will make candidate B mesons from
    combinations of tracks and calorimeter clusters.
    You will work through an example of this in a :ref:`later lesson
    <onlinebook_first_steering_file>`, but for now, consider the energy of your
    candidate B in the centre-of-momentum system.

    .. admonition:: Question
        :class: exercise stacked

        What is the the energy of a B meson produced in the decay of an
        :math:`\Upsilon(4S)` in the rest frame of the :math:`\Upsilon`?

    .. admonition:: Solution
        :class: toggle solution

        Half of its rest mass: :math:`\sim 5.3\ \textrm{GeV}`

    The difference between half of the energy in the centre-of-momentum
    and the total energy of the B candidate is called :math:`\Delta E`.
    An early  **cut**  you might make is to require that this quantity
    is close to zero.

    That is, you would accept B meson candidates which satisfy:
    :math:`-150\ \textrm{MeV} < \Delta E < 150\ \textrm{MeV}`
    and reject those which don't.

.. _backgrounds:

Backgrounds, backgrounds, backgrounds
-------------------------------------

An interesting event for most B physics analyses is one where the
:math:`e^+e^-` produced an :math:`\Upsilon(4S)`, which subsequently decay
into a :math:`B\bar{B}` meson pair.
However this is not the most probable result in an :math:`e^+e^-` collision.

.. admonition:: Question
    :class: exercise stacked

    What is the most likely final state for an :math:`e^+e^-` collision at
    :math:`\sqrt{s}=10` GeV?
    What is its cross section?
    Also look up the cross section for hadronic events and for
    :math:`B\bar B` hadronic events.

.. admonition:: Hint
    :class: toggle xhint stacked

    You should be able to find this information on confluence.

.. admonition:: Another hint
    :class: toggle xhint stacked

    Probably you are looking for `this page
    <https://confluence.desy.de/x/AQkWAg>`_.

.. admonition:: Solution
    :class: toggle solution

    At around 125 nb, the most probably process is :math:`e^+e^-\to e^+e^-`.
    The cross section for hadronic events is around 5.8 nb, the cross section
    for :math:`B\bar B` hadronic events is around 1.1 nb.


We call anything that is not "what you want to analyse": **background**.
But this is a bit of a sloppy definition.
In fact, you will encounter roughly four things in a Belle II analysis that
people call "background".
It depends a bit on how one counts.

.. warning::

    We will always specify in these lessons.
    But in your working life (in meetings etc), you might hear the word
    "background" and you will need to infer from the context precisely what is
    being discussed.

The example we've just discussed (such as :math:`e^+e^- \to e^+e^-`) are
background *events* or background *processes*.
These are relatively easy to reject and can be done in the **trigger** or by
rather simple cuts.
More on this :ref:`later on in this lesson
<onlinebook_fundamentals_triggers_filters>`.
You don't need to worry too much about these if you are doing B physics.
But these background processes can be important for low-multiplicity analyses.

The second kind of background arises from physics processes that mimic your
signal.

.. admonition:: Example

    If you want to analyse :math:`B\to K^{(*)}\ell^+\ell^-` decays then you
    would be concerned with the (much higher branching fraction)
    :math:`B\to J/\psi K^{(*)}` process where the :math:`J/\psi` subsequently
    decays to a pair of leptons.

    Most people would call this a "physics background".

You would also get backgrounds of this second kind where there was some particle
mis-identification or mis-reconstruction.

The third kind of background arises from the non-resonant :math:`e^+e^- \to q\bar{q}`
hadronic events.
As you saw in the exercises before, :math:`B\bar B` is only part of the hadronic
cross section.
You will also get hadronisation of light quarks (:math:`uds`), and the charm
quark (which is a background to B physics, for example, but obviously the signal
for charm physics measurements).
These hadronic events produce many tracks (around 10 or 11) per event.
You are therefore, just by probability, likely to find some combination of
genuine tracks and clusters that mimic your signal but aren't from a :math:`B`
decay.
We call this **continuum background**.

This background can be suppressed to a certain extent, although many analyses
leave some part of this background in the data sample as it is relatively
straightforward to model and cutting too strictly on continuum suppression
variables will hurt signal efficiency at some stage.
You will have a lesson about :ref:`continuum suppression <onlinebook_cs>` and
:ref:`examples of modelling <onlinebook_fitting>` later in these tutorials.

The fourth thing people will refer to as "background" is something rather
different.
**Beam-induced background** are tracks and clusters that are not produced from
the primary :math:`e^+e^-` collision, but from other interactions in the beam
itself.
These are more prevalent in Belle II compared to Belle (and previous
experiments) since the beams are significantly more focused at SuperKEKB.
Beam background tracks and clusters are rather easy to reject at the final
stages of an analysis the presence of such tracks and clusters is usually
tolerable (you can just ignore them).
They are, however, relevant during reconstruction and in the high-level trigger.

It might be obvious but let us state an obvious thing: even events that are
really from your signal can contain these background clusters and tracks.
You do not need to reject the whole event just because of some beam background.

.. tip::

    You should discuss the backgrounds that you are expecting to encounter in
    your analysis with your supervisor.
    This is a very important and useful conversation.

.. admonition:: Key points
    :class: key-points

    There are four(ish) important kinds of "background".

    1. Trigger background and background processes.
    2. Physics background (more of a problem when you get into your analysis).
    3. Continuum background (from :math:`uds` and maybe :math:`c`).
    4. Beam-induced background.

.. include:: ../lesson_footer.rstinclude

.. topic:: Author(s) of this lesson

    Umberto Tamponi,
    Martin Ritter,
    Oskar Hartbrich,
    Michael Eliachevitch,
    Sam Cunliffe
