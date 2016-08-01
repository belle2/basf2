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


def download_cut_from_db(base_name, cut_name):
    """
    Python function to download a cut from the database. As each cut is uniquely identified by a
    base and a cut name, you have to give in both here.
    Please remember that the database access depends on the current event number. If you do not call
    this function in a basf2 module environment, you can use the set_event_number function in this
    python file to set the event number correctly nevertheless.
    :return: the downloaded cut or throw an error, if the name can not be found.
    """
    dbHandler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler()
    return dbHandler.download(base_name, cut_name)


def set_event_number(evt_number, run_number, exp_number):
    evtPtr = Belle2.PyStoreObj(Belle2.EventMetaData.Class(), "EventMetaData")

    Belle2.DataStore.Instance().setInitializeActive(True)

    evtPtr.registerInDataStore()
    evtPtr.create(False)
    evtPtr.setEvent(evt_number)
    evtPtr.setRun(run_number)
    evtPtr.setExperiment(exp_number)

    Belle2.DataStore.Instance().setInitializeActive(False)


if __name__ == '__main__':
    cut = Belle2.SoftwareTrigger.SoftwareTriggerCut.compile("[[highest_1_ecl > 0.1873] or [max_pt > 0.4047]]", 1)
    print(cut.decompile())

    iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    upload_cut_to_db(cut, "fast_reco", "test", iov)

    set_event_number(0, 0, 0)

    downloaded_cut = download_cut_from_db("fast_reco", "test")
    if downloaded_cut:
        print(downloaded_cut.decompile())
