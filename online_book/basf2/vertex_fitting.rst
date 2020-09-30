.. _onlinebook_vertex_fitting:

Vertex fitting
==============

.. sidebar:: Overview
    :class: overview

    **Teaching**: ?? min

    **Exercises**: ?? min

    **Prerequisites**: 
    	
    	* The previous lesson

    **Questions**:

        * What is a vertex fit?
        * When do I want to perform a vertex fit?
	* How should I choose my fit and constraints?

    **Objectives**:

        * Perform vertex fit of a decay

Introduction
------------

In the broadest sense, we call vertex fitting a technique in which one uses prior knowledge on the nature of a decay to improve the measurement of its observables.
The fits we are going to perform are of two main types:

* **Geometric Fitting:** We use the fit to determine the decay vertex of the particle. Usually this is done by fitting together the tracks of its charged decay products, 
  which we know originate from a common point.  Additional information could be available - for example, if the particle is short lived, we can improve this by adding 
  an IP constraint, i.e. fit the beam spot together with the tracks. If there's only one track, using the beam spot is the only way to obtain a vertex.

.. warning::

    If no vertex fit is performed, the corresponding variables for the vertex position will not be filled.


* **Kinematic Fitting:** We use the fit to improve our knowledge of the particle kinematics. By default, composite particle kinematics are built off the decay products
  using 4-momentum conservation. If the particle we are reconstructing has a well defined mass (either stable, or a narrow resonance) it might make sense to apply a mass
  constraint to help reject combinatorial background. 

.. warning::

   If you apply a mass constraint, the invariant mass will be fixed to the nominal mass. This is problematic if you then want to use this variable, for example
   if you want to fit a peak. In that case, make sure you save the pre-fit mass separately.
  
.. note:: 

   Several fitters exist. For this exercise we will focus on ``KFit`` which is the most basic one.

.. admonition:: Exercise
     :class: exercise stacked 
      
     Locate the documentation for vertex fitting functions and find KFit.

.. admonition:: Hint
     :class: toggle xhint stacked

     Use the search bar.
  
.. admonition:: Solution
     :class: toggle solution

     You can find it here: `Vertex`.


Basic Fitting
-------------

This lesson assumes you successfully reconstructed your :math:`B \to J/\Psi(\to e^+e^-)K_s(\to \pi^+\pi^+)` decay following the previous exercises.
Now suppose you are interested in reconstructing the :math:`B` decay vertex position using a fit (for example, you're trying to do a time-dependent CPV study).

.. admonition:: Question
     :class: exercise stacked

     Which particles do you need to fit in order to reconstruct the :math:`B` vertex?

.. admonition:: Hint
     :class: toggle xhint stacked

     It can't be the :math:`B` itself: out of its daughters, neither the :math:`J/\Psi` nor the :math:`K_s` are charged tracks.

.. admonition:: Answer
     :class: toggle solution
     
     You must fit the :math:`J/\Psi \to e^+e^-` vertex. The :math:`J/\Psi` is short-lived and therefore its vertex is a good approximation of the :math:`B`. 
     Meanwhile, the :math:`K_s` can decay several cm away from where it is produced.


.. admonition:: Exercise
     :class: exercise stacked

     Call the fit function with the correct parameters and save the output. Include the true vertex position from MC for comparison.

.. admonition:: Hint
     :class: toggle xhint stacked

     Look up the variable collections for vertices. Don't forget to import the vertex module!

.. admonition:: Solution
     :class: toggle solution

     .. code-block:: python

          import vertex
	  ...
	  vertex.kFit("J/psi:ee", conf_level=0.0, path=main)
	  ...
	  jpsi_ks_vars += vc.vertex + vc.mc_vertex
     
     You can also set the confidence level to -1, which means failed fits will be included. The fit p-value is saved as part of mc.vertex.

.. admonition:: Optional Exercise
     :class: exercise stacked

     Fit the :math:`K_s` as well. How does its flight lenght compare to the :math:`J/Psi`?

.. admonition:: Optional Exercise
     :class: exercise stacked

     Look up the documentation for ``TreeFitter`` and fit the whole :math:`B \to J/\Psi(\to e^+e^-)K_s(\to \pi^+\pi^+)` decay chain at once.


Tag Vertex Fitting
------------------

Since :math:`B` mesons are produced in pairs, for every signal candidate we reconstruct, there will also be another (the "tag") which is not explicitly reconstructed. We 
might be interested in knowing the decay position of this meson without placing any requirements on its decay. This is done using the ``TagV`` module. 
``TagV`` performs a geometric fit over the tracks in the ROE to determine the tag decay vertex. However, not all tracks will necessarily come from the tag itself; consider for example our signal, 
where the pion tracks originate from a long lived kaon vertex. TagV is designed to iteratively downweight those tracks, ultimately excluding them from the fit.

.. admonition:: Exercise
     :class: exercise stacked

     Locate the ``TagV`` documentation.

.. admonition:: Solution
     :class: toggle solution

     It's in the same page as ``KFit``.

.. admonition:: Question
     :class: exercise stacked

     By default, TagV only uses tracks with PXD hits. Why?

.. admonition:: Solution
     :class: toggle solution

     Those tracks provide the best resolution close to the interaction point. As a bonus, this selection rejects tracks from displaced vertices.


.. admonition:: Exercise
     :class: exercise stacked

   Call the TagV module and save the output.

.. admonition:: Hint
     :class: toggle xhint stacked

     In order to reinforce the fit, an IP constraint is applied to the TagV. If the signal is fully reconstructed, this condition can be relaxed along the signal :math:`B` flight direction.

.. admonition:: Solution
     :class: toggle solution

     .. code-block:: python

          vertex.TagV("B0", constraintType="tube", path=main)
          ...
          b_vars += vc.tag_vertex + vc.mc_tag_vertex


Plotting
--------

Congratulations! Your steering file is ready!
\... or maybe you just skipped ahead. Either way, time to run it and check the results.


.. admonition:: Exercise
     :class: exercise stacked

   Run the steering file.


.. admonition:: Solution
     :class: toggle solution

     .. code-block:: bash
          basf2 steering_file_name.py
     Replace with the name of your steering file as appropriate.

.. admonition:: Exercise
     :class: exercise stacked

   Plot some relevant variables for :math:`J/\Psi`, the tag vertex and, if you did the optional exercise, :math:`K_s`.
   
   * Vertex position (try various coordinates, in particular try the radial distance)
   * Resolution (:math:`\sigma(x)/x`) where x is each of the above variables.
   * Pull (:math:`(x-x(MC)/\sigma(x)`).
   
   Is this what you expect?

.. admonition:: Solution
     :class: toggle solution

     INSERT NOTEBOOK?
