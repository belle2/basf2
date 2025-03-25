What is pyhf?
*************

The `HistFactory p.d.f. template <https://cds.cern.ch/record/1456844/files/CERN-OPEN-2012-016.pdf>`_ is per-se independent of its implementation in ROOT and sometimes, it’s useful to be able to run statistical analysis outside of ROOT, RooFit, RooStats framework.

pyhf is a pure-python implementation of that statistical model for multi-bin histogram-based analysis and its interval estimation is based on the asymptotic formulas of `Asymptotic formulae for likelihood-based tests of new physics <https://arxiv.org/pdf/1007.1727>`_. The aim is also to support modern computational graph libraries such as `JAX`, `PyTorch` and `TensorFlow` in order to make use of features such as autodifferentiation and GPU acceleration. Supported minimization backends are `scipy` and `Minuit`.

Getting started
***************

The best way to learn pyhf is through tutorials, such as `this one <https://pyhf.github.io/pyhf-tutorial/introduction.html>`_ or `this one <https://github.com/lorenzennio/pyhf-tutorial>`_. Furthermore, for more details, you can consult the `documentation <https://pyhf.readthedocs.io>`_ or the `open-source code <https://github.com/scikit-hep/pyhf>`_.