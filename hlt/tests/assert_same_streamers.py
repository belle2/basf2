##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from b2test_utils import get_object_with_name, get_streamer_checksums


# A mapping name -> version, checksum of the expected objects
EXPECTED_CHECKSUMS = {
    'Belle2::EventMetaData': (4, 2999207747),
    'Belle2::ROIid': (1, 1743241213),
    'Belle2::ROIpayload': (2, 610869861),
    'Belle2::RawARICH': (1, 3497179043),
    'Belle2::RawCDC': (1, 1985095356),
    'Belle2::RawCOPPER': (3, 3824346631),
    'Belle2::RawDataBlock': (3, 2371206983),
    'Belle2::RawECL': (1, 2693252500),
    'Belle2::RawFTSW': (2, 1181247934),
    'Belle2::RawKLM': (1, 101994230),
    'Belle2::RawPXD': (2, 924270514),
    'Belle2::RawSVD': (1, 1772361339),
    'Belle2::RawTOP': (1, 1772361339),
    'Belle2::RawTRG': (1, 1772361339),
    'Belle2::RelationsInterface<TObject>': (0, 3862127315),
    'Belle2::SoftwareTrigger::SoftwareTriggerVariables': (1, 638196437),
    'Belle2::SoftwareTriggerResult': (5, 241059817),
    'Belle2::TRGSummary': (7, 1658421299),
    'Belle2::OnlineEventT0': (1, 0x42ae59a2),
    'Belle2::Const::DetectorSet': (1, 0x48dd0c12),
}

# Map the name of the DataStore objects to their corresponding C++ object names
NAME_TO_CPP_REPLACEMENTS = {
    "SoftwareTriggerVariables": "SoftwareTrigger::SoftwareTriggerVariables",
    "OnlineEventT0s": "OnlineEventT0",
    "ROIs": "ROIid",
    "RawARICHs": "RawARICH",
    "RawCDCs": "RawCDC",
    "RawCOPPERs": "RawCOPPER",
    "RawECLs": "RawECL",
    "RawFTSWs": "RawFTSW",
    "RawKLMs": "RawKLM",
    "RawPXDs": "RawPXD",
    "RawSVDs": "RawSVD",
    "RawTOPs": "RawTOP",
    "RawTRGs": "RawTRG",
}


if __name__ == "__main__":
    from softwaretrigger.constants import ALWAYS_SAVE_OBJECTS, RAWDATA_OBJECTS

    # Lets use everything which will be stored to raw data
    objects_names = ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS

    # We need to fix some names, as they do not correspond to the ROOT object classes
    objects_names = [NAME_TO_CPP_REPLACEMENTS.get(name, name) for name in objects_names]

    # Now get the actual objects corresponding to the names
    objects = [get_object_with_name(object_name)() for object_name in objects_names]

    # Check the checksums of every entry
    problems = []

    for key, checksum in get_streamer_checksums(objects).items():
        if key not in EXPECTED_CHECKSUMS:
            problems.append(f"There is no {key} in the checksum dictionary!")

        expected_checksum = EXPECTED_CHECKSUMS[key]

        if expected_checksum != checksum:
            problems.append(f"The checksum {checksum[1]} for {key} (version {checksum[0]}) "
                            f"is not equal to the expected checksum {expected_checksum[1]} (version {expected_checksum[0]}).")

    if not problems:
        print("Check finished")
    else:
        problems = "\n\t" + "\n\t".join(problems)
        print("Check finished with problems. ",
              "Either you changed them by accident or you need to adjust the expected checksum list in this test):",
              problems)
        exit(1)
