#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Check with decaylanguage-validate package for common mistakes in decfiles
"""

import os
import subprocess

from b2test_utils import check_error_free, is_ci, skip_test

# The validator is not part of the basf2 externals/tools, so run it from a
# throw-away environment with uvx (uv itself comes from the externals). uv
# caches that environment keyed by the '--from' specification, so both
# invocations below share it. '-q' keeps uv quiet on stderr, which is merged
# into the checked tool output.
# Pin the version so the set of diagnostics applied to the decfiles cannot
# change by itself; bump it deliberately when a new decaylanguage is wanted.
VALIDATE_COMMAND = ["uvx", "-q", "--from", "decaylanguage==1.1.4", "decaylanguage-validate"]


def changed_decfiles():
    """Return the files in ``decfiles/`` which the current branch changed with
    respect to the branch it is going to be merged into.

    That is the merge request target branch for merge request pipelines, and
    ``main`` otherwise.

    Returns:
        A list of paths, or ``None`` if the changes cannot be determined, for
        example in a central release, or if there is no ``origin/<target>``
        reference to compare against.
    """
    directory = os.environ.get("BELLE2_LOCAL_DIR")
    if not directory:
        return None
    target = os.environ.get("CI_MERGE_REQUEST_TARGET_BRANCH_NAME", "main")
    try:
        # '...' makes git compare against the merge base with the target branch,
        # so changes which only happened in the target branch are not reported
        changed = subprocess.run(["git", "-C", directory, "diff", "--name-only",
                                  f"origin/{target}...HEAD", "--", "decfiles"],
                                 check=True, capture_output=True, text=True).stdout
    except (OSError, subprocess.CalledProcessError):
        return None
    return changed.split()


if __name__ == "__main__":
    # The check is only worth running when the decfiles could have changed, so
    # skip it otherwise. When running locally always do the check, and do it
    # too if the changes cannot be determined: never silently skip because of
    # missing git information.
    if is_ci() and changed_decfiles() == []:
        skip_test("No files in decfiles/ changed since main, nothing to validate")

    # Run validate on all decfiles where missing CDecay is allowed
    check_error_free(
        VALIDATE_COMMAND,
        "decaylanguage-validate",
        "decfiles",
        toolopts=[
            "--ignore",
            "DLW004",
            "--ignore",
            "DLW010",
            "--ignore",
            "DLW005",
            "--ignore",
            "DLW011",
        ],
    )
    # Run validate again in generic decfile where missing CDecay is not allowed
    check_error_free(
        VALIDATE_COMMAND,
        "decaylanguage-validate",
        "decfiles/dec/DECAY_BELLE2.DEC",
    )
