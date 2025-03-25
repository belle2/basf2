.. _simple-1d-fit:

Simple 1D fit
=============

Unbinned likelihood fit to the beam-constrained mass distribution of :math:`B^{0}\to K^{*}\gamma` decay candidates reconstructed in simulation, to determine the fraction of signal decays.

The input data is provided by the ROOT ntuple ``BtoKstG`` contained in the file ``example-data/fitme.root``. The branch of the tree corresponding to the beam-constrained mass is ``B0_mbc``.

To find the maximum of the likelihood, the following quantity is minimized:

.. math::

	-2\log L(\vec{\theta}) = -2\sum_{i\in\mathrm{data}} \log\,\mathrm{pdf}(m_i|\vec{\theta}),

where the index :math:`i` runs over the data candidates, :math:`m_i` is the observed beam-constrained mass for candidate :math:`i`, which PDF depends on some unknown parameters identified by the vector :math:`\vec{\theta}`.

The fit model assumes a signal component peaking at the nominal :math:`B^{0}` mass, described by a `Crystal Ball distribution <https://en.wikipedia.org/wiki/Crystal_Ball_function>`_ with :math:`n=15`

.. math::

	\mathrm{pdf}_\mathrm{sgn}(m|\mu,\sigma,\alpha) \propto \left\{\begin{array}{lcccl} e^{-\frac{1}{2}\left(\frac{m-\mu}{\sigma}\right)^2} & & \mathrm{if} & & \frac{m-\mu}{\sigma} > \alpha\\ \left(\frac{n}{|\alpha|}-|\alpha|-\frac{m-\mu}{\sigma}\right)^{-n}& & \mathrm{if} & & \frac{m-\mu}{\sigma} \leq -\alpha\end{array}\right.,

and a background component, described by an `Argus distribution <https://en.wikipedia.org/wiki/ARGUS_distribution>`_

.. math::

	\mathrm{pdf}_\mathrm{bkg}(m|m_0,c) \propto \left\{\begin{array}{lcccl} \frac{m}{m_0^3}\,\sqrt{1-\frac{m^2}{m_0^2}}\, e^{-\frac{1}{2}c^2\left(1-\frac{m^2}{m_0^2}\right)}& & \mathrm{if} & & m \leq m_0\\ 0& & \mathrm{if} & & m > m_0\end{array}\right..

Each PDF is normalized in the fit range that goes from 5.2 GeV/:math:`c^{2}` up to the threshold corresponding to :math:`m_0`. The total PDF is sum of the signal and background PDFs weighted by the signal fraction :math:`f_\mathrm{sgn}`

.. math::

	\mathrm{pdf}(m|\mu,\sigma,\alpha,m_0,c) = f_\mathrm{sgn}\,\mathrm{pdf}_\mathrm{sgn}(m|\mu,\sigma,\alpha) + (1-f_\mathrm{sgn})\, \mathrm{pdf}_\mathrm{bkg}(m|m_0,c).

The fit is developed using the following frameworks:

	* `RooFit <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/roofit/simple-1d-fit.py>`_
	* `zfit <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/zfit/simple-1d-fit.py>`_
	* `Minuit <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/minuit/src/simple-1d-fit.cpp>`_
	* `Hydra <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/hydra/include/simple-1d-fit.inl>`_
	* Bat: :ref:`bat_simple_1d_fit` and :ref:`bat_simple_1d_fit_comparison`
