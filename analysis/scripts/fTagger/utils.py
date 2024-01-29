# This script contains the utility functions used for flavor tagging

# imports
import b2bii


def get_Belle_or_Belle2():
    """
    Gets the global ModeCode.
    """
    if b2bii.isB2BII():
        return 'Belle'
    else:
        return 'Belle2'
