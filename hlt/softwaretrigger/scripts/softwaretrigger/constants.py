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
    #: DQM modules which should run before the HLT filter
    before_filter = "before_filter"
    #: DQM modules which should only run on the filtered ones
    filtered = "filtered"
    #: If not in HLT, just all all DQM modules
    dont_care = "dont_care"


#: Always store those objects
ALWAYS_SAVE_OBJECTS = ["EventMetaData", "RawFTSWs", "ROIpayload", "SoftwareTriggerResult", "SoftwareTriggerVariables",
                       "TRGSummary"]

#: Objects to be left on output
RAWDATA_OBJECTS = [
    "RawCDCs",
    "RawSVDs",
    "RawPXDs",
    "RawTOPs",
    "RawARICHs",
    "RawKLMs",
    "RawECLs",
    "RawTRGs",
    "ROIs",
    "OnlineEventT0"]

#: Objects which will be kept after the ExpressReconstruction, for example for the Event Display
PROCESSED_OBJECTS = ['Tracks', 'TrackFitResults',
                     'SVDClusters', 'PXDClusters',
                     'CDCHits', 'TOPDigits',
                     'ARICHHits', 'ECLClusters',
                     'BKLMHit1ds', 'BKLMHit2ds',
                     'EKLMHit2ds', 'SoftwareTriggerResult']

#: List of DataStore names that are present when data enters the HLT.
HLT_INPUT_OBJECTS = RAWDATA_OBJECTS + ["EventMetaData", "RawFTSWs"]
HLT_INPUT_OBJECTS.remove("ROIs")

#: List of DataStore names that are present when data enters the expressreco
EXPRESSRECO_INPUT_OBJECTS = RAWDATA_OBJECTS + ALWAYS_SAVE_OBJECTS

# Detectors to be included in hlt
DEFAULT_HLT_COMPONENTS = ["CDC", "SVD", "ECL", "TOP", "ARICH", "KLM", "TRG"]

# Detectors to be included in expressreco
DEFAULT_EXPRESSRECO_COMPONENTS = DEFAULT_HLT_COMPONENTS + ["PXD"]

#: Location of the database in the online system
DEFAULT_DB_FILE_LOCATION = "/cvmfs/basf2.daqnet.kek.jp/conditions"
