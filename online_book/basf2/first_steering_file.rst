.. _onlinebook_first_steering_file:

First steering file
===================

.. include:: ../todo.rst

.. sidebar:: Overview
    :class: overview

    **Teaching**: 

    **Exercises**: 

    **Prerequisites**: 
    	
    	* Creating and running scripts in the terminal
    	* Basic python

    **Questions**:

        * How can I load data?
        * How can I reconstruct a decay?
        * How can I match MC?
        * Can I see some pretty pictures?

    **Objectives**:

        * Reconstruct :math:`B \to J/\Psi(\to e^+e^-)K_s(\to \pi^+\pi^+)`
        * Create a plot of 


.. admonition:: Key points
    :class: key-points

    * The ``modularAnalysis`` module contains most of what you'll need for now
    * ``inputMdstList`` is used to load data
    * ``fillParticleList`` adds particles into a list
    * ``reconstructDecay`` combined FSPs from different lists to "reconstruct" particles
    * ``matchMCTruth`` matches MC
    * ``variablesToNtuple`` saves an output file
    * Don't forget ``process(path)`` or nothing happens

