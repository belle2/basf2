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
    from ROOT import Belle2

    if not iov:
        iov = Belle2.IntervalOfValidity(0, 0, -1, -1)

    if isinstance(prescale_factor, list):
        raise AttributeError("The only allowed type for the prescaling is a single factor")

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
    from ROOT import Belle2

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
    from ROOT import Belle2

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
    from ROOT import Belle2

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
    from ROOT import Belle2

    event_meta_data_pointer = Belle2.PyStoreObj(Belle2.EventMetaData.Class(), "EventMetaData")

    Belle2.DataStore.Instance().setInitializeActive(True)

    event_meta_data_pointer.registerInDataStore()
    event_meta_data_pointer.create(False)
    event_meta_data_pointer.setEvent(evt_number)
    event_meta_data_pointer.setRun(run_number)
    event_meta_data_pointer.setExperiment(exp_number)

    Belle2.DataStore.Instance().setInitializeActive(False)


if __name__ == '__main__':
    import argparse
    import basf2
    import pandas as pd

    parser = argparse.ArgumentParser()
    parser.add_argument("--global-tag", help="If given, use this global tag instead")
    parser.add_argument("--experiment-number", help="If given, use this experiment number instead")
    parser.add_argument("--run-number", help="If given, use this run number instead")
    parser.add_argument("--format", help="Choose the format how to print the trigger cuts.",
                        choices=["human-readable", "jira", "grid"], default="human-readable")

    args = parser.parse_args()

    if args.global_tag:
        basf2.reset_database()
        basf2.use_central_database(args.global_tag)

    set_event_number(evt_number=0, run_number=int(args.run_number or 0), exp_number=int(args.experiment_number or 0))

    def get_cuts():
        for base_identifier in ["filter", "skim"]:
            menu = download_trigger_menu_from_db(base_name=base_identifier, do_set_event_number=False)
            cuts = menu.getCutIdentifiers()
            for cut_identifier in cuts:
                cut = download_cut_from_db(base_name=base_identifier, cut_name=cut_identifier, do_set_event_number=False)
                yield {
                    "Base Identifier": base_identifier,
                    "Reject Menu": menu.isAcceptMode(),
                    "Cut Identifier": cut_identifier,
                    "Cut Condition": cut.decompile(),
                    "Cut Prescaling": cut.getPreScaleFactor(),
                    "Reject Cut": cut.isRejectCut()
                }

    df = pd.DataFrame(list(get_cuts()))

    if args.format == "jira":
        from tabulate import tabulate
        print(tabulate(df, tablefmt="jira", showindex=False, headers="keys"))
    if args.format == "grid":
        from tabulate import tabulate
        print(tabulate(df, tablefmt="grid", showindex=False, headers="keys"))
    elif args.format == "human-readable":
        print("Currently, the following menus and triggers are in the database")
        for base_identifier, cuts in df.groupby("Base Identifier"):
            print(base_identifier)
            print("")
            print("\tUsed triggers:\n\t\t" + ", ".join(list(cuts["Cut Identifier"])))
            print("\tIs in accept mode:\n\t\t" + str(cuts["Reject Menu"].iloc[0]))
            for _, cut in cuts.iterrows():
                print("\t\tCut Name:\n\t\t\t" + cut["Cut Identifier"])
                print("\t\tCut condition:\n\t\t\t" + cut["Cut Condition"])
                print("\t\tCut prescaling\n\t\t\t" + str(cut["Cut Prescaling"]))
                print("\t\tCut is a reject cut:\n\t\t\t" + str(cut["Reject Cut"]))
                print()
