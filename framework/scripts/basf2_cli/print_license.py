#!/usr/bin/env python3

from basf2.core import basf2label
from basf2 import find_file

short_license = ""

with open(find_file("LICENSE.md")) as license_file:
    lines = [line for line in license_file.readlines()]
    short_license = "".join(lines[:20])
    # magic number to only grab the short version of the license

short_license += "\nFor the full text of the LGPL-3.0 License see $BELLE2_RELEASE_DIR/LICENSE.md"

print('')
print(short_license)
