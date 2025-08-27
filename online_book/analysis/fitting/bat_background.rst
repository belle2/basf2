What is BAT?
############

`BAT <https://github.com/bat/bat>`_ is a C++ library to perform
Bayesian analysis using Markov-chain Monte Carlo techniques. It is
based on ROOT, and therefore easily interfaces with the standard code
of HEP analyses.

It will sample from the posterior probability distribution of the parameters of
your model given the data you test it against. It will show you the full
posterior distribution for you parameters without approximation and will show
you the full (often complicated) correlations among your parameters. It can also
show you the probability distribution for any function of your
parameters---whether it is involved in the fit or not.

Bayesian Statistics in a nutshell
---------------------------------

Bayes' theorem can be used to relate the probability for parameters in
a model to have certain values to the ability of that model to
describe the data.

The ability of the model to the describe the data is known as the
likelihood. I will assume you are familiar with the basics of
likelihoods---as you might have used them with (ROOT) Minuit or
RooFit.

Bayes' theorem tells us

.. math::
   P(\vec{\lambda}) \propto L(\mathrm{data}|\vec{\lambda}) \times P_0(\vec{\lambda}).

The left-hand side is the posterior probability distribution for the parameters
of our model, :math:`\vec{\lambda}`---what we are interested in learning. Here
"posterior" means that it is your state of knowledge after performing an
analysis.

The right-hand side is the product of the likelihood of the data given
a point in our parameter space and a prior probability for that
parameter point---the state of knowledge of the parameters before the analysis.

In BAT (and in many codes), we work with the logarithms of our probabilities, so
Bayes theorem becomes

.. math::
   \log P(\vec{\lambda}) = \log\ L(\mathrm{data}|\vec{\lambda}) + \log\ P_0(\vec{\lambda})

plus a constant we ignore.

In a binned fit, the log-likelihood is a sum of Poisson log-likelihoods
for each bin

.. math::
   \log L(\mathrm{data}|\vec{\lambda}) = \sum_b \ \log \ \text{Poisson}(N_b|\nu_b(\vec{\lambda}))

where :math:`N_b` is the number of events observed in bin :math:`b`
and :math:`\nu_b` is the number of events expected in bin :math:`b`
calculated from your fit model. The prior function may be any function
that maps from the parameter space to the positive real numbers; but
the most common situation is that is factorized,

.. math::
   P_0(\vec{\lambda}) = \prod_i \text{ pdf}_i(\lambda_i).

In an unbinned fit, the log-likelihood is the sum of the log of your fit
function for each event:

.. math::
   \log\ L(\mathrm{data}|\vec{\lambda}) = \sum_i \log\ L(\vec{x}_i|\vec{\lambda}_i)

where the data set consists of data points :math:`\vec{x}_i`.

:math:`P(\vec\lambda)` is the multidimensional posterior probability
density. If we want to get the probability density as a function of
only one parameter of interest, we integrate out all other parameters:

.. math::
   P(\lambda_i) = \int P(\vec\lambda) d\lambda_0 d\lambda_1 \cdots d\lambda_{i-1} d\lambda_{i+1} \cdots.

This is known as the marginalized posterior probability
distribution. We can marginalize down to any number of dimensions
(smaller than the original dimensionality of :math:`\vec\lambda`). BAT
can create 1D and 2D margninalizations; but you can use the output of
BAT to create higher-dimension marginalizations.

How does BAT explore the posterior?
-----------------------------------

BAT uses the so-called *random walk Metropolis-Hastings* variety of a
Markov chain. It walks its way through the posterior probability
distribution in such a way that it tries to see all corners of
parameter space, no matter how unlikely they are, while yet spending
most of its time in the most likely areas.

It does this via the following pseudo-algorithm:

1. Starting at some location :math:`\vec\lambda_0`, BAT proposes a new
   point to move to, :math:`\vec\lambda_*` that is in the vicinity of
   :math:`\vec\lambda_0`.

   (The proposal function accounts for correlations between the
   individual parameters in the parameter vector, and chooses a point
   at random within a radius that is tuned by BAT.)

2. BAT calculates the posterior probabilities at :math:`\vec\lambda_0`
   and :math:`\vec\lambda_*`: :math:`P(\vec\lambda_0)` and
   :math:`P(\vec\lambda_*)`

3. BAT decides whether or not to move from :math:`\vec\lambda_0` to
   :math:`\vec\lambda_*`---that is, it chooses :math:`\vec\lambda_1`:

   - If :math:`P(\vec\lambda_*) > P(\vec\lambda_0)`, then
     :math:`\vec\lambda_1 = \vec\lambda_*`.

     That is, if :math:`\vec\lambda_*` is more probable than
     :math:`\vec\lambda_0`, BAT moves there.

   - Else, BAT chooses a random number, :math:`r`, between 0 and 1; if
     :math:`r < P(\vec\lambda_*)\ /\ P(\vec\lambda_0)`, then
     :math:`\vec\lambda_1 = \vec\lambda_*`, else :math:`\vec\lambda_1
     = \vec\lambda_0`.

     That is, if :math:`\vec\lambda_*` is less probable than
     :math:`\vec\lambda_0`, BAT moves to it but only with probability
     :math:`P(\vec\lambda_*)\ /\ P(\vec\lambda_0)`; the probability
     that it stays at :math:`\vec\lambda_0` is :math:`1 -
     P(\vec\lambda_*)\ /\ P(\vec\lambda_0)`.

Now with this new point, :math:`\vec\lambda_1`, BAT starts again from
step 1. By doing this over and over, it moves around the parameter
space according to the posterior probability. By storing the locations
it visits in histograms, we get *marginalizations* of the posterior.

The basics necessary for a BAT fit
##################################

There are a few basic requirements from your side to run a fit in BAT. Each of
the requirements is coded into a model, that in C++ is a class that inherits
from BAT's ``BCModel`` class. To create a BAT project and a BAT model within
this project, from your command line, run

.. code-block:: sh

   bat-project [project-name] [model-name]

This will create a directory with the name of the project and a program source
file ``run[project-name].cxx`` to run your analysis. It will also create a
header and source for your model: ``[model-name].h`` and ``[model-name].cxx``;
and a Makefile for compiling everything.

1. Declare parameters of your model, giving their ranges and defining their priors.

   This is usually done inside the ``constructor`` of your model by

   .. code-block:: c++

      AddParameter("signal mode", 5.2, 5.3, "#mu", "[events]");

   The first three arguments---the parameter name and its lower and upper
   limits---are required. The fourth (optional) argument is a name in
   (ROOT-style) LaTeX formatting for plotting; and the fifth (optional) argument
   is units string for plotting.

   Individual priors may be set for each parameter; or, if applicable to your
   problem, you can set all priors constant via

   .. code-block:: c++

      SetPriorConstantAll();

   You can also set any arbitrary multi-dimensional prior by overriding the
   member function

   .. code-block:: c++

      double LogAPrioriProbability(const std::vector<double>& P);

2. Code a likelihood for a parameter point. This is accomplished by overriding
   the member function

   .. code-block:: c++

      double LogLikelihood(const std::vector<double>& P);

   For a binned fit, typically you will loop over entries in your histogram and
   calculate a Poisson likelihood from the number of events in the histogram and
   the number of events your fit function predicts:

   .. code-block:: c++

      double log_likelihood = 0;
      for (int b = 1; b < h1.GetNbinsX(); ++b) {
         double n_obs = h1.GetBinContent(b);
         double n_exp = my_function( [bin location info], ..., P);
         log_likelihood += BCMath::LogPoisson(n_obs, n_exp);
      }
      return log_likelihood;

   For an unbinned fit, you will loop over your data points:

   .. code-block:: c++

      double log_likelihood = 0;
      for (auto data_point : data_set)
         log_likelihood += log(my_function(data_point, P));
      return log_likelihood;

When you compile your code (by executing ``make``) and run it (by executing
``./run[project-name]``) BAT will sample from your model and output the results
to the screen and a log file. It will also print the marginalized distributions
to an output file. To visually check the shape of your model (at its best-fit
point), you will need to code your own visualization; but an example is given in
the BAT tutorial code.

Learning more about BAT
-----------------------

To use more advanced features of BAT, consult the
`documentation <https://github.com/bat/bat>`_.
