.. Can link to this as `EventShape`. No need to define anker
   here. Will confuse sphinx because document also has the same
   filename

EventShape
==========

Introduction
------------
This page will illustrate how to use the eventShape module, a tool
that allows to calculate quantities sensitive to the global shape of the event. The goal of these
variables is to deduce the original partonic state from the geometrical correlations among the final state particles.
This topic is related to the continuum suppression, that is nothing but an application of
the event shape concepts specific to the problem of separating :math:`e^+e^- \to B\bar{B}` from :math:`e^+e^- \to  q\bar{q}` events.

Event shapes are discussed in several papers. Two comprehensive introductions can be found in:

`The Pythia 6 manual <http://home.thep.lu.se/~torbjorn/pythia6/pythia6200.pdf>`_ 

`G. Fox, S. Wolfram, Event shapes in e+e- annihilation <https://www.stephenwolfram.com/publications/academic/event-shapes-annihilation.pdf>`_

For an introduction on both the theory of the event shapes variables and their use for continuum suppression and event
characterization, you can also check the `slides of the starter kit workshop <https://indico.belle2.org/event/5/sessions/10/attachments/85/122/Continuum_suppression_lecture_V2.pdf>`_ .







What is the difference between event shape and continuum suppression?
---------------------------------------------------------------------
This section is inspired by `a B2question <https://questions.belle2.org/question/4764/continuum-suppression-or-event-shape/>`_ .
The event shape variables are closely related to the continuum suppression one, but return completely different values and have also a different meaning.
The Belle II continuum suppression (in short CS) uses the classic event shape variables, but calculates them separately on a B candidate and its rest of the event
(the story is  actually much longer: please see the `ContinuumSuppression` section).
This makes the CS variables highly optimized for the continuum suppression when a  B meson is reconstructed in an exclusive channel,
but completely useless to any other application.

Unlike the CS variables, the event shape variables are calculated starting from lists of particles
(usually a list of all the good tracks and a list of the good photons) intended to describe the whole event. While the CS variables are
candidate-based (in a given event each B candidate will have different daughters and a different :doc:`RestOfEvent`, and therefore different CS variables),
the event shape variables are event-based.

In summary, the event shape variables are suitable for:
  * studying the geometrical shape for the events
  * separating qq, BB, tau tau and other components without reconstruction any B or tau candidate
  * providing continuum suppression to all the non-B analyses


How to use eventShape
---------------------

To access the event shape variables you have to append the EventShapeCalculator module to your path, which will calculate all the quantities you need and store them in the datastore
making them available to the `VariableManager`. You can do this simply adding the `modularAnalysis.buildEventShape()` function to your analysis path:

.. autofunction:: modularAnalysis.buildEventShape
   :noindex:


This is a very minimal implementation of the event shape in a steering file:

.. code-block:: python

		import basf2 as b2
		import modularAnalysis as ma

		# create path
		my_path = b2.create_path()

		# add the event shape module with the default settings
		ma.buildEventShape(path=my_path)

		# Now the event shape variables are accessible
		# to the VariableManager


For a complete steering file, see the example `B2A704-EventShape.py <https://stash.desy.de/projects/B2/repos/software/browse/analysis/examples/tutorials/B2A704-EventShape.py>`_ .

Event shape variables
---------------------


Once `modularAnalysis.buildEventShape()` has been added to the analysis path, the event shape variables will be accessible via the  `VariableManager` as any other event-level variable.
Here you can find some deeper theoretical explanation about their meaning, and how they are constructed.


.. glossary::

   Sphericity-related variables
      First introduced by J.D. Bjorken and S.J. Brodsky in Phys. Rev. D1 (1970) 1416, the sphericity is a linear
      combination of the second and third eigenvalue of the sphericity tensor. This tensor :math:`S` is constructed from the
      3-vectors of all the particles (both charged and neutral) in the event:

      :math:`S^{\alpha, \beta} = \frac{\sum_i p_i^\alpha p_i^\beta}{\sum_i p_i^2}`,

      where :math:`i` runs over all the reconstructed particles in the event and :math:`\alpha, \beta` are the cartesian components
      of the momenta.  :math:`S` is not Lorentz-invariant, and it's meaningful only if calculated in the center of mass frame.
      The eigenvectors of :math:`S` define the ellipsoid that best matches with the particle distribution in the event. The eigenvalues
      :math:`\lambda_1, \lambda_2, \lambda_3`, ordered from the largest to the smallest, are proportional to the lengths of the axes of this ellipsoid.
      Since :math:`S` is unitary, only two independent quantities can be constructed out of its eigenvalues. The `VariableManager` provides the :b2:var:`sphericity` (:math:`3(\lambda_2+\lambda_3)/2`)
      and the :b2:var:`aplanarity` (:math:`3 lambda_3/2`).

   Thrust and thrust axis
      The thrust axis is the axis that maximizes the sum the projections of all the particles' momenta onto it. It is the best proxy for the main symmetry axis
      of the event and tends to be aligned to the first eigenvector of the sphericity matrix.
      The Thrust is the normalized sum the projections of all the particles' momenta onto the thrust axis.  It is close to 1 for a jet-like event, and close to 1/3 for a spherical one
      The calculation of the thrust axis allows the event to be divided into two hemispheres; total energy, momentum, and mass can be calculated for each of them individually.
      The `VariableManager` provides the thrust value (:b2:var:`thrust`), the three cartesian components of the thrust axis (:b2:var:`thrustAxisX`, :b2:var:`thrustAxisY`, :b2:var:`thrustAxisZ`),
      the polar direction of the thrust axis (:b2:var:`thrustAxisCosTheta`) and the metavariable :b2:var:`useThrustFrame` that allows to calculate any variable in a
      rotated reference frame where the z axis coincides with the thrust axis.
      Finally, you can access the invariant mass (:b2:var:`forwardHemisphereMass`), the total energy (:b2:var:`forwardHemisphereEnergy`), the total
      momentum (:b2:var:`forwardHemisphereMomentum`) and the components of the total momentum (:b2:var:`forwardHemisphereX`, :b2:var:`forwardHemisphereY`,
      :b2:var:`forwardHemisphereZ`) of the particles belonging to the forward emisphere. Similar variables are also defined for the backward hemisphere.
      
   Fox-Wolfram moments
      The Fox-Wolfram moments are among the most powerful quantities to define the shape of an event. They are based on the idea of expanding the geometrical distribution of the
      particles in the event in a suitable basis of special function. Neglecting the math, described in the references given above, the result is a rather simple definition:

      :math:`H_l = \sum_{i,j}p_i p_j P_l(cos\alpha_{i,j})`,

      where :math:`i,j` runs over all the particles in the event,  :math:`P_l` is the Legendre polynomial of order :math:`l`, and :math:`\alpha_{i,j}` is the angle
      between the momentum of the i-th and j-th particle.
      Instead of the raw moments, the event shape is always studied using the normalized moments :math:`R_i = H_i/H_0`, defined between 0 and 1.
      The `VariableManager` provides access to any Fox-Wolfram moment up to order 8 with the metavariables :b2:var:`foxWolframH` (raw moments) and :b2:var:`foxWolframR` (normalized moments).
      For convenience the normalized moments from 1 to 4 are also accessible using the variables :b2:var:`foxWolframR1` to :b2:var:`foxWolframR4`.
	    
   Harmonic moments
      The harmonic moments are the coefficients of the expansion of the event in spherical harmonics around an arbitrary axis. Unlike the Fox-Wolfram moments, they are not
      rotationally invariant. Currently the event shape tools allow you to calculate them respect to either the thrust axis, or the collision axis or both.
      They are defined as:
      
      :math:`B_l = \sum_i\frac{p_i}{\sqrt{s}}P_l(cos\alpha_i)`,

      where :math:`P_l` is the Legendre polynomial of order :math:`l`, and :math:`\alpha_i` is the angle between the momentum of the i-th particle and the chosen expansion axis.
      The `VariableManager` provides access to any harmonic moment up to order 8, calculated either using the collision axis or the thrust axis,  via the metavariable :b2:var:`harmonicMoment`.
      For convenience the harmonic moments from 0 to 4 calculated using the thrust axis as polar axis are also accessible using the variables
      :b2:var:`harmonicMomentThrust0` to :b2:var:`harmonicMomentThrust4`.
	    
      
   Cleo cones
     The CLEO cones are a set of concentric cones with different opening angle having the thrust axis as symmetry axis. In our current implementation we use 9 cones,
     of opening angle between 10 and 90 degrees, 10 degrees apart one from the other. The measurement of the energy flux in each cone provides a set of variables that
     have been proven to be very powerful for continuum suppression. Jet like events will be more likely to have all the energy flow
     concentrated in the narrower cones, while spherical events will have a more evenly-distributed flow. 
     The `VariableManager` provides access to any CLEO cone value up to order 8, calculated either using the collision axis or the thrust axis,  via the metavariable :b2:var:`cleoCone`.
     For convenience the CLEO cones variables calculated using using the thrust axis are also accessible using the variables :b2:var:`cleoConeThrust0` to :b2:var:`cleoConeThrust8`.
	    
      
     
Complete list of the eventShape variables:

.. b2-variables::
    :group: EventShape
    :noindex: 

 


