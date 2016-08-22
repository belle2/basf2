from ROOT import Belle2


def upload_cut_to_db(software_trigger_cut, base_identifier, cut_identifier, iov=None):
    """
    Python function to upload the given software trigger cut to the database.
    Additional to the software trigger cut, the base- as well as the cut identifier
    have to be given. Optionally, the interval of validity can be defined
    (default is always valid).
    """
    if not iov:
        iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    dbHandler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler()
    dbHandler.upload(software_trigger_cut, base_identifier, cut_identifier, iov)


def download_cut_from_db(base_name, cut_name, not_set_event_number=False):
    """
    Python function to download a cut from the database. As each cut is uniquely identified by a
    base and a cut name, you have to give in both here.
    Please remember that the database access depends on the current event number. If you do not call
    this function in a basf2 module environment, you can use the set_event_number function in this
    python file to set the event number correctly nevertheless.
    :param base_name: the base name of the cut
    :param base_name: the specific name of the cut
    :param not_set_event_number: it is important to always have a proper event number set for the database to work.
        This is why this functions sets the event number in all cases to (1, 0, 0). If you want to prevent this
        (because you maybe want to use another event number), set this flag to True.
    :return: the downloaded cut or throw an error, if the name can not be found.
    """
    if not not_set_event_number:
        set_event_number(1, 0, 0)

    dbHandler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler()
    return dbHandler.download(base_name, cut_name)


def set_event_number(evt_number, run_number, exp_number):
    """
    Helper function to set the event number although we are not in a typical
    "module-path" setup. This is done by initializing the database,
    creating an EventMetaData and string the requested numbers in it.
    """
    event_meta_data_pointer = Belle2.PyStoreObj(Belle2.EventMetaData.Class(), "EventMetaData")

    Belle2.DataStore.Instance().setInitializeActive(True)

    event_meta_data_pointer.registerInDataStore()
    event_meta_data_pointer.create(False)
    event_meta_data_pointer.setEvent(evt_number)
    event_meta_data_pointer.setRun(run_number)
    event_meta_data_pointer.setExperiment(exp_number)

    Belle2.DataStore.Instance().setInitializeActive(False)


if __name__ == '__main__':
    # Create an example cut.
    cut = Belle2.SoftwareTrigger.SoftwareTriggerCut.compile("[[highest_1_ecl > 0.1873] or [max_pt > 0.4047]]", 1)
    print(cut.decompile())

    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    upload_cut_to_db(cut, "fast_reco", "test", iov)

    set_event_number(0, 0, 0)

    downloaded_cut = download_cut_from_db("fast_reco", "test")
    if downloaded_cut:
        print(downloaded_cut.decompile())
