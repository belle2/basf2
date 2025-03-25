.. _bat_simple_1d_fit_comparison:

Simple 1D Model Comparison with BAT
###################################

This directory contains an example of using BAT to compare models for
a 1D fit. Please see the examples in :ref:`bat_simple_1d_fit` to learn the
basics of creating a such a model in BAT and how the binned likelihood
is formulated.

To run this code, you will need to install BAT with the Cuba
library. See the intructions in :ref:`bat_intro` for how install
BAT with Cuba.

The Data
========

The data to be analyzed in this example is stored in a file in a
directory above this one at: ``../../example-data/fitme.root``. In
this ROOT file is a ``TTree`` ("BtoKstG") with a branch named
"B0_mbc", that contains the beam-constrained masses of reconstructed
B0 mesons. We will perform binned fits to this data. Nothing in this
example is exclusive to bin fits, but for ease of presentation and
speed, we used a binned fit.

The Models
==========

In all fits, we model the background as being distributed according to
an ARGUS function

.. math::

   P(x) \propto \left\{\begin{array}{lcccl} x \ \sqrt{k(x)} \ E^{-\frac{1}{2} \chi^2 k(x)} & & \mathrm{if} & & x \leq x_0\\ 0& & \mathrm{if} & & x > x_0.\end{array}\right.

where :math:`k(x) \equiv 1 - x / x_0`.

We use three different models for the signal (described below). The
signal and background components are each normalized over the range of
the data. All functions are coded in ``Functions.h``



Model: Crystal Ball
-------------------

In this model, the signal is distributed according to a Crystal Ball
function

.. math::

   P(x) \propto \left\{\begin{array}{lcccl} e^{-\frac{1}{2} [k(x)]^2} & & \mathrm{if} & & x \leq \alpha (k - \alpha) \leq 0\\ e^{-\frac{1}{2} \alpha^2} [1 + \frac{\alpha}{n}(k(x) - \alpha)]^{-n} & & \mathrm{if} & & \alpha (k - \alpha) > 0.\end{array}\right.

where :math:`k(x) \equiv (x - \mu) / \sigma`. This function is normal
at its mode, with a tail to one side (the direction of which depends
on the sign of :math:`\alpha`) that is a power distribution. This model
has eight parameters: two yields (:math:`Y_S, Y_B`); two parameters
for the background (:math:`x_0, \chi`); and four for the signal
(:math:`\mu, \sigma, \alpha, n`).

Mode: Limited Crystal Ball
--------------------------

As we saw in the examples in :ref:`bat_simple_1d_fit`, the Crystal Ball
function possibly has one unnecessary degree of freedom for our
fit. In this model, this degree of freedom is removed and the signal
is distributed according to a Crystal Ball function in the limit of
large :math:`n`:

.. math::

   P(x) \propto \left\{\begin{array}{lcccl} e^{-\frac{1}{2} [k(x)]^2} & & \mathrm{if} & & x \leq \alpha (k - \alpha) \leq 0\\ e^{\frac{1}{2} \alpha^2} e^{-\alpha k(x)} & & \mathrm{if} & & \alpha (k - \alpha) > 0.\end{array}\right.

where :math:`k(x) \equiv (x - \mu) / \sigma`. This function is normal
at its mode, with an exponential tail to one side (the direction of
which depends on the sign of :math:`\alpha`). This model has seven
parameters: two yields (:math:`Y_S, Y_B`); two parameters for the
background (:math:`x_0, \chi`); and three for the signal (:math:`\mu,
\sigma, \alpha`).

Model: Double Normal
--------------------

In this model, the signal is distributed according to the (weighted)
sum of two normal functions:

.. math::

   P(x) \propto f \mathcal{N}(x | \mu_1, \sigma_1) + (1 - f) \mathcal{N}(x | \mu_2, \sigma_2).

This model has nine parameters: two yields (:math:`Y_S, Y_B`); two
parameters for the background (:math:`x_0, \chi`); and three for the
signal (:math:`\mu_1, \sigma_1, \mu_2 - \mu_1, \sigma_2/\sigma_1, f`).


Model Comparison
----------------

In Bayesian statistics, one compares models by the ratio of their
"evidences." The evidence of a model is the integral of the likelihood
times the prior:

.. math::

   Z \equiv \int L(\mathrm{data}|\vec\lambda) \ P_0(\vec\lambda) d\vec\lambda

This is the probability of the model to describe the data regardless
of any specific values of parameters of the model. The integral is
over all parameters of the model and over their entire allowed
ranges. The Markov-chain algorithm of BAT cannot tell us the
evidence. To integrate

When comparing two models, A and B, the ratio of evidences---known as
the Bayes factor---tells you which model best explains the data:

.. math::

   K_{AB} = Z_A / Z_B.

If it is greater than 1, model A is more probable; if it is less than
1, model B is more probable. However, your personal belief in which
model is best depends on your prior beliefs in the models. The ratio
of your posterior beliefs---your beliefs after analyzing the data---is
proportional to the ratio of your prior beliefs by the Bayes factor:

.. math::

   P(A) / P(B) \equiv K_{AB} \times P_0(A) / P_0(B).

When comparing several models, BAT calculates the probability the
posterior probability for an individual model by

.. math::

  P(A) \equiv Z_A\ P_0(A) / [Z_A\ P_0(A) + Z_B\ P_0(B) + Z_C\ P_0(C) + \dots];

thus the sum of probabilities for each model is unity.

The Code
----------

To allow for easy generation of the models, we use one class
``Simple1DBinnedFit`` that takes a function in its constructor for
calculating the signal component integrals.

The code in ``runSimple1DBinnedFit.cxx`` resembles that in
``../Simple1Dfit/runSimple1DBinnedFit.cxx``, exept now we use
``BCModelManager`` to store and marginalize our models.

Rather than defining our parameters in the constructors of our models,
we define them in ``runSimple1DBinnedFix.cxx`` after instantiating
them.

By calling ``BCModelManager::MarginalizeAll()``, we marginalize all
the models contained in the model manager. Then inside the loop that
follows on the next lines, for each model, we improve the mode finding
with Minuit (via ``FindMode()``) and print the results of that model
to a pdf file.

``BCModelManager::Integrate()`` will integrate all the models,
calculating their evidences, which we need to compare them. BAT has
its own built-in methods to integrate models, but then can only handle
the very simplest models; the simple models in this example are
already too complicated. We rely on Cuba to integrate them. Be sure
you have configured BAT with the ``--with-cuba=download`` option
before installing it.

``BCModelManager::PrintSummary()`` will print the summaries of each
model it contains as well as a model comparison summary. The first
block in the model-comparison summary is of the priors:

::

   Summary :  - A priori probabilities:
   Summary :  (0) p(Crystal Ball) = 1.000000
   Summary :  (1) p(Limited Crystal Ball) = 1.000000
   Summary :  (2) p(Double Normal) = 1.000000

We added the models without the additional argument for a prior
belief, which defaults to unity, meaning all models have equal weight.

The second block contains the posteriors, calculated from both the
evidences and the priors:

::

   Summary :  - A posteriori probabilities:
   Summary :  (0) p(Crystal Ball | data) = 0.028975
   Summary :  (1) p(Limited Crystal Ball | data) = 0.970998
   Summary :  (2) p(Double Normal | data) = 0.000026

We see that the limited Crystal-Ball function, with one fewer degree
of freedom, is far more probable than the full Crystal-Ball function;
this is Occam's razor at work.

The final block of the output contains the Bayes factors for the
individual comparisons:

::

   Summary :  - Bayes factors:
   Summary :      K := p(data | Crystal Ball) / p(data | Limited Crystal Ball) = 0.029841
   Summary :      K := p(data | Crystal Ball) / p(data | Double Normal) = 1096.147862
   Summary :      K := p(data | Limited Crystal Ball) / p(data | Double Normal) = 36733.255213

We see the limited Crystal-Ball function is greatly preferred to both
the Crystal-Ball function and the double-normal function; and that the
Crystal-Ball is greatly preferred over the double-normal.

This tells us more than we can learn from looking at just a
:math:`\chi^2/\mathrm{ndf}` (as is given in
``Simple1DBinnedFit_comparison.pdf``).

As with the examples in :ref:`bat_simple_1d_fit`, try uncommenting the blocks
of code that set priors for the parameters---as we might do if we had
information about them from fits to simulated data.

If you uncomment all prior-setting blocks, the difference in degrees
of freedom between the models shrinks because now the parameters are
guided by the priors. Think about what would happen if you fixed the
parameters to values you got from fits to simulated data, instead of
using priors: each parameter you fix removes a degree of freedom. Here
the effect is somewhat the same, but more robust.

Having set priors, our posterior probabilities for the models
completely invert:

::

   Summary :  - A posteriori probabilities:
   Summary :  (0) p(Crystal Ball | data) = 0.188235
   Summary :  (1) p(Limited Crystal Ball | data) = 0.004994
   Summary :  (2) p(Double Normal | data) = 0.806771

---the double-normal is now most probable and the limited crystal-ball
is now least probable.
