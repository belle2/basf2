# -*- coding: utf-8 -*-

"""CDC Wire Efficiency and BadWire creator. Creates layer-by-layer and wire-by-wire efficiencies and returns bad wire list"""

from prompt import CalibrationSettings
from caf.framework import Calibration


#: Tells the automated system some details of this script
settings = CalibrationSettings(name="CDC bad wires",
                               expert_username="uchida",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hlt_mumu"],
                               depends_on=[])

################################################
# Required function called by b2caf-prompt-run #
################################################


def get_calibrations(input_data, **kwargs):
    import basf2
    from prompt.utils import filter_by_max_files_per_run
    # Gets the input files and IoV objects associated with the files.
    file_to_iov_mumu = input_data["hlt_mumu"]

    max_files_per_run = 10
    min_events_per_file = 1000

    reduced_file_to_iov_mumu = filter_by_max_files_per_run(file_to_iov_mumu, max_files_per_run, min_events_per_file)
    input_files_mumu = list(reduced_file_to_iov_mumu.keys())
    basf2.B2INFO(f"Total number of hlt_mumu files actually used as input = {len(input_files_mumu)}")

    input_file_dict = {"hlt_mumu": reduced_file_to_iov_mumu}

    # Get the overall IoV we want to cover, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actuall IoV we want for any prompt request is open-ended
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # wire efficiency
    cal = CDCCalibration(name='wire_eff',
                         algorithms=[wire_algo()],
                         input_file_dict=input_file_dict,
                         max_iterations=1
                         )

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in cal.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal]


#################################################

def pre_collector(max_events=None):
    """
    Define pre collection (reconstruction).
    Needs to perform reconstruction with all wire (including bad) enabled.
    Parameters:
        max_events [int] : number of events to be processed.
                           All events by Default.
    Returns:
        path : path for pre collection
    """
    from basf2 import create_path, register_module
    reco_path = create_path()
    if max_events is None:
        root_input = register_module('RootInput')
    else:
        root_input = register_module('RootInput',
                                     entrySequences=['0:{}'.format(max_events)]
                                     )
    reco_path.add_module(root_input)

    gearbox = register_module('Gearbox')
    reco_path.add_module(gearbox)
    reco_path.add_module('Geometry', useDB=True)

    from rawdata import add_unpackers
    # unpack raw data
    add_unpackers(reco_path)

    from reconstruction import add_reconstruction
    add_reconstruction(reco_path,
                       add_trigger_calculation=False,
                       trackFitHypotheses=[13],  # muon hypothesis but should give the same result with pion
                       pruneTracks=False)

    for module in reco_path.modules():
        if module.name() == "TFCDC_WireHitPreparer":
            print("Enabling bad wires during reconstruction.")
            module.param({"useBadWires": True})

    return reco_path


def collector(bField=True, is_cosmic=False):
    """
    Create a cdc calibration collector
    Parameters:
        bField [bool] : True if B field is on, else False
        isCosmic [bool] : True if cosmic events,
                          else (collision) False.
    Returns:
        collector : collector module
    """
    from basf2 import register_module
    col = register_module('CDCCalibrationCollector',
                          granularity='all',
                          calExpectedDriftTime=True,
                          eventT0Extraction=True,
                          bField=bField,
                          isCosmic=is_cosmic,
                          effStudy=True
                          )
    return col


def wire_algo():
    """
    Create wire efficiency plots.
    Returns:
        algo : Wire efficiency algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.WireEfficiencyAlgorithm()
    return algo


class CDCCalibration(Calibration):
    '''
    CDCCalibration is a specialized calibration class for cdc.
    Since collector is same in all elements, no need to specify it.
    '''

    def __init__(self,
                 name,
                 algorithms,
                 input_file_dict,
                 max_iterations=1,
                 dependencies=None,
                 max_events=10000000):
        for algo in algorithms:
            algo.setHistFileName(name)

        super().__init__(name=name,
                         algorithms=algorithms
                         )

        from caf.framework import Collection

        for skim_type, file_list in input_file_dict.items():
            collection = Collection(collector=collector(),
                                    input_files=file_list,
                                    pre_collector_path=pre_collector(max_events=max_events),
                                    )
            self.add_collection(name=skim_type, collection=collection)

        self.max_iterations = max_iterations

        if dependencies is not None:
            for dep in dependencies:
                self.depends_on(dep)
