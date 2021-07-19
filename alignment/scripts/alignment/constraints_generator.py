##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2

import os
import sys
import pickle

# Needed for pickling of constraints
from alignment.constraints import *  # noqa


class ConstraintsGenerator(b2.Module):
    """
    basf2 python module to generate a file with a single set of constraints

    (Needed especially for time-dependent constraints)
    """

    def __init__(self, generator):
        """
        Constructor

        Parameters
        ----------
        generator : alignment.Constraints
          Set of constraints to be generated (defines also filename)

        """

        super(ConstraintsGenerator, self).__init__()
        #: The generator: alignment.Constraints
        self.generator = generator
        #: Generated constraints (key is constraint checksum)
        self.consts = dict()
        super(ConstraintsGenerator, self).set_name(super(ConstraintsGenerator, self).name() + ":" + generator.filename)

    def event(self):
        """ Event: attempt to add constraints
        At each event, attempt to generate the constraints and add them
        to the dict (same cheksum - using only labels - means it gets overriden!)
        Thus is only works if the time-dependence is set (otherwise you always get the same labels and
        only the last constraint coefficients will be stored)
        """
        consts = self.generator.generate()
        for const in consts:
            self.consts[const.get_checksum()] = const.data

    def terminate(self):
        """ Terminate: write-down collect constraint data to file
        """
        if len(self.consts):
            with open(self.generator.filename, 'w') as f:
                for checksum, data in self.consts.items():
                    f.write('Constraint 0.\n')
                    for label, coef in data:
                        f.write('{} {}\n'.format(label, coef))
                print("Finished file: ", self.generator.filename)
        print("Done: ", self.generator.filename)


def save_config(constraint_sets, timedep_config=None, global_tags=None, init_event=None):
    """
    Save constraints configuration to a file (using pickle)

    Parameters
    ----------
    constraint_sets : list (alignment.Constraints)
      List of sets of constraints
    timedep_config : list(tuple(list(int), list(tuple(int, int, int))))
        Time-depence configuration.
        Each list item is 2-tuple with list of parameter numbers (use alignment.parameters to get them) and
        the (event, run, exp) numbers at which values of these parameters can change.
    global_tags : list (str)
        List of global tag names and/or (absolute) file paths to local databases
    init_event : tuple( int, int, int)
        Event (event, run, exp) at which to initialize time-INdependent constraints

    """
    a = {'constraint_sets': constraint_sets, 'timedep_config': timedep_config, 'global_tags': global_tags, 'init_event': init_event}

    file_Name = "constraint_config.pickled"
    # open the file for writing
    with open(file_Name, 'wb') as fileObject:

        # this writes the object a to the
        # file named 'testfile'
        pickle.dump(a, fileObject)
        print('Saving config to: ', file_Name)

    return file_Name


def read_config(filename='constraint_config.pickled'):
    """ Read the pickled constraint configuration from a file

    filename : str
      File with pickled constraints' configuration
    """
    print('Reading config from: ', filename)
    with open(filename, 'rb') as fileObject:
        # load the object from the file into var b
        b = pickle.load(fileObject)
    return (b['constraint_sets'], b['timedep_config'], b['global_tags'], b['init_event'])


def gen_constraints(constraint_sets, timedep_config=None, global_tags=None, init_event=None):
    """
    Generate "event files" from timedep config and run over them to collect constraint data
    and write it to files.

    This is a bit tricky. I did not found a way to run basf2 over just a specified list of events
    other than generating the files with metadata for event and then running over the files.
    This uses unning basf2 multiple times in one script - seems to work despite the warnings - but only
    for the generation of "event files"

    Parameters
    ----------
    constraint_sets : list (alignment.Constraints)
      List of sets of constraints
    timedep_config : list(tuple(list(int), list(tuple(int, int, int))))
        Time-depence configuration.
        Each list item is 2-tuple with list of parameter numbers (use alignment.parameters to get them) and
        the (event, run, exp) numbers at which values of these parameters can change.
    global_tags : list (str)
        List of global tag names and/or (absolute) file paths to local databases
    init_event : tuple( int, int, int)
        Event (event, run, exp) at which to initialize time-INdependent constraints

    """
    if timedep_config is None:
        timedep_config = []

    if global_tags is None:
        global_tags = [tag for tag in b2.conditions.default_globaltags]

    events = []
    for (labels, events_) in timedep_config:
        events += [event for event in events_]

    if not len(timedep_config):
        if init_event is None:
            init_event = (0, 0, 0)
        events = [init_event]

    events = [(exp, run, ev) for (ev, run, exp) in events]
    events = sorted(list(set(events)))
    events = [(ev_, run_, exp_) for (exp_, run_, ev_) in events]

    fileName = 'TimedepConfigEvent_exp{}run{}ev{}.root'
    files = []

    print('Global tags:')
    print(global_tags)
    print('Global tags reversed (this will be used for b2.conditions.override_globaltags(...)):')
    print([tag for tag in reversed(global_tags)])

    for tag in [tag for tag in reversed(global_tags)]:
        if os.path.exists(tag):
            b2.conditions.append_testing_payloads(os.path.abspath(tag))
        else:
            b2.conditions.append_globaltag(tag)

    for index, event in enumerate(events):
        #  conditions.reset()

        ev, run, exp = event
        path = b2.create_path()
        path.add_module("EventInfoSetter",
                        skipNEvents=ev,
                        evtNumList=[ev + 1],
                        runList=[run],
                        expList=[exp])
        path.add_module('Progress')
        this_filename = fileName.format(exp, run, ev)
        path.add_module('RootOutput', outputFileName=this_filename, ignoreCommandLineOverride=True)
        files.append(this_filename)
        b2.process(path)
        print(b2.statistics)

    print(files)

    # conditions.override_globaltags(global_tags)

    path = b2.create_path()
    path.add_module("RootInput", inputFileNames=files, ignoreCommandLineOverride=True)
    path.add_module('HistoManager')
    path.add_module('Progress')
    path.add_module('Gearbox')
    path.add_module('Geometry')

    collector = path.add_module('MillepedeCollector',
                                timedepConfig=timedep_config)

    constraint_files = []
    for constraint_set in constraint_sets:
        constraint_set.configure_collector(collector)
        constraint_files.append(constraint_set.filename)
        path.add_module(ConstraintsGenerator(constraint_set))

    b2.process(path)
    print(b2.statistics)

    return [os.path.abspath(file) for file in constraint_files]


def gen_constraints_from_config(filename='constraint_config.pickled'):
    """
    Generate constraints from pickled configuration
    """
    constraint_sets, timedep_config, global_tags, init_event = read_config(filename)
    gen_constraints(constraint_sets, timedep_config, global_tags, init_event)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: basf2 constraints_generator.py config_filename')
        exit()

    #: File name
    filename = sys.argv[1]
    gen_constraints_from_config(filename)
