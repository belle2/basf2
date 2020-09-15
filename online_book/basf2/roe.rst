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
        * What is an ROE mask? 

    **Objectives**:

        * Learn the basics of ROE
        * Get the best ROE 4-momentum resolution


After the reconstruction of the signal particle list it is very useful 
to look into the the particles that are not associated to the signal particle list.
In `basf2` these particles are called "Rest of Event" and this is the main topic of the chapter.

The Rest of Event (ROE) contains a lot of information, because in case of B-physics, it includes 
particles from the partner B-meson and in case of charm and tau analysis, it has the partner 
charm and tau-lepton, respectively. 

.. admonition:: Exercise
     :class: exercise stacked

      Find the documentation of the ROE module and look up its use cases for both tagged and untagged analyses.

.. admonition:: Hint
     :class: toggle xhint stacked

     It's included in the "Advanced Topics" section of the analysis module
  
.. admonition:: Solution
     :class: toggle solution

     Documentation is here: `RestOfEvent`

Basics
------

In this chapter we will assume that you have successfully constructed your first 
`basf2` script and have your signal particle list in place. 
Lets assume that the signal particle list is called ``X:signal`` and already successfully loaded or 
reconstructed, then one has to have the following code to reconstruct the Rest of Event:

.. code-block:: python

        import basf2 as b2
        import modularAnalysis as ma
        main_path = b2.create_path()
        ma.buildRestOfEvent('X:signal', path=main_path)

That's it, the ROE has been reconstructed!
Behind these python curtains, a ``RestOfEvent`` object is created for each particle in ``X:signal`` 
particle list and it includes all other charged or neutral particles, that have not been 
associated to the corresponding signal candidate. By default, the charged particles assumed to be pions,
and the neutral particles have photon or :math:`K_L^0` hypothesis.

In principle, one can already try to use some of the Rest of Event variables, like ROE invariant 
mass :b2:var:`roeM` or ROE energy :b2:var:`roeE`, but we need to clean up the ROE particles first to
make the best use of them. 

Rest of Event masks
~~~~~~~~~~~~~~~~~~~

The main philosophy of the Rest of Event is to include *every* particle in the event,
that has not been associated to the signal candidate and it is up to the analyst to 
decide what particles are actually matter for the analysis.
That is why, a typical ROE contains not only the partner particle, but also all other particles, like 
hadron split-off particles, :math:`\delta`-rays, unused radiative photons, beam-induced background particles
or products of kaon or pion decays. Thus, it is important to clean up the ROE particles 
to get the best possible use of it.

That is why we have a concept of the ROE masks, which are just sets of selection cuts 
to be applied on the ROE particles. The ROE masks can be created in the following way:

.. code-block:: python

        import basf2 as b2
        import modularAnalysis as ma
        main_path = b2.create_path()
        ma.buildRestOfEvent('X:signal', path=main_path)
        charged_cuts = 'nCDCHits > 0'
        photon_cuts = 'clusterNHits > 1'
        ma.appendROEMask('X:signal', 'my_mask', 
                charged_cuts, photon_cuts, path=main_path)

Here we have created a mask with a name ``my_mask``, that will contain only track-based 
particles that have :b2:var:`nCDCHits` and ECL-based particles, that will have 
at least one ECL cluster hit. 

.. tip::

    These are example cuts, please use official guidelines from 
    Charged or Neutral Performance groups to set up your own selection.

The analyst can create as many ROE masks as needed and use them in different ROE-dependent 
algorithms or ROE variables, like ``roeM(my_mask)`` or ``roeE(my_mask)``. Also, one can call 
:b2:var:`nROE_Charged` or :b2:var:`nROE_Photons` to know how many charged particles or 
photons entered the ROE or the ROE mask. 

.. tip::

    There are also KLM-based hadrons in ROE, like :math:`K_L^0` or neutrons, but they are 
    not participating in ROE 4-momentum computation, because of various temporary 
    difficulties in KLM reconstruction. Nevertheless, one can count them using 
    :b2:var:`nROE_NeutralHadrons` variable. 

This concludes the Rest of Event setup as a middle stage algorithm to run :ref:`onlinebook_cs`, 
:ref:`onlinebook_flavor_tagging` or tag :ref:`onlinebook_vertex_fitting`.

.. admonition:: Question
     :class: exercise stacked

      How can I improve ROE 4-momentum resolution in my analysis?

.. admonition:: Hint
     :class: toggle xhint stacked

     There can be a lot of extra particles in ROE, that are deteriorating its 4-momentum
     resolution. Typically, they have low momentum or energy, so one can set up a 
     ROE mask, that will reject them. Also, particles, which are outside acceptance of
     main tracker have badly measured momentum or energy. 

     .. code-block:: python

         charged_cuts = 'pt > 0.05 and thetaInCDCAcceptance'
         photon_cuts = 'E > 0.05 and thetaInCDCAcceptance'

     Another hint is to use mostlikely hypothesis for charged particles in ROE, 
     as stated in the `RestOfEvent` documentation.
     Also, on newest releases, one can apply `updateROEUsingV0Lists` to include the 
     long-lived particles (V0) into the ROE mask.

.. admonition:: Solution
     :class: toggle solution

     .. code-block:: python

         # Build ROE:
         ma.buildRestOfEvent('X:signal', fillWithMostLikely=True, path=main_path)
         # Define cuts against the beam-background and badly reconstructed particles:
         charged_cuts = 'pt > 0.05 and thetaInCDCAcceptance'
         photon_cuts = 'E > 0.05 and thetaInCDCAcceptance'
         # Append ROE mask:
         ma.appendROEMask('X:signal', 'my_mask',
             charged_cuts, photon_cuts, path=main_path)
         # Use V0 in ROE mask:
         ma.updateROEUsingV0Lists('X:signal', 'my_mask', path=main_path)



.. admonition:: Key points
    :class: key-points

    * The ROE of a selection is build with `buildRestOfEvent`
    * ROE masks are added with `appendROEMask` or `appendROEMasks`. 
      Use them to clean up beam-induced or other background particles.
    * For many analyses ROE is used as middleware to get tag vertex fit, 
      continuum suppression or flavor tag. 
    * Usage of ROE without a mask is **not** recommended. 


.. topic:: Author(s) of this lesson

    Sviatoslav Bilokin
