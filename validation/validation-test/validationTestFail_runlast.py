#!/usr/bin/env python3

# @cond SUPPRESS_DOXYGEN

"""
<header>
<contact>Kilian Lieret, Kilian.Lieret@campus.lmu.de</contact>


<description>
This file will exit with an exit code != 0 to probe the
validation correctly detects and reports this.
</description>
</header>
"""


import sys

if __name__ == "__main__":
    print("This script will exit with code 20 on purpose")

    sys.exit(20)

# @endcond
