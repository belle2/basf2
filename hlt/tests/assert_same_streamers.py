import ROOT
from ROOT import Belle2


def get_streamer_checksums(objects):
    f = ROOT.TMemFile("test_mem_file", "RECREATE")
    f.cd()

    for o in objects:
        o.Write()
    f.Write()

    streamer_checksums = dict()
    for streamer_info in f.GetStreamerInfoList():
        if not isinstance(streamer_info, ROOT.TStreamerInfo):
            continue
        streamer_checksums[streamer_info.GetName()] = streamer_info.GetCheckSum()

    f.Close()
    return streamer_checksums


def get_object(object_name, root=Belle2):
    if "." in object_name:
        namespace, object_name = object_name.split(".", 1)

        return get_object(object_name, get_object(namespace))

    try:
        return getattr(root, object_name)
    except AttributeError:
        if object_name.endswith("s"):
            return get_object(object_name[:-1], root=root)
        else:
            raise


EXPECTED_CHECKSUMS = {
    'Belle2::EventMetaData': 2999207747,
    'Belle2::ROIid': 1743241213,
    'Belle2::ROIpayload': 610869861,
    'Belle2::RawARICH': 3497179043,
    'Belle2::RawCDC': 1985095356,
    'Belle2::RawCOPPER': 3824346631,
    'Belle2::RawDataBlock': 2371206983,
    'Belle2::RawECL': 2693252500,
    'Belle2::RawFTSW': 1181247934,
    'Belle2::RawKLM': 101994230,
    'Belle2::RawPXD': 924270514,
    'Belle2::RawSVD': 1772361339,
    'Belle2::RawTOP': 1772361339,
    'Belle2::RawTRG': 1772361339,
    'Belle2::RelationsInterface<TObject>': 3862127315,
    'Belle2::SoftwareTrigger::SoftwareTriggerVariables': 638196437,
    'Belle2::SoftwareTriggerResult': 3176463974,
    'Belle2::TRGSummary': 2529704859
}

NAME_TO_CPP_REPLACEMENTS = {
    "SoftwareTriggerVariables": "SoftwareTrigger::SoftwareTriggerVariables",
    "ROIs": "ROIid",
}


if __name__ == "__main__":
    from softwaretrigger.constants import ALWAYS_SAVE_OBJECTS, RAWDATA_OBJECTS

    objects_names = ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS

    # We need to fix some names, as they do not correspond to the ROOT object classes
    objects_names = [NAME_TO_CPP_REPLACEMENTS.get(name, name) for name in objects_names]

    objects = [get_object(object_name)() for object_name in objects_names]

    for key, checksum in get_streamer_checksums(objects).items():
        if key not in EXPECTED_CHECKSUMS:
            raise KeyError(f"There is no {key} in the checksum dictionary!")

        if EXPECTED_CHECKSUMS[key] != checksum:
            raise ValueError(f"The checksum {checksum} for {key} is not equal to the expected checksum {EXPECTED_CHECKSUMS[key]}."
                             "Either you changed the data objects by accident or you need to adjust the expected checksums.")

    print("Check finished")
