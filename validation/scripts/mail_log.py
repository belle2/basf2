#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# std
import copy
from datetime import date
import re
import os
import json
import sys
from typing import Dict, Union, List, Optional

# ours
import validationpath
from validationfunctions import available_revisions

# martin's mail utils
import mail_utils
from validationscript import Script


def parse_mail_address(obj: Union[str, List[str]]) -> List[str]:
    """!
    Take a string or list and return list of email addresses that appear in it
    """
    if isinstance(obj, str):
        return re.findall(r"[\w.-]+@[\w.-]+", obj)
    elif isinstance(obj, list):
        return [
            re.search(r"[\w.-]+@[\w.-]+", c).group()
            for c in obj
            if re.search(r"[\w.-]+@[\w.-]+", c) is not None
        ]
    else:
        raise TypeError("must be string or list of strings")


class Mails:

    """!
    Provides functionality to send mails in case of failed scripts / validation
    plots.
    The mail data is built upon instantiation, the `send_mails` method
    sends the actual mails.
    """

    def __init__(self, validation, include_expert_plots=False):
        """!
        Initializes an instance of the Mail class from an instance of the
        Validation class. Assumes that a comparison json file exists,
        reads it and parses it to extract information about failed plots.
        This information, together with information about failed scripts,
        gets stored in self.mail_data_new. If there is mail_data.json inside
        the log folder, its contents get stored in self.mail_data_old for
        later comparison.

        @param validation: validation.Validation instance
        @param include_expert_plots: Should expert plots be included?
        """

        # Cannot import Validation to type hint because this would give us a
        # circular import.
        #: Instance of validation.Validation
        self._validator = validation

        # read contents from comparison.json
        work_folder = self._validator.work_folder
        revisions = ["reference"] + available_revisions(work_folder)
        comparison_json_file = validationpath.get_html_plots_tag_comparison_json(
            work_folder, revisions
        )
        with open(comparison_json_file) as f:
            comparison_json = json.load(f)

        # yesterday's mail data
        old_mail_data_path = os.path.join(
            self._validator.get_log_folder(), "mail_data.json"
        )
        #: Yesterday's mail data (generated from comparison_json). Check
        #: docstring of _create_mail_log for exact format
        self._mail_data_old: Optional[dict] = None
        try:
            with open(old_mail_data_path) as f:
                self._mail_data_old = json.load(f)
        except FileNotFoundError:
            print(
                f"Could not find old mail_data.json at {old_mail_data_path}.",
                file=sys.stderr,
            )

        #: Current mail data. Will be filled on instantiation. Check
        #: docstring of _create_mail_log for exact format
        self._mail_data_new = self._create_mail_log(
            comparison_json, include_expert_plots=include_expert_plots
        )

    def _create_mail_log_failed_scripts(self) -> Dict[str, Dict[str, str]]:
        """!
        Looks up all scripts that failed and collects information about them.
        See :meth:`_create_mail_log` for the structure of the resulting
        dictionary.
        """

        # get failed scripts
        with open(
            os.path.join(
                self._validator.get_log_folder(), "list_of_failed_scripts.log"
            )
        ) as f:
            failed_scripts = f.read().splitlines()

        # collect information about failed scripts
        mail_log = {}
        for failed_script in failed_scripts:

            # get_script_by_name works with _ only ...
            failed_script = Script.sanitize_file_name(failed_script)
            script = self._validator.get_script_by_name(failed_script)
            if script is None:
                continue

            script.load_header()

            failed_script = {}
            failed_script["warnings"] = []
            # give failed_script the same format as error_data in method
            # create_mail_log
            failed_script["package"] = script.package
            failed_script["rootfile"] = ", ".join(script.input_files)
            failed_script["comparison_text"] = " -- "
            failed_script["description"] = script.description
            # this is called comparison_result but it is handled as error
            # type when composing mail
            failed_script["comparison_result"] = "script failed to execute"
            # add contact of failed script to mail_log
            try:
                for contact in parse_mail_address(script.contact):
                    if contact not in mail_log:
                        mail_log[contact] = {}
                    mail_log[contact][script.name] = failed_script
            except (KeyError, TypeError):
                # this means no contact is given
                continue

        return mail_log

    def _create_mail_log(
        self, comparison, include_expert_plots=False
    ) -> Dict[str, Dict[str, Dict[str, str]]]:
        """!
        Takes the entire comparison json file, finds all the plots where
        comparison failed, finds info about failed scripts and saves them in
        the following format:

        {
             "email@address.test" : {
                 "title1": {
                         "package": str,
                         "description": str,
                         "rootfile": str,
                         "comparison_text": str,
                         "description": str,
                         "comparison_result": str,
                         "warnings": str
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
            for plotfile in package["plotfiles"]:
                for plot in plotfile["plots"]:
                    if not include_expert_plots and plot["is_expert"]:
                        continue
                    skip = True
                    if plot["comparison_result"] in ["error"]:
                        skip = False
                    if set(plot["warnings"]) - {"No reference object"}:
                        skip = False
                    if skip:
                        continue
                    # save all the information that's needed for
                    # an informative email
                    error_data = {
                        "package": plotfile["package"],
                        "rootfile": plotfile["rootfile"],
                        "comparison_text": plot["comparison_text"],
                        "description": plot["description"],
                        "comparison_result": plot["comparison_result"],
                        "warnings": sorted(
                            list(
                                set(plot["warnings"]) - {"No reference object"}
                            )
                        ),
                    }
                    # every contact gets an email
                    for contact in parse_mail_address(plot["contact"]):
                        # check if this contact already gets mail
                        if contact not in mail_log:
                            # create new key for this contact
                            mail_log[contact] = {}
                        mail_log[contact][plot["title"]] = error_data

        # now get failed scripts and merge information into mail_log
        failed_scripts = self._create_mail_log_failed_scripts()
        for contact in failed_scripts:
            # if this user is not yet represented in mail_log, create new key
            if contact not in mail_log:
                mail_log[contact] = failed_scripts[contact]
            # if user already is in mail_log, add the failed scripts
            else:
                for script in failed_scripts[contact]:
                    mail_log[contact][script] = failed_scripts[contact][script]

        return self._flag_new_failures(mail_log, self._mail_data_old)

    @staticmethod
    def _flag_new_failures(
        mail_log: Dict[str, Dict[str, Dict[str, str]]],
        old_mail_log: Optional[Dict[str, Dict[str, Dict[str, str]]]],
    ) -> Dict[str, Dict[str, Dict[str, str]]]:
        """ Add a new field 'compared_to_yesterday' which takes one of the
        values 'unchanged' (same revision comparison result as in yesterday's
        mail log, 'new' (new warning/failure), 'changed' (comparison result
        changed). """
        mail_log_flagged = copy.deepcopy(mail_log)
        for contact in mail_log:
            for plot in mail_log[contact]:
                if old_mail_log is None:
                    mail_log_flagged[contact][plot][
                        "compared_to_yesterday"
                    ] = "n/a"
                elif contact not in old_mail_log:
                    mail_log_flagged[contact][plot][
                        "compared_to_yesterday"
                    ] = "new"
                elif plot not in old_mail_log[contact]:
                    mail_log_flagged[contact][plot][
                        "compared_to_yesterday"
                    ] = "new"
                elif (
                    mail_log[contact][plot]["comparison_result"]
                    != old_mail_log[contact][plot]["comparison_result"]
                    or mail_log[contact][plot]["warnings"]
                    != old_mail_log[contact][plot]["warnings"]
                ):
                    mail_log_flagged[contact][plot][
                        "compared_to_yesterday"
                    ] = "changed"
                else:
                    mail_log_flagged[contact][plot][
                        "compared_to_yesterday"
                    ] = "unchanged"
        return mail_log_flagged

    @staticmethod
    def _check_if_same(plot_errors: Dict[str, Dict[str, str]]) -> bool:
        """
        @param plot_errors: ``_create_mail_log[contact]``.
        @return True, if there is at least one new/changed plot status
        """
        for plot in plot_errors:
            if plot_errors[plot]["compared_to_yesterday"] != "unchanged":
                return False
        return True

    @staticmethod
    def _compose_message(plots, incremental=True):
        """!
        Takes a dict (like in _create_mail_log) and composes a mail body
        @param plots
        @param incremental (bool): Is this an incremental report or a full
            ("Monday") report?
        """

        # link to validation page
        url = "https://b2-master.belle2.org/validation/static/validation.html"
        # url = "http://localhost:8000/static/validation.html"

        if incremental:
            body = (
                "You are receiving this email, because additional"
                " validation plots/scripts (that include you as contact "
                "person) produced warnings/errors or "
                "because their warning/error status "
                "changed. \n"
                "Below is a detailed list of all new/changed offenders:\n\n"
            )
        else:
            body = (
                "This is a full list of validation plots/scripts that"
                " produced warnings/errors and include you as contact"
                "person (sent out once a week).\n\n"
            )

        body += (
            "There were problems with the validation of the "
            "following plots/scripts:\n\n"
        )
        for plot in plots:
            compared_to_yesterday = plots[plot]["compared_to_yesterday"]
            body_plot = ""
            if compared_to_yesterday == "unchanged":
                if incremental:
                    # Do not include.
                    continue
            elif compared_to_yesterday == "new":
                body_plot = '<b style="color: red;">[NEW]</b><br>'
            elif compared_to_yesterday == "changed":
                body_plot = (
                    '<b style="color: red;">'
                    "[Warnings/comparison CHANGED]</b><br>"
                )
            else:
                body_plot = (
                    f'<b style="color: red;">[UNEXPECTED compared_to_yesterday '
                    f'flag: "{compared_to_yesterday}". Please alert the '
                    f"validation maintainer.]</b><br>"
                )

            # compose descriptive error message
            if plots[plot]["comparison_result"] == "error":
                errormsg = "comparison unequal"
            elif plots[plot]["comparison_result"] == "not_compared":
                errormsg = ""
            else:
                errormsg = plots[plot]["comparison_result"]

            body_plot += "<b>{plot}</b><br>"
            body_plot += "<b>Package:</b> {package}<br>"
            body_plot += "<b>Rootfile:</b> {rootfile}.root<br>"
            body_plot += "<b>Description:</b> {description}<br>"
            body_plot += "<b>Comparison:</b> {comparison_text}<br>"
            if errormsg:
                body_plot += f"<b>Error:</b> {errormsg}<br>"
            warnings_str = ", ".join(plots[plot]["warnings"]).strip()
            if warnings_str:
                body_plot += f"<b>Warnings:</b> {warnings_str}<br>"
            # URLs are currently not working.
            # if plots[plot]["rootfile"] != "--":
            #     body_plot += '<a href="{url}#{package}-{rootfile}">' \
            #                  'Click me for details</a>'
            body_plot += "\n\n"

            # Fill in fields
            body_plot = body_plot.format(
                plot=plot,
                package=plots[plot]["package"],
                rootfile=plots[plot]["rootfile"],
                description=plots[plot]["description"],
                comparison_text=plots[plot]["comparison_text"],
                url=url,
            )

            body += body_plot

        body += (
            f"You can take a look at the plots/scripts "
            f'<a href="{url}">here</a>.'
        )

        return body

    # todo: this logic should probably be put somewhere else
    @staticmethod
    def _force_full_report() -> bool:
        """ Should a full (=non incremental) report be sent?
        Use case e.g.: Send a full report every Monday.
        """
        is_monday = date.today().weekday() == 0
        if is_monday:
            print("Forcing full report because today is Monday.")
            return True
        return False

    def send_all_mails(self, incremental=None):
        """
        Send mails to all contacts in self.mail_data_new. If
        self.mail_data_old is given, a mail is only sent if there are new
        failed plots
        @param incremental: True/False/None (=automatic). Whether to send a
            full or incremental report.
        """
        if incremental is None:
            incremental = not self._force_full_report()
        if not incremental:
            print("Sending full ('Monday') report.")
        else:
            print("Sending incremental report.")

        recipients = []
        for contact in self._mail_data_new:
            # if the errors are the same as yesterday, don't send a new mail
            if incremental and self._check_if_same(
                self._mail_data_new[contact]
            ):
                # don't send mail
                continue
            recipients.append(contact)

            # set the mood of the b2bot
            if len(self._mail_data_new[contact]) < 4:
                mood = "meh"
            elif len(self._mail_data_new[contact]) < 7:
                mood = "angry"
            elif len(self._mail_data_new[contact]) < 10:
                mood = "livid"
            else:
                mood = "dead"

            body = self._compose_message(
                self._mail_data_new[contact], incremental=incremental
            )

            if incremental:
                header = "Validation: New/changed warnings/errors"
            else:
                header = "Validation: Monday report"

            mail_utils.send_mail(
                contact.split("@")[0], contact, header, body, mood=mood
            )

        # send a happy mail to folks whose failed plots work now
        if self._mail_data_old:
            for contact in self._mail_data_old:
                if contact not in self._mail_data_new:
                    recipients.append(contact)
                    body = "Your validation plots work fine now!"
                    mail_utils.send_mail(
                        contact.split("@")[0],
                        contact,
                        "Validation confirmation",
                        body,
                        mood="happy",
                    )

        recipient_string = "\n".join([f"* {r}" for r in recipients])
        print(f"Sent mails to the following people: \n{recipient_string}\n")

    def write_log(self):
        """
        Dump mail json.
        """
        with open(
            os.path.join(self._validator.get_log_folder(), "mail_data.json"),
            "w",
        ) as f:
            json.dump(self._mail_data_new, f, sort_keys=True, indent=4)
