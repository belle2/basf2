#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
iov_conditional - Functions to Execute Paths Depending on Experiment Phases
===========================================================================

This module contains some convenience functions to execute different paths
depending on the experiment run number. This allows to write one common steering
file for phases 2 and 3.

Warning:
    All Modules will be initialized on startup of the framework. This might
    cause problems if the modules initialize some common global state. For
    example adding :b2:mod:`Geometry` modules with different parameters will not
    work as expected.
"""
import basf2

PHASE2_IOVS = [(1, 0, 4, -1), (1002, 0, 1002, -1)]


def make_conditional_at(path, iov_list, path_when_in_iov, path_when_not_in_iov=None):
    """
    Branch the current path execution based on the given iov list.  If the
    current exp/run is met by one of the IoV conditions, execute the path given
    in ``path_when_in_iov`` and continue the normal path afterwards. Otherwise
    execute the ``path_when_not_in_iov`` when given.

    Basically, this function is just a wrapper around adding the
    :b2:mod:`IoVDependentCondition` module to the path.

    See Also:
        If you just need to distinguish between phase 2/3 please use
        `phase_2_conditional()`

    Example:

        >>> make_conditional_at(path, iov_list=[(0, 0, 0, -1)],
        ...                     path_when_in_iov=exp_0_path,
        ...                     path_when_not_in_iov=not_exp_0_path)

    will branch the path for all events with experiment number 0 to the modules
    in ``exp_0_path`` and into ``not_exp_0_path`` in all other cases.

    Parameters:
        path (basf2.Path): Branch the execution of the given path.
        iov_list (list(tuple)): Under which IoV conditions should the
            ``path_when_in_iov`` be executed. It should be a list in the form
            ``[(min exp, min run, max exp, max run), ...]``
        path_when_in_iov (basf2.Path): Which branch to execute, if one of the IoV conditions is met.
        path_when_not_in_iov (basf2.Path): If given, execute this path in all cases, none IoV condition is met.
    """
    condition_module = path.add_module("IoVDependentCondition", iovList=iov_list)
    condition_module.if_true(path_when_in_iov, basf2.AfterConditionPath.CONTINUE)
    if path_when_not_in_iov:
        condition_module.if_false(path_when_not_in_iov, basf2.AfterConditionPath.CONTINUE)


def phase_2_conditional(path, phase2_path, phase3_path=None):
    """
    Handy shortcut for phase 2/3 conditional module execution.

    See Also:
        `make_conditional_at()`

    Parameters:
        path (basf2.Path): Branch the execution of the given path.
        phase2_path (basf2.Path): Call this path only when in phase 2.
        phase3_path (basf2.Path): When given, call this path only when in phase 3.
    """
    make_conditional_at(path=path, iov_list=PHASE2_IOVS,
                        path_when_in_iov=phase2_path, path_when_not_in_iov=phase3_path)
