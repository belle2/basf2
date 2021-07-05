##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""Various functions to interact with ROOT objects and the runtime environment"""

import ROOT
import contextlib
from functools import singledispatch


def root_walk(tdirectory):
    """Walks the content of a TDirectory similar to os.walk.

    Yields 3-tuples of current TDirectories, contained TObjects and contained TDirectories
    for each of the directories nested inside the given TDirectory in a depth first manner.

    Yields
    ------
    (TDirectory, list(TObject), list(TDirectory))
    """
    tkeys = tdirectory.GetListOfKeys()

    tobjects = []
    tdirectories = []
    for tkey in tkeys:
        tobject = tdirectory.Get(tkey.GetName())
        if isinstance(tobject, ROOT.TDirectory):
            tdirectories.append(tobject)
        else:
            tobjects.append(tobject)

    yield tdirectory, tobjects, tdirectories

    # Continue depth first
    for sub_tdirectory in tdirectories:
        for tdirectory, tobjects, tdirectories in root_walk(sub_tdirectory):
            yield tdirectory, tobjects, tdirectories


@contextlib.contextmanager
def root_open(tfile_or_file_path, tfile_options=None):
    """Context manager to open a TFile.

    If a file path is given open the TFile and close it after the context is left.
    If an already opened TFile is received simply return it and do not close on exit.

    Parameters
    ----------
    tfile_or_file_path : str or ROOT.TFile
        Path to the file or the TFile that should be activated.
    tfile_options : str
        Option string forwarded to the ROOT.TFile constructor
        Typical options as "RECREATE", "READ" or "UPDATE".
    """
    if isinstance(tfile_or_file_path, ROOT.TFile):
        tfile = tfile_or_file_path
        with root_cd(tfile) as tfile:
            yield tfile
    else:
        save_tdirectory = ROOT.gROOT.CurrentDirectory().load()

        if tfile_options is None:
            tfile = ROOT.TFile(tfile_or_file_path)
        else:
            tfile = ROOT.TFile(tfile_or_file_path, tfile_options)

        try:
            yield tfile
        finally:
            tfile.Close()
            save_tdirectory.cd()


@contextlib.contextmanager
def root_cd(tdirectory):
    """Context manager that temporarily switches the current global ROOT directory while in the context.

    If a string as the name of a directory is given as the argument
    try to switch to the directory with that name in the current ROOT folder.

    If it is not present create it.

    Parameters
    ----------
    tdirectory : ROOT.TDirectory or str
        ROOT directory to switch to or name of a folder to switch.

    Returns
    -------
    ROOT.TDirectory
        The new current ROOT directory.
    """

    # Do not use ROOT.gDirectory here.
    # Since ROOT.gDirectory gets transported as a reference it changes on a call to cd() as well,
    # and can therefore not serve to save the former directory.
    save_tdirectory = ROOT.gROOT.CurrentDirectory().load()

    if not tdirectory or "." == tdirectory:
        tdirectory = save_tdirectory

    elif isinstance(tdirectory, str):
        tdirectory_name = tdirectory

        # Look for the tdirectory with the name
        # before trying to create it
        tdirectory = save_tdirectory.GetDirectory(tdirectory_name)
        if not tdirectory:
            tdirectory = save_tdirectory.mkdir(tdirectory_name, tdirectory_name)
            if not tdirectory:
                raise RuntimeError("Could not create or find folder %s" % tdirectory_name)

            # If tdirectory_name is as hierachy like a/b/c make sure we select the most nested folder
            # (and not a as the documentation of TDirectory.mkdir suggests).
            tdirectory = save_tdirectory.GetDirectory(tdirectory_name)

    try:
        if tdirectory is not None:
            tdirectory.cd()
        yield tdirectory

    finally:
        save_tdirectory.cd()


def root_save_name(name):
    """Strips all meta characters that might be unsafe to use as a ROOT name.

    Parameters
    ----------
    name : str
        A name that should be transformed

    Returns
    -------
    str
        Name with potentially harmful characters deleted / replaced.
    """
    deletechars = str.maketrans("", "", r"/$\#{}()[]=")
    name = name.replace(' ', '_').replace('-', '_').replace(',', '_').translate(deletechars)
    return name


def root_browse(tobject):
    """Open a browser and show the given object.

    Parameters
    ----------
    tobject : ROOT.TObject
        The object to be shown

    Returns
    -------
    ROOT.TBrowser
        The new TBrowser used to show the object.
    """
    # Set the style to the style desired by the validation plots.
    import tracking.validation.plot
    tracking.validation.plot.ValidationPlot.set_tstyle()

    tbrowser = ROOT.TBrowser()
    if isinstance(tobject, ROOT.TObject):
        if hasattr(tobject, "Browse"):
            tobject.Browse(tbrowser)
        else:
            tbrowser.BrowseObject(tobject)
    else:
        raise ValueError("Can only browse ROOT objects inheriting from TObject.")
    return tbrowser


@singledispatch
def root_ls(obj):
    """Returns a list of names that are contained in the given obj.

    This is a convinience function to invesitigate the content of ROOT objects,
    that dispatches on to object type and retieves different things depending on the type.
    If the obj is a string it is interpreted as a filename.
    """
    return list(obj)


@root_ls.register(str)
def __(filename):
    """Overloaded function for root_ls for filenames (e.g. opens the file and ls its content)."""
    rootFile = ROOT.TFile(filename)
    result = root_ls(rootFile)

    rootFile.Close()
    del rootFile
    return result


@root_ls.register(ROOT.TDirectory)
def __(tDirectory):  # noqa
    """Overloaded function for root_ls for ROOT directories (e.g. list the keys in the directory)."""
    tKeys = list(tDirectory.GetListOfKeys())
    result = sorted([tKey.GetName() for tKey in tKeys])
    return result


@root_ls.register(ROOT.TTree)
@root_ls.register(ROOT.TNtuple)
def __(tTree):  # noqa
    """Overloaded function for root_ls for trees and ntuples (e.g. list the keys in the tuple/tree)."""
    tBranches = list(tTree.GetListOfBranches())
    result = sorted([tBranch.GetName() for tBranch in tBranches])
    return result
