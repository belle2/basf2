# -*- coding: utf-8 -*-

"""Common functions for KLM calibration strategies."""


def calibration_result_string(result):
    """
    Convert calibration result to text message.
    """
    if (result == 0):
        res = 'successful'
    elif (result == 1):
        res = 'iteration is necessary'
    elif (result == 2):
        res = 'not enough data'
    elif (result == 3):
        res = 'failure'
    elif (result == 4):
        res = 'undefined'
    return res
