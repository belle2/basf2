Tools for file handling
+++++++++++++++++++++++

``b2file-metadata-show``: Show the metadata of a basf2 output file
------------------------------------------------------------------

This tool shows the recorded metadata of a basf2 output file like number of
events, lowest event number and so forth. It can either work on a root file or
look for the file in a local xml file catalog using an logical file name (LFN)::

    usage: b2file-metadata-show [-h] [-a] [-s] [--json] (FILENAME|-l LFN)

.. rubric:: Optional Arguments

-h, --help         print all available options
-l LFN, --lfn LFN  logical file name
-a, --all          print all information
--json             print machine-readable information in JSON format.
                   Implies ``--all`` and ``--steering``.
-s, --steering     print steering file contents

``b2file-metadata-add``: Add/Edit LFN in given file
---------------------------------------------------

This tools allows to modify the LFN and the data descriptions stored in a given
basf2 output file. It will also update the xml file catalog.

The keys and values for the data descriptions can take any value and are not
used by the offline software in any way. They can be used for bookkeeping or
additional information not otherwise included in the metadata.

At the moment the only commonly used keys are

dataLevel
  is automatically set when using `reconstruction.add_mdst_output`,
  `modularAnalysis.outputMdst`, `modularAnalysis.outputUdst`, or
  `modularAnalysis.skimOutputUdst` functions and will be set to either "mdst"
  or "udst".

skimDecayMode
  is automatically set when using `modularAnalysis.skimOutputUdst` and will
  contain the name of the skim.

::

  usage: b2file-metadata-add [-h] [-l LFN] [-d KEY=VALUE] FILENAME


.. rubric:: Optional Arguments

-h, --help         print all available options
--file arg         file name
-l LFN, --lfn LFN  logical file name
-d KEYVALUE, --description KEYVALUE
                   data description to set of the form key=value. If the
                   argument does not contain an equal sign it's interpeted as a
                   key to delete from the dataDescriptions.

``b2file-catalog-add``: Add a file to a local XML file catalog
--------------------------------------------------------------

This simple tool adds a file to a local XML file catalog so that it can be
found as a parent file independent of the local name or folder structure::

    usage: b2file-catalog-add [--help] FILENAME

``b2file-check``: Check a basf2 output file for consistency
-----------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2file-check
    :func: get_argument_parser
    :prog: check_basf2_file
    :nodefaultconst:
    :nogroupsections:

.. rubric:: Examples

The ``--json`` parameter can be used to get detailed output for all tests in a machine readable format::

    $ check_basf2_file --json framework/tests/root_input.root
    {
      "checks_passed": {
        "all_entries_readable": true,
        "contains_persistent": true,
        "contains_tree": true,
        "entries_eq_filemetedata": true,
        "eventmetadata_readable": true,
        "file_readable": true,
        "filemetadata_readable": true
      },
      "stats": {
        "compression_algorithm": 0,
        "compression_factor": 2.591318368911743,
        "compression_level": 1,
        "events": 5,
        "filesize_kib": 61.8984375,
        "mcevents": 0,
        "size_per_event_kib": 12.3796875
      }
    }

``b2file-merge``: Merge multiple basf2 output files
---------------------------------------------------

This program merges files created by separate ``basf2`` jobs. It's similar to
ROOT ``hadd`` but does correctly update the metadata in the file and merges the
objects in the persistent tree correctly.

This tool is intended to only merge output files from identical jobs which were
just split into smaller ones for convinience. As such the following
restrictions apply:

* The files have to be created with the same release and steering file
* The persistent tree is only allowed to contain FileMetaData and objects
  inheriting from Mergeable and the same list of objects needs to be present in
  all files.
* The event tree needs to contain the same DataStore entries in all files.

::

    usage: b2file-merge [-h] [-f] [-q] [--no-catalog] OUTPUTFILENAME
                        INPUTFILENAME [INPUTFILENAME ...]

.. rubric:: Optional Arguments

-f, --force   overwrite the output file if already present
-q, --quiet   if given only warnings and errors are printed
--no-catalog  don't register output file in file catalog

.. rubric:: Examples

* Combine all output files in a given directory::

    $ b2file-merge full.root jobs/*.root

* Merge all output files but be less verbose, overwrite the output if it
  exists and don't register in file catalog::

    $ b2file-merge -f -q --no-catalog full.root jobs/*.root

``b2file-mix``: Create a run of mixed data from a set of input files
---------------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2file-mix
    :func: create_argumentparser
    :prog: b2file-mix
    :nodefaultconst:
    :nogroupsections:


``b2file-size``: Show detailed size information about the content of a file
---------------------------------------------------------------------------

.. argparse::
    :filename: framework/tools/b2file-size
    :func: get_argument_parser
    :prog: b2file-size
    :nodefault:
    :nogroupsections:
