#!/usr/bin/env python3
# -*- coding: utf-8 -*-


class AlgorithmStrategyBase():
    """Base class for Algorithm strategies. These do the actual execution of
CalibrationAlgorithm classes on collected data. Each strategy may be quite different
in terms of how fast it may be, how database payloads are passed between executions, and
if the final payloads have an IoV that is independent to the actual runs used to calculates
them."""

    def __init__(self):
        self.algorithm = None
        self.central_database_gt = ""
        self.input_files = []
        self.local_database_chain = []
        self.result = None

    def run():
        raise NotImplementedError('Need to implement a run() method in {} Strategy.'.format(self.__class__.__name__))


class SingleIOV(AlgorithmStrategyBase):
    """The fastest and simplest Algorithm strategy. Runs the algorithm only once over all of the input
data. The payload IoV is automatically calculated from the collected data."""

    def __init__(self):
        super().__init__()

    def run(machine, iov_to_calibrate, child_conn):
        """
        Runs a single AlgorithmMachine inside a subprocess over the IoV vector from the
        input data. Should produce the best constants requested and create a list of results
        to send out.
        """
        B2INFO("Setting up {}".format(machine.name))
        machine.setup_algorithm()
        B2INFO("Beginning execution of {}".format(machine.name))

        if not iov_to_calibrate:
            iov_to_execute = ROOT.vector("std::pair<int,int>")()
            for iov in machine.all_iov_collected:
                iov_to_execute.push_back(iov)
                machine.execute_iov(vec_iov=iov_to_execute)
                readable_iov_to_execute = iov_from_vector(iov_to_execute)
                if machine.results[-1].result == AlgResult.ok.value:
                    machine.success(successful_iov=iov_to_execute)
                    try:
                        machine.complete()
                    except ConditionError:
                        machine.setup_algorithm(vec_iovs=iov_to_execute)
                elif machine.results[-1].result == AlgResult.iterate.value:
                    machine.iteration_requested(successful_iov=iov_to_execute)
                    try:
                        machine.complete()
                    except ConditionError:
                        machine.setup_algorithm(vec_iovs=iov_to_execute)
                elif machine.results[-1].result == AlgResult.failure.value:
                    machine.fail()
                elif machine.results[-1].result == AlgResult.not_enough_data.value:
                    machine.not_enough_data()
                    try:  # Try to include next IoV in vector and recalculate
                        machine.merge_next_iov()
                    except ConditionError:
                        try:
                            merged_iov = ROOT.vector("std::pair<int,int>")()
                            machine.merge_previous_iov(merged_iov=merged_iov, final_iov=iov_to_execute)
                            list_payloads = machine.algorithm.algorithm.getPayloads()
                            list_payloads.pop_back()
                            machine.execute_iov(vec_iov=merged_iov)
                        except ConditionError:
                            machine.fail()  # If there's no next or previous IoVs then there wasn't enough data in the full set
                            return

            # Commit all the payloads and send out the results
            machine.algorithm.algorithm.commit()
            child_conn.send(machine.results)
        # If we were given a specific IoV to calibrate we just execute all runs in that IoV at once
        else:
            iov_to_execute = runs_overlapping_iov(iov_to_calibrate, machine.all_iov_collected)
            machine.execute_iov(vec_iov=iov_to_execute)
            readable_iov_to_execute = iov_from_vector(iov_to_execute)
            if machine.results[-1].result == AlgResult.ok.value:
                machine.success(successful_iov=iov_to_execute)
                machine.complete(single_iov=True)
            elif machine.results[-1].result == AlgResult.iterate.value:
                machine.iteration_requested(successful_iov=iov_to_execute)
                machine.complete(single_iov=True)
            elif machine.results[-1].result == AlgResult.failure.value:
                machine.fail()
            elif machine.results[-1].result == AlgResult.not_enough_data.value:
                machine.not_enough_data()
                machine.fail()  # Since we're only doing one overall IoV and there wasn't enough data, we fail
                return

            # Commit all the payloads and send out the results
            machine.algorithm.algorithm.commit()
            child_conn.send(machine.results)
