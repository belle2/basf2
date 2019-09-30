# -*- coding: utf-8 -*-

"""CDC tracking calibration. Performs the T0 determination using HLT skimmed raw data."""

from prompt import CalibrationSettings

#: Tells the automated system some details of this script
settings = CalibrationSettings(name="CDC Tracking",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hlt_mumu", "hlt_hadron"]
                               )


################################################
# Required function called by b2caf-prompt-run #
################################################

def get_calibrations(input_data, **kwargs):
    # Gets the input files, but not the IoV objects of those files, which are also passed in.
    input_file_dict = {"hlt_mumu": list(input_data["hlt_mumu"].keys()),
                       "hlt_hadron": list(input_data["hlt_hadron"].keys()),
                       }

    # Get the overall IoV we want to cover, including the end values
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actuall IoV we want for any prompt request is open-ended
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # t0
    cal0 = CDCCalibration(name='tz0',
                          algorithms=[tz_algo()],
                          input_file_dict=input_file_dict,
                          max_iterations=1,
                          )

    # Force the output payload IoV to be correct.
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in cal0.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    return [cal0, ]


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
                       # components=components,
                       # components=['CDC'],
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
    # algo.storeHisto(True)
    algo.storeHisto(False)
    # algo.setDebug(True)
    algo.setMaxMeanDt(0.5)
    algo.setMaxRMSDt(0.1)
    algo.setMinimumNDF(20)
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
                 dependencies=None):
        for algo in algorithms:
            algo.setHistFileName(name)

        super().__init__(name=name,
                         algorithms=algorithms
                         )

        from caf.framework import Collection

        max_events = 10
        for skim_type, file_list in input_file_dict.items():
            collection = Collection(collector=collector(),
                                    input_files=file_list,
                                    pre_collector_path=pre_collector(max_events=max_events),
                                    )
            collection.backend_args = {'queue': queue}
            self.add_collection(name=skim_type, collection=collection)

        self.max_iterations = max_iterations

        if dependencies is not None:
            for dep in dependencies:
                self.depends_on(dep)
