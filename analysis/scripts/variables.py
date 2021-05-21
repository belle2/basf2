#!/usr/bin/env python3

"""
You can run ``b2help-variables`` to list all available variables.
"""
from variables import getCommandLineOptions
import subprocess
import basf2


if __name__ == "__main__":
    #: get command line arguments
    args = getCommandLineOptions()
    basf2.B2WARNING("Calling basf2 variables.py is discouraged. The canonical way is to call b2help-variables.")

    if args.pager:
        subprocess.call("b2help-variables")
    else:
        subprocess.call(["b2help-variables", "--no-pager"])
