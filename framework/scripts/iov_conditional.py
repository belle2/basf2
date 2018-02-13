#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Already prepared and prefilled conditional module for the different experiment phases
"""
from functools import partial

import basf2

PHASE2_CONDITIONS = [(1, 0, 4, -1), (1002, 0, 1002, -1)]


def make_conditional_at(path, iov_list, path_when_in_iov, path_when_not_in_iov=None):
    """
    Branch the current path execution based on the given iov list.
    If the current exp/run is met by one of the IoV conditions, execute the path given in
    `path_when_in_iov` and continue the normal path afterwards. Else, execute the `path_when_not_in_iov`
    when given.

    Basically, this function is just a wrapper around adding the IoVDependentCondition module to the path.

    Example:

    >>> make_conditional_at(path, iov_list=[(0, 0, 0, -1)],
    ...                     path_when_in_iov=exp_0_path,
    ...                     path_when_not_in_iov=not_exp_0_path)

    will branch the path for all events with experiment number 0 to the modules in exp_0_path and into
    not_exp_0_path in all other cases.

    Parameters:
        path: Branch the execution of the given path.
        iov_list: Under which IoV conditions should the `path_when_in_iov` be executed. It should be a list in the form
                  [(min exp, min run, max exp, max run), ...]
        path_when_in_iov: Which branch to execute, if one of the IoV conditions is met.
        path_when_not_in_iov: If given, execute this path in all cases, none IoV condition is met.
    """
    condition_module = path.add_module("IoVDependentCondition", iovList=iov_list)
    condition_module.if_true(path_when_in_iov, basf2.AfterConditionPath.CONTINUE)
    if path_when_not_in_iov:
        condition_module.if_false(path_when_not_in_iov, basf2.AfterConditionPath.CONTINUE)


def phase_2_conditional(path, phase2_path, phase3_path=None):
    """Handy shortcut for phase 2 conditions"""
    make_conditional_at(path=path, iov_list=PHASE2_CONDITIONS,
                        path_when_in_iov=phase2_path, path_when_not_in_iov=phase3_path)
