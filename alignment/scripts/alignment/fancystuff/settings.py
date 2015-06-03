# -*- coding: utf-8 -*-
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
    attr1 : name
        Name if the Class.
    """

    def __init__(self, name):
        """

        Parameters
        ----------
        name : str
            Name of the class
        """
        self.name = name

    @property
    def io(self):
        return logging.getLogger(self.name)


def create_figure(self, width=None, square=False, ratio=None):
    if ratio is None:
        ratio = 1.618
    if width is None:
        width = 9
    height = width if square else width / ratio
    return plt.subplots(figsize=(width, height))
