# -*- coding: utf-8 -*-

"""CDC tracking calibration. Performs the T0 determination using HLT skimmed raw data."""

from prompt import CalibrationSettings

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="CDC Tracking",
                               expert_username="uchida",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hlt_mumu", "hlt_hadron"],
                               depends_on=[])


################################################
# Required function called by b2caf-prompt-run #
################################################

def get_calibrations(input_data, **kwargs):
    import basf2
    # Gets the input files and IoV objects associated with the files.
    file_to_iov_mumu = input_data["hlt_mumu"]
    file_to_iov_hadron = input_data["hlt_hadron"]
    # print(file_to_iov_mumu)
    # print(file_to_iov_hadron)

    # We might have requested an enormous amount of data across a requested range.
    # There's a LOT more files than runs!
    # Lets set some limits because this calibration doesn't need that much to run.
    max_files_per_run = 100

    # If you are using Raw data there's a chance that input files could have zero events.
    # This causes a B2FATAL in basf2 RootInput so the collector job will fail.
    # Currently we don't have a good way of filtering this on the automated side, so we can check here.
    min_events_per_file = 100

    # We filter out any more than 100 files per run. The input data files are sorted alphabetically by b2caf-prompt-run
    # already. This procedure respects that ordering
    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_mumu = filter_by_max_files_per_run(file_to_iov_mumu, max_files_per_run, min_events_per_file)
    input_files_mumu = list(reduced_file_to_iov_mumu.keys())
    basf2.B2INFO(f"Total number of hlt_mumu files actually used as input = {len(input_files_mumu)}")

    reduced_file_to_iov_hadron = filter_by_max_files_per_run(file_to_iov_hadron, max_files_per_run, min_events_per_file)
    input_files_hadron = list(reduced_file_to_iov_hadron.keys())
    basf2.B2INFO(f"Total number of hlt_hadron files actually used as input = {len(input_files_hadron)}")

    input_file_dict = {"hlt_mumu": input_files_mumu, "hlt_hadron": input_files_hadron}

    # Get the overall IoV we want to cover, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actuall IoV we want for any prompt request is open-ended
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # t0
    cal0 = CDCCalibration(name='tz0',
                          algorithms=[tz_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=4,
                          )

    # tw
    cal1 = CDCCalibration(name='tw0',
                          algorithms=[tw_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=1,
                          dependencies=[cal0]
                          )

    cal2 = CDCCalibration(name='tz1',
                          algorithms=[tz_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=4,
                          dependencies=[cal1]
                          )

    # xt
    cal3 = CDCCalibration(name='xt0',
                          algorithms=[xt_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=1,
                          dependencies=[cal2]
                          )

    # space resolution
    cal4 = CDCCalibration(name='sr0',
                          algorithms=[sr_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=1,
                          dependencies=[cal3]
                          )
    # t0
    cal5 = CDCCalibration(name='tz2',
                          algorithms=[tz_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=4,
                          dependencies=[cal4]
                          )

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in cal0.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal1.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal2.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal3.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal4.algorithms:
        algorithm.params = {"apply_iov": output_iov}
    for algorithm in cal5.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal0, cal1, cal2, cal3, cal4, cal5]


#################################################

def pre_collector(max_events=None):
    """
    Define pre collection (reconstruction in our purpose).
    Probably, we need only CDC and ECL data.
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
                       trackFitHypotheses=[211, 13],
                       pruneTracks=False)

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
                          isCosmic=is_cosmic
                          )
    return col


def tz_algo():
    """
    Create a T0 calibration algorithm.
    Returns:
        algo : T0 algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.T0CalibrationAlgorithm()
    algo.storeHisto(True)
    algo.setMaxMeanDt(0.5)
    algo.setMaxRMSDt(0.1)
    algo.setMinimumNDF(20)
    return algo


def tw_algo():
    """
    Create a time walk calibration algorithm.
    Returns:
        algo : TW algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.TimeWalkCalibrationAlgorithm()
    algo.setStoreHisto(True)
    algo.setMode(1)
    return algo


def xt_algo():
    """
    Create a XT calibration algorithm.
    Parameters:
        prefix : prefixed name for algorithm,
                 which should be consistent with one of collector..
    Returns:
        algo : XT algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.XTCalibrationAlgorithm()
    algo.setStoreHisto(True)
    algo.setLRSeparate(True)
    algo.setThreshold(0.55)
    return algo


def sr_algo():
    """
    Create a Spacial resolution calibration algorithm.
    Parameters:
        prefix : prefixed name for algorithm,
                 which should be consistent with one of collector..
    Returns:
        algo : Spacial algorithm
    """
    from ROOT import Belle2
    algo = Belle2.CDC.SpaceResolutionCalibrationAlgorithm()
    algo.setStoreHisto(True)
    algo.setThreshold(0.4)
    return algo


from caf.framework import Calibration


class CDCCalibration(Calibration):
    '''
    CDCCalibration is a specialized calibration class for cdc.
    Since collector is same in all elements, no need to specify it.
    '''

    def __init__(self,
                 name,
                 algorithms,
                 input_file_dict,
                 max_iterations=5,
                 dependencies=None,
                 max_events=10000):
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
