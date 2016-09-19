#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
NOW OUT OF DATE AND SHOULD NOT BE USED. PLEASE REFER TO THE EXAMPLES DIRECTORY
FOR EXAMPLES OF THE NEW CALIBRATION FRAMEWORK.

Calibration Framework

This script implements a simple calibration framework runner which does not yet
have dependencies. It's currently not capable of multiprocessing but that is not
a general problem, just currently done for simplicity

To use, define your reconstruction path as usual. Then you need one calibration
module and one or more calibration algorithms and can simply do

    >>> from calibration_framework import add_calibration, run_calibration
    >>> collector = register_module(...)
    >>> algorithm = [ ... list of CalibrationAlgorithm instances ... ]
    >>> add_calibration(collector, algorithms, "mycalibration")
    >>> run_calibration(path)
"""

from basf2 import *
import sys
import os
import multiprocessing
import shutil
from collections import OrderedDict
from datetime import datetime
from lxml import etree

import ROOT
from ROOT.Belle2 import PyStoreObj, CalibrationAlgorithm


class SimpleCalibrationRunner(Module):
    """
    Simple calibration runner without any dependencies.

    This module is responsible of calling the CalibrationAlgorithm with the
    correct run iovs and record all calibration results.
    """

    def __init__(self, algorithm, status):
        """Constructor"""
        super().__init__()
        #: pointer to the algorithm
        self.algorithm = algorithm
        #: list of currently collected iovs
        self.runs = set()
        #: pointer to list of calibration results and runs to iterate
        self.status = status

    def beginRun(self):
        """Collect all runs we have seen"""
        event = PyStoreObj("EventMetaData").obj()
        self.runs.add((event.getExperiment(), event.getRun()))

    def execute(self, runs):
        """Execute the algorithm over list of runs"""
        global _iteration
        # create std::vector<ExpRun> for the argument
        iov_vec = ROOT.vector("std::pair<int,int>")()
        pair = ROOT.pair("int", "int")()
        for run in runs:
            pair.first, pair.second = run
            iov_vec.push_back(pair)
        # run the algorithm
        result = self.algorithm.execute(iov_vec, _iteration)
        # remember the results
        self.status["results"].append((tuple(runs), result))
        return result

    def terminate(self):
        """Run the calibration algorithm"""
        # if we are in iteration >0 we only rerun the calibration on runsets
        # which needed iteration
        if "iterate" in self.status:
            for runs in self.status["iterate"]:
                self.execute(runs)
        else:
            runs = []
            for run in sorted(self.runs):
                runs.append(run)
                result = self.execute(runs)
                # if anything else then NotEnoughData is returned then we
                # empty the list of runs for the next call
                if result != CalibrationAlgorithm.c_NotEnoughData:
                    runs = []

#: global variable to the one and only calibration path, that is all collectors
# and algorithms
_calib_path = None

#: global variable to keep track of which algorithm belongs to which calibration
# and what are the results and which run sets need another iteration
_calibrations = OrderedDict()


def add_calibration(collector, algorithms, name=None):
    """Add a calibration.

    A calibration consists of one collector which collects data during event
    processing and one or multiple algorithms which create payloads from the
    collected data. Each calibration should have a name

    Args:
        collector (str or basf2.Module): collector module (or name of the module)
        algorithms (CalibrationAlgorithm instance or list thereof): one or
            multiple CalibrationAlgorithm instances to run
        name (str, optional): name of the calibration, if None, name of the
            collector module is used
    """

    global _calib_path, _calibrations

    # check that we have the algorithms as a list to make handling of one and
    # many algorithms identical
    if not isinstance(algorithms, list) and not isinstance(algorithms, tuple):
        algorithms = [algorithms]

    # check if collector is already a module or if we need to create one from
    # the name
    if isinstance(collector, str):
        collector = register_module(collector)

    # check if it's really a module
    if not isinstance(collector, Module):
        B2FATAL("Collector needs to be either a Module or the name of such a module")

    # check if we have a name
    if name is None:
        # if not take the collector name
        name = collector.name()
    else:
        # otherwise set the collector name
        collector.set_name(name)

    # name sure the name is not already in the list of calibrations
    if name in _calibrations:
        B2FATAL("Another calibration with the name '{0}' is already registered".format(name))

    # add the calibration to the global dictionary
    _calibrations[name] = {"name": name, "type": collector.type(), "algorithms": []}

    # make sure the global calibration path is actually created
    if _calib_path is None:
        _calib_path = create_path()

    # and add the collector module
    _calib_path.add_module(collector)

    # now add all algorithms after the collector
    for i, a in enumerate(algorithms):
        if not isinstance(a, CalibrationAlgorithm):
            B2FATAL("Algorithms need to inherit from Belle2::CalibrationAlgorithm")

        algorithm_type = type(a).__name__
        if collector.type() != a.getCollectorName():
            B2WARNING("Algorithm '%s' requested collector '%s' but got '%s'" %
                      (algorithm_type, a.getCollectorName(), collector.type()))
        a.setPrefix(name)
        # add empty list to contain results
        _calibrations[name]["algorithms"].append({"type": algorithm_type, "results": [], "description": a.getDescription()})
        # create runner with pointer to algorithm and to results
        runner = SimpleCalibrationRunner(a, _calibrations[name]["algorithms"][i])
        # and add calibration runner to path
        _calib_path.add_module(runner)


def __run_child(path, iteration, database_filename, child_conn, iteration_callback):
    """Run the calibration processing. This function is executed in a child
    process which inherited all the necessary objects. This is done to guarantee
    that we can actually execute more than once if we need to iterate.

    Args:
        path: basf2.Path instance to process
        iteration: iteration number
        child_conn: Pipe to send data to parent process
    """

    global _calibrations, _iteration

    # set the global variable for the iteration number
    _iteration = iteration

    # add local database to save payloads
    use_local_database(database_filename, "", LogLevel.DEBUG)
    # call the callback if possible
    if iteration_callback is not None:
        iteration_callback(iteration)
    # do the actuall processing
    process(path)

    # send _calibrations object to parent process
    child_conn.send(_calibrations)


def run_calibration(path, output_path=".", max_iterations=5, iteration_callback=None):
    """Run the calibration on a given path.

    You need to call add_calibration at least once before calling this function.
    This function will *not return*.

    Args:
        path (basf2.Path): path to process before running the collectors/algorithms
        output_path (str): output directory for the payloads, the database.txt
            and the CalibrationResults.xml file
        max_iterations (int): maximum number of repeats before calibration is
            considered failed.
        iteration_callback (function): function to be called before the
            iteration takes place, for example to set the random seed. This
            function should take one argument which is the iteration number
    """

    global _calib_path
    if _calib_path is None:
        B2FATAL("No calibrations registered")

    # get friendly names for the return values
    result_names = {
        CalibrationAlgorithm.c_OK: "OK",
        CalibrationAlgorithm.c_NotEnoughData: "NotEnoughData",
        CalibrationAlgorithm.c_Iterate: "Iterate",
        CalibrationAlgorithm.c_Failure: "Failure",
    }

    # prepare the xml tree for the results
    xml = etree.ElementTree(etree.Element("CalibrationResults", start=datetime.now().isoformat()))

    # add the calibration modules to the path
    path.add_path(_calib_path)

    # prepare a multiprocessing context which uses fork
    ctx = multiprocessing.get_context("fork")
    # and a pipe between parent and child process
    parent_conn, child_conn = ctx.Pipe()

    # make sure output directory exists
    os.makedirs(output_path, exist_ok=True)
    # set filename for xml output
    result_filename = os.path.join(output_path, "CalibrationResults.xml")
    B2RESULT("Saving calibration results to '%s'" % result_filename)
    if os.path.exists(result_filename):
        B2WARNING("CalibrationResults.xml already exists in output path, renaming old one ...")
        os.rename(result_filename, result_filename + "~")
    # set filename for database.txt output
    database_filename = os.path.join(output_path, "calibration-database.txt")
    if os.path.exists(database_filename):
        B2WARNING("There is already a calibration-database.txt in the output path, "
                  "it's content will be used during this calibration run and then replaced")
        shutil.copy(database_filename, database_filename + "~")

    # make sure we use a database chain: this will create one and add the
    # current database to it if no chain exists so far
    use_database_chain(True)
    # add local database
    use_local_database(database_filename, "", LogLevel.DEBUG)

    # and start iterating
    for iteration in range(max_iterations):
        B2INFO("Calibration: starting iteration %d" % (iteration + 1))
        new_database_filename = os.path.join(output_path, "calibration-database-iteration%d.txt" % (iteration + 1))
        # make sure it doesn't have any contents
        try:
            os.unlink(new_database_filename)
        except FileNotFoundError:
            pass
        # run processing in child process
        child = ctx.Process(target=__run_child, args=(path, iteration, new_database_filename, child_conn, iteration_callback))
        child.start()
        # wait for it to finish
        child.join()
        if child.exitcode != 0:
            B2FATAL("Calibration process exited with non-zero exit code")
        # and get results from pipe
        results = parent_conn.recv()
        B2INFO("Calibration: finished iteration %d" % (iteration + 1))
        # ok, calibration is done for this iteration, now check results and add
        # to output xml
        xml_results = etree.SubElement(xml.getroot(), "CalibrationRun", iteration=str(iteration + 1))
        # if the database file exists somebody saved some payloads so remember
        # the payload file
        if os.path.exists(new_database_filename):
            xml_results.set("payloads", os.path.relpath(new_database_filename, output_path))
        # now check the results and remember if we need to iterate and count the
        # errors
        need_iterate = False
        failures = 0
        for name, calibration in results.items():
            xml_cal = etree.SubElement(xml_results, "Calibration", collector=calibration["type"], name=calibration["name"])
            for i, algorithm in enumerate(calibration["algorithms"]):
                # let's assume there's nothing to iterate over. We'll add all
                # run sets we need to iterate over in here
                _calibrations[name]["algorithms"][i]["iterate"] = []
                xml_algo = etree.SubElement(xml_cal, "Algorithm", type=algorithm["type"])
                xml_desc = etree.SubElement(xml_algo, "Description")
                xml_desc.text = algorithm["description"]
                for runs, result in algorithm["results"]:
                    xml_runs = etree.SubElement(xml_algo, "RunRange", result=result_names[result])
                    if result == CalibrationAlgorithm.c_Iterate:
                        need_iterate = True
                        # remember this set of runs to iterate again
                        _calibrations[name]["algorithms"][i]["iterate"].append(runs)
                    elif result == CalibrationAlgorithm.c_Failure:
                        failures += 1
                    for run in runs:
                        etree.SubElement(xml_runs, "Run", experiment=str(run[0]), run=str(run[1]))

        # copy last iteration database file to calibration database
        if os.path.exists(new_database_filename):
            shutil.copy(new_database_filename, database_filename)

        # write output file
        with open(os.path.join(output_path, "CalibrationResults.xml"), "wb") as fx:
            xml.getroot().set("end", datetime.now().isoformat())
            # save the final database file name if it got created
            if os.path.exists(database_filename):
                xml.getroot().set("payloads", os.path.relpath(database_filename, output_path))
            xml.write(fx, pretty_print=True)

        # see if we need to iterate
        if not need_iterate:
            break
    else:
        B2ERROR("Maximum Iterations reached")
        sys.exit(1)

    # if there's a failure then return non non empty error code
    if failures > 0:
        B2ERROR("Calibration: %d failures reported" % failures)
    sys.exit(failures > 0 and 1 or 0)


if __name__ == "__main__":
    # Here comes some stupid testing. We want a simple module which just returns
    #  - not enough data on the first run
    #  - failure on the third run
    #  - iterate in first iteration
    # To keep it contained we define it here for now
    ROOT.gInterpreter.AddIncludePath(os.path.join(os.environ["BELLE2_LOCAL_DIR"], "include"))
    ROOT.gInterpreter.ProcessLine("#include <calibration/CalibrationAlgorithm.h>")
    ROOT.gInterpreter.ProcessLine("""
namespace Belle2 {
  class TestAlgo: public CalibrationAlgorithm {
    public:
      TestAlgo():CalibrationAlgorithm("ParticleGun"){setDescription("foo");}
      EResult calibrate() override {
        auto runs = getRunList();
        if(runs.back() == ExpRun(1,1)){
            return c_NotEnoughData;
        }else if(runs.back() == ExpRun(1,3)){
            return c_Failure;
        }
        if(getIteration()<1) return c_Iterate;
        saveCalibration(new TVector3(0,1,2), "mystuff");
        commit();
        return c_OK;
    }
    ClassDefOverride(TestAlgo,1);
  };
}""")
    from ROOT.Belle2 import TestAlgo, TestCalibrationAlgorithm

    # now run the calibration with just EventInfoSetter but with both algorithms
    main = create_path()
    set_random_seed(12345)
    main.add_module("EventInfoSetter", evtNumList=[50, 50, 200], expList=[1, 1, 1], runList=[1, 2, 3])
    add_calibration("CaTest", [TestCalibrationAlgorithm()])
    add_calibration(register_module("CaTest"), TestAlgo(), name="foo")
    run_calibration(main, "testcal")
