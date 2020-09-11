.. _onlinebook_roe:

The Rest of Event (ROE)
=======================

.. include:: ../todo.rst

.. sidebar:: Overview
    :class: overview

    **Teaching**: 10-20 min 

    **Exercises**: None

    **Prerequisites**: 
    	
    	* The previous lesson

    **Questions**:

        * I combined several particles into ``X``. How do I select everything
          that is not "in" ``X``?
        * What is an ROE mask and why do I need them? 

    **Objectives**:

        * Learn the basics of ROE
        * Get the best ROE 4-momentum resolution
        * Learn about neat tips and tricks in ROE algorithms


.. admonition:: Key points
    :class: key-points

    * The ROE of a selection is build with `buildRestOfEvent`
    * ROE masks are added with `appendROEMask`. Use them to clean up detector noise.
    * For many analyses ROE is used as middleware to get tag vertex fit, continuum suppression or flavor tag. 
    * Usage of ROE without a mask is not recommended. 

After the reconstruction of the signal particle list it is very useful 
to look into the the particles that are not associated to the signal particle list.
In `basf2` these particles are called "Rest of Event" and this is the main topic of the chapter.

The Rest of Event (ROE) contains a lot of information, because in case of B-physics, it includes 
particles from the partner B-meson and in case of charm and tau analysis, it has the partner 
charm and tau-lepton, respectively. 
However, the ROE contains not only the partner particle, but also all other particles, like 
hadron split-off particles, :math:`\delta`-rays, unused radiative photons, beam-induced background particles
or products of kaon or pion decays. Thus, it is important to clean up the ROE particles 
to get the best possible use of it.

Basics
------

In this chapter we will assume that you have successfully constructed your first 
`basf2` script and have your signal particle list in place. 
Let's assume that the signal particle list is called ``X:signal`` and already successfully loaded or 
reconstructed, then one has to have the following code to reconstruct the Rest of Event:

.. code-block:: python

        import basf2 as b2
        import modularAnalysis as ma
        main_path = b2.create_path()
        ma.buildRestOfEvent('X:signal', path=main_path)

That's it, the ROE has been reconstructed!
Behind these python curtains, a `RestOfEvent` object is created for each particle in ``X:signal`` 
particle list and it includes all other charged or neutral particles, that have not been 
associated to the corresponding signal candidate.

In principle, one can already try to use some of the Rest of Event variables, like ROE invariant 
mass :b2:var:`roeM` or ROE energy :b2:var:`roeE`, but we need to clean up the ROE particles first to
make the best use of them. 

Rest of Event masks
~~~~~~~~~~~~~~~~~~~

The main philosophy of the Rest of Event is to include *every* particle in the event,
that has not been associated to the signal candidate and it is up to the analyst to 
decide what particles are actually matter for the analysis.
That is why we have a concept of the ROE masks, which are just sets of selection cuts 
to be applied on the ROE particles.

.. code-block:: python

        import basf2 as b2
        import modularAnalysis as ma
        main_path = b2.create_path()
        ma.buildRestOfEvent('X:signal', path=main_path)
        charged_cuts = 'nCDCHits > 0'
        photon_cuts = 'clusterNHits > 1'
        ma.appendROEMask('X:signal', 'my_mask', 
                charged_cuts, photon_cuts)

Here we have created a mask with a name ``my_mask``, that will contain only track-based 
particles that have :b2:var:`nCDCHits` and ECL-based particles, that will have 
at least one ECL cluster hit. 

.. tip::

    These are example cuts, please use official guidelines from 
    Charged or Neutral Performance groups to set up your own selection.

The analyst can create as many ROE masks as needed and use them in different ROE-dependent 
algorithms or variables, like ``roeM(my_mask)`` or ``roeE(my_mask)``. Also, one can call 
:b2:var:`nROE_Charged` or :b2:var:`nROE_Photons` to know how many charged particles or 
photons entered the ROE. 

.. tip::

    There are also KLM-based hadrons in ROE, like :math:`K_L^0` or neutrons, but they are 
    not participating in ROE 4-momentum computation, because of various temporary 
    difficulties in KLM reconstruction. Nevertheless, one can count them using 
    :b2:var:`nROE_NeutralHadrons`. 

This concludes the Rest of Event setup as a middle stage algorithm to run :ref:`onlinebook_cs`, 
:ref:`onlinebook_flavor_tagging` or tag vertex fitting (reference needed).

Advanced use
------------

Best ROE 4-momentum resolution
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rest Of Event loop
~~~~~~~~~~~~~~~~~~

Nested Rest of Event
~~~~~~~~~~~~~~~~~~~~

Rest of Event as a particle
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rest Of Event from generated particles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Further Research and Developement
---------------------------------
