What is Minuit?
###############

`Minuit <http://seal.web.cern.ch/seal/snapshot/work-packages/mathlibs/minuit/>`_ is a standalone package to find/calculate numerically

* the (local) minimum of any arbitrary function :math:`F(p)` (typically least-squares or negative log-likelihood), where :math:`p` is a set of parameters, and
* the covariance matrix of these parameters (at the minimum).

It was originally written in `fortran <https://root.cern.ch/download/minuit.pdf>`_ by F. James,
and later adapted to `C++ within ROOT <https://root.cern.ch/doc/master/classTMinuit.html>`_ by R. Brun.
`Minuit2 <https://root.cern.ch/doc/master/Minuit2Page.html>`_ is instead a compeltely re-designed and re-implemented version of Minuit in C++ by F. James and M. Winkler.
A python interface to Minuit2, called `iminuit <https://iminuit.readthedocs.io/en/latest/index.html>`_, also exists.

The function to minimize, internally called FCN, does not need to be known analytically. It is sufficient to know its value :math:`F(p)` at any point :math:`p`.
Minuit looks for a local minimum: *i.e.*, the point :math:`\hat{p}` where :math:`F(\hat{p}) < F(p)` for any :math:`p` in some neighborhood around :math:`\hat{p}`.
The main algorithm that performs the minimization in Minuit is called ``MIGRAD``. Its strategy to find the local minimum is simply to vary the set of
parameters :math:`p`, by small (variable-sized) steps, in a direction which causes :math:`F` to decrease until it finds the
point :math:`\hat{p}` from which :math:`F` increases in all allowed directions. Although not needed, if the numerical values of the derivative 
:math:`\partial F(p)/\partial p` at any point :math:`p` are known, they can be provided to ``MIGRAD`` to help in the minimization.

The minimization produces as a by-product also the covariance matrix of the parameters, though computed with limited accuracy. The algorithm ``HESSE`` is then provided to
calculate the full second-derivative matrix of the FCN, using a finite difference method, and improve the estimation of the parabolic uncertainties obtained by ``MIGRAD``.
The algorithm ``MINOS`` can instead be used to perform a scan of the FCN, profiled in each given dimension (*i.e.*, by minimizing all other parameters at each
scan point), around the local minimum to estimate asymmetric uncertainties. Finally, the algorithm ``CONTOUR`` can be used to profile the FCN in any given two
dimensions around the local minimum to estimate the border (contour) of 2D confidence-level intervals.

Contrarily to other frameworks, Minuit does not offert any interface/functionality to perform all other tasks related to fitting, such as data handling,
plotting (data visualization, fit projections, *etc.*), generation of pseudoexperiments, *etc.*

How to design a fitter based on Minuit
######################################

The design can be split in the following conceptual steps:

1. Prepare the data to fit to
2. Code the function to minimize
3. Configure Minuit
4. Specify the sequence of algorithms to use for minimization and estimation of the covariance matrix
5. Access fit results
6. Plot the results for graphical visualization
7. Prepare tools for validation of the fitter (*e.g.*, generation of pseudoexperiments)

Some general guidelines/instructions are given below about each of these steps with the exception of steps 1, 6 and 7, which implementation is 
completely independent from Minuit and hence left to the user. The instructions are based on the Minuit implementation available in ROOT, but can be easily ported to the
other Minuit versions. A few complete examples, which show also possible implementations of steps 1, 6 and 7, are instead made available `in the minuit subdirectory <https://gitlab.desy.de/belle2/software/training/b2-fitting-training/-/tree/main/minuit>`_.

Code the function to minimize
-----------------------------

In Minuit, the computation of the FCN should be implemented in a static external function with signature

.. code-block:: c++

	void fcn(int &npars, double *gin, double &f, double *pars, int flag);

where

* ``npars``: number of free parameters involved in minimization
* ``gin``: computed gradient values (optional)
* ``f`` : the function value itself
* ``pars``: vector of constant and variable parameters
* ``flag``: to switch between several actions of FCN

Since the FCN is an external function, to access the data used for the computation you need to put the data into an external static object, *e.g.*,

.. code-block:: c++

	std::vector<double> data;
	
	void fcn(int &, double *, double &f, double *pars, int ) {
		// compute -2*log(Likelihood)
		f = 0.;
		for (auto event : data) {
			double prob = pdf(event,pars);
			if (prob<=0.) prob = 1e-300;
			f -= 2.*log(prob);
		}
	}


Configure Minuit
----------------

After having initialized a Minuit object, the minimum configuration requires to set the FCN and define the fit parameters.
In general, however, a few additional configuration steps are needed. A typical case is shown below, using the ROOT class
`TFitter <https://root.cern.ch/doc/master/classTFitter.html>`_.
This interface can be used in a very close manner as the original fortran package, *i.e.*, passing commands through a character string (a detailed description
of the commands is available in `Chapter 4 of the original Minuit documentation <https://root.cern.ch/download/minuit.pdf>`_).

Initialize a ``TFitter`` object with a maximum of ``nparx`` total parameters (for memory allocation purposes) and set the FCN with

.. code-block:: c++

	TFitter *fitter = new TFitter(nparx);
	fitter->SetFCN(fcn);

Define and initialize the fit parameters with

.. code-block:: c++

	fitter->SetParameter(ipar,pname,pstart,pstep,plow,pup);

where

* ``ipar``: parameter index
* ``pname``: parameter name
* ``pstart``: initial value
* ``pstep``: initial step used to evaluate the gradient (if ``0`` parameter is set to a constant)
* ``plow``, ``pup``: lower and upper bounds (no bounds if both ``0``)

.. warning::
	In complicated problems, where multiple local minima are present, the fit result may depend on the choice of the initial values of the parameters.
	It is always advisable to check that this does not happen by sampling different starting points and be sure to have converged in the global minimum.

When lower and upper boundaries are specified on a parameter, Minuit internally converts the parameter using the following transformation

.. math::
	p_\mathrm{int} = \mathrm{arcsin}\left(2\frac{p-p_\mathrm{low}}{p_\mathrm{up}-p_\mathrm{low}} - 1\right),

such that the boundaries cannot be exceeded. `One-sided boundaries are possible only in Minuit2 <https://root.cern.ch/root/htmldoc/guides/minuit2/Minuit2.html#internal-and-external-parameters>`_.

.. warning::
	Boundaries should be avoided whenever possible: they complicate the problem (because the above transformation is non-linear) and, more importantly,
	they may affect the estimation of the error matrix by ``HESSE`` (because when a parameter gets close to the limit, the error matrix becomes singular).
	
.. hint::
	When using boundaries, try to place them as far away as possible from the guessed position of the minimum. Moreover, a good practice
	to ensure that the presence of the boundaries did not cause issues in the minimization/covariance estimation is to: (1) find the minimum with boundaries,
	(2) release the boundaries, (3) rerun ``MIGRAD`` and ``HESSE`` to confirm to be in a minimum and compute the uncertainties.

Parameters can also be fixed/released with

.. code-block:: c++

	fitter->FixParameter(ipar);
	fitter->ReleaseParameter(ipar);

For a reliable minimization and to ensure accurate results, always set strategy to 2

.. code-block:: c++

	double strategy(2.);
	fitter->ExecuteCommand("SET STRAT",&strategy,1);

You may need to set the error definition with

.. code-block:: c++

	double up(1.);
	fitter->ExecuteCommand("SET ERR",&up,1);

The errors are defined by the change in parameter value required to change the FCN value by up w.r.t. its minimum value.
The default value of 1 must be used to get the 1 :math:`\sigma` uncertainties when minimizing a least-squares or -2 log(likelihood) function.

For more stable fits, it may be useful to also set by hand the machine precision with

.. code-block:: c++

	double eps_machine(std::numeric_limits<double>::epsilon());
	fitter->ExecuteCommand("SET EPS",&eps_machine,1);


Run the minimization and compute uncertainties
----------------------------------------------

To perform the minimization use

.. code-block:: c++

	double maxcalls(5000.), tolerance(0.1);
	double arglist[] = {maxcalls, tolerance};
	unsigned int nargs(2);
	fitter->ExecuteCommand("MIGRAD",arglist,nargs);
	fitter->ExecuteCommand("HESSE",arglist,nargs);

The (optional) arguments in ``arglist`` correspond to the maximum allowed number of iterations and to the tolerance, respectively.
The tolerance specifies when the minimization will stop, *i.e.* when the estimated distance to the minimum (EDM) is less than 0.001*[tolerance]*[up].

.. hint::
	What if ``MIGRAD`` does not converge? First, check the implementation of the FCN (*e.g.*, incorrect PDF normalization in the likelihood,
	ill-defined problem with too many free parameters, parameters with too large correlations, *etc*.).
	It may be that the starting point is too far away from the solution and/or the FCN may have unphysical local minima,
	especially at infinity in some variables. Change starting values to avoid these regions, change parametrization, or add boundaries (but remember the caveats
	mentioned above).

.. warning::
	The fit may converge even for ill-defined problem. Always check that the error matrix is positive-definite at the minimum (if not, the estimated uncertainties
	are meaningless).

For the best estimate of the uncertainties run ``MINOS`` with

.. code-block:: c++

	double arglist[] = {maxcalls, ipar1, ipar2, ...};
	fitter->ExecuteCommand("MINOS",arglist,nargs);

The (optional) arguments are again the maximum number of iterations and the indices of the parameters for which to perform the computation (if none are specified,
``MINOS`` uncertainties are calculated for all variable parameters).

.. warning::
	``MINOS`` may be computationally expensive, particularly for large numbers of free parameters, but they are a must
	whenever there is need to account for non-linearities in the problem as well as for strong parameter correlations.


Access fit results
------------------

Fit results will be printed on screen and can be accessed with

.. code-block:: c++

	fitter->GetParameter(ipar);

	fitter->GetParError(ipar);
	
	fitter->GetCovarianceMatrixElement(ipar,jpar);

	char name[20];
	double value, eparab, elow, ehigh;
	fitter->GetParameter(ipar,name,value,eparab,elow,ehigh);

where ``eparab`` is the parabolic uncertainty and ``elow``, ``ehigh`` are the asymmetric uncertainties (available if ``MINOS`` did run).

.. warning::
	If there are fixed parameters, to retrieve the covariance between ``ipar`` and ``jpar`` you should first shift the parameter indices accordingly, *e.g.*
	
	.. code-block:: c++

		int ioff(0), joff(0);
		for(int k=0; k<ipar; ++k)
			if(fitter->IsFixed(k)) ioff++;
		for(int k=0; k<jpar; ++k)
			if(Fitter()->IsFixed(k)) joff++;
	
		double covij = fitter->GetCovarianceMatrixElement(ipar-ioff,jpar-joff);



Details about the minimum can be accessed from the underlying ``TMinuit`` object with

.. code-block:: c++

	TMinuit *minuit = fitter->GetMinuit();
	double fmin, fedm, up;
	int npari, nparx, istat;
	minuit->mnstat(fmin,fedm,up,npari,nparx,istat);

where

* ``fmin``: value of the function at the current position in the parameters space (the minimum is the fit converged)
* ``fedm``: the estimated vertical distance remaining to minimum
* ``up``: the value defining the parameter uncertainties
* ``npari``: the number of currently variable parameters
* ``nparx``: the highest (external) parameter number defined by the user when initializing the fitter
* ``istat``: a status integer indicating how good is the covariance matrix:
	* ``0`` = not calculated at all
	* ``1`` = approximation only, not accurate
	* ``2`` = full matrix, but forced positive-definite
	* ``3`` = full accurate covariance matrix
	
	

