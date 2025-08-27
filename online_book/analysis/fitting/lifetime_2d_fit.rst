.. _lifetime-2d-fit:

Lifetime 2D fit
===============

Unbinned likelihood fit to the 2D joint distribution of decay time :math:`t` and decay-time uncertainty :math:`\sigma_t` of :math:`D^{0}\to K^{-}\pi^+\pi^0` decay candidates reconstructed in simulation, to determine the lifetime of the :math:`D^{0}` meson.

The input data is provided by the ROOT ntuple ``ntp`` contained in the file ``example-data/lifetime.root``. The branches of the tree corresponding to the decay time and decay-time uncertainty are ``Dz_t`` and ``Dz_t_err``, respectively.

To find the maximum of the likelihood, the following quantity is minimized:

.. math::

	-2\log L(\vec{\theta}) = -2\sum_{i\in\mathrm{data}} \log\,\mathrm{pdf}(t_i,\sigma_{t,i}|\vec{\theta}),

where the index :math:`i` runs over the data candidates, :math:`t_i` is the observed decay time for candidate :math:`i`, :math:`\sigma_{t,i}` is the observed decay time for candidate :math:`i`, which 2D PDF depends on some unknown parameters identified by the vector :math:`\vec{\theta}`.

The fit assumes no background contamination. The 2D joint PDF is written as a conditional PDF of the decay time given the value of the decay-time uncertainty as

.. math::

	\mathrm{pdf}(t,\sigma_t|\tau,b,s,\mu,\lambda,\gamma,\delta) = \mathrm{pdf}(t|\sigma_t,\tau,b,s)\,\mathrm{pdf}(\sigma_t|\mu,\lambda,\gamma,\delta).

The decay-time PDF is written as the convolution between the exponential decay and the experimental resolution function

.. math::

	\mathrm{pdf}(t|\sigma_t,\tau,b,s) \propto \int_0^\infty e^{-t_\mathrm{true}/\tau}G(t-t_\mathrm{true}|b,s\sigma_t) dt_\mathrm{true}

where :math:`\tau` is the lifetime, :math:`t_\mathrm{true}` is the true decay time, and :math:`G` is a Gaussian with mean :math:`b` and width :math:`s\sigma_t` describing the resolution model. The PDF is normalized in the range :math:`-2 < t < 4` ps for any value of :math:`\sigma_t`.

The decay-time uncertainty PDF is parametrized by a `Johnson's SU distribution <https://en.wikipedia.org/wiki/Johnson%27s_SU-distribution>`_

.. math::

	\mathrm{pdf}(\sigma_t|\mu,\lambda,\gamma,\delta) \propto \frac{e^{-\frac{1}{2}\left[\gamma+\delta\text{sinh}^{-1}\left(\frac{\sigma_t-\mu}{\lambda}\right)\right]^2}}{\sqrt{1+\left(\frac{\sigma_t-\mu}{\lambda}\right)^2}},

and is normalized in the range :math:`0 < \sigma_t < 0.5` ps.

The fit is developed using the following frameworks:

	* `RooFit <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/roofit/lifetime-2d-fit.py>`_
	* `zfit <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/zfit/lifetime-2d-fit.py>`_
	* `Minuit <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/minuit/src/lifetime-2d-fit.cpp>`_
	* Bat: :ref:`bat_lifetime_2d_fit`
