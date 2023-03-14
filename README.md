# Belle II Analysis Software Framework (basf2)

[![DOI:10.5281/zenodo.5574115](https://zenodo.org/badge/DOI/10.5281/zenodo.5574115.svg)](https://doi.org/10.5281/zenodo.5574115)
[![License: LGPL v3 or later](https://img.shields.io/badge/License-LGPL%20v3%20or%20later-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)

This repository contains the main software of the
[Belle II experiment](https://www.belle2.org/).

It is used in all aspects of the data-processing chain, such as

- unpacking of real raw data,
- generating simulated data,
- reconstruction of particle decays (tracking, clustering, ...)

and high-level "analysis" reconstruction (such as applying selection criteria,
vertex fitting, ...).

In addition to the code of this repository, the following three repositories are required:

* Belle II tools ([public repository](https://github.com/belle2/tools), [internal repository](https://gitlab.desy.de/belle2/software/tools)): Scripts to install and set up basf2
* Belle II externals ([public repository](https://github.com/belle2/externals), [internal repository](https://gitlab.desy.de/belle2/software/externals)): Manages external dependencies of basf2
* Belle II versioning ([public repository](https://github.com/belle2/versioning), [internal repository](https://gitlab.desy.de/belle2/software/versioning/)): Manages the different versions of basf2 and related conditions database global tags

Note that the history of the code is split in a part before ([public repository](https://github.com/belle2/basf2/commits/history), [internal repository](https://stash.desy.de/projects/B2/repos/history/commits)) and after ([public repository](https://github.com/belle2/basf2/commits/main), [internal repository](https://gitlab.desy.de/belle2/software/basf2/-/commits/main)) July 7, 2021.
Both parts are combined to a complete history with the `b2code-history` command.

Head to [our documentation](https://software.belle2.org/) to learn about
how to use basf2. If you are unsure about which version of the documentation to
read, click on `release-xx-xx-xx (recommended)`. We
recommend you to start with the lessons of the `Beginners' tutorials` if you are
just starting out.

More information about the core software can be found in the
[2018 paper](https://arxiv.org/abs/1809.04299).