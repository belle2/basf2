#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# standard
import socket
import sys
import subprocess
import tempfile
import os
import time
import requests
import splinter
import traceback

# ours
from b2test_utils import is_ci, skip_test
import validationpath
from validationtestutil import check_execute

# url of the local validation webserver
validation_url = None  # will be set at runtime


def http_post(command, json_args, retries=10, delay=1.0):
    call_url = validation_url + command
    print(f"Posting {json_args} to {command}")
    for i in range(retries):
        try:
            r = requests.post(call_url, json=json_args, timeout=5)
            if r.ok:
                return r
        except requests.exceptions.ConnectionError:
            print(f"Server not reachable yet (attempt {i+1}/{retries})")
            time.sleep(delay)
    print(f"Failed to reach server at {call_url}")
    return None


def wait_for_port_any(port: int, timeout: float = 30.0):
    """Wait until something listens on either 127.0.0.1 or ::1."""
    start = time.time()
    while time.time() - start < timeout:
        for host in ("127.0.0.1", "::1"):
            try:
                with socket.create_connection((host, port), timeout=1):
                    return host  # returns the one that worked
            except OSError:
                pass
        time.sleep(0.1)
    return None


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

    # check the plot status with the webserver and only exit after a timeout
    # or if the plot combination has been created

    # 10 retries × 1 s delay per http_post -> 10 s worst‑case
    wait_time = 1.0  # in seconds
    max_wait_time = 15
    start = time.time()
    while True:
        res = http_post("check_comparison_status", {"input": prog_key})
        if not res:
            return False

        if res.json():
            if res.json()["status"] == "complete":
                # plots are done
                break

        time.sleep(wait_time)
        if time.time() - start > max_wait_time:
            print(
                f"Waited for {max_wait_time} seconds for the requested plots to complete and nothing happened"
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
        "validationTestPlots_gaus_histogram.pdf",
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
    with splinter.Browser("firefox", headless=True) as browser:
        url = validation_url + "static/validation.html"
        print(f"Opening {url} to perform checks")
        browser.visit(url)

        if not browser.title:
            print("Validation website cannot be loaded")
            return False

        found_revs = browser.find_by_css(".revision-label")
        found_rev_values = {r.value for r in found_revs}

        for r in revs:
            if r not in found_rev_values:
                print(
                    f"Revision {r} was not found on validation website. "
                    "It should be there."
                )
                return False

        print("Waiting for plot objects to be rendered...")

        timeout = 20
        start = time.time()
        plot_count = 0

        while time.time() - start < timeout:
            plot_count = browser.execute_script(
                "return document.querySelectorAll('.object').length"
            )
            if plot_count >= min_plot_objects:
                break
            time.sleep(0.5)

        print(f"Found {plot_count} plot objects via JS")

        if plot_count < min_plot_objects:
            print(
                f"Only {plot_count} plots found, while "
                f"{min_plot_objects} are expected"
            )
            return False

        checkbox_overview = browser.find_by_id("check_show_overview")

        if not checkbox_overview:
            print("Overview checkbox not found")
            return False

        browser.execute_script(
            """
            const cb = document.getElementById('check_show_overview');
            if (!cb.checked) {
                cb.click();
            }
            """
        )

        browser.execute_script("document.body.offsetHeight")

        print("Waiting for matrix plots to be rendered...")

        start = time.time()
        matrix_count = 0

        while time.time() - start < timeout:
            matrix_count = browser.execute_script(
                "return document.querySelectorAll('.plot_matrix_item').length"
            )
            if matrix_count >= min_matrix_plots:
                break
            time.sleep(0.5)

        print(f"Found {matrix_count} matrix plots via JS")

        if matrix_count < min_matrix_plots:
            print(
                f"Only {matrix_count} matrix plots found, while "
                f"{min_matrix_plots} are expected"
            )
            return False

    return True


def main():
    """
    Runs two test validations, starts the web server and queries data
    """

    skip_test("The test is problematic when run during our CI/CD pipelines")

    success = True

    revs_to_gen = ["stack_test_1", "stack_test_2", "stack_test_3"]

    # create a temporary test folder in order not to interfere with
    # already existing validation results
    with tempfile.TemporaryDirectory() as tmpdir:

        # switch to this folder
        os.chdir(str(tmpdir))

        for r in revs_to_gen:
            check_execute(f"b2validation -p 4 --test --tag {r}")

        # make sure the webserver process is terminated in any case
        try:
            # start webserver to serve json output files, plots and
            # interactive website
            # try IPv6 first; if that fails, fall back to IPv4.
            try:
                server_process = subprocess.Popen(["b2validation-server", "--ip", "::"])
            except OSError:
                server_process = subprocess.Popen(["b2validation-server", "--ip", "127.0.0.1"])

            # wait for up to 30 seconds for the server to start
            active_host = wait_for_port_any(8000, timeout=30)
            if not active_host:
                print("Validation server did not start within 30 s")
                server_process.terminate()
                sys.exit(1)

            # build URL from the address that really worked
            global validation_url
            if ":" in active_host:
                validation_url = f"http://[{active_host}]:8000/"
            else:
                validation_url = f"http://{active_host}:8000/"

            # check the content of the webserver if not running in GitLab pipeline
            if not is_ci():
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
            print("The test was NOT successful")
            sys.exit(1)
        else:
            print("The test was successful")


if __name__ == "__main__":
    main()
