from enum import Enum


class RunTypes(Enum):
    """Enum of possible run types"""
    #: beam (thank you documentation check)
    beam = "beam"
    #: cosmics (thank you documentation check)
    cosmic = "cosmic"


class SoftwareTriggerModes(Enum):
    """Enum of possible software trigger modes"""
    #: monitor (thank you documentation check)
    monitor = "monitor"
    #: filter (thank you documentation check)
    filter = "filter"


class Location(Enum):
    """Enum of possible run locations"""
    #: hlt (thank you documentation check)
    hlt = "hlt"
    #: expressreco (thank you documentation check)
    expressreco = "expressreco"


class DQMModes(Enum):
    """Split between DQM modules that need to run on all events and on the filtered ones"""
    #: DQM modules which should run on dismissed and non_dismissed events
    all_events = "all_events"
    #: DQM modules which should run before every reconstruction
    before_reco = "before_reco"
    #: DQM modules which should only run on the filtered ones
    filtered = "filtered"
    #: If not in HLT, just all all DQM modules
    dont_care = "dont_care"


#: Always store those objects
ALWAYS_SAVE_OBJECTS = ["EventMetaData", "SoftwareTriggerResult", "TRGSummary", "ROIpayload", "SoftwareTriggerVariables"]
#: Objects to be left on output
RAWDATA_OBJECTS = ["RawCDCs", "RawSVDs", "RawPXDs", "RawTOPs", "RawARICHs", "RawKLMs", "RawECLs", "RawFTSWs", "RawTRGs",
                   "ROIs"]
#: Objects which will be kept after the ExpressReconstruction, for example for the Event Display
PROCESSED_OBJECTS = ['Tracks', 'TrackFitResults',
                     'SVDClusters', 'PXDClusters',
                     'CDCHits', 'TOPDigits', 'ARICHHits',
                     'ECLClusters',
                     'BKLMHit1ds', 'BKLMHit2ds',
                     'EKLMHit1ds', 'EKLMHit2ds',
                     'SoftwareTriggerResult']

#: list of DataStore names that are present when data enters the HLT.
HLT_INPUT_OBJECTS = RAWDATA_OBJECTS + ["EventMetaData"]
HLT_INPUT_OBJECTS.remove("ROIs")

#: list of DataStore names that are present when data enters the expressreco
EXPRESSRECO_INPUT_OBJECTS = RAWDATA_OBJECTS + ALWAYS_SAVE_OBJECTS

# Detectors to be included in hlt
DEFAULT_HLT_COMPONENTS = ["CDC", "SVD", "ECL", "TOP", "ARICH", "KLM", "TRG"]
# Detectors to be included in expressreco
DEFAULT_EXPRESSRECO_COMPONENTS = DEFAULT_HLT_COMPONENTS + ["PXD"]

#: Location of the database in the online system
DEFAULT_DB_FILE_LOCATION = "/cvmfs/basf2.daqnet.kek.jp/database/database.txt"
