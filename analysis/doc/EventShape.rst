EventShape
=====================

Introduction
------------
This page will illustrate how to use the eventShape module, a tool
that allows to calculate quantities sensitive to the global shape of the event. The goal of these
variables is to deduce the original partonic state from the geometrical correlations among the final state particles.
This topic is related to the continuum suppression, that is nothing but an application of
the event shape concepts specific to the problem of separating :math:`e^+e^- \to B\bar{B}` from :math:`e^+e^- \to  q\bar{q}` events.

Events shapes are discussed in several papers. Two comprehensive introductions can be found in:

`The Pythia 6 manual <http://home.thep.lu.se/~torbjorn/pythia6/pythia6200.pdf>`_ 

`G. Fox, S. Wolfram, Events shapes in e+e- annihilation <https://www.stephenwolfram.com/publications/academic/event-shapes-annihilation.pdf>`_

`B2SFW slides <https://indico.belle2.org/event/5/sessions/10/attachments/85/122/Continuum_suppression_lecture_V2.pdf>`_ 





What is the difference between event shape and continuum suppression?
---------------------------------------------------------------------
This section is inspired by `a B2question <https://questions.belle2.org/question/4764/continuum-suppression-or-event-shape/>`_ .
The event shape variables are strictly related to the continuum suppression one, but return completely different values and have also a different meaning.
The Belle II CS uses the classic event shape variables, but calculating them separately on a B candidate and its rest of the event
(the story is  actually much longer: please see the Continuum Suppression section).
This makes the CS variables highly optimized for the continuum suppression when a  B meson is reconstructed in an exclusive channel,
but completely useless to any other application.

Unlike the CS variables, the event shape variables are calculated starting from list of particles
(usually a list of all the good tracks and a list of the good photons) intended to describe the whole event. While the CS variables are
candidate-based (in a given event each B candidate will have different daughters and a different rest-of-the-event, and therefore different CS variables),
the event shape variables are event-based.

In summary, the event shape variables are suitable for:
  * studying the geometrical shape for the events
  * separate qq, BB, tau tau and other components without reconstruction any B or tau candidate
  * provide continuum suppression to all the non-B analysis


How to use eventShape
---------------------

To access the event shape variable you have to create an evetShape data object, which stores the relevant information and is the accessed
by the variable manager. To do it add the `modularAnalysis.buildEventShape()` function:

.. autofunction:: modularAnalysis.buildEventShape


An example of how to use it is given here:

.. code-block:: python

		import basf2 as b2
		import modularAnalysis as ma
		import variables.collections

		# create path
		my_path = b2.create_path()


		# Creates a list of all the good tracks (using the pion mass hypothesys)
		# and good gammas with very minimal cuts
		ma.fillParticleList(decayString='pi+:all',
                                    cut='pt> 0.1',
				    path=my_path) 
				    ma.fillParticleList(decayString='gamma:all',
				    cut='E > 0.1',
				    path=my_path)

		# Builds the event shape enabling explicitly ALL the variables.
		# Most of the are actually enabled by defoult, but here we prefer
		# to list explicitly all the flags
		ma.buildEventShape(inputListNames=['pi+:all', 'gamma:all'],
                                   allMoments=True,
				   foxWolfram=True,
				   harmonicMoments=True,
				   cleoCones=True,
				   thrust=True,
				   collisionAxis=True,
				   jets=True,
				   sphericity=True,
				   checkForDuplicates=False,
				   path=my_path)

		# Here we use the pre-defined collection 'event_shape', that contains
		# thrust, sphericity, aplanarity, FW ratios up to 4, harmonic moments w/respect to
		# the thrust axis up to 4 and all the cleo cones w/respect to the thrust axis.
		# In addition, we will save also the forward and backward hemisphere (or "jet") energies,
		# and the 2nd order harmonic moment calculate respect to the collision axis (i.e. the z axis)
		ma.variablesToNtuple('',
                                     variables=['event_shape',
				     'backwardHemisphereEnergy',
				     'forwardHemisphereEnergy',
				     'harmonicMoment(2, collision)'],
				     filename='B2A704-EventShape.root',
				     path=my_path)
 



Event shape variables
---------------------


The variableManager allows to access many variables. Here you can find some deeper theoretical explanation about their meaning, and how they are constructed.


.. glossary::

   Sphericity-related variables
      First introduced by J.D. Bjorken and S.J. Brodsky in Phys. Rev. D1 (1970) 1416, the sphericity is a linear
      combination of the second and third eigenvalue of the sphericity tensor. This tensor :math:`S` is constructed from the
      3-vectors of all the particles (both charged and neutral) in the event:

      :math:`S^{\alpha, \beta} = \frac{\sum_i p_i^\alpha p_i^\beta}{\sum_i p_i^2}`,

      where :math:`i` runs over all the reconstructed particles in the event and :math:`\alpha, \beta` are the cartesian components
      of the momenta.  :math:`S` is not Lorentz-invariant, and it's meaningful only if calculated in the center of mass frame.
      The eigenvecotrs of :math:`S` define the ellipsoid that better matches with the particle distribution in the event. The eigenvalues
      :math:`\lambda_1, \lambda_2, \lambda_3`, ordered from the largest to the smallest, are proportional to the lengths of the axes of this ellipsoid.
      Since `S` is unitary, only two independent quantities can be constructed out of its eigenvalues. The variableManager provides the sphericity (:math:`s = (3/2)(\lambda_2+\lambda_3)`)
      and the aplanarity (:math:`A = 3/2\lambda_3`)

   Thrust and thrust axis
      The thrust axis is the axis that maximizes the sum the projection of all the particles' momenta onto it. It is the best proxy for the main symmetry axis
      of the event and tends to be aligned to the first eigenvector of the sphericity matrix.
      The Thrust is the normalized sum the projection of all the particles' momenta onto the thrust axis.  It is close to 1 for a jet-like event, and close to 1/3 for a spherical one
      The calculation of the thrust axis allows to define two hemispheres in the event, and calculate the total energy, momentum and mass of each of them.

      
   Fox-Wolfram moments
      The Fox-Wolfram moments are among the most powerful quantities to define the shape of an event. They are based on the idea of expanding the geometrical distribution of the
      particle in the events in a suitable basis of special function. Neglecting the math, described in the references given above, the result is a rather simple definition:

      :math:`H_l = \sum_{i,j}p_i p_j P_l(cos\alpha_{i,j})`,

      where :math:`i,j` runs over all the particles in the event,  :math:`P_l` is the Legendre polynomial of order :math:`l`, and :math:`\alpha_{i,j}` is the angle
      between the momentum of the i-th and j-th particle.

      Instead of the raw momenta, the event shape is always studies using the normalized momenta :math:`R_i = H_i/H_0`, defined between 0 and 1.
      
   Harmonic moments
      The harmonic moments are the coefficients of the expansion of the event in spherical harmonics around an arbitrary axis. Unlike the Fox-Wolfram moments, they are not
      rotational invariant. Currently the event shape tools allow you to calculate them respect to both the thrust axis or the collision axis.
      They are defined as:
      
      :math:`B_l = \sum_i\frac{p_i}{\sqrt{s}}P_l(cos\alpha_i)`,

      where :math:`P_l` is the Legendre polynomial of order :math:`l`, and :math:`\alpha_i` is the agle between the momentum of the i-th particle and the chosen expansion axis.

   Cleo cones
     A Cleo cone is the fractional energy flow with a cone of give aperture (usually a multiple of 10 degrees), around the thrust axis.
     A very powerful continuum suppression is given by a set of ten Cleo cones of different openings (10-90) degrees. jet like events will be more likely to have all the energy flow
     concentrated in the narrower cones, while spherical events will have a more evenly-distributed flow.

Complete list of the eventShape variables:

.. b2-variables::
    :group: EventShape
 

 


