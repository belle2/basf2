.. _analysis_particles:

Particles
=========

Particles in basf2 are created from reconstructed dataobjects: Tracks, tracking vertices (V0s), ECLClusters, and KLMClusters.
The :doxygen:`Particle <classBelle2_1_1Particle>` class, provides a unified way to treat different objects at a high level interpreting them as physical particles.
All of the :doxygen:`Particle <classBelle2_1_1Particle>` s in an event that fall within certain criteria are collected into :doxygen:`ParticleList <classBelle2_1_1ParticleList>` s.

The Belle II physics performance group already has several predefined :doxygen:`ParticleList <classBelle2_1_1ParticleList>` s.
To use them you need to use functions described in the standard particle lists.

.. toctree:: StandardParticles

Creation of new particle lists by combination of particles is essential part of almost any analysis job.
The syntax used during this procedure is the :doc:`DecayString` and is described below:

.. toctree:: DecayString

It is often desirable to apply selections to :doxygen:`ParticleList <classBelle2_1_1ParticleList>` s, and events.
This is done with cut strings.

.. toctree:: CutStrings
