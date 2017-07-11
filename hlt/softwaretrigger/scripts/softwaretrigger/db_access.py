from ROOT import Belle2
from softwaretrigger import SOFTWARE_TRIGGER_GLOBAL_TAG_NAME


def upload_cut_to_db(software_trigger_cut, base_identifier, cut_identifier, iov=None):
    """
    Python function to upload the given software trigger cut to the database.
    Additional to the software trigger cut, the base- as well as the cut identifier
    have to be given. Optionally, the interval of validity can be defined
    (default is always valid).
    """
    if not iov:
        iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    db_handler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler()
    db_handler.upload(software_trigger_cut, base_identifier, cut_identifier, iov)


def download_cut_from_db(base_name, cut_name, not_set_event_number=False):
    """
    Python function to download a cut from the database. As each cut is uniquely identified by a
    base and a cut name, you have to give in both here.
    Please remember that the database access depends on the current event number. If you do not call
    this function in a basf2 module environment, you can use the set_event_number function in this
    python file to set the event number correctly nevertheless.
    :param base_name: the base name of the cut
    :param cut_name: the specific name of the cut
    :param not_set_event_number: it is important to always have a proper event number set for the database to work.
        This is why this functions sets the event number in all cases to (1, 0, 0). If you want to prevent this
        (because you maybe want to use another event number), set this flag to True.
    :return: the downloaded cut or throw an error, if the name can not be found.
    """
    if not not_set_event_number:
        set_event_number(1, 0, 0)

    db_handler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler()
    return db_handler.download(base_name, cut_name)


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


def get_all_cuts_in_database(base_identifier=None, software_trigger_global_tag_name=SOFTWARE_TRIGGER_GLOBAL_TAG_NAME):
    """
    Helper function to download a last of all cuts from the condition database in the given tag (and for
    the given base identifier, if not None).
    :param base_identifier: The base identifier for which the cut list should be downloaded. Leave empty to get a list
        of all cuts for all base identifiers.
    :param software_trigger_global_tag_name: The global tag in the database to look at.
    :return: A list of tuples (base_identifier, cut_identifier) for each cut in the database.
    """
    from conditions_db import ConditionsDB
    from ROOT import Belle2
    identifier = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler.s_dbPackageIdentifier

    db = ConditionsDB()
    payloads = db.get_payloads(software_trigger_global_tag_name)
    cuts_in_database = [tuple(cut_name.split("&")[1:]) for cut_name, checksum in payloads
                        if "&" in cut_name and len(cut_name.split("&")) == 3 and cut_name.startswith(identifier)]

    if base_identifier:
        cuts_in_database = list(filter(lambda x: x[0] == base_identifier, cuts_in_database))

    return sorted(cuts_in_database)


if __name__ == '__main__':
    from softwaretrigger.path_functions import setup_softwaretrigger_database_access
    import basf2

    # Create an interval of validity
    validity_interval = Belle2.IntervalOfValidity(0, 0, -1, -1)

    # Set a valid event number for the following calculations
    set_event_number(1, 0, 0)

    # Setup the correct database chain
    setup_softwaretrigger_database_access()

    cuts = []

    basf2.B2RESULT("Currently, there are the following cuts in the global condition database:")
    for base_identifier, cut_identifier in get_all_cuts_in_database():
        basf2.B2RESULT(base_identifier + " " + cut_identifier)
        cut = download_cut_from_db(base_identifier, cut_identifier, True)
        basf2.B2RESULT("Cut condition: " + cut.decompile())
        basf2.B2RESULT("Cut is a reject cut: " + str(cut.isRejectCut()))
        cuts.append({"cut": cut, "base_identifier": base_identifier, "cut_identifier": cut_identifier})

    basf2.reset_database()
    for cut_params in cuts:
        upload_cut_to_db(cut_params["cut"], cut_params["base_identifier"], cut_params["cut_identifier"], validity_interval)
    exit(0)

    basf2.B2RESULT("We will now create an example cut and upload it to the *local* database.")
    # Create an example cut.
    cut = Belle2.SoftwareTrigger.SoftwareTriggerCut.compile("[[highest_1_ecl > 0.1873] or [max_pt > 0.4047]]", 1)
    basf2.B2RESULT(cut.decompile())

    # Upload the cut to the local database
    upload_cut_to_db(cut, "fast_reco", "test", validity_interval)

    # Download the cut from the local database - set the even number for this.

    downloaded_cut = download_cut_from_db("fast_reco", "test")
    if downloaded_cut:
        basf2.B2RESULT(downloaded_cut.decompile())
