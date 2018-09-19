#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import smtplib
from email.mime.text import MIMEText


class issue:
    """
    Describe an issue that occured while validating
    """

    # issue types
    EXEC_ERROR = "execution error"
    COMP_ERROR = "comparison error"

    def __init__(self, type, message):
        self.type = type
        self.message = message

    def format_issue(self):
        return "Issue type: {}\nError message: {}".format(self.type, self.message)


class mailto:
    """
    Class to collect issues and send them as mail
    """

    def __init__(self, contact):
        self.contact = contact
        self.issues = []

    def add_issue(self, issue):
        self.issues.append(issue)

    def compose_message(self):
        body = "Dear user,\nthere were some issues.\n"
        for issue in self.issues:
            body += "\n" + issue.format_issue() + "\n"

        msg = MIMEText(body)
        msg['Subject'] = "Error in validation"
        # msg['From'] = what to do here??
        msg['To'] = self.contact

        return msg

    def send_mail(self):
        # connect to some server
        s = smtplib.SMTP('localhost')
        s.send_mail(self.compose_message())
        s.quit()


class maillog:

    def __init__(self, comparison_packages):
        self.packages = comparison_packages
