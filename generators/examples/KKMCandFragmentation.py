#!/usr/bin/env python3
# -*- coding: utf-8 -*-

########################################################
# Steering for the new continuum production scheme,
# running KKMC and PYTHIA in a single job.
#
# Produces ee->uubar(g) events. For dd, ss or cc you have to
# change the KKMC 'tauinputfile'.
#
# Author(s): Martin Ritter (martin.ritter@lmu.de)
#            Torben Ferber (ferber@physics.ubc.ca)
#
########################################################

from basf2 import *
from ROOT import Belle2
import multiprocessing
import tempfile

# Suppress messages and warnings during processing.
set_log_level(LogLevel.RESULT)


def kkmcgeneration(outputfile):
    """Steering to generate kkmc events, use outputfile to store events"""
    main = create_path()

    # make sure random seed is unique by just appending a suffix to it
    set_random_seed(get_random_seed() + "_kkmcgeneration")

    # event info setter
    main.add_module("EventInfoSetter", expList=1, runList=1, evtNumList=100)

    # use KKMC to generate uubar events (no fragmentation at this stage)
    kkgeninput = main.add_module(
        'KKGenInput',
        tauinputFile=Belle2.FileSystem.findFile('data/generators/kkmc/uubar_nohadronization.input.dat'),
        KKdefaultFile=Belle2.FileSystem.findFile('data/generators/kkmc/KK2f_defaults.dat'),
        taudecaytableFile='',
        kkmcoutputfilename='kkmc_uu.txt',
    )

    # add RootOutput module to write to tmp file but this means we have to
    # ingore the -o command line option to basf2
    main.add_module("RootOutput", outputFileName=outputfile, ignoreCommandLineOverride=True)

    process(main)


def kkmcfragmentation(inputfile):
    """Steering to run pythia on generated events, generated events are read from inputfile"""
    main = create_path()

    # make sure random seed is unique by just appending a suffix to it
    set_random_seed(get_random_seed() + "_kkmcfragmentation")

    # read temporary file we created before, which means we have to ignore the
    # -i command line option to basf2
    main.add_module('RootInput', inputFileName=inputfile, ignoreCommandLineOverride=True)

    fragmentation = main.add_module(
        'Fragmentation',
        ParameterFile=Belle2.FileSystem.findFile('data/generators/modules/fragmentation/pythia_belle2.dat'),
        ListPYTHIAEvent=0,
        UseEvtGen=1,
        DecFile=os.path.expandvars('$BELLE2_EXTERNALS_DIR/share/evtgen/DECAY_2010.DEC'),
        UserDecFile=Belle2.FileSystem.findFile('data/generators/modules/fragmentation/dec_belle2_qqbar.dec'),
    )

    # branch to an empty path if PYTHIA failed, this will change the number of events that we need to track somehow...!
    emptypath = create_path()
    fragmentation.if_value('<1', emptypath)

    # add simulation
    # ...

    # add reconstruction
    # ...

    # Finally add output module
    main.add_module("RootOutput")
    # and process the job
    process(main)


def fork_process(target, args):
    """Run function in forked child to avoid side effects.
    This funtion will call target(*args) in a forked subprocess which means that
    we can run process() more than once and don't run into problems with symbol
    collisions between kkmc and pythia."""

    process = multiprocessing.Process(target=target, args=args)
    process.start()
    process.join()
    if process.exitcode != 0:
        B2FATAL("Child process exited with code %d" % process.exitcode)

if __name__ == "__main__":
    # creates a tmp file that will be deleted in the end.
    with tempfile.NamedTemporaryFile() as tmp:
        # run generation in subprocess
        fork_process(kkmcgeneration, (tmp.name,))
        # run fragmentation in subprocess
        fork_process(kkmcfragmentation, (tmp.name,))
