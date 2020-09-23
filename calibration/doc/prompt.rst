========================
Prompt Calibration Tools
========================

Overview
========

The prompt calibration is a set of processes that need to happen before first mDST recontsruction
on raw data can proceed.
A run range must be defined and all necessary calibration payloads must be created and uploaded
to the correct conditions database global tag.
This procedure can be quite complex since:

* Local calibrations occur outside the scope of any automated system, as the data is often inaccessible.
  They may have scripts that are very different to each other, and which require expert knowledge.
* Calibrations from different sub-detectors may depend on each other but do not want to be included
  in a single CAF job.
* Calibrations may require the production of cDST files from raw data before they can begin.
  But cDST production may also depend on earlier (tracking) calibrations.
* Experts may create very complex CAF scripts which require different inputs and setup.
  The interface to them is generally not standardised, other than using the CAF.
* These CAF scripts are also not often committed to basf2 and are kept private.
  This means that there is usually no record of the *correct* scripts to use for a basf2 release.

Automated systems for controlling the creation, submission, and monitoring of CAF jobs for prompt calibration
are being created.
In order to use them effectively some tools have been created in basf2:

* A directory for official CAF scripts for prompt calibration, to keep them up to date with basf2 releases.
* A standardised structure for these scripts so that an automatic system can discover available calibration scripts for a
  basf2 release.
* Command line tools that the automatic system will use to run these scripts.
  Developers can also use these tools to check that their scripts will run in the automatic system.

.. note:: This is to be used for **prompt calibration only**.
          You do not need to follow this guide for reprocessings of very large amounts of old data.

Getting Started
===============

Prompt Calibration Scripts
--------------------------

First Steps:

#. Create a CalibrationCollectorModule (if a new one is necessary), and a CalibrationAlgorithm.
#. Create a working CAF calibration python script using the :py:class:`Calibration <caf.framework.Calibration>` class.

At this point you are working with the CAF and should be ready to add a prompt calibration script to basf2 and test it.
Now identify the requirements of your calibration:

* Which data formats will it take as input (raw, cDST)?
* What kinds of valid calibration data will it take as input e.g. HLT hadron skim
* How many events from a run/in total will it need?
* Does this calibration depend on the accurate payloads from another prompt calibration?

If you need to create a new script for your calibration you should create it in the ``calibration/scripts/prompt/calibrations``
directory.
These scripts have a standard format (see below) which you must use or your prompt calibration won't work!

If you decide that your calibration should run within the same overall job as another related calibration,
you should identify which one in the ``calibration/scripts/prompt/calibrations`` directory.
Then add it to the returned calibrations list in the ``get_calibrations`` function below.

The format for a prompt calibration script is:

.. literalinclude:: prompt_example.py.txt

As you can see you need to define only two things at the top-level of the script. A ``settings`` variable
and a ``get_calibrations(input_data, **kwargs)`` function.
You are free to import from other basf2 packages and create more variables and functions in this script.
But these two must exist.

The ``settings`` variable
+++++++++++++++++++++++++

This must be a variable of the type :py:class:`prompt.CalibrationSettings`.
It defines the input data requirements for the script and gives a human readable name and description.
It also defines the contact expert username (which should be same as the one in JIRA), and a list of
other prompt calibrations that the script depends on.
This list will be used to define the task order in the automatic system, it will not affect running
the script standalone.

.. warning:: If you encounter an `ImportError` when running your script. Please check that you haven't created
    a circular dependency by setting your ``depends_on`` calibrations to a calibration that depends on yours.
    This dependency may well be implicit in the chain of dependencies (A -> B -> C means C depends on A), rather than explicit
    (A -> C means C depends on A).

    It may also be the case that you need to run ``scons`` again to make your new ``prompt/calibrations/`` script known to
    basf2.

.. autoclass:: prompt.CalibrationSettings
    :members: allowed_data_formats

The ``get_calibrations`` function
+++++++++++++++++++++++++++++++++

This function must have the format ``get_calibrations(input_data, **kwargs)``.
The ``input_data`` argument is a a dictionary where the keys should be the same as the values for ``input_data_names``
in your :py:class:`settings <prompt.CalibrationSettings>` variable.
``input_data`` is filled automatically by the :ref:`b2caf_prompt_run` tool from the input data JSON file.
The format of ``input_data`` is:

.. code-block:: python

  input_data = {"Your_Input_Name_A": 
                    {"/path/to/type/a/input/file_1_1.root": caf.utils.IoV(1, 1, 1, 1),
                     "/path/to/type/a/input/file_1_2.root": caf.utils.IoV(1, 2, 1, 2),
                     ...
                    },
                "Your_Input_Name_B":
                    {"/path/to/type/b/input/file_1_1.root": caf.utils.IoV(1, 1, 1, 1),
                     "/path/to/type/b/input/file_1_2.root": caf.utils.IoV(1, 2, 1, 2),
                     ...
                    },
               }

So ``input_data`` is a dictionary that contains input files separated into *categories*, and each file has an associated
:py:class:`IoV <caf.utils.IoV>` object telling you which Experiment and Run this file comes from.

.. warning:: You are not forced to use every file or run given as input.
          You can always filter/reduce the number of input files to a more manageable amount depending on how much
          data you expect to need per run (or in total).

The ``**kwargs`` argument is used to send in the ``requested_iov`` and ``expert_config``  valuse at the moment.
``requested_iov`` value is the overall bucket IoV, and is the run range that your output payloads should cover.

.. note:: Although ``kwargs["requested_iov"]`` has both a defined lower and upper bound e.g. IoV(2, 1, 2, 100).
          For prompt processing you should endeavour to have your output payloads be open-ended e.g. IoV(2, 1, -1, -1).

.. note:: We use ``**kwargs`` so that in the future if we change what is being sent into ``get_calibrations`` we won't have to
          worry about adding a new argument to the function in every script.

Command Line Tools
==================

.. _b2caf_prompt_show:

b2caf-prompt-show
-----------------

.. argparse::
    :filename: calibration/tools/b2caf-prompt-show
    :func: get_argparser
    :prog: b2caf-prompt-show
    :nodefaultconst:
    :nogroupsections:

.. _b2caf_prompt_run:

b2caf-prompt-run
----------------

.. argparse::
    :filename: calibration/tools/b2caf-prompt-run
    :func: get_argparser
    :prog: b2caf-prompt-run
    :nodefaultconst:
    :nogroupsections:

.. _b2caf_prompt_check:

b2caf-prompt-check
------------------

.. argparse::
    :filename: calibration/tools/b2caf-prompt-check
    :func: get_argparser
    :prog: b2caf-prompt-check
    :nodefaultconst:
    :nogroupsections:


Utility Functions
=================

.. automodule:: prompt.utils
    :members:
