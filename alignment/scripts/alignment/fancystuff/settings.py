#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""Settings file

The logger for all classes in the distribution is defined here.

"""
__author__ = 'swehle'

import logging
import matplotlib.pyplot as plt

logging.basicConfig(level=logging.DEBUG,
                    format='%(name)-18s \t %(levelname)-8s %(message)s',
                    datefmt='%m-%d %H:%M',
                    )


class ProTool(object):

    """
    All pro tools inherit form this class.
    For now each class get a logger.

    Attributes
    ----------
    name : str
        Name of the Class.
    io : function
        Returns the logger

    """

    def __init__(self, name):
        """ init function
        :param name: Name of the class
        """
        #: Name of the class
        self.name = name

    @property
    def io(self):
        """
        Logging function
        :return: logger
        """
        return logging.getLogger(self.name)


def create_figure(self, width=None, square=False, ratio=None):
    """
    Create a new figure
    :param self:
    :param width:
    :param square:
    :param ratio:
    :return:
    """
    if ratio is None:
        ratio = 1.618
    if width is None:
        width = 9
    height = width if square else width / ratio
    return plt.subplots(figsize=(width, height))
