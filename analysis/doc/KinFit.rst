.. _kinfit:

Kinematic-fitting convenience functions
=======================================

The `Orca kinematic fitter <https://github.com/tferber/OrcaKinfit>`_ is a
global fitting tool.

A typical use-case would be to constrain the 4-momentum of all particles
provided to the 4-momentum of the beam.  This is a "4C" fit (four constraints).

.. note::

        By default, the 4-momentum of the beam is taken without uncertainties.

Running this fit updates the :b2:var:`chiProb` of the mother particle.

.. important::

         Please also see the list of special :ref:`orca_kin_fit_variables` variables.

.. automodule:: kinfit
   :members:
   :undoc-members:

.. seealso:: 

   The :b2:mod:`ParticleKinematicFitter` documentation if you prefer to
   directly configure the module yourself.
