#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import functools


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
    """Returns an iterator over and std::vector like structure that supports .size() and.at(i)

    Use like:
    for item in vector_iterator(container):
        print item

    """

    return (v.at(i) for i in xrange(v.size()))


def is_primary(mcParticle):
    """Indicates if the given MCParticle is primary."""

    return mcParticle.hasStatus(1)


