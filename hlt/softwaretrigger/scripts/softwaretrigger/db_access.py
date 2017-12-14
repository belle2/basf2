from ROOT import Belle2
from softwaretrigger import SOFTWARE_TRIGGER_GLOBAL_TAG_NAME


def list_to_vector(l):
    """
    Helper function to convert a python list into a std::vector of the same type.
    Is not a very general and good method, but works for the different use cases in
    the STM.
    :param l: The list to convert
    :return: A std::vector with the same content as the input list.
    """
    from ROOT import std
    type_of_first_element = type(l[0]).__name__
    if type_of_first_element == "str":
        type_of_first_element = "string"

    vec = std.vector(type_of_first_element)()

    for x in l:
        vec.push_back(x)

    return vec


def upload_cut_to_db(cut_string, base_identifier, cut_identifier, prescale_factor=1, reject_cut=False, iov=None):
    """
    Python function to upload the given software trigger cut to the database.
    Additional to the software trigger cut, the base- as well as the cut identifier
    have to be given. Optionally, the interval of validity can be defined
    (default is always valid).
    """
    if not iov:
        iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    if isinstance(prescale_factor, list):
        prescale_factor = list_to_vector(prescale_factor)

    db_handler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler
    software_trigger_cut = Belle2.SoftwareTrigger.SoftwareTriggerCut.compile(
        cut_string, prescale_factor, reject_cut)

    db_handler.upload(software_trigger_cut, base_identifier, cut_identifier, iov)


def upload_trigger_menu_to_db(base_identifier, cut_identifiers, accept_mode=False, iov=None):
    """
    Python function to upload the given software trigger enu to the database.
    Additional to the software trigger menu, the base identifier
    has to be given. Optionally, the interval of validity can be defined
    (default is always valid).
    """
    if not iov:
        iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    cut_identifiers = list_to_vector(cut_identifiers)

    db_handler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler
    db_handler.uploadTriggerMenu(base_identifier, cut_identifiers, accept_mode, iov)


def download_cut_from_db(base_name, cut_name, do_set_event_number=True):
    """
    Python function to download a cut from the database. As each cut is uniquely identified by a
    base and a cut name, you have to give in both here.
    Please remember that the database access depends on the current event number. If you do not call
    this function in a basf2 module environment, you can use the set_event_number function in this
    python file to set the event number correctly nevertheless.
    :param base_name: the base name of the cut
    :param cut_name: the specific name of the cut
    :param do_set_event_number: it is important to always have a proper event number set for the database to work.
        This is why this functions sets the event number in all cases to (1, 0, 0). If you want to prevent this
        (because you maybe want to use another event number), set this flag to False.
    :return: the downloaded cut or None, if the name can not be found.
    """
    if do_set_event_number:
        set_event_number(1, 0, 0)

    db_handler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler
    result = db_handler.download(base_name, cut_name)
    if not result:
        result = None
    return result


def download_trigger_menu_from_db(base_name, do_set_event_number=True):
    """
    Python function to download a trigger menu from the database. As each trigger menu is uniquely identified by a
    base name, you have to give here.
    Please remember that the database access depends on the current event number. If you do not call
    this function in a basf2 module environment, you can use the set_event_number function in this
    python file to set the event number correctly nevertheless.
    :param base_name: the base name of the menu
    :param do_set_event_number: it is important to always have a proper event number set for the database to work.
        This is why this functions sets the event number in all cases to (1, 0, 0). If you want to prevent this
        (because you maybe want to use another event number), set this flag to False.
    :return: the downloaded cut or None, if the name can not be found.
    """
    if do_set_event_number:
        set_event_number(1, 0, 0)

    db_handler = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler
    result = db_handler.downloadTriggerMenu(base_name)
    if not result:
        result = None
    return result


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


def _get_all_objects_in_database(base_identifier,
                                 number_of_splits,
                                 software_trigger_global_tag_name=SOFTWARE_TRIGGER_GLOBAL_TAG_NAME):
    """
    Helper function to download a last of all items from the condition database in the given tag (and for
    the given base identifier, if not None).
    :param base_identifier: The base identifier for which the cut list should be downloaded. Leave empty to get a list
        of all cuts for all base identifiers.
    :param software_trigger_global_tag_name: The global tag in the database to look at.
    :return: A list of tuples (base_identifier, cut_identifier) for each item in the database.
    """
    from conditions_db import ConditionsDB
    from ROOT import Belle2
    identifier = Belle2.SoftwareTrigger.SoftwareTriggerDBHandler.s_dbPackageIdentifier

    db = ConditionsDB()
    payloads = db.get_payloads(software_trigger_global_tag_name)
    cuts_in_database = [tuple(cut_name.split("&")[1:]) for cut_name, checksum in payloads
                        if "&" in cut_name and len(cut_name.split("&")) == number_of_splits and cut_name.startswith(identifier)]

    if base_identifier:
        cuts_in_database = list(filter(lambda x: x[0] == base_identifier, cuts_in_database))

    return sorted(cuts_in_database)


def get_all_cuts_in_database(base_identifier=None,
                             software_trigger_global_tag_name=SOFTWARE_TRIGGER_GLOBAL_TAG_NAME):
    """
    Helper function to download a last of all cuts from the condition database in the given tag (and for
    the given base identifier, if not None).
    :param base_identifier: The base identifier for which the cut list should be downloaded. Leave empty to get a list
        of all cuts for all base identifiers.
    :param software_trigger_global_tag_name: The global tag in the database to look at.
    :return: A list of tuples (base_identifier, cut_identifier) for each cut in the database.
    """
    return _get_all_objects_in_database(base_identifier=base_identifier, number_of_splits=3,
                                        software_trigger_global_tag_name=software_trigger_global_tag_name)


def get_all_menus_in_database(base_identifier=None,
                              software_trigger_global_tag_name=SOFTWARE_TRIGGER_GLOBAL_TAG_NAME):
    """
    Helper function to download a last of all menus from the condition database in the given tag (and for
    the given base identifier, if not None).
    :param base_identifier: The base identifier for which the cut list should be downloaded. Leave empty to get a list
        of all cuts for all base identifiers.
    :param software_trigger_global_tag_name: The global tag in the database to look at.
    :return: A list of tuples (base_identifier,) for each menu in the database.
    """
    return _get_all_objects_in_database(base_identifier=base_identifier, number_of_splits=2,
                                        software_trigger_global_tag_name=software_trigger_global_tag_name)


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
        cuts.append({"cut": cut.decompile(), "base_identifier": base_identifier, "cut_identifier": cut_identifier})

    basf2.reset_database()
    for cut_params in cuts:
        upload_cut_to_db(
            cut_params["cut"],
            cut_params["base_identifier"],
            cut_params["cut_identifier"],
            1,
            False,
            validity_interval)
    exit(0)

    basf2.B2RESULT("We will now create an example cut and upload it to the *local* database.")
    # Create an example cut.
    cut = "[[highest_1_ecl > 0.1873] or [max_pt > 0.4047]]"

    # Upload the cut to the local database
    upload_cut_to_db(cut, "fast_reco", "test", 1, False, validity_interval)

    # Download the cut from the local database - set the even number for this.

    downloaded_cut = download_cut_from_db("fast_reco", "test")
    if downloaded_cut:
        basf2.B2RESULT(downloaded_cut.decompile())
