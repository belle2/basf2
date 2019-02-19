#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import os
import json

import validationpath
from validationfunctions import available_revisions
# martin's mail utils
import mail_utils


def parse_mail_address(obj):
    """!
    Take a string or list and return list of email addresses that appear in it
    """
    if isinstance(obj, str):
        return re.findall(r'[\w\.-]+@[\w\.-]+', obj)
    elif isinstance(obj, list):
        return [re.search(r'[\w\.-]+@[\w\.-]+', c).group() for c in obj if re.search(r'[\w\.-]+@[\w\.-]+', c) is not None]
    else:
        raise TypeError("must be string or list of strings")


def check_if_same(new, old):
    """!
    Checks if the failed plots in new are the same as in old. new and old are
    only considered to be different if the comparison_result of a same plot
    changed or if new contains _more_ failed plots than old.

    @return: True if something has changed.
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


class Mails:

    """!
    Provides functionality to send mails in case of failed scripts / validation
    plots.

    @var validator: Instance of validation.Validation
    @var mail_data_new: Current mail data
    @var comparison_json: Get JSON object with comparison data (serialization
        of json_objects.Comparison)
    @var mail_data_old: Yesterday's mail data (generated from comparison_json)
    """

    def __init__(self, validation):
        """!
        Initializes an instance of the Mail class from an instance of the
        Validation class. Assumes that a comparison json file exists,
        reads it and parses it to extract information about failed plots.
        This information, together with information about failed scripts,
        gets stored in self.mail_data_new. If there is mail_data.json inside
        the log folder, its contents get stored in self.mail_data_old for
        later comparison.

        @param validation: validation.Validation instance
        """

        self.validator = validation

        # read contents from comparison.json
        work_folder = self.validator.work_folder
        revisions = ['reference'] + available_revisions(work_folder)
        comparison_json_file = \
            validationpath.get_html_plots_tag_comparison_json(
                work_folder,
                revisions
            )
        with open(comparison_json_file) as f:
            self.comparison_json = json.load(f)

        # current mail data
        self.mail_data_new = self.create_mail_log(self.comparison_json)

        # yesterday's mail data
        try:
            with open(os.path.join(self.validator.get_log_folder(),
                                   "mail_data.json")) as f:
                self.mail_data_old = json.load(f)
        except BaseException:
            # todo: shouldn't we at least warn about this?
            self.mail_data_old = None

    def create_mail_log_failed_scripts(self):
        """!
        Looks up all scripts that failed and collects information about them.
        """

        # get failed scripts
        with open(os.path.join(self.validator.get_log_folder(),
                               "list_of_failed_scripts.log")) as f:
            failed_scripts = f.read().splitlines()

        # collect information about failed scripts
        mail_log = {}
        for failed_script in failed_scripts:

            # get_script_by_name works with _ only ...
            failed_script = failed_script.replace(".py", "_py").replace(".C", "_C")
            if self.validator.get_script_by_name(failed_script):
                script = self.validator.get_script_by_name(failed_script)
            else:
                # cant do anything if script is not found
                continue

            script.load_header()

            failed_script = {}
            # give failed_script the same format as error_data in method
            # create_mail_log
            failed_script["package"] = script.package
            try:
                failed_script["rootfile"] = ", ".join(script.header["input"])
            except (KeyError, TypeError) as e:
                # TypeError occurs if script.header is None
                failed_script["rootfile"] = " -- "
            failed_script["comparison_text"] = " -- "
            try:
                failed_script["description"] = script.header["description"]
            except (KeyError, TypeError) as e:
                failed_script["description"] = " -- "
            # this is called comparison_result but it is handled as error
            # type when composing mail
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

    def create_mail_log(self, comparison):
        """!
        Takes the entire comparison json file, finds all the plots where
        comparison failed, finds info about failed scripts and saves them in the
        following format:

        {
             "email@address.test" : {
                 "title1": {
                         "package": ... "description": ....
                     },
                 "title2": {...}
             },
             "mail@...": {...}
        }

        The top level ordering is the email address of the contact to make
        sure every user gets only one mail with everything in it.
        """

        mail_log = {}
        # search for plots where comparison resulted in an error
        for package in comparison["packages"]:
            if package["comparison_error"] > 0:
                for plotfile in package["plotfiles"]:
                    if plotfile["comparison_error"] > 0:
                        for plot in plotfile["plots"]:
                            if plot["comparison_result"] == "error" or \
                                    plot["comparison_result"] == "not_compared":
                                # save all the information that's needed for
                                # an informative email
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
        failed_scripts = self.create_mail_log_failed_scripts()
        for contact in failed_scripts:
            # if this user is not yet represented in mail_log, create new key
            if contact not in mail_log:
                mail_log[contact] = failed_scripts[contact]
            # if user already is in mail_log, add the failed scripts
            else:
                for script in contact:
                    mail_log[contact][script] = failed_scripts[contact][script]

        return mail_log

    def compose_message(self, plots):
        """!
        Takes a dict (like in create_mail_log) and composes a mail body
        """

        # link to validation page
        url = "https://b2-master.belle2.org/validation/static/validation.html"
        # url = "http://localhost:8000/static/validation.html"

        body = "There were problem(s) with the validation of the following plots/scripts:\n\n"
        for plot in plots:
            body += "<b>" + plot + "</b><br>"
            body += "<b>Package:</b> " + plots[plot]["package"] + "<br>"
            body += "<b>Rootfile:</b> " + plots[plot]["rootfile"] + ".root<br>"
            body += "<b>Description:</b> " + plots[plot]["description"] + "<br>"
            body += "<b>Comparison:</b> " + plots[plot]["comparison_text"] + "<br>"
            body += "<b>Error type:</b> "
            # compose descriptive error message
            if plots[plot]["comparison_result"] == "error":
                errormsg = "comparison unequal"
            elif plots[plot]["comparison_result"] == "not_compared":
                errormsg = "not compared"
            else:
                errormsg = plots[plot]["comparison_result"]
            body += errormsg + "<br>"
            body += "<a href=\"" + url + "#" + plots[plot]["package"] + "-" + \
                plots[plot]["rootfile"] + "\">Click me for details</a>\n\n"

        body += "You can take a look on the plots/scripts in more detail at " \
                "the links provided for each failed plot/script. "

        return body

    def send_all_mails(self):
        """
        Send mails to all contacts in self.mail_data_new. If
        self.mail_data_old is given, a mail is only sent if there are new
        failed plots
        """

        for contact in self.mail_data_new:
            # if the errors are the same as yesterday, don't send a new mail
            if self.mail_data_old and contact in self.mail_data_old:
                if check_if_same(self.mail_data_new[contact],
                                 self.mail_data_old[contact]):
                    # don't send mail
                    continue

            # set the mood of the b2bot
            if len(self.mail_data_new[contact]) < 4:
                mood = "meh"
            elif len(self.mail_data_new[contact]) < 7:
                mood = "angry"
            elif len(self.mail_data_new[contact]) < 10:
                mood = "livid"
            else:
                mood = "dead"

            body = self.compose_message(self.mail_data_new[contact])
            mail_utils.send_mail(
                contact.split('@')[0],
                contact,
                "Validation failure",
                body,
                mood=mood
            )

        # send a happy mail to folks whose failed plots work now
        if self.mail_data_old:
            for contact in self.mail_data_old:
                if contact not in self.mail_data_new:
                    body = "Your validation plots work fine now!"
                    mail_utils.send_mail(
                        contact.split('@')[0],
                        contact,
                        "Validation confirmation",
                        body,
                        mood="happy")

    def write_log(self):
        """
        Dump mail json.
        """
        with open(os.path.join(self.validator.get_log_folder(),
                               "mail_data.json"), "w") as f:
            json.dump(self.mail_data_new, f, sort_keys=True, indent=4)
