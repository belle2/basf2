'''
Created on 28 May 2018

@author: kleinwrt
'''
from basf2 import *
from ROOT import Belle2

import os
import sys
import pickle

# Needed for pickling of constraints
from alignment.constraints import *


class ConstraintsGenerator(Module):
    def __init__(self, generator):
        """ init """
        super(ConstraintsGenerator, self).__init__()
        self.generator = generator
        self.consts = dict()
        super(ConstraintsGenerator, self).set_name(super(ConstraintsGenerator, self).name() + ":" + generator.filename)

    def event(self):
        consts = self.generator.generate()
        print(consts)
        for const in consts:
            self.consts[const.get_checksum()] = const.data

    def terminate(self):
        if len(self.consts):
            with open(self.generator.filename, 'w') as f:
                for checksum, data in self.consts.items():
                    f.write('Constraint 0.\n')
                    for label, coef in data:
                        f.write('{} {}\n'.format(label, coef))
                print("Done: ", self.generator.filename)


def save_config(constraint_sets, timedep_config=None, global_tags=None):
    a = {'constraint_sets': constraint_sets, 'timedep_config': timedep_config, 'global_tags': global_tags}

    file_Name = "constraint_config.pickled"
    # open the file for writing
    with open(file_Name, 'wb') as fileObject:

        # this writes the object a to the
        # file named 'testfile'
        pickle.dump(a, fileObject)
        print('Saving config to: ', file_Name)

    return file_Name


def read_config(filename='constraint_config.pickled'):
    with open(filename, 'rb') as fileObject:
        # load the object from the file into var b
        b = pickle.load(fileObject)
    return (b['constraint_sets'], b['timedep_config'], b['global_tags'])


def gen_constraints(constraint_sets, timedep_config=None, global_tags=None):
    if timedep_config is None:
        timedep_config = []

    if global_tags is None:
        global_tags = [tag for tag in conditions.default_globaltags]

    events = []
    for (labels, events_) in timedep_config:
        events += [event for event in events_]

    if not len(timedep_config):
        events = [(0, 0, 0)]

    events = [(exp, run, ev) for (ev, run, exp) in events]
    events = sorted(list(set(events)))
    events = [(ev_, run_, exp_) for (exp_, run_, ev_) in events]

    fileName = 'TimedepConfigEvent_exp{}run{}ev{}.root'
    files = []

    print(global_tags)
    print(reversed(global_tags))

    conditions.override_globaltags([tag for tag in reversed(global_tags)])
    conditions.prepend_testing_payloads(os.path.abspath(
        "/home/tadeas/belle2/head/alignment/examples/fuckingworkaround/database.txt"))

    for index, event in enumerate(events):
        #  conditions.reset()

        ev, run, exp = event
        path = create_path()
        path.add_module("EventInfoSetter",
                        skipNEvents=ev,
                        evtNumList=[ev + 1],
                        runList=[run],
                        expList=[exp])
        path.add_module('Progress')
        this_filename = fileName.format(exp, run, ev)
        path.add_module('RootOutput', outputFileName=this_filename, ignoreCommandLineOverride=True)
        files.append(this_filename)
        process(path)
        print(statistics)

    print(files)

    # conditions.override_globaltags(global_tags)

    path = create_path()
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

    process(path)
    print(statistics)

    return [os.path.abspath(file) for file in constraint_files]


def gen_constraints_from_config(filename='constraint_config.pickled'):
    constraint_sets, timedep_config, global_tags = read_config(filename)
    gen_constraints(constraint_sets, timedep_config, global_tags)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: basf2 constraints_generator.py config_filename')
        exit()

    filename = sys.argv[1]
    gen_constraints_from_config(filename)
