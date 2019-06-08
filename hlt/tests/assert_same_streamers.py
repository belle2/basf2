import ROOT
from ROOT import Belle2


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
    'Belle2::SoftwareTriggerResult': (5, 3176463974),
    'Belle2::TRGSummary': (5, 2529704859),
}

# Map the name of the DataStore objects to their corresponding C++ object names
NAME_TO_CPP_REPLACEMENTS = {
    "SoftwareTriggerVariables": "SoftwareTrigger::SoftwareTriggerVariables",
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


def get_streamer_checksums(objects):
    """
    Extract the streamer checksum and version of the C++ objects in the given list
    by writing them all to a TMemFile and getting back the streamer info list
    automatically created by ROOT afterwards.
    Please note, that this list also includes the streamer infos of all
    base objects of the objects you gave.
    """
    # Write out the objects to a mem file
    f = ROOT.TMemFile("test_mem_file", "RECREATE")
    f.cd()

    for o in objects:
        o.Write()
    f.Write()

    # Go through all streamer infos and extract checksum and version
    streamer_checksums = dict()
    for streamer_info in f.GetStreamerInfoList():
        if not isinstance(streamer_info, ROOT.TStreamerInfo):
            continue
        streamer_checksums[streamer_info.GetName()] = (streamer_info.GetClassVersion(), streamer_info.GetCheckSum())

    f.Close()
    return streamer_checksums


def get_object(object_name, root=Belle2):
    """
    (Possibly) recursively get the object with the given name from the Belle2 namespace.

    If the object name includes a ".", the first part will be turned into an object (probably a module)
    and the function is continued with this object as the root and the rest of the name.

    If not, the object is extracted via a getattr call.
    """
    if "." in object_name:
        namespace, object_name = object_name.split(".", 1)

        return get_object(object_name, get_object(namespace))

    return getattr(root, object_name)


if __name__ == "__main__":
    from softwaretrigger.constants import ALWAYS_SAVE_OBJECTS, RAWDATA_OBJECTS

    # Lets use everything which will be stored to raw data
    objects_names = ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS

    # We need to fix some names, as they do not correspond to the ROOT object classes
    objects_names = [NAME_TO_CPP_REPLACEMENTS.get(name, name) for name in objects_names]

    # Now get the actual objects corresponding to the names
    objects = [get_object(object_name)() for object_name in objects_names]

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
        raise RuntimeError
