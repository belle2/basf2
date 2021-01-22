#!/usr/bin/env python3

import os
import sys


def check_execute(cmd, terminate_on_error=True):
    """
    Executes a shell commands and check for =! 0 return codes
    """

    print(f"Executing command '{cmd}'")
    res = os.system(cmd)
    print(f"Command '{cmd}' exited with code {res}")
    if not res == 0:
        print("FATAL: Exit code is not 0")
        if terminate_on_error:
            sys.exit(res)
        return False

    return True


def check_path_exists(paths, terminate_on_error=True):
    """
    Checks if a path exists.

    @param paths: list of file system path (directories or files)
    @param terminate_on_error: if true, the execution is terminated if one part
        is not present
    @return: None
    """

    for p in paths:
        print(f"Checking for existance of file {p}")
        if not os.path.exists(p):
            print(f"Path {p} does not exist")
            if terminate_on_error:
                sys.exit(1)
        print("Jep !")


def create_fake_scripts(folders, scriptname):
    """
    Creates a fake script inside a nested path
    :param folders: folders to hold the script
    :param scriptname: name of the script itself
    :return: nothing
    """
    if not os.path.exists(folders):
        os.makedirs(folders)
    with open(os.path.join(folders, scriptname), "w") as f:
        f.write("# not content on purpose")
