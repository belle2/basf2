What is Hydra?
##############

`Hydra <https://github.com/MultithreadCorner/Hydra>`_ is a header-only C++ framework designed to perform common data analysis tasks on massively parallel platforms. It is implemented on top of the C++14 Standard Library and a `variadic version of the Thrust library <https://github.com/andrewcorrigan/thrust-multi-permutation-iterator/tree/variadic>`_.

It provides a  collection of optimized containers and algorithms commonly used in HEP data analysis that can be deployed transparently on different backends, allowing the user to re-use the same code across a large range of available multicore CPUs and GPUs. Currently, Hydra supports four backends:

* CPP (*i.e.*, the sequential single-threaded backend defined in Thrust);
* `OpenMP <http://www.openmp.org/>`_;
* `TBB <http://www.threadingbuildingblocks.org/>`_;
* `CUDA <http://developer.nvidia.com/cuda-toolkit.>`_.
   
Code can be dispatched and executed in all supported backends concurrently and asynchronously in the same program, using the suitable policies represented by the symbols ``hydra::cpp::sys``, ``hydra::omp::sys``, ``hydra::tbb::sys``, ``hydra::cuda::sys``, ``hydra::host::sys`` and ``hydra::device::sys``. Where applicable, these policies define the memory space where resources should be allocated to run algorithms and store data.

For mono-backend applications source files written using Hydra and standard C++ compile for CPU and GPU just exchanging the extension from ``.cu`` to ``.cpp`` and one or two compiler flags. So, basically, there is no need to refactory code to deploy different backends.

For more information about the design features and available algorithms see Hydra's `reference guide <https://multithreadcorner.github.io/Hydra/index.html>`_ or the `manual <https://hydra-documentation.readthedocs.io/en/latest/index.html>`_ (but notice that, at the time of writing, the manual was not updated yet with the description of the Hydra 3 interface). Plenty of example code is available in the `repository <https://github.com/MultithreadCorner/Hydra/tree/master/examples>`_.

Coding with Hydra
#################

Hydra framework provides one dimensional STL-like vector containers for each supported backend, aliasing the underlying Thrust types:

* ``hydra::cpp::vector`` : storage allocated in the CPP backend (usually the CPU memory);
* ``hydra::omp::vector``: storage allocated in the OpenMP backend (usually the CPU memory space);
* ``hydra::tbb::vector``: storage allocated in the TBB backend (usually the CPU memory space);
* ``hydra::cuda::vector``: storage allocated in the CUDA backend (the GPU memory space);
* ``hydra::host::vector``: storage allocated in the host backend (CPP, OpenMP or TBB, defined at compile time);
* ``hydra::device::vector``: storage allocated in the device backend (CPP, OpenMP, TBB or CUDA, defined at compile time).

Data is always copiable across different backends and movable between containers on the same backend.

In addition, Hydra implements two native multidimensional containers (imagine them as a table, where each row corresponds to an entry and each column to a dimension):

* ``hydra::multivector``, suitable to store datasets where the dimensions consist of elements having different type, and
* ``hydra::multiarray``, designed to store datasets where all dimensions have the same type.

``hydra::multivector`` templates are instantiated passing the type list corresponding to each dimension via a ``hydra::tuple`` and the backend where memory will be allocated:

.. code:: c++

	hydra::multivector<hydra::tuple<int, int, double, double>, hydra::device::sys_t> mvector;
	
``hydra::multiarray`` templates are instantiated passing the number of dimensions, the type and the backend where memory will be allocated:

.. code:: c++

	hydra::multiarray<4, double, hydra::device::sys_t> marray;

Hydra’s multi-dimensional containers can hold any type of data per dimension, but the performance gains are bigger when using these containers for describing dimensions with *Plain Old Data* types.

The design of ``hydra::multivector`` and ``hydra::multiarray`` makes possible to iterate over the container to access a complete row or to iterate over one or more columns to access only the data of interest in a given entry, without loading the entire row. As an example,

.. code:: c++

	for(int i=0; i<10; i++) mvector.push_back(hydra::make_tuple(i, 2*i, i, 2*i));

	for(auto x : mvector) std::cout << x << std::endl;
	
will output

.. code:: sh

	(0, 0, 0.0, 0.0)
	(1, 2, 1.0, 2.0)
	(2, 4, 2.0, 4.0)
	...
	(9, 18, 9.0, 18.0)

while

.. code:: c++

	for(auto x = mvector.begin(hydra::placeholders::_1, hydra::placeholders::_3); x != mvector.end(hydra::placeholders::_1,hydra::placeholders::_3); x++ )
		std::cout << *x << std::endl;
	
will output only the second and fourth columns

.. code:: sh

	(0, 0.0)
	(2, 2.0)
	(4, 4.0)
	...
	(18, 18.0)


Hydra calls user’s code using functors (*i.e.*, objects representing a function). All functors derive from ``hydra::BaseFunctor<...>`` and need to implement the ``Evaluate(...)`` method. For example, a generic functor of two variables (``X`` and ``Y``) and 4 parameters, returning a ``double``, can be defined as:

.. code:: c++
	
	template<typename X, typename Y, typename Signature=double(X,Y)>
	class MyFunctor: public hydra::BaseFunctor<MyFunctor<X,Y>, Signature, 4>
	{
		typedef hydra::BaseFunctor<MyFunctor<X,Y>, Signature, 4> super_type;
		using super_type::_par;
	
	public:
		...
		
		__hydra_dual__ inline
		double Evaluate(X x, Y y) const
		{
			double p0 = _par[0];
			double p1 = _par[1];
			double p2 = _par[2];
			double p3 = _par[3];
			
			double value = function(x,y,p0,p1,p2,p3);
			
			return value;
		}
		
		...
	};

The instruction ``__hydra_dual__`` makes the functor callable on both the CPU and GPU host/device memory spaces. Functors can also be implemented as C++ lambdas, which can be wrapped into a suitable Hydra object invoking the function template ``hydra::wrap_lambda()``:

.. code:: c++

	auto myFunctor = hydra::wrap_lambda(
		[=] __hydra_dual__ (double x, double y){
			return sin(x)*cos(y);
		}
	);
	

If ``A``, ``B`` and ``C`` are Hydra functors, they can be combined with the following code

.. code:: c++
	
	// Basic arithmetic operations
	auto A_plus_B = A + B;
	auto A_minus_B = A - B;
	auto A_times_B = A * B;
	auto A_per_B = A/B;

	// Any composition of basic operations
	auto any_functor = (A - B)*(A + B)*(A/C);
	
	// A function of functions, e.g., C(A,B) is represented by
	auto compose_functor = hydra::compose(C, A, B)

There is no intrinsic limit on the number of functors participating on arithmetic or composition of mathematical expressions.

To call functors users are able to define new types, with *ad-hoc* names wrapping around primary types, using the macro ``declarg(NewVar, Type)``. These new types are searched at compile time to bind the function call. If the type is not found a compile error is emitted, avoiding the generation of invalid or error-prone code. In the following example

.. code:: c++

	declarg(X, double)
	declarg(Y, double)
	declarg(Z, double)
	using namespace hydra::arguments;

	int main(int argv, char** argc)
	{
		...
		
		hydra::multivector< hydra::tuple<X,Y>,   hydra::device::sys_t> data1(nentries);
		hydra::multivector< hydra::tuple<Y,X>,   hydra::device::sys_t> data2(nentries);
		hydra::multivector< hydra::tuple<X,Y,Z>, hydra::device::sys_t> data3(nentries);
		hydra::multivector< hydra::tuple<X,Z,Y>, hydra::device::sys_t> data4(nentries);
		
		...
				
		// define a functor to evaluate and print the product of X and Y
		auto xy_printer = hydra::wrap_lambda(
			[] __hydra_dual__ (X x, Y y) {
				std::cout << x << " * " << y << " = " << x*y << std::endl;
			}
		);
		
		// runs on all containers independenly of the order with which X and Y appear
		// and/or independently of whether other variables (e.g., Z) are present
		for(auto entry : data1) xy_printer(entry);
		for(auto entry : data2) xy_printer(entry);
		for(auto entry : data3) xy_printer(entry);
		for(auto entry : data4) xy_printer(entry);
		
		// print only Z
		for(auto z : hydra::column<Z>(data3) ) std::cout << z << std::endl;
		for(auto z : hydra::column<Z>(data4) ) std::cout << z << std::endl;
		
	}
	
the functor ``xy_printer`` can be evaluated using as argument any data format containing the variables types ``X`` and ``Y``. The type identifiers will be searched among the elements and, if found, conversion to the appropriate types is performed.

Fitting with Hydra
##################

Hydra implements an interface to `Minuit2 <https://root.cern.ch/doc/master/Minuit2Page.html>`_ that parallelizes the FCN calculation. This dramatically accelerates the minimization, particulalry over large datasets. Hydra normalizes the PDFs on-the-fly using analytical or numerical integration algorithms provided by the framework itself and handles data using iterators.

Parameters
----------

Parameters are represented by the ``hydra::Parameter`` class, which can hold information about the parameter's name, value, error and limits. A parameter can be initialized using any of the following syntax:

.. code-block:: c++

	auto P1 = hydra::Parameter::Create().Name("P1").Value(5.291).Error(0.0001).Limits(5.28, 5.3);
	auto P2 = hydra::Parameter::Create("P2").Value(5.291).Limits(5.28, 5.3).Error(0.0001);
	hydra::Parameter P3("P3" ,5.291 ,0.0001, 5.28, 5.3) ;

PDFs
----

PDFs are represented by the ``hydra::Pdf<Functor, Integrator>`` class template, which is defined by binding a positive defined ``Functor`` to an ``Integrator``. PDFs can be conveniently built using the template function ``hydra::make_pdf(Functor, Integrator)``, as shown in the following example:

.. code-block:: c++

	hydra::Parameter mean = hydra::Parameter::Create("mean").Value( 5.28).Limits(5.27,5.29).Error(0.0001);
	hydra::Parameter sigma = hydra::Parameter::Create("sigma").Value(0.0027).Error(0.0001).Limits(0.0025,0.0029);
	
	double min(5.2), max(5.3);
	auto Signal_PDF = hydra::make_pdf( hydra::Gaussian<double>(mean, sigma), hydra::AnalyticalIntegral<hydra::Gaussian<double>>(min, max));

The example below shows instead how to build a PDF by wrapping a parametric C++ lambda representing a Gaussian and binding it to a Gauss-Kronrod (numerical) integrator:

.. code-block:: c++

	auto myGaussian = hydra::wrap_lambda(
		[=] __hydra_dual__ (unsigned int npar, const hydra::Parameter* params, double x)
		{
			double m = params[0].GetValue();
			double s = params[1].GetValue();
			double z = (x-m)/s;
			return exp(-z*z/2.);
	}, mean, sigma);
	
	auto Gaus_PDF = hydra::make_pdf(myGaussian, hydra::GaussKronrodQuadrature<61,100, hydra::device::sys_t> GKQ61(min, max); );

The PDF evaluation and normalization can be executed in different backends. PDF objects cache the normalization integrals results and the user can monitor the cached values and corresponding uncertainties.

It is also possible to represent models composed by the sum of two or more PDFs. Such models are represented by the class templates ``hydra::PDFSumExtendable<Pdf1, Pdf2,...>`` and ``hydra::PDFSumNonExtendable<Pdf1, Pdf2,...>`` and can be built using the function ``hydra::add_pdfs({c1, c2,...}, pdf1, pdf2,...)``. If the number of coefficients ``c1, c2,...`` is equal to the number of PDFs, then they are interpreted as yields ``hydra::PDFSumExtendable<Pdf1, Pdf2,...>`` is used:

.. code-block:: c++
	
	auto slope = hydra::Parameter::Create("slope").Value(1.0).Error(0.0001).Limits(-2.0, 2.0);
	
	auto Bkg_PDF = hydra::make_pdf( hydra::Exponential<double>(slope), hydra::AnalyticalIntegral<hydra::Exponential<double>>(min, max));
	
	auto nsig = hydra::Parameter::Create("nsig").Value(1e5).Error(0.1);
	auto nbkg = hydra::Parameter::Create("nbkg").Value(1e3).Error(0.1);
	
	auto model = hydra::add_pdfs( {nsig, nbkg}, Signal_PDF, Bkg_PDF);
	model.SetExtended(1);
	

If the number of coefficients is equal to the number of PDFs minus 1, then they are interpreted as fractions defined in [0,1] and ``hydra::PDFSumNonExtendable<Pdf1, Pdf2,...>`` is used instead.
 
The user can get a reference to one of the component PDFs using the method ``PDF(hydra::placeholder)``. Same operation can be performed for coefficients using the method ``Coefficient(unsigned int)``:
 
.. code-block:: c++
	
	// Change the mean of the Gaussian
	model.PDF( hydra::placeholders::_0 ).SetParameter(0, 2.);

	// Set signal yield to 1.5e4
	model.Coefficient(0).SetValue(1.5e4);
	
The minimization
----------------

The FCN is defined by binding a PDF to the data. Hydra implements classes and interfaces to allow the definition of FCNs suitable to perform maximum likelihood fits on unbinned and binned datasets. Different types of log-likelihood FCNs can be created by specializing the class template ``hydra::LogLikelihoodFCN<PDF, Iterator, Extensions...>`` or, for example, by using the function templates ``hydra::make_loglikelihood_fcn(...)``:

.. code-block:: c++

	// Generate data from the model
	hydra::device::vector<double> data(nentries);
	auto range = hydra::sample(data.begin(), data.end(), min, max, model.GetFunctor());
	
	// Build likelihood
	auto fcn = hydra::make_loglikehood_fcn( model, range );
	
	// Setup Minuit
	ROOT::Minuit2::MnPrint::SetLevel(3);
	hydra::Print::SetLevel(hydra::WARNING);
	MnStrategy strategy(2);
	
	MnMigrad migrad(fcn, fcn.GetParameters().GetMnState(), strategy);
	
	auto minimum = FunctionMinimum( migrad(5000,1.) );

Simultaneous fitting
--------------------

Hydra supports multi-layered simultaneous fits of different models, over different datasets, deploying different parallelization strategies for each model. No categorization of the dataset is needed. It is sufficient to setup the different components of the FCN, which can be optimized in isolation or in the context of the simultaneous FCN. Simultaneous FCNs can be created via direct instantiation or using the convenience function ``hydra::make_simultaneous_fcn(...)``, as shown in the following

.. code:: c++

	//=====================================================================================
	//                                                           +----< fcn(model-x)
	//                           +----< simultaneous fcn 1 ----- |
	//                           |                               +----< fcn(model-y)
	//   simultaneous fcn   <----+
	//                           |                               +----< fcn(model-w)
	//                           +----< simultaneous fcn 2 ------|
	//                           |                               +----< fcn(model-z)
	//                           +----< fcn(model-v)
	//=====================================================================================
	auto fcnX    = hydra::make_loglikehood_fcn(modelX, dataX);
	auto fcnY    = hydra::make_loglikehood_fcn(modelY, dataY);
	auto fcnW    = hydra::make_loglikehood_fcn(modelY, dataY);
	auto fcnZ    = hydra::make_loglikehood_fcn(modelZ, dataZ);
	auto fcnV    = hydra::make_loglikehood_fcn(modelv, datav);
	
	auto sim_fcn1 = hydra::make_simultaneous_fcn(fcnx, fcny);
	auto sim_fcn2 = hydra::make_simultaneous_fcn(fcnw, fcnz);
	auto sim_fcn  = hydra::make_simultaneous_fcn(sim_fcn1, sim_fcn2, fcnV);

