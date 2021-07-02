#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# standard
import sys
import subprocess
import tempfile
import os
import time
import requests
import traceback

# ours
import validationserver
import validationpath
from validationtestutil import check_execute

# url of the local validation webserver
validation_url = "http://localhost:8000/"


def start_webserver():
    """
    Start the validation server process, this will not
    return. Therefore this function must be started within
    a new subprocess
    """
    validationserver.run_server()


def http_post(command, json_args):
    call_url = validation_url + command
    print(f"Posting {json_args} to {command}")
    r = requests.post(call_url, json=json_args)
    if not r.ok:
        print(
            "REST call {} with arguments {} failed".format(call_url, json_args)
        )
        print(str(r))
        return None

    return r


def check_for_plotting(revs, tmp_folder):
    """
    Checks if creating new plots for a revision combination works
    :param revs: List of revisions
    :type revs: List[str]
    :param tmp_folder: Temporary folder
    """

    print(f"Trying to recreate plots for revisions {revs}")

    res = http_post("create_comparison", {"revision_list": revs})
    if not res:
        return False

    # will be used to check on the progress
    prog_key = res.json()["progress_key"]

    done = False
    wait_time = 0.1  # in seconds
    max_wait_time = 3
    summed_wait_time = 0

    # check the plot status with the webserver and only exit after a timeout
    # or if the plot combination has been created
    while not done:
        res = http_post("check_comparison_status", {"input": prog_key})
        if not res:
            return False

        if res.json():
            if res.json()["status"] == "complete":
                # plots are done
                break

        time.sleep(wait_time)
        summed_wait_time += wait_time
        if summed_wait_time > max_wait_time:
            print(
                "Waited for {} seconds for the requested plots to complete "
                "and nothing happened".format(summed_wait_time)
            )
            return False

    # check if the plots are really present
    comp_folder = validationpath.get_html_plots_tag_comparison_folder(
        tmp_folder, revs
    )
    comp_json = validationpath.get_html_plots_tag_comparison_json(
        tmp_folder, revs
    )

    if not os.path.exists(comp_folder):
        print(f"Comparison folder {comp_folder} does not exist")
        return False
    if not os.path.isfile(comp_json):
        print(f"Comparison json {comp_json} does not exist")
        return False

    # just check for one random plot
    some_plot = os.path.join(
        comp_folder,
        "validation-test",
        "validationTestPlotsB_gaus_histogram.pdf",
    )
    if not os.path.isfile(some_plot):
        print(f"Comparison plot {some_plot} does not exist")
        return False

    print("Comparison properly created")
    return True


def check_for_content(revs, min_matrix_plots, min_plot_objects):
    """
    Checks for the expected content on the validation website
    """
    try:
        import splinter
    except ImportError:
        print(
            "The splinter package is required to run this test. Run 'pip3 "
            "install splinter' to install"
        )
        # don't give an error exit code here to not fail if splinter is not
        # available
        return True

    with splinter.Browser() as browser:
        # Visit URL
        url = validation_url + "static/validation.html"
        print("Opening {} to perform checks", url)
        browser.visit(url)

        if len(browser.title) == 0:
            print("Validation website cannot be loaded")
            return False

        found_revs = browser.find_by_css(".revision-label")

        for r in revs:
            rr = [web_r for web_r in found_revs if web_r.value == r]
            if len(rr) == 0:
                print(
                    "Revsion {} was not found on validation website. It "
                    "should be there.".format(r)
                )
                return False

        plot_objects = browser.find_by_css(".object")

        print(
            "Checking for a minimum number of {} plot objects", min_plot_objects
        )
        if len(plot_objects) < min_plot_objects:
            print(
                "Only {} plots found, while {} are expected".format(
                    len(plot_objects), min_plot_objects
                )
            )
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

    # fixme: See if we can reenable this test or at least run it locally
    print("TEST SKIPPED: Not properly runnable on build bot", file=sys.stderr)
    sys.exit(1)
    # noinspection PyUnreachableCode

    # only run the test on dev machines with splinter installed. Also for the
    # tests which don't use splinter, there are currently some connection
    # problems to the test webserver on the central build system
    try:
        import splinter  # noqa

        pass
    except ImportError:
        print(
            "TEST SKIPPED: The splinter package is required to run this test."
            + "Run 'pip3 install splinter' to install",
            file=sys.stderr,
        )
        sys.exit(1)

    success = True

    revs_to_gen = ["stack_test_1", "stack_test_2", "stack_test_3"]

    # create a temporary test folder in order not to interfere with
    # already existing validation results
    with tempfile.TemporaryDirectory() as tmpdir:

        # switch to this folder
        os.chdir(str(tmpdir))

        for r in revs_to_gen:
            check_execute(f"validate_basf2 --test --tag {r}")

        # make sure the webserver process is terminated in any case
        try:
            # start webserver to serve json output files, plots and
            # interactive website
            server_process = subprocess.Popen(["run_validation_server"])

            # wait for one second for the server to start
            time.sleep(2)
            # check the content of the webserver, will need splinter
            success = success and check_for_content(
                revs_to_gen + ["reference"], 7, 7
            )

            # check if the plott creating triggering works
            success = success and check_for_plotting(
                revs_to_gen[:-1], str(tmpdir)
            )
        except BaseException:
            # catch any exceptions so the finally block can terminate the
            # webserver process properly
            e = sys.exc_info()[0]
            # print exception again
            print(f"Error {e}")
            print(traceback.format_exc())
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
