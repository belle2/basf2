#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

from daqdqm.commondqm import add_common_dqm


def add_collision_dqm(path, components=None):
    """
    This function adds DQMs for collisions
    """

    add_common_dqm(path)
