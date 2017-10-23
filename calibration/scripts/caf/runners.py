#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from abc import ABC, abstractmethod
from threading import Thread
import ROOT
from caf.utils import decode_json_string
from caf.state_machines import MachineError, ConditionError, TransitionError


class Runner(ABC):
    """Abstract Base Class for Runner type object"""
    @abstractmethod
    def run(self):
        pass


class AlgorithmsRunner(Runner):
    """
    Base class for `AlgorithmsRunner` classes. Defines the necessary information that will be provided to every
    `AlgorithmsRunner` used by the `framework.CAF`

    An `AlgorithmsRunner` will be given a list of `framework.Algorithm` objects defined during the setup of a
    `framework.Calibration` instance. The `AlgorithmsRunner` describes how to run each of the `strategies.AlgorithmStrategy`
    objects. As an example, assume that a single `framework.Calibration` was given and list of two `framework.Algorithm`
    instances to run.

    In this example the chosen `AlgorithmsRunner.run` is simple and just loops over the list of `framework.Algorithm`
    calling each one's `strategies.AlgorithmStrategy.run` and `strategies.AlgorithmStrategy.commit` methods in order.
    Thereby generating a localdb with the only communication between the `AlgorithmStrategy` instances coming from the
    database payloads being available from one algorithm to the next.

    But you could imagine a more complex situation. The `AlgorithmsRunner` might take the first `framework.Algorithm` and
    call its `AlgorithmStrategy.run` for only the first (exp,run) in the collected data. Then it might not commit the payloads
    to a localdb but instead pass some calculated values to the next algorithm to run on the same IoV. Then it might go back
    and re-run the first AlgorithmStrategy with new information and commit payloads this time. Then move onto the next IoV.

    Hopefully you can see that while the default provided `AlgorithmsRunner` and `AlgorithmStrategy` classes should be good for
    most situations, you have lot of freedom to define your own strategy if needed. By following the basic requirements for the
    interface to the `framework.CAF` you can easily plugin a different special case, or mix and match a custom class with
    default CAF ones.

    The run() method should be defined for every derived `AlgorithmsRunner`. It will be called once and only once for each
    iteration of (collector -> algorithm).

    Input files are automatically given via the `framework.Calibration.output_patterns` which constructs
    a list of all files in the collector output directories that match the output_patterns. If you have multiple types of
    output data it is your job to filter through the input files and assign them correctly.

    A list of local database paths are given to the `AlgorithmsRunner` based on the `framework.Calibration` dependencies and
    any overall localdb given to the CAF. By default you can call the `setup_database_chain` method to automatically set a
    database chain based on this list. But you have freedom to not call this at all in `run`, or to implement a different method
    to deal with this.
    """

    def __init__(self):
        self.algorithms = []
        self.input_files = []
        self.local_database_chain = []
        self.output_database_dir = ""

    def run(self):
        pass


class RunnerError(Exception):
    """
    Base exception class for Runners
    """
    pass


class CalibrationRunner(Thread, Runner):
    """
    Runs a `state_machines.CalibrationMachine` in a Thread. Will process from intial state
    to the final state. Because we use the same method naming as the Thread class and implement
    a `run` method, the `run` method we define gets called by the Thread defined `start` method when
    we start a Thread.
    """

    #: Allowed transitions that we will use to progress
    moves = ["submit_collector", "complete", "run_algorithms", "iterate"]

    def __init__(self, machine=None, heartbeat=None):
        """
        """
        super().__init__()
        #: The `CalibrationMachine` that we will run
        self.machine = machine
        #: The class that runs all the algorithms in this Calibration using their assigned `strategies.AlgorithmStrategy`
        self.algorithms_runner = AlgorithmsRunner()
        self.setup_defaults()
        if heartbeat:
            #: Heartbeat of the monitoring
            self.heartbeat = heartbeat

    def run(self):
        """
        Will be run in a new Thread by calling the start() method
        """
        from time import sleep
        while self.machine.state != "completed":
            for trigger in self.moves:
                try:
                    if trigger in self.machine.get_transitions(self.machine.state):
                        getattr(self.machine, trigger)()
                    sleep(self.heartbeat)  # Only sleeps if transition completed
                except ConditionError:
                    continue

            if self.machine.state == "failed":
                break

    def setup_defaults(self):
        """
        Anything that is setup by outside config files by default goes here.
        """
        import configparser
        config_file_path = ROOT.Belle2.FileSystem.findFile('calibration/data/caf.cfg')
        if config_file_path:
            config = configparser.ConfigParser()
            config.read(config_file_path)
        else:
            B2FATAL("Tried to find the default CAF config file but it wasn't there. Is basf2 set up?")
        self.heartbeat = decode_json_string(config['CAF_DEFAULTS']['Heartbeat'])
