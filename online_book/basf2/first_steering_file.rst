.. _onlinebook_first_steering_file:

First steering file
===================

.. sidebar:: Overview
    :class: overview

    **Teaching**: 30 min

    **Exercises**: 30 min

    **Prerequisites**: 
    	
    	* Creating and running scripts in the terminal
    	* Basic python

    **Questions**:

        * How can I load data?
        * How can I reconstruct a decay?
        * How can I match MC?
        * Can I see some pretty pictures?

    **Objectives**:

        * Reconstruct :math:`B \to J/\Psi(\to e^+e^-)K_s(\to \pi^+\pi^+)`

In this hands-on tutorial you'll be writing your first steering file.
But before we can start, you have to set up the necessary environment.

.. admonition:: Task
    :class: exercise stacked

    Set up the basf2 environment using the currently recommended software version.

.. admonition:: Hint
    :class: toggle xhint stacked

    Go back to :ref:`onlinebook_basf2_introduction` to see the step-by-step instructions.

.. admonition:: Solution
    :class: toggle solution

    .. code-block:: bash

        source /cvmfs/belle.cern.ch/tools/b2setup
        b2help-releases
        b2setup <release-shown-at-prompt-by-previous-script>

There are three lines of code that are part of each steering script:

   * Importing the ``basf2`` python library
   * Creating a `basf2.Path`
   * Processing the path

.. admonition:: Task
    :class: exercise stacked

    Open an empty file with an editor of your choice. Add those three lines.
    Run the script. Don't worry, if you've done everything correct, you should
    see some error messages. Read them carefully!

.. admonition:: Hint
    :class: toggle xhint stacked

    Have a look at `basf2` and `basf2.process`

.. admonition:: Solution
    :class: toggle solution

    .. literalinclude:: `010_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/010_first_steering_file.py>`_

Of course, no events could be processed because no data has been loaded yet.
Let's do it. As already described in the previous lesson almost all
convenience functions that are needed can be found in `modularAnalysis`.

It is recommended to use `inputMdstList` or `inputMdst`. If you look at the
source code, you'll notice that the latter actually calls the more general
former.

.. admonition:: Exercise
    :class: exercise stacked

    How many arguments are required for the `inputMdstList` function?
    Which value has to be set for the environment type?

.. admonition:: Solution
    :class: toggle solution

    Three parameters have no default value and are therefore required:

    * the environment type
    * a list of root input files
    * the path

    In 99 % ``default`` should be set as ``environmentType``.

.. admonition:: Solution
    :class: toggle solution

    `011_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/011_first_steering_file.py>`_

fill final state particle lists (e+ and KS)

.. admonition:: Solution
    :class: toggle solution

    `012_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/012_first_steering_file.py>`_

combine electrons to form J/psi and J/psi and KS to form B0

.. admonition:: Solution
    :class: toggle solution

    `013_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/013_first_steering_file.py>`_

write out ntuple

.. admonition:: Solution
    :class: toggle solution

    `014_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/014_first_steering_file.py>`_

run MC matching

.. admonition:: Solution
    :class: toggle solution

    `015_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/015_first_steering_file.py>`_

add more variables via variable collections

.. admonition:: Solution
    :class: toggle solution

    `016_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/016_first_steering_file.py>`_

create aliases

.. admonition:: Solution
    :class: toggle solution

    `019_first_steering_file.py <https://stash.desy.de/projects/B2/repos/software/browse/online_book/basf2/steering_files/019_first_steering_file.py>`_

.. admonition:: Key points
    :class: key-points

    * The ``modularAnalysis`` module contains most of what you'll need for now
    * ``inputMdstList`` is used to load data
    * ``fillParticleList`` adds particles into a list
    * ``reconstructDecay`` combined FSPs from different lists to "reconstruct" particles
    * ``matchMCTruth`` matches MC
    * ``variablesToNtuple`` saves an output file
    * Don't forget ``process(path)`` or nothing happens

