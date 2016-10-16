#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os


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
