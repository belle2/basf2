.. _efficiency-fit:

Efficiency fit
==============

Simultaneous least-squares fit to the :math:`D^{0}`-mass distributions of :math:`D^{*+}\to D^{0}(\to K^{-}\pi^+\pi^-\pi^+)\pi^+` decay candidates passing and failing the kaon-momentum requirement :math:`p(K^{-})>1` GeV/:math:`c`, to determine the efficiency of the requirement on both signal and background decays.

The input data is provided by the ROOT ntuple ``ntp`` contained in the file ``example-data/momentum-scale.root``. The tree contains branches corresponding to the 4-momenta of the final state particles (``K_P``, ``pi1_p``, ``pi2_p``, ``pi3_p``), a branch with the :math:`D^{0}` mass (``Dz_M``), a branch with the difference between the :math:`D^{*+}` and :math:`D^{0}` masses (``DM``).

Two histograms of the :math:`D^{0}` mass, each consisting of 150 bins in the range 1.8-1.95 GeV/:math:`c^{2}`, are filled from the data. The first with candidates passing the kaon-momentum requirement (:math:`h_\mathrm{pass}`), the second with candidates failing the requirement (:math:`h_\mathrm{fail}`).

The least-squares are computed as

.. math::

	\mathrm{LS}(\vec{\theta}_\mathrm{pass},\vec{\theta}_\mathrm{fail}) = \sum_{i \in h_\mathrm{pass}}\left(\frac{n_i - n_i^\mathrm{pass}(\vec{\theta}_\mathrm{pass})}{\sigma_i}\right)^2 + \sum_{i \in h_\mathrm{fail}}\left(\frac{n_i - n_i^\mathrm{fail}(\vec{\theta}_\mathrm{fail})}{\sigma_i}\right)^2,

where the index :math:`i` runs over the histogram bins, :math:`n_i` is the observed number of candidates in the bin, :math:`\sigma_i` is the uncertainty on :math:`n_i`, :math:`n_i^\mathrm{pass/fail}` is the predicted number of candidates in the bin passing/failing the kaon-momentum requirement, which depends on some unknown parameters identified by the vector :math:`\vec{\theta}_\mathrm{pass/fail}`.

To compute the predicted number of candidates, the fit model assumes that the :math:`D^{0}`-mass distributions of the candidates passing/failing the selection can be described by a signal component peaking around the nominal :math:`D^{0}` mass, described by a Gaussian distribution

.. math::
	
	\mathrm{pdf}_\mathrm{sgn}(m|\mu,\sigma) \propto e^{-\frac{1}{2}\left(\frac{m-\mu}{\sigma^2}\right)^2},

and a background component, described by an exponential distribution

.. math::

	\mathrm{pdf}_\mathrm{bkg}(m|\lambda) \propto e^{-\lambda m}.

Each PDF is normalized in the fit range 1.8-1.95 GeV/:math:`c^{2}`. The predicted numbers of candidates passing/failing the selection in bin :math:`i` are estimated by evaluating the above PDFs at the center of the bin :math:`m_i` as

.. math::

	n_i^\mathrm{pass}(N_\mathrm{sgn},\epsilon_\mathrm{sgn},\mu,\sigma_\mathrm{pass},N_\mathrm{bkg},\epsilon_\mathrm{bkg},\lambda_\mathrm{pass}) = N_\mathrm{sgn}\epsilon_\mathrm{sgn}\mathrm{pdf}_\mathrm{sgn}(m_i|\mu,\sigma_\mathrm{pass}) + N_\mathrm{bkg}\epsilon_\mathrm{bkg}\mathrm{pdf}_\mathrm{bkg}(m_i|\lambda_\mathrm{pass}),
	
	n_i^\mathrm{fail}(N_\mathrm{sgn},\epsilon_\mathrm{sgn},\mu,\sigma_\mathrm{fail},N_\mathrm{bkg},\epsilon_\mathrm{bkg},\lambda_\mathrm{fail}) = N_\mathrm{sgn}(1-\epsilon_\mathrm{sgn})\mathrm{pdf}_\mathrm{sgn}(m_i|\mu,\sigma_\mathrm{fail}) + N_\mathrm{bkg}(1-\epsilon_\mathrm{bkg})\mathrm{pdf}_\mathrm{bkg}(m_i|\lambda_\mathrm{fail}),
	
where :math:`N_\mathrm{sgn/bkg}` is the total number of signal/background candidates and :math:`\epsilon_\mathrm{sgn/bkg}` is the signal/background efficiency, and we have assumed that the signal/background shapes may have different widths/slopes depending on whether the candidates pass/fail the kaon-momentum requirement.

The fit is developed using the following frameworks:

	* `Minuit <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/blob/main/minuit/src/efficiency-fit.cpp>`_
