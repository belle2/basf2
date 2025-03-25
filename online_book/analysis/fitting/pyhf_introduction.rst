Introduction to HistFactory
***************************

Measurements in High Energy Physics (HEP) rely on determining the compatibility of observed collision events with theoretical predictions. The relationship between observed collision events and theoretical predictions is often formalized in a statistical model :math:`f(\boldsymbol{x}|\boldsymbol{\phi})` describing the probability of data :math:`\boldsymbol{x}` given model parameters :math:`\boldsymbol{\phi}`,

.. math::

   f(\boldsymbol{x}|\boldsymbol{\phi}) = f(\boldsymbol{x}|\overbrace{\boldsymbol{\eta}}^{\text{free}},\underbrace{\boldsymbol{\chi}}_{\text{constrained}}) = f(\boldsymbol{x}|\overbrace{\psi}^{\text{parameters of interest}},\underbrace{\theta}_{\text{nuisance parameters}}).

Statistical models described using `HistFactory <https://cds.cern.ch/record/1456844/files/CERN-OPEN-2012-016.pdf>`_ center around the simultaneous measurement of disjoint binned distributions (channels) observed as event counts :math:`\boldsymbol{n}`,

.. math::

   f(\boldsymbol{n}, \boldsymbol{a} \,|\,\boldsymbol{\eta},\boldsymbol{\chi}) = \underbrace{\prod_{c\in\mathrm{\,channels}} \prod_{b \in \mathrm{\,bins}_c}\textrm{Pois}\left(n_{cb} \,\middle|\, \nu_{cb}\left(\boldsymbol{\eta},\boldsymbol{\chi}\right)\right)}_{\substack{\text{Simultaneous measurement}\\ \text{of multiple channels}}}
   \underbrace{\prod_{\chi \in \boldsymbol{\chi}} c_{\chi}(a_{\chi} |\, \chi)}_{\substack{\text{constraint terms}\\ \text{for auxiliary measurements}}}.

For each channel, the overall expected event rate is the sum over a number of physics processes (samples). The sample rates may be subject to parameterized variations, both to express the effect of free parameters :math:`\boldsymbol{\eta}` and to account for systematic uncertainties as a function of constrained parameters :math:`\boldsymbol{\chi}`. The degree to which the latter can cause a deviation of the expected event rates from the nominal rates is limited by constraint terms. In a frequentist framework, these constraint terms can be viewed as auxiliary measurements with additional global observable data :math:`\boldsymbol{a}`, which paired with the channel data :math:`\boldsymbol{n}` completes the observation :math:`\boldsymbol{x} = (\boldsymbol{n},\boldsymbol{a})`.

The expected rate of events :math:`\nu_{cb}(\boldsymbol{\eta},\boldsymbol{\chi})` is a function of unconstrained parameters :math:`\boldsymbol{\eta}` and constrained parameters :math:`\boldsymbol{\chi}`. The latter has corresponding one-dimensional constraint terms :math:`c_{\chi}(a_{\chi} |\, \chi)` with auxiliary data :math:`a_{\chi}` constraining the parameter :math:`\chi`. The event rates :math:`\nu_{cb}` are defined as:

.. math::

   \nu_{cb}\left(\boldsymbol{\phi}\right) = \sum_{s\in\mathrm{\,samples}} \nu_{scb}\left(\boldsymbol{\eta},\boldsymbol{\chi}\right) = \sum_{s\in\mathrm{\,samples}}\underbrace{\left(\prod_{\kappa\in\,\boldsymbol{\kappa}} \kappa_{scb}\left(\boldsymbol{\eta},\boldsymbol{\chi}\right)\right)}_{\text{multiplicative modifiers}}\,
   \Bigg(\nu_{scb}^0\left(\boldsymbol{\eta}, \boldsymbol{\chi}\right) + \underbrace{\sum_{\Delta\in\boldsymbol{\Delta}} \Delta_{scb}\left(\boldsymbol{\eta},\boldsymbol{\chi}\right)}_{\text{additive modifiers}}\Bigg)\,.

The total rates are the sum over sample rates :math:`\nu_{csb}`, each determined from a nominal rate :math:`\nu_{scb}^0` and a set of multiplicative and additive rate modifiers :math:`\boldsymbol{\kappa}(\boldsymbol{\phi})` and :math:`\boldsymbol{\Delta}(\boldsymbol{\phi})`. These modifiers are functions of (usually a single) model parameter.

Rate modifications are defined in `HistFactory <https://cds.cern.ch/record/1456844/files/CERN-OPEN-2012-016.pdf>`_ for bin :math:`b`, sample :math:`s`, and channel :math:`c`. Each modifier is represented by a parameter :math:`\phi \in \{\gamma, \alpha, \lambda, \mu\}`. By convention, bin-wise parameters are denoted with :math:`\gamma` and interpolation parameters with :math:`\alpha`. The luminosity :math:`\lambda` and scale factors :math:`\mu` affect all bins equally. For constrained modifiers, the implied constraint term is given as well as the necessary input data required to construct it. :math:`\sigma_b` corresponds to the relative uncertainty of the event rate, whereas :math:`\delta_b` is the event rate uncertainty of the sample relative to the total event rate :math:`\nu_b = \sum_s \nu^0_{sb}`.

.. image:: https://pyhf.github.io/pyhf-tutorial/_images/modifiers_and_constraints.png
   :align: center