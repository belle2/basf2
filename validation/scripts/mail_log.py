#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json

# # load comparison json file
# with open("../../html/plots/current_reference/comparison.json") as f:
#     comparison = json.load(f)


def create_mail_log(comparison):
    """
    takes the entire comparison json file, finds all the plots where comparison failed
    and saves them the following format:
    {
         "email@address.test" : [
             {
                 "package": ... "plot": ....
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
                        if plot["comparison_result"] == "error":
                            # save all the information that's needed for an informative email
                            error_data = {}
                            error_data["package"] = plotfile["package"]
                            error_data["plot"] = plot
                            # check if this contact already gets mail
                            if plot["contact"] not in mail_log:
                                # create new key for this contact
                                mail_log[plot["contact"]] = []
                            mail_log[plot["contact"]].append(error_data)
    return mail_log
