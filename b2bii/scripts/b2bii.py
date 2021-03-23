# !/usr/bin/env python3

import os


def isB2BII():
    if os.environ.get("B2BII", "").lower() in ['true', 'yes', 'on', '1']:
        return True
    else:
        return False


def setB2BII():
    os.environ["B2BII"] = 'TRUE'


def unsetB2BII():
    os.environ["B2BII"] = 'FALSE'
