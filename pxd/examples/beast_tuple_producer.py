#!/usr/bin/env python3
# -*- coding: utf-8 -*-


# This steering script steers the production of ROOT tuples for beast.
#
# Before running, you have to put a file to IoV mapping called 'file_iov_map.pkl' in your
# working directory. You can create such a file using the script 'create_file_to_iov_map.py'
# in basf2 folder calibration/examples.
#
# basf2 beast_tuple_producer.py -- --runLow=5613 --runHigh=5613 --expNo=3
#
# This will compute tuples run-by-run for all runs between runLow and runHigh for the
# given experiment number.
#
# author: benjamin.schwenker@pyhs.uni-goettingen.de

NUMBER_OF_PROCESSES = 20

from basf2 import *
set_log_level(LogLevel.ERROR)

reset_database()
use_central_database("Calibration_Offline_Development")

from caf.utils import IoV
import SetMetaTimeModule

import multiprocessing

# Some ROOT tools
import ROOT
from ROOT import Belle2


class CalculationProcess(multiprocessing.Process):
    def __init__(self, iov, file_paths, output_dir):
        super(CalculationProcess, self).__init__()
        self.iov = iov
        self.file_paths = file_paths
        self.output_dir = output_dir

    def run(self):

        # Register modules
        rootinput = register_module('RootInput')
        rootinput.param('inputFileNames', self.file_paths)
        rootinput.param('branchNames', ['EventMetaData', 'RawPXDs', 'RawSVDs', 'RawCDCs'])
        gearbox = register_module('Gearbox')
        gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
        geometry = register_module('Geometry')
        geometry.param('components', ['PXD', 'SVD', 'CDC'])
        pxdclusterizer = register_module('PXDClusterizer')
        pxdclusterizer.param('ElectronicNoise', 1.0)
        pxdclusterizer.param('SeedSN', 9.0)
        pxdtupleproducer = register_module('PXDBgTupleProducer')
        pxdtupleproducer.param('outputFileName',
                               '{}/pxd_beast_tuple_exp_{}_run_{}.root'.format(self.output_dir,
                                                                              self.iov.exp_low,
                                                                              self.iov.run_low))

        # Create the path
        main = create_path()
        main.add_module(rootinput)
        main.add_module(SetMetaTimeModule.SetMetaTimeModule())
        main.add_module(gearbox)
        main.add_module(geometry)
        main.add_module('PXDUnpacker')
        main.add_module("ActivatePXDPixelMasker")
        main.add_module("ActivatePXDGainCalibrator")
        main.add_module("PXDRawHitSorter")
        main.add_module(pxdclusterizer)
        main.add_module(pxdtupleproducer)
        main.add_module(register_module('Progress'))

        # Process the run
        process(main)


#
# Function run by worker processes
#

def worker(task_q, done_q):
    for iov, file_paths, output_dir in iter(task_q.get, 'STOP'):
        print("Start processing IoV={}".format(str(iov)))
        p = CalculationProcess(iov, file_paths, output_dir)
        p.start()
        p.join()
        done_q.put(iov)


if __name__ == "__main__":

    import pickle
    import argparse
    parser = argparse.ArgumentParser(description="Produce pxd tuples and histofiles from ROOT formatted raw data")
    parser.add_argument('--runLow', default=0, type=int, help='Compute mask for specific IoV')
    parser.add_argument('--runHigh', default=-1, type=int, help='Compute mask for specific IoV')
    parser.add_argument('--expNo', default=3, type=int, help='Compute mask for specific IoV')
    parser.add_argument('--outputDir', default='./', type=str, help='Name of output directory for tuples')
    args = parser.parse_args()

    # Set the IoV range for this calibration
    iov_to_calibrate = IoV(exp_low=args.expNo, run_low=args.runLow, exp_high=args.expNo, run_high=args.runHigh)

    map_file_path = "file_iov_map.pkl"
    with open(map_file_path, 'br') as map_file:
        files_to_iovs = pickle.load(map_file)

    # Set of all currently known single run iovs
    iov_set = set(files_to_iovs.values())

    # Dict mapping single run iovs to their input files
    iovs_to_files = {}
    for iov in iov_set:
        if iov_to_calibrate.contains(iov):
            file_paths = [k for k, v in files_to_iovs.items() if v == iov]
            iovs_to_files[iov] = file_paths

    # Create queues
    task_queue = multiprocessing.Queue()
    done_queue = multiprocessing.Queue()

    # Submit tasks
    for iov, file_paths in iovs_to_files.items():
        task_queue.put((iov, file_paths, args.outputDir))

    # Start worker processes
    for i in range(NUMBER_OF_PROCESSES):
        multiprocessing.Process(target=worker, args=(task_queue, done_queue)).start()

    # Get and print results
    print('Unordered results:')
    for i in range(len(iovs_to_files)):
        print('\t', done_queue.get())

    # Tell child processes to stop
    for i in range(NUMBER_OF_PROCESSES):
        task_queue.put('STOP')
