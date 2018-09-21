#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import json
# martin's mail utils
import utils
import getpass


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
    return mail_log


def parse_mail_address(string):
    """
    take a string and return list of email addresses that appear in it
    """
    return re.findall(r'[\w\.-]+@[\w\.-]+', string)


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

    # only for testing
    pw = getpass.getpass("passwort: ")
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
        utils.send_mail(contact.split('@')[0], "david.koch@physik.uni-muenchen.de", "Validation failure", body, pw, mood=mood)

    # send a happy mail to folks whose failed plots work now
    if mail_data_old:
        for contact in mail_data_old:
            if contact not in mail_data:
                body = "Your validation plots work fine now!"
                utils.send_mail(
                    contact.split('@')[0],
                    "david.koch@physik.uni-muenchen.de",
                    "Validation confirmation",
                    body,
                    pw,
                    mood="happy")

    del pw


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
