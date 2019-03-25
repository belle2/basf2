from enum import Enum


class RunTypes(Enum):
    """Enum of possible run types"""
    beam = "beam"
    cosmic = "cosmic"


class SoftwareTriggerModes(Enum):
    """Enum of possible software trigger modes"""
    monitor = "monitor"
    filter = "filter"


class Location(Enum):
    """Enum of possible run locations"""
    hlt = "hlt"
    expressreco = "expressreco"


# Always store those objects
ALWAYS_SAVE_OBJECTS = ["EventMetaData", "SoftwareTriggerResult", "TRGSummary", "ROIpayload", "SoftwareTriggerVariables"]
# Objects to be left on output
RAWDATA_OBJECTS = ["RawCDCs", "RawSVDs", "RawPXDs", "RawTOPs", "RawARICHs", "RawKLMs", "RawECLs", "RawFTSWs", "RawTRGs",
                   "ROIs"]
# Objects which will be kept after the ExpressReconstruction, for example for the Event Display
PROCESSED_OBJECTS = ['Tracks', 'TrackFitResults',
                     'SVDClusters', 'PXDClusters',
                     'CDCHits', 'TOPDigits', 'ARICHHits',
                     'ECLClusters',
                     'BKLMHit1ds', 'BKLMHit2ds',
                     'EKLMHit1ds', 'EKLMHit2ds',
                     'SoftwareTriggerResult']

# list of DataStore names that are present when data enters the HLT.
HLT_INPUT_OBJECTS = RAWDATA_OBJECTS + ["EventMetaData"]
HLT_INPUT_OBJECTS.remove("ROIs")

# list of DataStore names that are present when data enters the expressreco
EXPRESSRECO_INPUT_OBJECTS = RAWDATA_OBJECTS + ALWAYS_SAVE_OBJECTS

# Detectors to be included in hlt
DEFAULT_HLT_COMPONENTS = ["CDC", "SVD", "ECL", "TOP", "ARICH", "BKLM", "EKLM", "TRG"]
# Detectors to be included in expressreco
DEFAULT_EXPRESSRECO_COMPONENTS = DEFAULT_HLT_COMPONENTS + ["PXD"]

# Location of the database in the online system
DEFAULT_DB_FILE_LOCATION = "/dev/shm/localdb/database.txt"
