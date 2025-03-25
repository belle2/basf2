Fitting examples
================

Fits used in Belle II analyses can be simple or challenging. The examples listed below cover different levels of complexity and represent a good starting point for anybody interested to learn the basics of fitting:

.. toctree::
	:maxdepth: 1
	
	simple_1d_fit.rst
	efficiency_fit.rst
	lifetime_2d_fit.rst
	dalitz_fit.rst

For each example, fitting code has been developed within at least one of the following frameworks:

* `RooFit <https://root.cern/manual/roofit/>`_ - the recommended framework included in ROOT
* `zfit <https://github.com/zfit/zfit>`_ - a python-based fitter
* `MINUIT <http://seal.web.cern.ch/seal/snapshot/work-packages/mathlibs/minuit/>`_ - the basic tool upon which RooFit and zfit rely, can be useful if you want to have better control over the details of the fit
* `bat <https://github.com/bat>`_ - the Bayesian Analysis Toolkit
* `Hydra <https://github.com/MultithreadCorner/Hydra>`_ - a framework for multithreaded data analysis

