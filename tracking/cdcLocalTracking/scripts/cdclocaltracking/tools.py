#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import functools
import collections
import argparse


class DefaultHelpArgumentParser(argparse.ArgumentParser):

    """An argparse.Argument parse slightly changed such 
    that it always prints an extended help message incase of a parsing error."""

    def error(self, message):
        """Method invoked when a parsing error occured.
        Writes an extended help over the base ArgumentParser.
        """

        self.print_help()
        sys.stderr.write('error: %s\n' % message)
        sys.exit(2)


class ChDirContext(object):

    """Helper object that temporarily changes the current working. 
    Creates the folder if it is not present.
    Implements the context manager and function decorator interface.
    
    Use like:
    @ChDirContext("myfolder")
    def function_that_writes_output_to_folder():
        pass

    or like
    with ChDirContext("myfolder"):
        command_that_writes_output_to_folder()
    """

    def __init__(self, folder_name):
        """Creates a context object that switches the current working directory <folder_name>."""

        self.folder_name = folder_name
        self.saved_folder_name = None

    def __enter__(self):
        """Enters the context. Changes the working directory. Creates it if needed."""

        folder_name = self.folder_name
        if self.saved_folder_name:
            raise RuntimeError('ChDirContext can not be entered twice')

        self.saved_folder_name = os.getcwd()

        if not os.path.exists(folder_name):
            os.makedirs(folder_name)

        os.chdir(folder_name)

    def __exit__(
        self,
        err_type,
        err_obj,
        err_traceback,
        ):
        """Exits the context. Changes the working directory back to the value before."""

        os.chdir(self.saved_folder_name)
        self.saved_folder_name = None

    def __call__(self, func):
        """Decorates the given function to be executed in context where the current working directory is changed."""

        @functools.wraps(func)
        def decorated(*args, **kwds):
            with self:
                func(*args, **kwds)

        return decorated


def vector_iterator(v):
    """Returns an iterator over an std::vector like structure that supports .size() and.at(i)

    Example
    -------
    for item in vector_iterator(container):
        print item

    """

    return (v.at(i) for i in xrange(v.size()))


def is_iterable_collection(obj):
    """Test if the object is an iterable collection..

    Targets the duck typing of container types.
    Stings are not considered iterable by this definition.
    
    Parameters
    ----------
    obj : any type
        Object to be checked.
    """

    return isinstance(obj, collections.Iterable)


def is_primary(mcParticle):
    """Indicates if the given MCParticle is primary.
    
    Parameters
    ----------
    mcParticle : Belle2.MCParticle
        MCParticle to be checked"""

    return mcParticle.hasStatus(1)


