#!/usr/bin/env python3

"""Small module containing helper functions to set the metadata on objects
created for the validation correctly """

# std
from typing import Optional, Union, List, Tuple
import pathlib

import basf2
import ROOT
from ROOT import Belle2

# circumvent BII-1264
ROOT.gInterpreter.Declare("#include <framework/utilities/MakeROOTCompatible.h>")

# Unfortunately doxygen doesn't recognize our docstrings in this file :/
# @cond SUPPRESS_DOXYGEN


def file_description_set(
    rootfile: Union[ROOT.TFile, str, pathlib.PurePath], description: str
) -> None:
    """
    Add file description validation metdata to a ROOT file.

    Args:
        rootfile (TFile, str or pathlib.PurePath): Name of the root file
            to open or an already open TFile instance
        description (str):  Common description/information of/about all plots
            in this ROOT file (will be displayed above the plots)

    Returns:
        None
    """
    opened = False
    if not isinstance(rootfile, ROOT.TFile):
        if isinstance(rootfile, pathlib.PurePath):
            rootfile = str(rootfile)
        rootfile = ROOT.TFile(rootfile, "UPDATE")
        opened = True
    if not rootfile.IsOpen() or not rootfile.IsWritable():
        raise RuntimeError(
            f"ROOT file {rootfile.GetName()} is not open for writing"
        )
    # scope guard to avoid side effects by changing the global gDirectory
    # in modules ...
    # noinspection PyUnusedLocal
    directory_guard = ROOT.TDirectory.TContext(rootfile)  # noqa
    desc = ROOT.TNamed("Description", description)
    desc.Write()
    if opened:
        rootfile.Close()


def validation_metadata_set(
    obj: ROOT.TObject,
    title: str,
    contact: str,
    description: str,
    check: str,
    xlabel: Optional[str] = None,
    ylabel: Optional[str] = None,
    metaoptions="",
) -> None:
    """
    Set the validation metadata for a given object by setting the necessary
    values. This function can be used on any object supported by the
    Validation (histograms, profiles, ntuples)

    Arguments:
        obj: Instance of the object which should get the metadata
        title (str): Title to use for the object
        contact (str): Contact person, usually in the form "Name <email>"
        description (str): Text description what can be seen in the plot
        check (str): Text description what to look for in the validation for
            shifters to easily see if the distribution looks ok
        xlabel (str): If given try to set this as the label for the x axis
        ylabel (str): If given try to set this as the label for the y axis
        metaoptions (str): Metaoptions (additional options to influence the
            comparison between revisions, styling of the plot, etc.)

    .. warning::

        Different ways to specify LaTeX for different arguments:
        see `create_validation_histograms`
    """
    obj.SetTitle(title)
    # For ntuples we add the metadata as aliases ...
    try:
        obj.SetAlias("Contact", contact)
        obj.SetAlias("Description", description)
        obj.SetAlias("Check", check)
        obj.SetAlias("MetaOptions", metaoptions)
    except AttributeError:
        pass
    # for TH*, TProfile we add it to the list of functions
    try:
        function_list = obj.GetListOfFunctions()
        function_list.Add(ROOT.TNamed("Contact", contact))
        function_list.Add(ROOT.TNamed("Description", description))
        function_list.Add(ROOT.TNamed("Check", check))
        function_list.Add(ROOT.TNamed("MetaOptions", metaoptions))
    except AttributeError:
        pass

    # And maybe try to set a label on the axis
    if xlabel is not None:
        try:
            obj.GetXaxis().SetTitle(xlabel)
        except AttributeError:
            pass

    if ylabel is not None:
        try:
            obj.GetYaxis().SetTitle(ylabel)
        except AttributeError:
            pass


# noinspection PyIncorrectDocstring
def validation_metadata_update(
    rootfile: Union[str, ROOT.TFile, pathlib.PurePath], name: str, *args, **argk
) -> None:
    """
    This is a convenience helper for `validation_metadata_set` in case the
    objects have already been saved in a ROOT file before: It will open the
    file (or use an existing TFile), extract the object, add the metadata and
    save the new version to the file

    Arguments:
        rootfile (str, ROOT.TFile or pathlib.PurePath): Name of the root file
            to open or an already open TFile instance
        name (str): Name of the object in the file
        title (str): Title to use for the object
        contact (str): Contact person, usually in the form "Name <email>"
        description (str): Text description what can be seen in the plot
        check (str): Text description what to look for in the validation for
            shifters to easily see if the distribution looks ok
        xlabel (str): If given try to set this as the label for the x axis
        ylabel (str): If given try to set this as the label for the y axis
        metaoptions (str): Metaoptions (additional options to influence the
            comparison between revisions, styling of the plot, etc.)

    .. warning::

        Different ways to specify LaTeX for different arguments:
        see `create_validation_histograms`
    """

    opened = False
    if not isinstance(rootfile, ROOT.TFile):
        if isinstance(rootfile, pathlib.PurePath):
            rootfile = str(rootfile)
        rootfile = ROOT.TFile(rootfile, "UPDATE")
        opened = True
    if not rootfile.IsOpen() or not rootfile.IsWritable():
        raise RuntimeError(
            f"ROOT file {rootfile.GetName()} is not open for writing"
        )
    obj = rootfile.Get(name)
    if not obj:
        raise RuntimeError(
            f"Cannot find object named {name} in {rootfile.GetName()}"
        )
    validation_metadata_set(obj, *args, **argk)
    # scope guard to avoid side effects by changing the global gDirectory
    # in modules ...
    # noinspection PyUnusedLocal
    directory_guard = ROOT.TDirectory.TContext(rootfile)  # noqa
    obj.Write("", ROOT.TObject.kOverwrite)
    if opened:
        rootfile.Close()


class ValidationMetadataSetter(basf2.Module):
    """
    Simple module to set the valdiation metadata for a given list of objects
    automatically at the end of event processing

    Just add this module **before** any
    VariablesToNtuple/VariablesToHistogram modules and it will set the
    correct validation metadata at the end of processing

    Warning:
        The module needs to be before the modules creating the objects
        as terminate() functions are executed in reverse order from last to
        first module.  If this module is after the creation modules the metadata
        might not be set correctly
    """

    def __init__(
        self,
        variables: List[Tuple[str]],
        rootfile: Union[str, pathlib.PurePath],
        description="",
    ):
        """
        Initialize ValidationMetadataSetter

        Arguments:
            variables (list(tuple(str))): List of objects to set the metadata
                for. Each entry should be the name of an object followed by the
                metadata values which will be forwarded to
                `validation_metadata_set`:
                ``(name, title, contact, description, check, xlabel, ylabel,
                metaoptions)``
                where ``xlabel``, ``ylabel`` and ``metaoptions`` are optional
            rootfile (str or pathlib.PurePath): The name of the ROOT file where
                the objects can be found
            description (str): Common description/information of/about all plots
                in this ROOT file (will be displayed above the plots)
        """
        super().__init__()
        #: Remember the metadata
        self._variables = variables
        if isinstance(rootfile, pathlib.PurePath):
            rootfile = str(rootfile)
        #: And the name of the root file
        self._rootfile: str = rootfile
        #: Common description/information of/about all plots
        #: in this ROOT file (will be displayed above the plots)
        self._description = description
        #: Shared pointer to the root file that will be closed when the last
        #: user disconnects
        self._tfile: ROOT.TFile = None

    def initialize(self):
        """Make sure we keep the file open"""
        self._tfile = Belle2.RootFileCreationManager.getInstance().getFile(
            self._rootfile
        )

    def terminate(self):
        """And update the metadata at the end"""
        for name, *metadata in self._variables:
            name = Belle2.makeROOTCompatible(name)
            validation_metadata_update(self._tfile, name, *metadata)
        if self._description:
            file_description_set(self._tfile, self._description)
        del self._tfile


def create_validation_histograms(
    path: basf2.Path,
    rootfile: Union[str, pathlib.PurePath],
    particlelist: str,
    variables_1d: Optional[List[Tuple]] = None,
    variables_2d: Optional[List[Tuple]] = None,
    description="",
) -> None:
    """
    Create histograms for all the variables and also label them to be useful
    in validation plots in one go. This is similar to the
    `modularAnalysis.variablesToHistogram` function but also sets the
    metadata correctly to be used by the validation

    Arguments:
        path (basf2.Path): Path where to put the modules
        rootfile (str or pathlib.PurePath): Name of the output root file
        particlelist (str): Name of the particle list, can be empty for event
            dependent variables
        variables_1d: List of 1D histogram definitions of the form
            ``var, bins, min, max, title, contact, description, check_for
            [, xlabel [, ylabel [, metaoptions]]]``
        variables_2d: List of 2D histogram definitions of the form
            ``var1, bins1, min1, max1, var2, bins2, min2, max2, title, contact,
            description, check_for [, xlabel [, ylabel [, metaoptions]]]``
        description: Common description/information of/about all plots in this
            ROOT file (will be displayed above the plots)

    .. warning::

        Sadly, there are two different ways to specify latex formulas.

        1. The ROOT-style using ``#``:
           ``"Einstein-Pythagoras a^{2} + b^{2} = #frac{E}{m}"``

           This style should be used for histogram title and labels, that is the
           ``title``, ``xlabel`` and ``ylabel`` arguments

        2. The normal Latex style (with escaped backslashes or raw
           string literals):
           ``"Einstein-Pythagoras $a^2 + b^2 = \\\\frac{E}{m}$"``.

           This style should be used for all other fields like ``description``,
           ``check_for``

    You can use the normal Latex style also for histogram title and descriptions
    but the PDF version of the plots will still be buggy and not show all
    formulas correctly.
    """
    if isinstance(rootfile, pathlib.PurePath):
        rootfile = str(rootfile)
    histograms_1d = []
    histograms_2d = []
    metadata = []
    if variables_1d is not None:
        for var, nbins, vmin, vmax, *data in variables_1d:
            histograms_1d.append((var, nbins, vmin, vmax))
            metadata.append([var] + data)

    if variables_2d is not None:
        for row in variables_2d:
            var1 = row[0]
            var2 = row[4]
            histograms_2d.append(row[:8])
            metadata.append([var1 + var2] + list(row[8:]))

    path.add_module(
        ValidationMetadataSetter(metadata, rootfile, description=description)
    )
    path.add_module(
        "VariablesToHistogram",
        particleList=particlelist,
        variables=histograms_1d,
        variables_2d=histograms_2d,
        fileName=rootfile,
    )


# End suppression of doxygen checks
# @endcond
