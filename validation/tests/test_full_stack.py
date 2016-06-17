#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import time
import subprocess
from multiprocessing import Process
import validationserver

try:
    import splinter
except ImportError:
    print("The splinter package is required to run this test. Run 'pip3 install splinter' to install")
    # don't give an error exit code here to not fail if splinter is not available
    sys.exit(0)


def check_excecute(cmd, terminate_on_error=True):
    """
    Executes a shell commands and check for =! 0 return codes
    """

    print("Executing command '{}'".format(cmd))
    res = os.system(cmd)
    print("Command '{}' exited with code {}".format(cmd, res))
    if not res == 0:
        print("FATAL: Exit code is not 0")
        if terminate_on_error:
            sys.exit(res)
        return False

    return True


def start_webserver():
    """
    Start the validation server process, this will not
    return. Therefor this function must be started within
    a new subprocess
    """
    validationserver.run_server()


def check_for_content(revs, min_matrix_plots, min_plot_objects):
    """
    Checks for the expected content on the validation website
    """

    with splinter.Browser() as browser:
        # Visit URL
        url = "localhost:8000/static/validation.html"
        print("Opening {} to perform checks", url)
        browser.visit(url)

        if len(browser.title) == 0:
            print("Validation website cannot be loaded")
            return False

        found_revs = browser.find_by_css(".revision-label")

        for r in revs:
            rr = [web_r for web_r in found_revs if web_r.value == r]
            if len(rr) == 0:
                print("Revsion {} was not found on validation website. It should be there.".format(r))
                return False

        plot_objects = browser.find_by_css(".object")

        print("Checking for a minimum number of {} plot objects", min_plot_objects)
        if len(plot_objects) < min_plot_objects:
            print("Only {} plots found, while {} are expected".format(len(plot_objects), min_plot_objects))
            return False

        # click the overview check box
        checkbox_overview = browser.find_by_id("check_show_overview")
        # todo: does not work yet, checkbox is directly unchecked again
        checkbox_overview.check()
        # found_matrix_plots = browser.find_by_css(".plot_matrix_item")

        # if len(found_matrix_plots) < min_matrix_plots:
        #    print ("Only {} matrix plots found, while {} are expected".format(len(found_matrix_plots), min_matrix_plots))
        #    return False

    return True


def main():
    """
    Runs two test validations, starts the web server and queries data
    """
    success = True

    revs_to_gen = ["stack_test_1", "stack_test_2"]

    for r in revs_to_gen:
        check_excecute("validate_basf2 --test --tag {}".format(r))

    # make sure the webserver process is terminated in any case
    try:
        # start webserver to serve json output files, plots and
        # interactive website
        server_process = subprocess.Popen(["python3", "validation/scripts/validationserver.py"])

        # wait for one second for the server to start
        time.sleep(1)
        success = success and check_for_content(revs_to_gen + ["reference"], 7, 7)
    except:
        e = sys.exc_info()[0]
        # print exception again
        print("Error {}".format(e))
        success = False
    finally:
        # send terminate command
        server_process.terminate()
        # wait for the webserver to actually terminate
        server_process.wait()

    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main()
