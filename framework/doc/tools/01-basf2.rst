``basf2``: The Main Belle2 Software Executable
++++++++++++++++++++++++++++++++++++++++++++++

The main command line tool for the Belle2 Software is ``basf2``::

    usage: basf2 [OPTIONS] [STEERING_FILE] [-- STEERING_FILE_OPTIONS...]

.. rubric:: Generic Options (to be used instead of steering file)

-h, --help
                       print all available options
-v, --version          print the version of the Belle2 Software
--info                 print information about basf2 and the environment
-m ARG, --modules ARG  Without argument this will print a list of all available
                       `Module`. With an extra argument the list can be limited
                       to one package or one specific module.

.. rubric:: Configuration

-l LOGLEVEL, --log_level LOGLEVEL
                          Set global log level (one of DEBUG, INFO, RESULT,
                          WARNING, or ERROR). Takes precedence over
                          set_log_level() in steering file.
-d DEBUGLEVEL, --debug_level DEBUGLEVEL
                          Set the default debug level and also enable DEBUG as
                          the default log level. This does not override
                          any settings in the steering file but just changes
                          the defaults. If you want to force debug level please
                          use ``-l DEBUG``
-n N, --events N          Override number of events to be processed, either when
                          reading files or when generating events.
--run RUNNUMBER           Override run for EventInfoSetter, must be used with
                          -n and --experiment
--experiment EXPNUMBER    Override experiment for EventInfoSetter, must be used
                          with -n and --run
--skip-events SKIPNEVENTS
                          Override skipNEvents for EventInfoSetter and
                          RootInput. Skips this many events before starting.
-i FILENAME, --input FILENAME
                          Override name of input file for (Seq)RootInput. Can
                          be specified multiple times to use more than one
                          file. For RootInput, wildcards (as in ``*.root`` or
                          ``[1-3].root``) can be used, but need to be escaped with
                          ``\`` or by quoting the argument to avoid expansion by
                          the shell.
-S ARG, --sequence ARG    Override the number sequence (e.g. ``23:42,101``)
                          defining the entries (starting from 0) which are
                          processed by RootInput.Must be specified exactly once
                          for each file to be opened.This means one sequence
                          per input file AFTER wildcard expansion. This are not
                          event numbers but the entry numbers in the input
                          files and the first entry always has number 0.
-o FILENAME, --output FILENAME
                          Override name of output file for (Seq)RootOutput. In
                          case multiple modules are present in the path, only
                          the first will be affected.
-p NWORKER, --processes NWORKER
                          Override number of worker processes (>=1 enables, 0
                          disables parallel processing)

.. rubric:: Advanced Options

--module-io MODULENAME  Create diagram of inputs and outputs for a single
                        module, saved as ModuleName.dot. To create a
                        PostScript file, use e.g. 'dot ModuleName.dot -Tps -o
                        out.ps'.
--visualize-dataflow    Generate data flow diagram (dataflow.dot) for the
                        executed steering file.
--no-stats              Disable collection of statistics during event
                        processing. Useful for very high-rate applications,
                        but produces empty table with 'print(statistics)'.
--dry-run               Read steering file, but do not start any event
                        processing when process(path) is called. Prints
                        information on input/output files that would be used
                        during normal execution.
--dump-path FILENAME    Read steering file, but do not actually start any
                        event processing. The module path the steering file
                        would execute is instead pickled (serialized) into
                        the given file.
--execute-path FILENAME
                        Do not read any provided steering file, instead
                        execute the pickled (serialized) path from the given
                        file.
--profile MODULENAME    Name of a module to profile using callgrind. If more
                        than one module of that name is registered only the
                        first one will be profiled.

.. rubric:: Examples

The most simple invocation is to just run basf2 with the name of a python
steering file to execute::

    $ basf2 steeringfile.py

Additional arguments can be given to the steering file which will be available in `sys.argv`::

    $ basf2 steeringfile.py argument1 argument2

To make sure that the arguments are actually passed to the steering file and
not consumed by ``basf2`` one should use to dashes ``--`` to separate the
arguments which should be passed to the steering file. This allows the steering
file to handle options like ``-o`` which would otherwise be handled by basf2
and not available in the steering file::

    $ basf2 steeringfile.py -- argument1 -o argument3

To specify the input/output files which should be used by ``basf2`` instead of
the one configured in the steering file itself one can supply the ``-i`` and
``-o`` options::

    $ basf2 -i input1.root -i input2.root -o output.root steeringfile.py

