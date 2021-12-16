Other vertex (and kinematic) fitters in basf2
=============================================

Use cases
---------

#. Vertex fit of multiple particles
#. Vertex fit of multiple particles with mass constraint
#. Vertex fit of multiple particles with direction constraint
#. Fit of decay chains (with constraints)
#. Vertex fit of a V0 (with constraints)
#. Vertex fit of tag B (with constraints)
#. Momentum fit of a :math:`\pi^0`
#. Kinematic fits of multiple particles with full or partial soft or hard (four momentum) constraints.
#. Kinematic fits of multiple particles with full or partial soft or hard (four momentum) constraints with partially or completely missing particles.

RAVE
----

Rave is a Kalman-based vertex fitter implemented as an external library (CMS vertexing library)

* **Physics Use Case**

  * Standard kinematic fit
  * Mass fit from mother 7x7 error matrix
  * Refit of the daughters
  * Geometric fit
  * Tag Vertex

* **basf2 implementation**

  * Kinematic fit included in ParticleVertexFitModule
  * Module for geometric fit not available but can be easily developed
  * TagVertex
  * V0Finder vertex fit implemented in tracking code

* **Issues**

 * Very slow: external libraries (Root) and shell
 * no developers

**RAVE: global**

  * RAVE algorithm to be tested
  * RAVE API to be developed and tested
  * RAVE API kinematic constraints to be tested and/or debugged

**RAVE: progressive**

  * Algorithm should be developed inside RAVE
  * Progressive: References are missing

**General RAVE development**

  * memory consumption
  * documentation
  * kinematic fit constraints: test and debug
  * rave is available at github `github <https://github.com/rave-package/rave/>`_
  * building/testing environment at `RAVE page <https://travis-ci.org/rave-package/rave>`_

.. _kfit_vertex_fitter:

KFit
----

Port of the Belle vertex fitter.

* **Physics Use Case**

  * Standard Kinematic Fit
  * Mass fit from daughters
  * Constrain the sum of four momentum of all final particles to that of e+ e- beams

* **basf2 implementation**

  * Kinemeatic fit available in ParticleVertexFitModule

* **Issues**

  * 4C constraint not completed

OrcaKinFit
----------

Kinematic fitter implementation, based on a fork of MarlinKinFit with modifications (remove all ILC dependency).

* **Physics Use Case**

  * Momentum constraints (soft and hard) with and without (partially) missing
    particles. Arbitrary constraints (e.g., recoil mass: `BELLE2-NOTE-PH-2017-002
    <https://docs.belle2.org/record/483?ln=en>`_)

TagV
----

Module to perform the Btag vertex geometric fit using the RAVE adaptive vertex fitting algorithm. It takes as input the tracks
of the Rest Of Event. More info about interface in :b2:mod:`TagVertex`, `vertex.TagV`

* **Physics Use Case**

  * TDCPV measurements

