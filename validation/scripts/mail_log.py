#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import os
import json

import validation
# martin's mail utils
import utils


def create_mail_log_failed_scripts():
    """
    looks up all scripts that failed and collects information about them
    """

    validator = validation.Validation()
    # get all the steering files
    validator.collect_steering_files(validation.IntervalSelector(['nightly']))

    # get failed scripts
    with open(os.path.join(validator.get_log_folder(), "list_of_failed_scripts.log")) as f:
        list_of_failed_scripts = f.read().splitlines()

    # collect information about failed scripts
    mail_log = {}
    for failed_script in list_of_failed_scripts:

        # get_script_by_name works with _ only ...
        failed_script = failed_script.replace(".py", "_py").replace(".C", "_C")
        if validator.get_script_by_name(failed_script):
            script = validator.get_script_by_name(failed_script)
        else:
            # cant do anything if script is not found
            continue

        script.load_header()

        failed_script = {}
        # give failed_script the same format as error_data in method create_mail_log
        failed_script["package"] = script.package
        try:
            failed_script["rootfile"] = ", ".join(script.header["input"])
        except (KeyError, TypeError) as e:  # TypeError occurs if script.header is None
            failed_script["rootfile"] = " -- "
        failed_script["comparison_text"] = " -- "
        try:
            failed_script["description"] = script.header["description"]
        except (KeyError, TypeError) as e:
            failed_script["description"] = " -- "
        # this is called comparison_result but it is handled as error type when composing mail
        failed_script["comparison_result"] = "script failed to execute"
        # add contact of failed script to mail_log
        try:
            for contact in parse_mail_address(script.header["contact"]):
                if contact not in mail_log:
                    mail_log[contact] = {}
                mail_log[contact][script.name] = failed_script
        except (KeyError, TypeError) as e:
            # this means no contact is given
            continue

    return mail_log


def create_mail_log(comparison):
    """
    takes the entire comparison json file, finds all the plots where comparison failed
    and saves them the following format:
    {
         "email@address.test" : {
             "title1": {
                     "package": ... "description": ....
                 },
             "title2": {...}
         },
         "mail@...": {...}
    }
    The top level ordering is the email address of the contact to make sure every user
    gets only one mail with everything in it.
    """

    mail_log = {}
    # search for plots where comparison resulted in an error
    for package in comparison["packages"]:
        if package["comparison_error"] > 0:
            for plotfile in package["plotfiles"]:
                if plotfile["comparison_error"] > 0:
                    for plot in plotfile["plots"]:
                        if plot["comparison_result"] == "error" or plot["comparison_result"] == "not_compared":
                            # save all the information that's needed for an informative email
                            error_data = {}
                            error_data["package"] = plotfile["package"]
                            error_data["rootfile"] = plotfile["rootfile"]
                            error_data["comparison_text"] = plot["comparison_text"]
                            error_data["description"] = plot["description"]
                            error_data["comparison_result"] = plot["comparison_result"]
                            # every contact gets an email
                            for contact in parse_mail_address(plot["contact"]):
                                # check if this contact already gets mail
                                if contact not in mail_log:
                                    # create new key for this contact
                                    mail_log[contact] = {}
                                mail_log[contact][plot["title"]] = error_data

    # now get failed scripts and merge information into mail_log
    failed_scripts = create_mail_log_failed_scripts()
    for contact in failed_scripts:
        # if this user is not yet represented in mail_log, create new key
        if contact not in mail_log:
            mail_log[contact] = failed_scripts[contact]
        # if user already is in mail_log, add the failed scripts
        else:
            for script in contact:
                mail_log[contact][script] = failed_script[contact][script]

    return mail_log


def parse_mail_address(obj):
    """
    take a string or list and return list of email addresses that appear in it
    """
    if isinstance(obj, str):
        return re.findall(r'[\w\.-]+@[\w\.-]+', obj)
    elif isinstance(obj, list):
        return [re.match(r'[\w\.-]+@[\w\.-]+', c).group() for c in obj if re.match(r'[\w\.-]+@[\w\.-]+', c) is not None]
    else:
        raise TypeError("must be string or list of strings")


def compose_message(plots):
    """
    takes a dict (like in create_mail_log) and composes a mail body
    """

    # link to validation page
    url = "https://b2-master.belle2.org/validation/static/validation.html"
    # url = "http://localhost:8000/static/validation.html"

    body = "There were problem(s) with the validation of the following plots:\n\n"
    for plot in plots:
        body += "<b>" + plot + "</b><br>"
        body += "<b>Package:</b> " + plots[plot]["package"] + "<br>"
        body += "<b>Rootfile:</b> " + plots[plot]["rootfile"] + ".root<br>"
        body += "<b>Description:</b> " + plots[plot]["description"] + "<br>"
        body += "<b>Comparison:</b> " + plots[plot]["comparison_text"] + "<br>"
        body += "<b>Error type:</b> " + \
            ("comparison unequal<br>" if plots[plot]["comparison_result"] == "error" else "comparison failed<br>")
        body += "<a href=\"" + url + "#" + plots[plot]["package"] + "-" + \
            plots[plot]["rootfile"] + "\">Click me for details</a>\n\n"

    body += "You can take a look on the plots in more detail at the links provided for each failed plot."

    return body


def send_all_mails(mail_data, mail_data_old=None):
    """
    send mails to all contacts in mail_data.
    if mail_data_old is given, a mail is only sent if there are new failed plots
    """

    for contact in mail_data:
        # if the errors are the same as yesterday, don't send a new mail
        if mail_data_old and contact in mail_data_old:
            if check_if_same(mail_data[contact], mail_data_old[contact]):
                # don't send mail
                continue

        # set the mood of the b2bot
        if len(mail_data[contact]) < 4:
            mood = "meh"
        elif len(mail_data[contact]) < 7:
            mood = "angry"
        elif len(mail_data[contact]) < 10:
            mood = "livid"
        else:
            mood = "dead"

        body = compose_message(mail_data[contact])
        utils.send_mail(contact.split('@')[0], contact, "Validation failure", body, mood=mood)

    # send a happy mail to folks whose failed plots work now
    if mail_data_old:
        for contact in mail_data_old:
            if contact not in mail_data:
                body = "Your validation plots work fine now!"
                utils.send_mail(
                    contact.split('@')[0],
                    contact,
                    "Validation confirmation",
                    body,
                    mood="happy")


def check_if_same(new, old):
    """
    checks if the failed plots in new are the same as in old
    new and old are only considered to be different if the comparison_result of a same plot
    changed or if new contains _more_ failed plots than old.
    """

    # if new is larger than old, there are new failed plots
    if len(new) > len(old):
        return False

    for plot in new:
        # new plot failed
        if plot not in old:
            return False
        # comparison_result changed
        if new[plot]["comparison_result"] != old[plot]["comparison_result"]:
            return False
    return True


def mail_log(comparison, comparison_old=None):
    """
    takes the comparison json and sends mails to contact of failed comparisons
    comparison_old: yesterday's comparison json
    """

    data = create_mail_log(comparison)
    if comparison_old:
        data_old = create_mail_log(comparison_old)
    else:
        data_old = None
    send_all_mails(data, data_old)
