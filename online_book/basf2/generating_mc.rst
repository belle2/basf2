.. _onlinebook_generating_mc:

Generating Montecarlo
=====================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 

    **Exercises**: 

    **Prerequisites**: 
    	
    	* The basics.
	* First steering file.

    **Questions**:

        * How can I generate my own MC?

    **Objectives**:

        * Generate MC for the decay :math:`X\to Y`


Why do we need Montecarlo simulated data?
-----------------------------------------

Montecarlo (MC) data are widely used in High Energy Physics. MC simulations allows us to mimic what happens in the
Belle II detector after the collisions to understand the experimental conditions and performance. MC data
are processed as real data in both reconstruction and physics analyses, with the advantage that we know the "truth"
(e.g. if a track observed in the CDC was originated by a muon or a pion). More realistic is the MC simulation,
better we can interpret the results obtained by analyzing collisons events.

Usually we need large MC samples (larger than the collisions dataset we want to use for the physics analysis) to
avoid spurious fluctuations and to reduce uncertainties driven by the size of the MC sample. But we do not have to
generate by our self the MC samples that we need for our analysis: this is one of the duty of the Data Production
group. Each physics Working Group has a dedicated person (the `Data Production liason`_) to assist the analysers
and to collect their requests concerning the MC production.

There are several reasons for discouraging the self-production of very large MC samples:

* our own-generated MC sample may use an inconsistent version of the software (or conditions database, beam
  background, generator settings, etc.) with respect to the ones produced centrally and used by our colleagues;

* we do not have the computing resources to produce large MC samples; the Data Production group uses the grid
  to properly handle a MC production campaign, while we can not; moreover, using a batch submission system for a
  private large production is not a good idea, because it takes a lot of time and, more important, we will subtract
  important computing resources to our colleaguers and to their jobs;

* we do not have the space to store a large number of MC samples.


.. admonition:: Question
     :class: exercise stacked

     How much disk space would you need to store mDST files equivalent to :math:`500\;\text{fb}^{-1}` of generic
     :math:`B^0 \bar{B}^0` events?

.. admonition:: Solution
     :class: toggle solution

     Assuming that the mDST size for a single :math:`B^0 \bar{B}^0` event is :math:`10\;\text{kB}`, and knowing
     that the cross section for the production of :math:`B^0 \bar{B}^0` events at Belle II is
     :math:`0.540\;\text{nb}`, we would need about :math:`2.55\;\text{TB}` for storing such a sample.


However, we may need to produce a small MC sample by our-self: for example, we want to test a new feature of the 
detector simulation, or we want to test the efficiency of a new trigger line, and so on.


.. admonition:: Key points
    :class: key-points

    * Don't generate larger samples yourself, contact the `Data Production liason`_ of your physics Working Group
      if you need a large production of MC signal samples!

      .. _Data production liason: https://confluence.desy.de/display/BI/Data+production+WebHome


Main steps for the MC production
--------------------------------

The production of a MC sample can be divided in three main steps:

* the generation of the MC particles using a MC generator; we did not develop within Belle II the generators that we are 
  currently using, but we inherit them from the HEP community;

* the simulation of the detector response and of the L1 trigger; the simulation of the interaction of the MC particles
  with the matter is handled by Geant4, while the digitization of the simulated hits is handled by subdetectors' modules;

* the reconstruction of tracks, electromagnetic clusters, etc.; the same modules and algorithms used at this step are used
  also for reconstructing collisions events.


.. warning::

   We can produce MC samples only by using a full release, and not a light one. In a light release we do not have
   the generators, simulation and reconstruction packages, meaning that running with a light release a steering file
   intended for some MC production will result in a ``basf2`` crash.


How to write a steering file for the MC production
--------------------------------------------------

Of course, for the production of a MC sample we have to write ``basf2`` a steering file. Let's see how to write it.

As usual, one has to start by importing ``basf2`` and creating a main path.

.. code-block:: python

   import basf2 as b2
   main = b2.Path()

Then, the first module to be added to the main path *must* be ``EventInfoSetter``. This module is necessary for 
correctly setting some of the production parameters, like the number of events to be produced or the experiment 
number.

.. admonition:: Question
     :class: exercise stacked

     You want to generate 1000 events using the full Belle II geometry (experiment 0). Which configuration do you 
     have to use for ``EventInfoSetter``?

.. admonition:: Hint
     :class: toggle xhint stacked

     Execute ``basf2 -m EventInfoSetter`` to look at the names of the module's parameters.

.. admonition:: Solution
     :class: toggle solution

     You have to add to your steering path the following line:

     .. code-block:: python

        main.add_module('EventInfoSetter', evtNumList=[1000], expList=[0])

     Alternatively, you can just add the module to the steering path without setting the module's parameters and
     you can set the number of events and the experiment number via command line using the ``basf2`` arguments:

     .. code-block:: bash

        basf2 my_steering_file.py -n 1000 --experiment 0


Now we have to add to the steering path the relevant functions for:

* generating MC events (for example, some :math:`B^0 \bar{B}^0` events): :py:func:`generators.add_evtgen_generator`

* simulating the detector response: :py:func:`simulation.add_simulation`

* simulating the L1 trigger: :py:func:`L1trigger.add_tsim`

* reconstructing the relevant objects, like tracks, electromagnetic clusters, etc.: :py:func:`reconstruction.add_reconstruction`

* storing the reconstructed objects into an output mDST file for further studies: :py:func:`mdst.add_mdst_output`


.. admonition:: Question
     :class: exercise stacked

     Can you figure out how to add all the previous functions to your steering file in order to produce a small sample
     of :math:`B^0 \bar{B}^0` MC events?
     
.. admonition:: Hint
     :class: toggle xhint stacked

     Do not forget to import all the relevant python modules (for example, ``import L1trigger`` for ``add_tsim()``).

.. admonition:: Solution
     :class: toggle solution

     .. literalinclude:: steering_files/069_generate_mc.py
        :language: python
        :lines: 1-35


.. warning::

   Do not try to generate a large sample: it will take a lot of time! For the excerise, limit the number
   of generated events to 10. If you want to produce a larger sample (e.g. 1000 events), consider to use
   a batch submission system (whose usage will be explained in next lessons: :ref:`onlinebook_bsub`
   for the KEKCC system and :ref:`onlinebook_htcondor`).


.. include:: ../lesson_footer.rstinclude

.. topic:: Author of this lesson

   Giacomo "the goat" De Pietro
