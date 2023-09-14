:tocdepth: 1

==========
What's New
==========

This page explains the new features in the current release. This cannot cover
all the changes in all packages but should help users to find out what needs to
be adapted when changing to the new release.

.. contents:: Table of Contents
   :depth: 3
   :local:

.. important changes should go here. Especially things that break backwards compatibility

Changes since release-08
========================

.. include:: analysis/doc/whatsnew-since/release-08-00.txt

Changes since release-07
========================

.. admonition:: New and updated packages in externals
   :class: toggle

   .. list-table:: New and updated packages in externals from v01-12-01 to v02-00-01
      :widths: 50 25 25
      :header-rows: 1

      * - package
        - old version
        - new version
      * - clhep
        - 2.4.1.3
        - 2.4.6.2
      * - Geant4
        - 10.06.p03
        - 11.1.1
      * - Vc
        - 1.4.1
        - 1.4.3
      * - VecCore
        - 0.6.0
        - 0.8.0
      * - VecGeom
        - 1.1.5
        - 1.2.1
      * - VGM
        - 4.6
        - 5.0
      * - beautifulsoup4
        -
        - 4.11.1
      * - pydata-sphinx-theme
        -
        - 0.8.1
      * - soupsieve
        -
        - 2.3.2.post1
      * - sphinx-codeautolink
        -
        - 0.15.0

.. rubric:: Authentication for Conditions Database using JWT

The authentication method that allows to upload payloads to the Conditions
Database and to modify existing global tags has been updated. It now uses
JSON web tokens that are created and verified based on the B2MMS username and
the associated password. More information can be found in the section
:ref:`b2conditionsdb`.

.. include:: analysis/doc/whatsnew-since/release-07-00.txt

.. List of changes for the b2bii package

.. include:: b2bii/doc/whatsnew-since/release-07-00.txt

.. List of changes for the mva package

.. include:: mva/doc/whatsnew-since/release-07-00.txt

Changes since release-06
========================

.. admonition:: New and updated packages in externals
   :class: toggle

   .. list-table:: New and updated packages in externals from v01-10-02 to v01-12-01
      :widths: 50 25 25
      :header-rows: 1

      * - package
        - old version
        - new version
      * - astyle
        - 2.05.1
        - 3.1
      * - boost
        - 1.72
        - 1.78
      * - cmake
        - 3.19.7
        - 3.22.2
      * - curl
        - 7.75.0
        - 7.77.0
      * - eigen
        - 3.3.9
        - 3.4.0
      * - EOS
        - 0.3.3
        - removed
      * - Frugally-deep
        -
        - 0.15.19-p0
      * - FunctionalPlus
        -
        - 0.2.18-p0
      * - gcc
        - 10.2
        - 11.2
      * - GDB
        - 10.1
        - 10.2
      * - Hammer
        - 1.1.0
        - 1.3.0
      * - Libxml2
        - 2.9.10
        - 2.9.13
      * - Libxslt
        - 1.1.34
        - 1.1.35
      * - Madgraph
        - 2.7.2
        - 3.4.0
      * - Millepede-II
        - V04-09-00
        - V04-10-00
      * - nlohmann/json
        - 3.9.1
        - 3.10.5
      * - ROOT
        - 6.24.0
        - 6.24/06
      * - RooUnfold
        -
        - 3.0.0
      * - XRootD
        - 5.1.1
        - 5.4.3
      * - zlib
        - 1.2.11
        - 1.2.12
      * - Integer Set Library
        - 0.21
        - 0.24
      * - cppcheck
        - 2.3
        - 2.6
      * - absl-py
        - 0.12.0
        - 1.3.0
      * - aiohttp
        -
        - 3.8.1
      * - aiosignal
        -
        - 1.2.0
      * - amply
        - 0.1.4
        - removed
      * - anyio
        - 3.3.0
        - removed
      * - argon2-cffi
        - 20.1.0
        - 21.3.0
      * - argon2-cffi-bindings
        -
        - 21.2.0
      * - async_generator
        - 1.10
        - removed
      * - async-timeout
        -
        - 4.0.2
      * - attrs
        - 21.2.0
        - 21.4.0
      * - autopep8
        - 1.5.7
        - 1.6.0
      * - awkward
        - 1.2.2
        - 1.7.0
      * - bleach
        - 3.3.0
        - 4.1.0
      * - boost-histogram
        - 1.0.2
        - 1.2.1
      * - cachetools
        - 4.2.2
        - 4.2.4
      * - certifi
        - 2020.12.5
        - 2021.10.8
      * - cffi
        - 1.14.5
        - 1.15.0
      * - chardet
        - 4.0.0
        - removed
      * - charset-normalizer
        -
        - 2.0.10
      * - cheroot
        - 8.5.2
        - 8.6.0
      * - CherryPy
        - 18.6.0
        - 18.6.1
      * - cloudpickle
        - 1.6.0
        - 2.0.0
      * - ConfigArgParse
        - 1.4
        - 1.5.3
      * - connection_pool
        -
        - 0.0.3
      * - cryptography
        - 3.4.7
        - 36.0.1
      * - cycler
        - 0.10.0
        - 0.11.0
      * - Cython
        - 0.29.23
        - 0.29.26
      * - debugpy
        -
        - 1.5.1
      * - decorator
        - 5.0.7
        - 5.1.1
      * - dgl
        -
        - 0.6.1
      * - eoshep
        -
        - 1.0.1
      * - filelock
        - 3.0.12
        - 3.4.2
      * - flake8
        - 3.9.2
        - 4.0.1
      * - flatbuffers
        - 1.12
        - 22.9.24
      * - fonttools
        -
        - 4.28.5
      * - frozenlist
        -
        - 1.3.0
      * - fsspec
        -
        - 2022.5.0
      * - gitdb
        - 4.0.7
        - 4.0.9
      * - GitPython
        - 3.1.14
        - 3.1.26
      * - google-auth
        - 1.30.0
        - 2.3.3
      * - google-auth-oauthlib
        - 0.4.4
        - 0.4.6
      * - idna
        - 2.10
        - 3.3
      * - imagesize
        - 1.2.0
        - 1.3.0
      * - importlib-metadata
        -
        - 4.10.0
      * - importlib-resources
        -
        - 5.4.0
      * - ipykernel
        - 5.5.4
        - 6.6.1
      * - ipython
        - 7.23.1
        - 7.31.0
      * - ipywidgets
        - 7.6.3
        - 7.6.5
      * - jaraco.collections
        - 3.3.0
        - 3.5.1
      * - jaraco.functools
        - 3.3.0
        - 3.5.0
      * - jaraco.text
        - 3.5.0
        - 3.6.0
      * - jedi
        - 0.18.0
        - 0.18.1
      * - jeepney
        -
        - 0.7.1
      * - Jinja2
        - 2.11.3
        - 3.0.3
      * - jira
        - 2.0.0
        - 3.1.1
      * - joblib
        - 1.0.1
        - 1.1.0
      * - json5
        - 0.9.6
        - removed
      * - jsonschema
        - 3.2.0
        - 4.3.3
      * - jupyter-client
        - 6.1.12
        - 7.1.0
      * - jupyter-core
        - 4.7.1
        - 4.9.1
      * - jupyter-server
        - 1.10.2
        - removed
      * - jupyterlab
        - 3.1.9
        - removed
      * - jupyterlab-server
        - 2.7.2
        - removed
      * - jupyterlab-widgets
        - 1.0.0
        - 1.0.2
      * - jupytext
        - 1.11.2
        - 1.13.6
      * - keras
        - 2.4.3
        - 2.10.0
      * - keyring
        -
        - 23.5.0
      * - kiwisolver
        - 1.3.1
        - 1.3.2
      * - libclang
        -
        - 14.0.6
      * - lightgbm
        -
        - 3.3.2
      * - lxml
        - 4.6.3
        - 4.7.1
      * - Markdown
        - 3.3.4
        - 3.3.6
      * - MarkupSafe
        - 1.1.1
        - 2.0.1
      * - matplotlib
        - 3.4.2
        - 3.5.1
      * - matplotlib-inline
        - 0.1.2
        - 0.1.3
      * - mdit-py-plugins
        - 0.2.8
        - 0.3.0
      * - metakernel
        - 0.27.5
        - 0.28.2
      * - more-itertools
        - 8.7.0
        - 8.12.0
      * - multidict
        -
        - 6.0.2
      * - mysql-connector-python
        - 8.0.24
        - 8.0.27
      * - nbclassic
        - 0.3.1
        - removed
      * - nbclient
        - 0.5.3
        - 0.5.9
      * - nbconvert
        - 6.0.7
        - 6.4.0
      * - nbsphinx
        - 0.8.4
        - 0.8.8
      * - nest-asyncio
        - 1.5.1
        - 1.5.4
      * - networkx
        -
        - 2.7.1
      * - notebook
        - 6.4.3
        - 6.4.6
      * - numexpr
        - 2.7.3
        - 2.8.1
      * - numpy
        - 1.19.5
        - 1.20.1
      * - oauthlib
        - 3.1.0
        - 3.1.1
      * - packaging
        - 20.9
        - 21.3
      * - pandas
        - 1.2.4
        - 1.3.5
      * - pandocfilters
        - 1.4.3
        - 1.5.0
      * - parso
        - 0.8.2
        - 0.8.3
      * - pbr
        - 5.6.0
        - removed
      * - Pillow
        - 8.2.0
        - 9.0.0
      * - pip
        - 21.1.1
        - 21.3.1
      * - pluggy
        - 0.13.1
        - 1.0.0
      * - portend
        - 2.7.1
        - 3.1.0
      * - prometheus-client
        - 0.10.1
        - 0.12.0
      * - prompt-toolkit
        - 3.0.18
        - 3.0.24
      * - protobuf
        - 3.16.0
        - 3.19.3
      * - psutil
        - 5.8.0
        - 5.9.0
      * - PuLP
        - 2.4
        - 2.6.0
      * - py
        - 1.10.0
        - 1.11.0
      * - pycodestyle
        - 2.7.0
        - 2.8.0
      * - pycparser
        - 2.20
        - 2.21
      * - pyDeprecate
        -
        - 0.3.1
      * - pydocstyle
        - 6.0.0
        - 6.1.1
      * - pyflakes
        - 2.3.1
        - 2.4.0
      * - Pygments
        - 2.9.0
        - 2.11.2
      * - PyJWT
        - 2.1.0
        - removed
      * - pylama
        - 7.7.1
        - 8.3.7
      * - pyparsing
        - 2.4.7
        - 3.0.6
      * - pypmc
        -
        - 1.2
      * - pyrsistent
        - 0.17.3
        - 0.18.0
      * - pytest
        - 6.2.4
        - 6.2.5
      * - python-dateutil
        - 2.8.1
        - 2.8.2
      * - python-gitlab
        -
        - 3.9.0
      * - pytorch-lightning
        -
        - 1.5.6
      * - pytz
        - 2021.1
        - 2021.3
      * - PyYAML
        - 5.4.1
        - 6.0
      * - pyzmq
        - 22.0.3
        - 22.3.0
      * - qtconsole
        - 5.1.0
        - 5.2.2
      * - QtPy
        - 1.9.0
        - 2.0.0
      * - requests
        - 2.25.1
        - 2.27.1
      * - requests-unixsocket
        - 0.2.0
        - removed
      * - rsa
        - 4.7.2
        - 4.8
      * - scikit-learn
        - 0.24.2
        - 1.0.2
      * - scipy
        - 1.6.3
        - 1.7.3
      * - SCons
        - 4.1.0post1
        - 4.3.0
      * - seaborn
        - 0.11.1
        - 0.11.2
      * - SecretStorage
        -
        - 3.3.1
      * - Send2Trash
        - 1.5.0
        - 1.8.0
      * - setuptools
        - 56.2.0
        - 60.5.0
      * - smart-open
        - 5.0.0
        - 5.2.1
      * - smmap
        - 4.0.0
        - 5.0.0
      * - snakemake
        - 6.3.0
        - 6.13.1
      * - sniffio
        - 1.2.0
        - removed
      * - snowballstemmer
        - 2.1.0
        - 2.2.0
      * - Sphinx
        - 4.0.0
        - 4.3.2
      * - sphinx-book-theme
        -
        - 0.3.3
      * - sphinxcontrib-htmlhelp
        - 1.0.3
        - 2.0.0
      * - sphinxcontrib-programoutput
        -
        - 0.17
      * - sphinxcontrib-serializinghtml
        - 1.1.4
        - 1.1.5
      * - sweights
        -
        - 1.0.0
      * - tables
        - 3.6.1
        - 3.7.0
      * - tempora
        - 4.0.2
        - 5.0.0
      * - tensorboard
        - 2.5.0
        - 2.10.1
      * - tensorboard-plugin-wit
        - 1.8.0
        - 1.8.1
      * - tensorflow
        - 2.4.1
        - 2.10.0
      * - tensorflow-estimator
        - 2.4.0
        - 2.10.0
      * - tensorflow-io-gcs-filesystem
        -
        - 0.27.0
      * - tensorflow-probability
        - 0.12.2
        - 0.15.0
      * - terminado
        - 0.9.4
        - 0.12.1
      * - testpath
        - 0.4.4
        - 0.5.0
      * - threadpoolctl
        - 2.1.0
        - 3.0.0
      * - toposort
        - 1.6
        - 1.7
      * - torch
        - 1.8.1
        - 1.11.0
      * - torch-geometric
        -
        - 2.0.4
      * - torch-scatter
        -
        - 2.0.9
      * - torch-sparse
        -
        - 0.6.14
      * - torchaudio
        - 0.8.1
        - 0.11.0
      * - torchmetrics
        -
        - 0.9.3
      * - torchvision
        - 0.9.1
        - 0.12.0
      * - tqdm
        - 4.60.0
        - 4.62.3
      * - traitlets
        - 5.0.5
        - 5.1.1
      * - uncertainties
        - 3.1.5
        - 3.1.6
      * - uproot
        - 4.0.7
        - 4.1.9
      * - urllib3
        - 1.26.4
        - 1.26.8
      * - websocket-client
        - 1.2.1
        - removed
      * - Werkzeug
        - 1.0.1
        - 2.0.2
      * - wheel
        - 0.36.2
        - 0.37.1
      * - widgetsnbextension
        - 3.5.1
        - 3.5.2
      * - xgboost
        -
        - 1.6.1
      * - yarl
        -
        - 1.7.2
      * - zipp
        -
        - 3.7.0
      * - zstandard
        - 0.15.2
        - 0.16.0

.. rubric:: Simplified arguments to :py:func:`modularAnalysis.inputMdst` and :py:func:`modularAnalysis.inputMdstList`.

The arguments of :py:func:`modularAnalysis.inputMdst` and :py:func:`modularAnalysis.inputMdstList` have been changed a little.
You no longer need to specify "default", it's done automatically.

The following code lines need to be changed from:

.. code-block:: python

   # old
   import modularAnalysis as ma
   ma.inputMdst("default", "/path/to/your/file.root", path=mypath)

   # or
   ma.inputMdst("Belle", "/path/to/your/file.root", path=mypath)

To:

.. code-block:: python

     # new
     import modularAnalysis as ma
     ma.inputMdst("/path/to/your/file.root", path=mypath)

     # or
     ma.inputMdst("/path/to/your/file.root", path=mypath, environmentType="Belle")

And similarly for :py:func:`modularAnalysis.inputMdstList`.

.. warning:: We no longer support MC5-10 files.

.. rubric:: Breaking of backward compatibility for kinematic variables

The floating-point members of the Particle class (invariant mass, momentum and
position components) are now stored as doubles and no longer as floats.
Previously, in some edge cases like for ISR photons, in particular the energy
calculation could suffer from a cancellation of significant digits. This
change might slightly modify the values of (derived) kinematic variables,
however it should be below most analysts sensitivity.

.. rubric:: Track time

When reconstructing data with release-07 the new variable `trackTime` becomes
available, which is computed by the ``TrackTimeEstimatorModule``. This track
time is the average time of the SVD clusters attached to the track minus the
``SVDEventT0``.

.. include:: analysis/doc/whatsnew-since/release-06-00.txt

.. List of changes for the framework package

.. include:: framework/doc/whatsnew-since/release-06-00.txt

.. List of changes for the b2bii package

.. include:: b2bii/doc/whatsnew-since/release-06-00.txt

.. List of changes for the mva package

.. include:: mva/doc/whatsnew-since/release-06-00.txt

.. List of changes for the tracking package

.. include:: tracking/doc/whatsnew-since/release-06-00.txt

.. List of changes for the svd package

.. include:: svd/doc/whatsnew-since/release-06-00.txt

Changes since release-05
========================

.. admonition:: New and updated packages in externals
   :class: toggle

   .. list-table:: New and updated packages in externals from v01-09-01 to v01-10-02
      :widths: 50 25 25
      :header-rows: 1

      * - package
        - old version
        - new version
      * - GNU Binutils
        - 2.34
        - 2.36.1
      * - boost
        - 1.72
        - 1.75
      * - cmake
        - 3.16.5
        - 3.19.7
      * - cppzmq
        - 4.6.0
        - 4.7.1
      * - curl
        - 7.69.1
        - 7.75.0
      * - davix
        - 0.7.4
        - 0.7.6
      * - Eigen
        - 3.3.7
        - 3.3.9
      * - EOS
        - 0.3.2
        - 0.3.3
      * - EPICS
        - 7.0.3.1
        - 7.0.5
      * - EvtGen
        - R01-04-00
        - R02-00-00
      * - FastJet
        -
        - 3.3.4
      * - FFTW3
        - 3.3.8
        - 3.3.9
      * - gcc
        - 9.3
        - 10.2
      * - GDB
        - 9.1
        - 10.1
      * - Geant4
        - 10.06.p01
        - 10.06.p03
      * - git
        - 2.25.2
        - 2.31.0
      * - git-lfs
        - 2.10.0
        - 2.13.2
      * - Hammer
        - 1.0.0
        - 1.1.0
      * - HDF5
        - 1.10.6
        - 1.12.0
      * - HepMC
        - 2.06.09
        - 2.06.11
      * - LZ4
        - 1.9.2
        - 1.9.3
      * - Millepede
        - V04-06-00
        - V04-09-00
      * - Ninja
        -
        - 1.10.2
      * - nlohmann/json
        - 3.7.3
        - 3.9.1
      * - pandoc
        - 2.9.2.1
        - 2.12
      * - Photos
        - 3.56
        - 3.64
      * - PostgreSQL
        - 9.2.4
        - 13.2
      * - Python
        - 3.6.9
        - 3.8.8
      * - rclone
        - v1.51.0
        - v1.54.1
      * - ROOT
        - 6.20/04
        - 6.24.0
      * - SQLite
        - 3.31.1
        - 3.35.1
      * - Tauola
        - 1.1.4
        - 1.1.8
      * - XRootD
        - 4.11.2
        - 5.1.1
      * - zeromq
        - 4.3.2
        - 4.3.4
      * - Integer Set Library
        - 0.23
        - 0.21
      * - cppcheck
        - 1.90
        - 2.3
      * - Doxygen
        - 1.8.17
        - 1.9.1
      * - LLVM/Clang
        - 10.0.0
        - 12.0.0
      * - Valgrind
        - 3.15
        - 3.17.0
      * - absl-py
        - 0.9.0
        - 0.12.0
      * - amply
        -
        - 0.1.4
      * - anyio
        -
        - 3.3.0
      * - appdirs
        - 1.4.3
        - 1.4.4
      * - argon2-cffi
        -
        - 20.1.0
      * - astor
        - 0.8.1
        - removed
      * - astunparse
        -
        - 1.6.3
      * - async_generator
        -
        - 1.10
      * - attrs
        - 19.3.0
        - 21.2.0
      * - autopep8e
        - 1.5.2
        - 1.5.7
      * - awkward
        - 0.12.20
        - 1.2.2
      * - awkward0
        -
        - 0.15.5
      * - b2luigi
        - 0.4.4
        - removed
      * - Babel
        -
        - 2.9.1
      * - backcall
        - 0.1.0
        - 0.2.0
      * - bleach
        - 3.1.4
        - 3.3.0
      * - boost-histogram
        - 0.7.0
        - 1.0.2
      * - cachetools
        - 4.1.0
        - 4.2.2
      * - certifi
        - 2020.4.5.1
        - 2020.12.5
      * - cffi
        - 1.14.0
        - 1.14.5
      * - chardet
        - 3.0.4
        - 4.0.0
      * - cheroot
        - 8.3.0
        - 8.5.2
      * - cloudpickle
        - 1.3.0
        - 1.6.0
      * - colorama
        - 0.4.3
        - removed
      * - ConfigArgParse
        - 1.2.3
        - 1.4
      * - cryptography
        - 2.9.2
        - 3.4.7
      * - cymove
        - 1.0.0
        - 1.0.2
      * - Cython
        - 0.29.15
        - 0.29.23
      * - decorator
        - 4.4.2
        - 5.0.7
      * - defusedxml
        - 0.6.0
        - 0.7.1
      * - dm-tree
        -
        - 0.1.6
      * - docutils
        - 0.15
        - 0.17.1
      * - filelock
        -
        - 3.0.12
      * - flake8
        - 3.7.9
        - 3.9.2
      * - flatbuffers
        -
        - 1.12
      * - future
        -
        - 0.18.2
      * - gast
        - 0.2.2
        - 0.3.3
      * - gitdb
        - 4.0.4
        - 4.0.7
      * - GitPython
        - 3.1.1
        - 3.1.14
      * - google-auth
        -
        - 1.30.0
      * - google-auth-oauthlib
        -
        - 0.4.4
      * - grpcio
        - 1.28.1
        - 1.32.0
      * - idna
        - 2.9
        - 2.10
      * - importlib-metadata
        - 1.6.0
        - removed
      * - importlib-resources
        - 1.4.0
        - removed
      * - iniconfig
        -
        - 1.1.1
      * - ipykernel
        - 5.2.1
        - 5.5.4
      * - ipython
        - 7.13.0
        - 7.23.1
      * - ipywidgets
        - 7.5.1
        - 7.6.3
      * - jaraco.classes
        - 3.1.0
        - 3.2.1
      * - jaraco.collections
        - 3.0.0
        - 3.3.0
      * - jaraco.functools
        - 3.0.0
        - 3.3.0
      * - jaraco.text
        - 3.2.0
        - 3.5.0
      * - jedi
        - 0.17.0
        - 0.18.0
      * - Jinja2
        - 2.11.2
        - 2.11.3
      * - joblib
        - 0.14.1
        - 1.0.1
      * - json5
        -
        - 0.9.6
      * - jupyter-client
        - 6.1.3
        - 6.1.12
      * - jupyter-console
        - 6.1.0
        - 6.4.0
      * - jupyter-core
        - 4.6.3
        - 4.7.1
      * - jupyter-server
        -
        - 1.10.2
      * - jupyterlab
        -
        - 3.1.9
      * - jupyterlab-pygments
        -
        - 0.1.2
      * - jupyterlab-server
        -
        - 2.7.2
      * - jupyterlab-widgets
        -
        - 1.0.0
      * - jupytext
        - 1.4.2
        - 1.11.2
      * - keras
        - 2.3.1
        - 2.4.3
      * - Keras-Applications
        - 1.0.8
        - removed
      * - Keras-Preprocessing
        - 1.1.0
        - 1.1.2
      * - kiwisolver
        - 1.2.0
        - 1.3.1
      * - lockfile
        - 0.12.2
        - removed
      * - luigi
        - 2.7.7
        - removed
      * - lxml
        - 4.5.0
        - 4.6.3
      * - Markdown
        - 3.2.1
        - 3.3.4
      * - markdown-it-py
        -
        - 1.1.0
      * - matplotlib
        - 3.2.1
        - 3.4.2
      * - matplotlib-inline
        -
        - 0.1.2
      * - mdit-py-plugins
        -
        - 0.2.8
      * - metakernel
        - 0.24.4
        - 0.27.5
      * - more-itertools
        - 8.2.0
        - 8.7.0
      * - mysql-connector-python
        -
        - 8.0.24
      * - nbclassic
        -
        - 0.3.1
      * - nbclient
        -
        - 0.5.3
      * - nbconvert
        - 5.6.1
        - 6.0.7
      * - nbformat
        - 5.0.6
        - 5.1.3
      * - nbsphinx
        - 0.6.1
        - 0.8.4
      * - nest-asyncio
        -
        - 1.5.1
      * - notebook
        - 6.0.3
        - 6.4.3
      * - numexpr
        - 2.7.1
        - 2.7.3
      * - numpy
        - 1.18.3
        - 1.19.5
      * - opt-einsum
        - 3.2.1
        - 3.3.0
      * - packaging
        - 20.3
        - 20.9
      * - pandas
        - 1.0.3
        - 1.2.4
      * - pandocfilters
        - 1.4.2
        - 1.4.3
      * - parse
        - 1.15.0
        - removed
      * - parso
        - 0.7.0
        - 0.8.2
      * - pbr
        - 5.4.5
        - 5.6.0
      * - Pillow
        - 7.1.1
        - 8.2.0
      * - pip
        - 20.0.2
        - 21.1.1
      * - pluggy
        -
        - 0.13.1
      * - portend
        - 2.6
        - 2.7.1
      * - prometheus-client
        - 0.7.1
        - 0.10.1
      * - prompt-toolkit
        - 3.0.5
        - 3.0.18
      * - protobuf
        - 3.11.3
        - 3.16.0
      * - psutil
        - 5.7.0
        - 5.8.0
      * - ptyprocess
        - 0.6.0
        - 0.7.0
      * - PuLP
        -
        - 2.4
      * - py
        -
        - 1.10.0
      * - pyasn1
        -
        - 0.4.8
      * - pyasn1-modules
        -
        - 0.2.8
      * - pycodestyle
        - 2.5.0
        - 2.7.0
      * - pydocstyle
        - 5.0.2
        - 6.0.0
      * - pyflakes
        - 2.1.1
        - 2.3.1
      * - Pygments
        - 2.6.1
        - 2.9.0
      * - PyJWT
        - 1.7.1
        - 2.1.0
      * - pyroofit
        - 0.9.4
        - 0.9.6
      * - pyrsistent
        - 0.16.0
        - 0.17.3
      * - pytest
        -
        - 6.2.4
      * - python-daemon
        - 2.2.4
        - removed
      * - pytz
        - 2019.3
        - 2021.1
      * - PyYAML
        - 5.3.1
        - 5.4.1
      * - pyzmq
        - 19.0.0
        - 22.0.3
      * - qtconsole
        - 4.7.3
        - 5.1.0
      * - requests
        - 2.23.0
        - 2.25.1
      * - requests-unixsocket
        -
        - 0.2.0
      * - rise
        - 5.6.1
        - 5.7.1
      * - rsa
        -
        - 4.7.2
      * - scikit-learn
        - 0.22.2.post1
        - 0.24.2
      * - scipy
        - 1.4.1
        - 1.6.3
      * - SCons
        - 3.1.2
        - 4.1.0.post1
      * - seaborn
        - 0.10.0
        - 0.11.1
      * - setuptools
        - 46.1.3
        - 56.2.0
      * - six
        - 1.14.0
        - 1.15.0
      * - sly
        -
        - 0.4
      * - smart-open
        -
        - 5.0.0
      * - smmap
        - 3.0.2
        - 4.0.0
      * - snakemake
        - 5.15.0
        - 6.3.0
      * - sniffio
        -
        - 1.2.0
      * - snowballstemmer
        - 2.0.0
        - 2.1.0
      * - Sphinx
        - 3.0.2
        - 4.0.0
      * - stopithread
        -
        - 1.1.2
      * - tabulate
        - 0.8.7
        - 0.8.9
      * - tempora
        - 3.0.0
        - 4.0.2
      * - tensorboard
        - 1.15.0
        - 2.5.0
      * - tensorboard-data-server
        -
        - 0.6.1
      * - tensorboard-plugin-wit
        -
        - 1.8.0
      * - tensorflow
        - 1.15.2
        - 2.4.1
      * - tensorflow-estimator
        - 1.15.1
        - 2.4.0
      * - tensorflow-probability
        - 0.9.0
        - 0.12.2
      * - terminado
        - 0.8.3
        - 0.9.4
      * - Theano
        - 1.0.4
        - 1.0.5
      * - threadpoolctl
        -
        - 2.1.0
      * - toml
        -
        - 0.10.2
      * - toposort
        - 1.5
        - 1.6
      * - torch
        - 1.4.0
        - 1.8.1
      * - torchaudio
        -
        - 0.8.1
      * - torchvision
        - 0.5.0
        - 0.9.1
      * - tornado
        - 5.1.1
        - 6.1
      * - tqdm
        -
        - 4.60.0
      * - traitlets
        - 4.3.3
        - 5.0.5
      * - typing-extensions
        -
        - 3.7.4.3
      * - uncertainties
        - 3.1.2
        - 3.1.5
      * - uproot
        -
        - 4.0.7
      * - uproot3
        - 3.11.3
        - 3.14.4
      * - uproot3-methods
        - 0.7.3
        - 0.10.1
      * - urllib3
        - 1.25.9
        - 1.26.4
      * - wcwidth
        - 0.1.9
        - 0.2.5
      * - websocket-client
        -
        - 1.2.1
      * - wheel
        - 0.34.2
        - 0.36.2
      * - zipp
        - 3.1.0
        - removed
      * - zstandard
        -
        - 0.15.2

.. only:: not light

   .. rubric:: ``HepMCInput``, ``HepevtInput`` and ``LHEInput`` modules do not anymore boost the ``MCParticles``

   The modules ``HepMCInput``, ``HepevtInput`` and ``LHEInput`` do not anymore boost the ``MCParticles``, and the
   parameter ``boost2Lab`` is now removed from the modules. These modules can not read the ``BeamParameters``
   payloads from the conditions database, so having the particles boosted correctly and in a reproducible way was
   non-trivial.
   A new module, ``BoostMCParticles``, is added for boosting into the laboratory frame the ``MCParticles`` using the
   information stored in the conditions database. The module must be appended to the steering path just after the
   ``HepMCInput``, ``HepevtInput`` or ``LHEInput`` module and before running the detector simulation.

.. only:: not light

   .. rubric:: The jitter of the L1 trigger is included in the standard simulation

   The L1 trigger jitter is randomly extracted from a double gaussian whose parameters have been tuned with 2020 data.
   The machine filling pattern is taken into account in the simulation of the jitter.


.. only:: not light

   .. rubric:: The L1 trigger simulation is included in :py:func:`simulation.add_simulation`

   The L1 trigger simulation (``tsim``) is now executed in the standard simulation: before SVD and PXD simulation but
   after the simulation of the rest of the subdetectors. For this reason, the python function ``add_tsim()`` is
   deprecated. If you already have a ``add_simulation`` in your path, you already get L1 trigger simulation.
   If you do not have ``add_simulation``, and you need the L1 trigger simulation, please use
   :py:func:`L1trigger.add_trigger_simulation`.


.. only:: not light

   .. rubric:: Discontinue the support of the old "fullFormat" for cDSTs and extend the "rawFormat" cDSTs to MC

   The support of the ``fullFormat`` cDSTs is discontinued. :py:func:`reconstruction.add_cdst_output` does not store
   anymore additional branches when the option ``rawFormat=False`` is selected, being simply an alias of
   :py:func:`mdst.add_mdst_output`. The users have to explicitly define the additional branches they want to store
   using the ``additionalBranches`` parameter.

   The only supported format is the ``rawFormat``, that is now extended to MC. If ``rawFormat=True`` and ``mc=False`` are
   selected, the rawdata + tracking data objects are stored, while with ``rawFormat=True`` and ``mc=True`` the digits +
   tracking data objects, including the ``MCParticles`` and the relations between them and the digits, are stored.

.. rubric:: Removal of old and deprecated database functions

Some functions used in the past to handle the conditions database (like ``basf2.use_local_database`` or ``basf2.reset_database``) are removed, and any script using them does not work anymore.
This removal does not imply any functionality loss, since the users can use the `basf2.conditions` object to properly configure the conditions database in their steering files (see also :ref:`configuring_cdb`).

.. rubric:: Photons generated by PHOTOS in continuum events

Fixed the issue where PHOTOS photons were not correctly flagged in continuum events, e.g., charm decays (:issue:`5828`).
This was present in ``release-05-00-01`` and earlier, including MC13 files.

.. rubric:: Unification of B2BII settings

A single switch between Belle and Belle II settings has been implemented, which is automatically set when reading in a Belle type mdst.
No individual options have to be set in modular analysis functions. 

.. include:: analysis/doc/whatsnew-since/release-05-02.txt

.. include:: analysis/doc/whatsnew-since/release-05-01.txt

.. include:: analysis/doc/whatsnew-since/release-05-00.txt

.. List of changes for the framework package

.. include:: framework/doc/whatsnew-since/release-05-01.txt

Changes since release-04
========================

.. rubric:: Neutral hadrons from ECLClusters get momentum from the cluster energy

Since ``release-04`` it has been possible to load ECLClusters under the neutral hadron hypothesis.
Previously we assumed a mass when calculating the particle momentum, however this leads to problems when, for example, a :math:`K_L^0` deposits less than its mass energy in the ECL. This happens about 50% of the time.

The momentum of neutral hadrons from the ECL is now set to the :b2:var:`clusterE`.

.. rubric:: Bremsstrahlung correction

The `BremsFinder` module has been developed to find relations between tracks
and photons that are likely to have been emitted by these tracks via
Bremsstrahlung. The matching quality figure of merit is based on the angular
distance between the photon ECL cluster and the extrapolated hit position of
the track at the ECL. The function `correctBrems` performs the actual
correction. There is also a reimplementation of Belle's Bremsstrahlung
correction approach of looking for photons in a cone around tracks
(`correctBremsBelle`), which is recommended for ``b2bii`` analyses.

.. warning:: While it is technically possible to perform a TreeFit after
             applying Bremsstrahlung correction, the fit performance is unfortunately quite
             bad. However, there is already an improvement in the pipeline that should fix
             this issue. It will probably be available in one of the next light releases.

.. rubric:: MC reconstruction and MC matching

The :b2:mod:`ParticleCombinerFromMC` module and its corresponding wrapper
function `reconstructMCDecay` should be used instead of `findMCDecay` to
reconstruct decay modes based on MC information.

The DecayStringGrammar has been extended with new exception markers for
Bremsstrahlung, decay in flight, and misidentification.

Exceptions for the MC matching of daughter particles with the DecayStringGrammar are propagated 
to the mother particle. 

.. rubric:: Redefinition of angle variables

The kinematic variables :b2:var:`decayAngle`, :b2:var:`daughterAngle` and
:b2:var:`pointingAngle` now return the angle instead of its cosine.

.. rubric:: Protection of ParticleLists and particle combinations

It is no longer allowed to use the label ``"all"`` for a particle list if a
cut is applied. Reconstructed decays need to preserve electric charge.
However, this can be deactivated if you know what you are doing, e.g. in
searches for New Physics.

.. Detailed changes for the analysis package first, that's
   what user will want to see

.. include:: analysis/doc/whatsnew-since/release-04-02.txt

.. include:: analysis/doc/whatsnew-since/release-04-00.txt

.. And then for framework as well. If something is more important we can always
   move it directly in here

.. include:: framework/doc/whatsnew-since/release-04-00.txt

.. Changes for decfiles package

.. include:: decfiles/doc/whatsnew-since/release-04-02.txt

.. Changes for b2bii here.

.. include:: b2bii/doc/whatsnew-since/release-04-01.txt

Changes since release-03
========================

.. rubric:: Removal of default analysis path and ``NtupleTools``

.. warning:: The default path ("``analysis_main``") and the ``NtupleTools`` are now **removed**.

This is a major backward-compatibility breaking change.
Please update your user scripts to create your own path (`basf2.create_path`) and to use the `variable manager tools <variablemanageroutput>` (such as `VariablesToNtuple <v2nt>`).

If your previously working example script from ``release-03`` looked something like this:

.. code-block:: python

         from basf2 import *
         from stdCharged import stdPi
         from modularAnalysis import *
         stdPi("good")
         ntupleFile("myFile.root") # <-- now removed
         ntupleTree("pi+:good", ['pi+', 'Momentum']) # <-- now removed
         process(analysis_main)
         print(statistics)


You should update it to this:

.. code-block:: python

         import basf2 # better not to import all
         from stdCharged import stdPi
         from modularAnalysis import variablesToNtuple
         mypath = basf2.Path() # create your own path (call it what you like)
         stdPi("good", path=mypath)
         variablesToNtuple("pi+:good", ['px', 'py', 'pz', 'E'], path=mypath)
         basf2.process(mypath)
         print(basf2.statistics)


.. seealso::
        The example scripts available here:

        .. code-block:: text

              $BELLE2_RELEASE_DIR/analysis/examples/VariableManager


.. rubric:: Switch of beam spot information from nominal to measured values.

The interaction point position and its uncertainties are now taken from the database with values provided by the tracking group.
All beam kinematics information is also moved to the database, which will eventually be measured on data.
For now they are the values provided by the accelerator.

.. warning::
    The previous definition of the boost included a small rotation to align it with the HER.
    This is no longer possible with the new structure.
    The definition of CMS is therefore slightly changed. The impact should be at the percent level.

If you have a physics analysis sensitive to this change: please discuss with the software / performance groups and add a comment to :issue:`4294`.

.. seealso:: The beam information can be accessed with :b2:var:`Ecms`, :b2:var:`beamPx`,  :b2:var:`beamPy`,  :b2:var:`beamPz`, and  :b2:var:`beamE`.

.. note::
        As a consequence of this, there is now a **difference** between the variables
        ( :b2:var:`dx`, :b2:var:`dy`, :b2:var:`dz` ) compared to ( :b2:var:`x`, :b2:var:`y`, :b2:var:`z` );
        and similarly for variables ( :b2:var:`mcDecayVertexX`, :b2:var:`mcDecayVertexY`, :b2:var:`mcDecayVertexZ` )  compared to ( :b2:var:`mcDecayVertexFromIPX`, :b2:var:`mcDecayVertexFromIPY` and :b2:var:`mcDecayVertexFromIPZ` ).

.. rubric:: Redesign of the Conditions Database Interface

The configuration and handling of the connection to the conditions database has
been completely rewritten in a more coherent and modular way. We now have a new
and consistent configuration interface, global tag replay and advanced checks:
If users specify a global tag to be used which is either marked as invalid in
the database or which cannot be found in the database the processing is now
aborted. See :ref:`conditionsdb_overview` for details.

.. rubric:: Restrict usage of ``useDB=False`` for Geometry creation

Creating the geometry from XML files instead of the configuration in the
Database may lead to wrong results. So while the option ``useDB=False`` is
still necessary to debug changes to the geometry definitions it is now
restricted to only be used for ``exp, run = 0, 0`` to protect users from
mistakes.

This also changes the behavior of `add_simulation()
<simulation.add_simulation>` and `add_reconstruction()
<reconstruction.add_reconstruction>`: If a list of components is provided this
will now only change the digitization or reconstruction setup but will always
use the full geometry from the database.


.. rubric:: Loading ECLClusters under multiple hypotheses

It is now possible to load :math:`K_L^0` particles from clusters in the ECL.
This has several important consequences for the creation of particles and using combinations containing :math:`K_L^0` s or other neutral hadrons in the analysis package.
This is handled correctly by the ParticleLoader and ParticleCombiner (the corresponding convenience functions are `modularAnalysis.fillParticleList` and `modularAnalysis.reconstructDecay`).
Essentially: it is forbidden from now onwards for any other analysis modules to create particles.

.. rubric:: Deprecated RAVE for analysis use

The (external) `RAVE <https://github.com/rave-package>`_ vertex fitter is not maintained.
Its use in analysis is therefore deprecated.
We do not expect to *remove* it, but *do not recommend* its use for any real physics analyses other than benchmarking or legacy studies.

Instead we recommend you use either KFit (`vertex.kFit`) for fast/simple fits, or TreeFit (`vertex.treeFit`) for more complex fits and fitting the full decay chain.
Please check the :ref:`TreeFitter` pages for details about the constraints available.
If you are unable to use TreeFitter because of missing functionality, please `submit a feature request <https://gitlab.desy.de/belle2/software/basf2/-/issues>`_!

.. warning:: The default fitter for `vertex.fitVertex` has been changed to KFit.


.. rubric:: Tidy up and rename of Helicity variables.

Renamed helicity variables in the VariableManager following consistent logic.
We added the new variable :b2:var:`cosAcoplanarityAngle`.

.. warning::  ``cosHelicityAngle`` is now :b2:var:`cosHelicityAngleMomentum`, and :b2:var:`cosHelicityAngle` has a new definition (as in the PDG 2018, p. 722).

+--------------------------------------+---------------------------------------------+
|                Old name              |                New name                     |
+======================================+=============================================+
|        ``cosHelicityAngle``          |       :b2:var:`cosHelicityAngleMomentum`    |
+--------------------------------------+---------------------------------------------+
|     ``cosHelicityAnglePi0Dalitz``    | :b2:var:`cosHelicityAngleMomentumPi0Dalitz` |
+--------------------------------------+---------------------------------------------+
| ``cosHelicityAngleIfCMSIsTheMother`` |    :b2:var:`cosHelicityAngleBeamMomentum`   |
+--------------------------------------+---------------------------------------------+

.. rubric:: New DecayStringGrammar for custom MCMatching

Users can use new DecayStringGrammar to set properties of the MCMatching. Then `isSignal`, `mcErrors` and other MCTruthVariables behave according to the property.

Once DecayStringGrammar is used with `reconstructDecay`, users can use `isSignal` instead of several specific variables such as `isSignalAcceptMissingNeutrino`.
If one doesn't use any new DecayStringGrammar, all MCTruthVariables work same as before.

The grammar is useful to analyze inclusive processes with both fully-inclusive-method and sum-of-exclusive-method.
There are also new helper functions `genNMissingDaughter` and `genNStepsToDaughter` to obtain the detailed MC information.

You can find examples of usage in :ref:`Marker_of_unspecified_particle`, :ref:`Grammar_for_custom_MCMatching`.

.. Now let's add the detailed changes for the analysis package first, that's
   what user will want to see

.. include:: analysis/doc/whatsnew-since/release-03-02.txt

.. And then for framework as well. If something is more important we can always
   move it directly in here

.. include:: framework/doc/whatsnew-since/release-03-00.txt

.. include:: ecl/doc/whatsnew-since/release-03-00.txt


Changes since release-02-01
===========================

.. rubric:: Moved to C++17

The whole software including the ROOT in the externals is now compiled using
the C++17 standard. This should not affect many users but there are a few rare
cases where this might lead to compilation problems of analysis code as some
deprecated features `have been removed <https://mariusbancila.ro/blog/2018/07/05/c17-removed-and-deprecated-features/>`_.
The most notable are

  * ``throw(TypeName)`` exception specifiers, just remove them.
  * ``std::auto_ptr`` which should be replaced by ``std::unique_ptr``
  * some older parts of the ``<functional>`` header.

In particular if you compile a standalone program that links against the ROOT
in the Belle2 externals this now also needs to be compiled in C++17 mode. You
can do this by adding ``-std=c++17`` to the compiler arguments.

.. note:: It's best to directly pass the output of ``root-config --cflags`` to
    the compiler. That way you always pass the correct flags needed for the
    particular ROOT version setup.

.. rubric:: Build system moved to Python3

This is a major update of `SCons <https://scons.org>`_ but most users should not
notice any difference except for two instances:

*  If you update an existing working directory from an older release you might
   run into an error

   .. code-block:: text

       scons: *** [...] TypeError : intern() argument 1 must be string, not unicode
       scons: building terminated because of errors.
       TypeError: intern() argument 1 must be string, not unicode:
       [...]

   In this case please remove all ``.scon*`` files in the top level of your
   software directory and rerun ``scons``

*  In the unlikely case that you have custom ``SConscript`` files
   which are not Python 3 compatible you will have to :ref:`update them <python:pyporting-howto>`.

.. Now let's add the detailed changes for the analysis package first, that's
   what user will want to see

.. include:: analysis/doc/whatsnew-since/release-02-01.txt

.. And then for framework as well. If something is more important we can always
   move it directly in here

.. include:: framework/doc/whatsnew-since/release-02-01.txt
