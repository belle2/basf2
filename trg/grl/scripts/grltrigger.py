# !/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def grl_match(path):
    """
    the match module in grl
    """
    match = register_module('TRGGRLMatch')
    path.add_module(match)


def grl_objects(path):
    """
    produce the objecst used in trigger logics
    """
    objects = register_module('TRGGRLProjects')
    path.add_module(objects)


def add_grl_trigger(path):
    """
    do match and produce the objects on grl
    """
    grl_match(path)
    grl_objects(path)
