.. _onlinebook_fundamentals_simulation:

Simulation: The Monte Carlo
===========================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 20 min

    **Prerequisites**:

        * :ref:`Fundamentals Introduction <onlinebook_fundamentals_introduction>`

    **Objectives**:

        * How to simulate physics processes.


We need to be able to compare data from our detector to the expectation we have.
In very rare cases this might not be necessary, for example the discovery of the
J/ψ was so clear a signal that we didn't need any comparison to understand
that it was something new. But most of the time we need to make sure what we see
is not some artefact of our very very complex experiment.

To do this we create simulated events which should behave as closely as possible
to the real detector events. This is done using sampling of random numbers
repeatedly and thus called the `Monte Carlo method <https://en.wikipedia.org/wiki/Monte_Carlo_method>`_.
In HEP we usually just call the whole process Monte Carlo or MC for short.

Now there are two parts of this procedure we need to distinguish: Generation of
an event and simulation of the event.

Event Generation
----------------

This is the physics part: the interaction we want to simulate. Given the initial
conditions of the electron and positron colliding we generate a number of
particles according to the physics model we want to study. This could be any
advanced physics model (SUSY, dark matter) or basic standard model physics.

It depends on the analysis: Usually we have specific samples for the decay we
analyse, the "signal MC". And we compare these to simulation of basic standard
model processes, the "generic MC". There might be additional simulations needed
for specific processes which we want to exclude in our analysis, the "background
MC".

For all these different samples the principle is the same: We generate positions
and four-vectors of particles according to a physics model. In Belle II this is
usually a very fast process and takes of the order of milliseconds per event to
generate.

There is a large variety of different generators for different use cases:
EvtGen, KKMC, Tauola, Madgraph, CRY, AAFH, babayaganlo, PHOKARA, ... . All
simulate specific physic processes and will be used for different use cases from
performance studies to different analysis types. There is an internal `Belle II
Note <https://docs.belle2.org/record/282?ln=en>`_ with more details if you're
interested.

Simulation
----------

After we generated the four-vectors of our event we need to now make it look
like output from the real detector. And the real detector measures the
interaction of these particles with the material of our detector: ionisation,
scintillation, bremsstrahlung, pair production, Cherenkov radiation and so forth.

All these processes are well known and can in be simulated. There has been a lot
of effort put into this by many experiments to create simulation software
capable of all of these processes. The most well known one is
`Geant4 <https://geant4.web.cern.ch/>`_ and we also use it in Belle II.

Geant4 takes the four-vectors and simulates their interaction with a virtual
Belle II detector. In the end we get deposited energy and particles produced by
the interactions in each sub detector.

On top of that we have custom software to convert the result from Geant4 into
signals as we see from the detector. For example the pixel detector software
will convert the energy deposited into information which pixels were fired.

Simulating the full detector is an expensive process and takes of the order of a
second for Belle II. For other experiments like ATLAS and CMS it can also get
close to minutes per event due to the much higher energy.

.. admonition:: Question
    :class: exercise stacked

    Assuming it takes one second per event, how long would it take to simulate
    all the 770 million :math:`B\bar{B}` events collected at Belle on one CPU?

    How long would it take to simulate all the :math:`B\bar{B}` events we
    intend to collect for Belle II?

.. admonition:: Hint
    :class: toggle xhint stacked

    You already know the cross section for :math:`B\bar{B}` events now you only
    need the planned total luminosity for Belle II.

.. admonition:: Solution
    :class: toggle solution

    Multiplying 770 million by one gives us 770 million seconds which is around
    8912 days or roughly 24 years.

    For Belle II we intend to collect :math:`50\ \textrm{ab}^{-1}` and the cross section
    is 1.1 nb. So we expect 55 billion :math:`B\bar{B}` events. Equivalent to
    636574 days or 1744 years.

.. admonition:: Question
    :class: exercise stacked

    Computing time doesn't come for free. Real numbers are hard to determine,
    especially for university operated computing centers. But in 2020 one hour
    of CPU time can be bought for around $0.025  on demand so lets take for a
    very quick estimate.

    How many CPUs do we need to buy in the cloud and how much would it cost to
    simulate the equivalent of :math:`50\ \textrm{ab}^{-1}` :math:`B\bar{B}`
    events in six months?

.. admonition:: Solution
    :class: toggle solution

    We need 55 billion seconds of CPU time, equivalent to 15.3 million hours. It
    would cost us roughly $382,000.

    Six months have roughly :math:`30 \times 6 \times 24 = 4320` hours
    so we need 3540 CPUs.

    Now bear in mind: this is only the simulation part, there is still more work
    to do during reconstruction as will be explained in the next section.

Differences between MC and real data
------------------------------------

Now after this simulation we have data which looks like what we might get from
the real detector and we can use it to compare our expectations to measurements.
But Geant4 uses an ideal detector description we put in. In reality the detector itself
consists of thousands of tons of hardware, some of it trying to measure
positions in micrometer precision. We don't know it perfectly and we cannot put
every little thing correctly in the simulation: We simply don't know the exact
material composition and place of every single screw precisely enough. And even
if we did this would slow down Geant4 massively because the system would become
much too complex to simulate.

There will thus always be simplifications we will have to live with but we need
to strive to make the differences as small as technically possible.

But especially in the early phases of the experiment we're still in the process
of understanding the real detector so we cannot have everything correct in the
MC yet. This is a long and tedious process where small detail in the detector
response need to be understood and modelled accordingly in the MC.

This is an ongoing work in the Performance group which tries to understand the
differences between MC and data by looking at specific samples and studies.


Generating MC samples
---------------------

As you saw above, generating sufficient MC is a tedious process which requires
large amount of CPU time. It also is prone to errors where something might not be
setup exactly correct. These mistakes would be costly for larger productions.

So we have the Data Production group to organize and manage the production of
large MC samples. They make sure that the requests of the physicists are met and
that the computing resources we have are not wasted.

.. seealso::

    You have already found the data production group confluence page.
    If not, take another look at :ref:`the previous lesson <onlinebook_collaborative_tools>`.
    Now might be a good time to bookmark or "watch" some pages.

.. admonition:: Key points
    :class: key-points

    * Simulated data (MC) is necessary to compare results to expectations
    * "Generation" is the first step to create particles according to some
      physics model
    * "Simulation" is then the simulation of these particles interacting with
      the matter in our the detector.
    * simulating large amounts of MC is expensive
    * there are always differences between MC and data, the Performance tries to
      understand, quantify and minimize them.
    * the data production group organizes and manages the MC production.

.. include:: ../lesson_footer.rstinclude

.. topic:: Author(s) of this lesson

    Umberto Tamponi,
    Martin Ritter,
    Oskar Hartbrich,
    Michael Eliachevitch,
    Sam Cunliffe
