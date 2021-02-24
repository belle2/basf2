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

     You want to generate 10 events using the full Belle II geometry (experiment 0). Which configuration do you 
     have to use for ``EventInfoSetter``?

.. admonition:: Hint
     :class: toggle xhint stacked

     Execute ``basf2 -m EventInfoSetter`` to look at the names of the module's parameters.

.. admonition:: Solution
     :class: toggle solution

     You have to add to your steering path the following line:

     .. code-block:: python

        main.add_module('EventInfoSetter', evtNumList=[10], expList=[0])

     Alternatively, you can just add the module to the steering path without setting the module's parameters and
     you can set the number of events and the experiment number via command line using the ``basf2`` arguments:

     .. code-block:: bash

        basf2 my_steering_file.py -n 10 --experiment 0


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

   Do not try to generate a large sample: it will take a lot of time! For the exercise, limit the number
   of generated events to 10. If you want to produce a larger sample (e.g. 1000 events), consider to use
   a batch submission system (whose usage will be explained in next lessons: :ref:`onlinebook_bsub`
   for the KEKCC system and :ref:`onlinebook_htcondor`).


How to generate a signal MC sample and the decay files
------------------------------------------------------

In the last exercise we learnt how to generate a generic :math:`B^0 \bar{B}^0` event. Probably you are wondering
where the decay channels of the :math:`B^0` / :math:`\bar{B}^0`) and the respective branching fractions are defined,
or if it possible to specify one or few specific decay channels for the generated particles.

All the known decay channels of the generated :math:`B^0` and :math:`\bar{B}^0` mesons are defined in our
`main decay file`_. This file is automatically loaded by the function :py:func:`generators.add_evtgen_generator` and
it is then passed to the `EvtGen generator`_.

.. _main decay file: https://stash.desy.de/projects/B2/repos/software/browse/decfiles/dec/DECAY_BELLE2.DEC

.. _EvtGen generator: https://evtgen.hepforge.org/

The main decay file is very long and probably confusing for a newcomer. In order to correctly read and understand it,
let us now move to a different topic: how can we generate events specifying the decay channels of our interest?
In our internal jargon, this is called "generating signal MC events".

Suppose that we want to study the :math:`B^0 \to J/\Psi K^0_S` decay, with :math:`J/\Psi \to \mu^+ \mu^-` and
:math:`K^0_S \to \pi^+ \pi^-`. A decay file that we can use is the following one (copy it into a file named
``my_decay_file.dec`` or something similar):

.. code-block::
   
   # This is my signal decay file.

   Decay Upsilon(4S)
   1.0 B0 anti-B0  VSS_BMIX dm;
   Enddecay

   Decay B0
   1.0 Jpsi K_S0 PHSP;
   Enddecay
   CDecay anti-B0

   Decay Jpsi
   1.0 mu+ mu- VLL;
   Enddecay

   Decay K_S0
   1.0 pi+ pi- PHSP;
   Enddecay

   End
   

Looking at this simple decay file, it is not difficult to understand its structure:

* the decay of a specific particle ``X`` starts with ``Decay X`` and ends with ``Enddecay``;

* the decay channels are then defined by the lines starting with numbers and each line
  defining a decay channel must end with a ``;``;

* a decay channel is fully identified by: its branching fraction (``1.0`` in all the cases above), its final state
  (for the ``B0`` case it is ``Jpsi K_S0``) and the `EvtGen decay model`_ that we want to use for that specific
  final state; in this case we used several models: ``VSS_BMIX dm`` is a model for vector particles decaying into two
  scalars in which the :math:`B^0` / :math:`\bar{B}^0` mesons mixing is turned on, ``VLL`` is a model for vector
  particles decaying into two leptons, while ``PHSP`` is a simple phase space decay model.

.. _EvtGen decay model: https://evtgen.hepforge.org/doc/models.html

.. warning::

   It may be complicated figuring out which decay model you have to use for a specific decay channel. You can
   always refer to our `main decay file`_ or, even better, you can contact the `Data Production liasons`_ of your
   physics working group.


* in case you want to use for a particle the same final states (assuming charge-conjugation) of its charge-conjucated
  partner, you have to specify it using the ``CDecay`` identifier, followed by the charge-conjucated state;

* the decay file must end with ``End``;

* each line starting with a ``#`` is interpreted as a comment by ``EvtGen`` and it is thus ignored.

Probably you are also wondering where all the particle names are defined (while for ``B0`` and ``mu+`` the naming
scheme is quite obvious, this is not the same for ``K_S0``). These names are defined in the `evt.pdl`_ file.
Unfortunately this file is a bit complicated to fully understand at this stage, but the most important thing to note 
is that the particle names are defined in the fourth column of each line (``add particle P name``).

.. _evt.pdl: https://stash.desy.de/projects/B2/repos/software/browse/framework/particledb/data/evt.pdl


.. admonition:: Question
     :class: exercise stacked

     Which line of the previous steering file has to be modified in order to generate our signal sample? And how
     it has to be modified?

.. admonition:: Solution
     :class: toggle solution

     The line to be modified is:

     .. code-block:: python

        ge.add_evtgen_generator(path=main, finalstate='mixed')

     and it has to be replaced with:

     .. code-block:: python

        ge.add_evtgen_generator(path=main, finalstate='signal',
                                signaldecfile=b2.find_file('/path/to/my_decay_file.dec'))


.. admonition:: Question
     :class: exercise stacked

     Suppose that now you are interested to a different final state: :math:`B^0 \to \phi K^0_S`, with:
     :math:`\text{BF}(\phi \to K^+ K^-) = 1`, :math:`\text{BF}(K^0_S \to \pi^+ \pi^-) = 0.7` and
     :math:`\text{BF}(K^0_S \to \pi^0 \pi^0) = 0.3`. Which decay file would you write?

.. admonition:: Hint
     :class: toggle xhint stacked

     Have a look to the `evt.pdl`_ file to identify the names to be used for :math:`\phi`, :math:`K^+`
     and :math:`pi^0`.

.. admonition:: Solution
     :class: toggle solution

     .. code-block::
   
	Decay Upsilon(4S)
	1.0 B0 anti-B0  VSS_BMIX dm;
	Enddecay

	Decay B0
	1.0 phi K_S0 PHSP;
	Enddecay
	CDecay anti-B0

	Decay phi
	1.0 K+ K- VSS;
	Enddecay

	Decay K_S0
	0.7 pi+ pi- PHSP;
	0.3 pi0 pi0 PHSP;
	Enddecay
     
	End


.. include:: ../lesson_footer.rstinclude

.. topic:: Author of this lesson

   Giacomo "the goat" De Pietro
