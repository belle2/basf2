#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import json


def create_mail_log(comparison):
    """
    takes the entire comparison json file, finds all the plots where comparison failed
    and saves them the following format:
    {
         "email@address.test" : [
             {
                 "package": ... "description": ....
             },
             {...}
         ],
         "mail@..." : [...]
    }
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
                            error_data["title"] = plot["title"]
                            error_data["comparison_text"] = plot["comparison_text"]
                            error_data["description"] = plot["description"]
                            # every contact gets an email
                            for contact in parse_mail_address(plot["contact"]):
                                # check if this contact already gets mail
                                if contact not in mail_log:
                                    # create new key for this contact
                                    mail_log[contact] = []
                                mail_log[contact].append(error_data)
    return mail_log


def parse_mail_address(string):
    """
    take a string and return list of email addresses that appear in it
    """
    return re.findall(r'[\w\.-]+@[\w\.-]+', string)


def compose_message(plotlist):
    """
    takes a list of dicts (like in create_mail_log) and composes a mail body
    """

    body = "There were problem(s) with the validation of the following plots:\n\n"
    for plot in plotlist:
        body += "Package: "+plot["package"]+"\n"
        body += "Title: "+plot["title"]+"\n"
        body += "Rootfile: "+plot["rootfile"]+".root\n"
        body += "Description: "+plot["description"]+"\n"
        body += "Comparison: "+plot["comparison_text"]+"\n\n"
    body += "You can take a look on the plots in more detail at <link to validation thing>."

    return body
