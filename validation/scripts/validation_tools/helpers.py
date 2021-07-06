##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
Helper scripts needed to run tools and checks on the validation.
Mainly for git checkout and compilation.
"""

from subprocess import check_call, check_output, CalledProcessError
import basf2
import sys
import os

try:
    from git import Repo
except ImportError:
    print(
        "This script requires GitPython, please install it via `pip3 install --user GitPython"
    )
    sys.exit(1)


def get_basf2_repo_folder():
    """
    Return the folder name of the basf2 repository currently set up
    """
    git_repository_folder = os.getenv("BELLE2_LOCAL_DIR")
    if not git_repository_folder:
        raise ValueError("You need to setup basf2 first")

    return git_repository_folder


def get_basf2_repo(non_dirty_check=False):
    """
    Return a git repo object of the current basf2 repository

    :param non_dirty_check: Check if the repo is dirty and issue a printed warning if yes.
    """
    git_repository = get_basf2_repo_folder()
    repo = Repo(git_repository)
    assert not repo.bare

    if non_dirty_check and repo.is_dirty():
        basf2.B2WARNING(
            "Your git repo is dirty! I can not guarantee for valid results..."
        )

    return repo


def checkout_git_revision(revision, repo=None, use_stash=False):
    """
    Checkout the given revision in the basf2 repository.
    ATTENTION: this does not check for dirty files etc.

    :param revision: which revision to checkout
    :param repo: basf2 repo object (None will use the default basf2 repo)
    """
    if not repo:
        repo = get_basf2_repo()

    if use_stash:
        repo.git.stash()
    repo.git.checkout(revision)
    if use_stash:
        repo.git.stash("pop")


def compile_basf2(compile_options=None):
    """
    Compile basf2 with the given options as list.

    :param compile_options: List of cmd options given to scons.
    """
    if compile_options is None:
        compile_options = []

    git_repository_folder = get_basf2_repo_folder()
    check_call(["scons"] + compile_options, cwd=git_repository_folder)


def run_basf2_validation(validation_options=None):
    """
    Run the basf2 validation.
    :param validation_options: List of options given to the b2validation call.
    """
    git_repository_folder = get_basf2_repo_folder()
    check_call(["b2validation"] + validation_options, cwd=git_repository_folder)


def fix_root_command_line():
    """
    Fix for ROOT to give the command line options directly to python.
    """
    from ROOT import PyConfig

    PyConfig.IgnoreCommandLineOptions = True


def get_git_hashes_between(git_end_hash, git_start_hash):
    """
    Return list of git hashes between `git_end_hash` and `git_start_hash`
    (but not including them).
    """
    try:
        git_hashs = (
            check_output(
                [
                    "git",
                    "log",
                    str(git_start_hash + ".." + git_end_hash),
                    "--pretty=format:%H",
                ]
            )
            .decode("utf-8")
            .split("\n")
        )
        return git_hashs
    except CalledProcessError:
        basf2.B2FATAL("Error while receiving the git history.")
