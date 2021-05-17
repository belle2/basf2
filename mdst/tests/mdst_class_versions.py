#!/usr/bin/env python3

"""
Check that all the classdef versions and class checksums are consistent to prevent accidental mismatch:

If one forgets to increase the ClassDef but the streamer info checksum changes this will trigger a test failure. We can distuingish

1. ClassDef version changed unnecessary
2. forgot to change ClassDef
3. both changed
"""

import sys
from b2test_utils import get_object_with_name, get_streamer_checksums

#: A mapping name -> version, checksum of the expected objects
EXPECTED_CHECKSUMS = {
    'Belle2::Const::DetectorSet': (1, 1222446098),
    'Belle2::ECLCluster': (14, 932603982),
    'Belle2::EventLevelClusteringInfo': (1, 162935545),
    'Belle2::EventLevelTrackingInfo': (2, 1667284927),
    'Belle2::EventMetaData': (4, 2999207747),
    'Belle2::FileMetaData': (10, 3436593892),
    'Belle2::KLMCluster': (2, 2615188022),
    'Belle2::KlId': (2, 230716330),
    'Belle2::PIDLikelihood': (3, 36434623),
    'Belle2::RelationContainer': (1, 1725678837),
    'Belle2::RelationElement': (1, 1883389510),
    'Belle2::RelationsInterface<TObject>': (0, 3862127315),
    'Belle2::SoftwareTriggerResult': (5, 241059817),
    'Belle2::TRGSummary': (7, 1658421299),
    'Belle2::Track': (4, 839781593),
    'Belle2::TrackFitResult': (8, 1247854432),
    'Belle2::V0': (3, 4006259140),
}

#: Object names part of mdst
OBJECT_NAMES = [
    "FileMetaData",
    "EventMetaData",
    "RelationContainer",
    "Track",
    "V0",
    "TrackFitResult",
    "EventLevelTrackingInfo",
    "PIDLikelihood",
    "ECLCluster",
    "EventLevelClusteringInfo",
    "KLMCluster",
    "KlId",
    "TRGSummary",
    "SoftwareTriggerResult",
]

if __name__ == "__main__":
    # Now get
    #: the actual objects corresponding to the names
    objects = [get_object_with_name(object_name)() for object_name in OBJECT_NAMES]

    # Check the checksums of every entry and
    #: keep track of found problems
    problems = []

    #: dictionary of objects to (version, checksum)
    found = get_streamer_checksums(objects)
    # we don't care about TObject ...
    if 'TObject' in found:
        del found['TObject']

    # print list we found to be able to copy pasta
    print("found_checksums = {\n    " + "\n    ".join(f"{k!r}: {v!r}," for k, v in sorted(found.items())) + "\n}")
    for key, (version, checksum) in found.items():
        if key not in EXPECTED_CHECKSUMS:
            problems.append(f"There is no {key} in the checksum dictionary!")
            continue

        #: \var expected_version
        #  version so far
        #: \var expected_checksum
        #  checksum so far
        expected_version, expected_checksum = EXPECTED_CHECKSUMS[key]

        if expected_version != version and expected_checksum == checksum:
            problems.append(f"The version for {key} has changed (expected={expected_version}, found={version}) "
                            f"while the checksum has not. This probably means the ClassDef version was increased unnecessarily. "
                            f"If this is intentional please update the expected version in this test")
        elif expected_version == version and expected_checksum != checksum:
            problems.append(f"The checksum for {key} has changed (expected={expected_checksum}, found={checksum}) "
                            f"while the version has not. This probably means you forgot to increase the ClassDef version "
                            f"after changing the class. Please update the ClassDef version and the expected values in this test")
        elif expected_version != version and expected_checksum != checksum:
            problems.append(f"The version and checksum for {key} have changed, (expected version={expected_version}, "
                            f"checksum={expected_checksum}, found version={version}, checksum={checksum}). "
                            f"Please update the expected values")
        del EXPECTED_CHECKSUMS[key]

    for remaining, (version, checksum) in EXPECTED_CHECKSUMS.items():
        problems.append(f"Additional class expected but not needed: {remaining} (version {version}, checksum {checksum})")

    if not problems:
        print("Check finished")
    else:
        problems = "\n\t * ".join([""] + problems)
        print("Check failed: Most likely a mdst class has been modified and the list of expected checksums needs to be adjusted:",
              problems)
        sys.exit(1)
