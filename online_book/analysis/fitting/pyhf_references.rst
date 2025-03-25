References
**********

* `pyhf documentation <https://pyhf.readthedocs.io>`_
* `HistFactory paper <https://cds.cern.ch/record/1456844/files/CERN-OPEN-2012-016.pdf>`_
* `Asymptotic formulae for likelihood-based tests of new physics <https://arxiv.org/pdf/1007.1727>`_

`See here <https://github.com/lorenzennio/pyhf-tutorial>`_ for a pyhf tutorial page, with Belle 2 specific examples (check out the different branches).

cabinetry
=========

A set of tools have developed around `pyhf <https://pyhf.readthedocs.io>`_, which are very handy. There is one especially worth mentioning: `cabinetry <https://cabinetry.readthedocs.io/en/latest/>`_. It facilitates the model-building and visualization steps. You can build `pyhf <https://pyhf.readthedocs.io>`_ directly from `ntuples`, visualize models (data-mc comparison, pulls, rank plots, correlations, ...) and perform standardized statistical tests, not part of `pyhf <https://pyhf.readthedocs.io>`_ directly.

`See here <https://github.com/cabinetry/cabinetry-tutorials>`_ for the cabinetry tutorial page.

pyhfcorr
========

In pyhf, per definition, modifier parameters are either independent or fully (anti-)correlated. But we frequently encounter the situation where we need to arbitrarily need to correlate parameters, which modify our histogram templates in non-trivial ways. A very handy tool for this job is `pyhfcorr <https://github.com/lorenzennio/pyhfcorr>`_. You can correlate parameters in an arbitrary fashion with a simple pre-processing approach. This means, that the result will be a valid pyhf model, without adding additional dependencies.

`See here <https://github.com/lorenzennio/pyhfcorr>`_ for the pyhfcorr repository. An example notebook can be found `here <https://github.com/lorenzennio/pyhfcorr/blob/main/examples.ipynb>`_.