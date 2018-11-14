Vertex Fitter
=============

This page summarises the development status for various vertex fitting algorithms and modules within basf2.

Ongoing issues, development threads, and open projects can be found on `JIRA <https://agira.desy.de/>`_ under the `VertexFitting <https://agira.desy.de/browse/BII-3602?jql=labels%253DVertexFitting>`_ label.

Interfaces to Vertex related functions and modules are in :doc:`Vertex`.


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

Available vertex fit libraries
------------------------------

RAVE
~~~~

Rave is an already highly developed vertex fitter.

* **Developers/basf2 maintener** None / Luigi Li Gioi
* **Status**

  * External library (CMS vertexing library)
  * Some potential for custom constraints

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


GFRave
~~~~~~

* **Developers/basf2 maintener** None / Luigi Li Gioi
* **Status** Implementation of RAVE in Genfit

* **Physics Use Case** V0 Fit

* **basf2 implementation** V0Finder

* **Issues**

KFit
~~~~

* **Developers/basf2 maintener** J. Tanaka / Luigi Li Gioi
* **Status**

  * Inside the analysis package
  * Ported from Belle
  * 4C constraint not completed

* **Physics Use Case**

  * Standard Kinematic Fit
  * Mass fit from daughters
  * Constrain the sum of four momentum of all final particles to that of e+ e- beams

* **basf2 implementation**

  * Kinemeatic fit available in ParticleVertexFitModule

* **Issues**

TreeFitter
~~~~~~~~~~

(`Nuclear Instruments and Methods in Physics Research A 552 (2005) 566–575 <https://doi.org/10.1016/j.nima.2005.06.078>`_)

This is a progressive decay chain fitting tool from BaBar and LHCb. It uses a
Kalman Filter. It is currently under development in basf2. More detail can be
found in the page :doc:`TreeFitter`.

* **Developers/basf2 maintener** Francesco Tenchini, Jo-Frederik Krohn

* **Status**
  
  * inside the analysis package
  * Ported from BaBar/LHCb

* **Physics Use Case**

  * Fit of the entire decay tree

* **basf2 implementation**

  * Now outperforming RAVE/KFit
  * More information at :doc:`TreeFitter`

* **Issues**

MarlinKinFit/OrcaKinFit
~~~~~~~~~~~~~~~~~~~~~~~

* **Developers/basf2 maintener** Torben Ferber

* **Status**

  * Ready for tests:  `PullRequest <https://stash.desy.de/projects/B2/repos/software/pull-requests/298/overview>`_

* **Physics Use Case**

  * Momentum constraints (soft and hard) with and without (partially) missing
    particles. Arbitrary constraints (e.g., recoil mass: `BELLE2-NOTE-PH-2017-002
    <https://docs.belle2.org/record/483?ln=en>`_)

* **basf2 implementation**

  * Ready for tests: (`Pull request <https://stash.desy.de/projects/B2/repos/software/pull-requests/298/overview>`_)
  * Actual implementation would be based on a fork of MarlinKinFit with modifications (remove all ILC dependency).

* **Issues**

ExKFitter
~~~~~~~~~

This is a global decay chain fitting tool from Belle (`Link to Belle page <http://belle.kek.jp/~kfchen/private/ExKFitter/>`_).
It uses the Lagrange multiplier formalism, which comes with high computing
costs.
Because of the existence of the TreeFitter and of a possible implementation of
the global fit in RAVE, it hasn't been ported over.
However, it could yet prove of some use, so that should be investigated.

* **Developers/basf2 maintener** 
* **Status**

* **Physics Use Case**

* **basf2 implementaion**

* **Issues**

TagV
~~~~

The module TagVertex performs the Btag Vertex Fit. It takes as input the tracks
of the Rest Of Event and performs a geometric fit using the RAVE AVF algorithm.

More info about interface in :b2:mod:`TagVertex`, `vertex.TagV`

* **Developers/basf2 maintener** 
* **Status**

* **Physics Use Case**

* **basf2 implementaion**

* **Issues**
