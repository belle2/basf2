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

Changes since release-04
========================

.. important changes should go here. Especially things that break backwards
      compatibility

.. .. rubric:: Some important feature


.. Detailed changes for the analysis package first, that's
   what user will want to see

.. include:: analysis/doc/whatsnew-since/release-04-00.txt

.. And then for framework as well. If something is more important we can always
   move it directly in here

.. .. include:: framework/doc/whatsnew-since/release-04-00.txt

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

If you have a physics analysis sensitive to this change: please discuss with the software / performance groups and add a comment to :issue:`BII-4360`.

.. seealso:: The beam information can be accessed with :b2:var:`Ecms`, :b2:var:`beamPx`,  :b2:var:`beamPy`,  :b2:var:`beamPz`, and  :b2:var:`beamE`.

.. note::
        As a consequence of this, there is now a **difference** between the variables
        ( :b2:var:`dx`, :b2:var:`dy`, :b2:var:`dz` ) compared to ( :b2:var:`x`, :b2:var:`y`, :b2:var:`z` );
        and similarly for variables ( :b2:var:`mcDX`, :b2:var:`mcDY`, :b2:var:`mcDZ` )  compared to ( :b2:var:`mcX`, :b2:var:`mcY` and :b2:var:`mcZ` ).

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

Instead we recommend you use either KFitter (`vertex.vertexKFit`, and similar functions) for fast/simple fits, or TreeFitter (`vertex.vertexTree`) for more complex fits and fitting the full decay chain.
Please check the :ref:`TreeFitter` pages for details about the constraints available.
If you are unable to use TreeFitter because of missing functionality, please `submit a feature request <https://agira.desy.de/projects/BII>`_!

.. warning:: The default fitter for `vertex.fitVertex` has been changed to KFitter.


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
