.. _dalitz-fit:

Dalitz fit
==========

Unbinned likelihood fit to the Dalitz-plot distribution of :math:`D^{0}\to\pi^{+}\pi^{-}\pi^{0}` decays. The fit uses a simplified amplitude model with only :math:`\rho(770)` resonances, plus a nonresonant component.

No input file is provided for this example, as the data to be fit is generated directly from the amplitude model.

To find the maximum of the likelihood, the following quantity is minimized:

.. math::

	-2\log L(\vec{\theta}) = -2\sum_{i\in\mathrm{data}} \log\,\mathrm{pdf}(\vec{x}_i|\vec{\theta}),
	
where the index :math:`i` runs over the data candidates, :math:`\vec{x}_i` are the phase-space coordinates for candidate :math:`i`, which PDF depends on some unknown parameters identified by the vector :math:`\vec{\theta}`.

The PDF is proportional to the squared amplitude,

.. math::
	
	\mathrm{pdf}_\mathrm{sgn}(\vec{x}|\vec{\theta}) \propto |\mathcal{A}(\vec{x}|\vec{\theta})|^{2},

and the amplitude is the sum of resonant and nonresonant components, following the isobar formalism as implemented by the CLEO collaboration in `Phys. Rev. D 63 (2001) 092001 <https://arxiv.org/abs/hep-ex/0011065>`_:

.. math::

	\mathcal{A}(\vec{x}|\vec{\theta}) = c_{nr} + \sum_{r\in\mathrm{resonances}} c_{r}\mathcal{A}_{r}^{J}(\vec{x}|M_{r},\Gamma_{r}),

where :math:`\mathcal{A}_{r}^{J}(\vec{x}|M_{r},\Gamma_{r})` describes the contribution due to the :math:`r`-th resonance with mass :math:`M_{r}`, width :math:`\Gamma_{r}` and spin :math:`J`. The masses and widths of the resonances are fixed, the only free parameters of the fit are the real and imaginary parts of the coefficients :math:`c_{r}` and :math:`c_{nr}`. The normalization and phase conventions are fixed by imposing :math:`c_{\rho(770)^{+}} = 1`.

The fit is developed using the following frameworks:

	* `Hydra <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/hydra/include/dalitz-fit.inl>`_
