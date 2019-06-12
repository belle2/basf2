#!/usr/bin/env python3

"""
Small module containing helper functions to set the metadata on objects created for the validation correctly
"""


import basf2
import ROOT
from ROOT import Belle2
# circumvent BII-1264
ROOT.gInterpreter.Declare("#include <framework/utilities/MakeROOTCompatible.h>")


def validation_metadata_set(obj, title, contact, description, check, xlabel=None, ylabel=None):
    """
    Set the validation metadata for a given object by setting the necessary values.
    This function can be used on any object supported by the Validation (histograms, profiles, ntuples)

    Arguments:
        obj: Instance of the object which should get the metadata
        title (str): Title to use for the object
        contact (str): Contact person, usually in the form "Name <email>"
        description (str): Text description what can be seen in the plot
        check (str): Text description what to look for in the validation for
            shifters to easily see if the distribution looks ok
        xlabel (str): If given try to set this as the label for the x axis
        ylabel (str): If given try to set this as the label for the y axis
    """
    obj.SetTitle(title)
    # For ntuples we add the metadata as aliases ...
    try:
        obj.SetAlias("Contact", contact)
        obj.SetAlias("Description", description)
        obj.SetAlias("Check", check)
    except AttributeError:
        pass
    # for TH*, TProfile we add it to the list of functions
    try:
        function_list = obj.GetListOfFunctions()
        function_list.Add(ROOT.TNamed("Contact", contact))
        function_list.Add(ROOT.TNamed("Description", description))
        function_list.Add(ROOT.TNamed("Check", check))
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


def validation_metadata_update(rootfile, name, *args, **argk):
    """
    This is a convenience helper for `validation_metadata_set` in case the objects
    have already been saved in a ROOT file before: It will open the file (or use
    an existing TFile), extract the object, add the metadata and save the new
    version to the file

    Arguments:
        rootfile (str or ROOT.TFile): Name of the root file to open or an already
            open TFile instance
        name (str): Name of the object in the file
        title (str): Title to use for the object
        contact (str): Contact person, usually in the form "Name <email>"
        description (str): Text description what can be seen in the plot
        check (str): Text description what to look for in the validation for
            shifters to easily see if the distribution looks ok
        xlabel (str): If given try to set this as the label for the x axis
        ylabel (str): If given try to set this as the label for the y axis
    """

    opened = False
    if not isinstance(rootfile, ROOT.TFile):
        rootfile = ROOT.TFile(rootfile, "UPDATE")
        opened = True
    if not rootfile.IsOpen() or not rootfile.IsWritable():
        raise RuntimeError(f"ROOT file {rootfile.GetName()} is not open for writing")
    obj = rootfile.Get(name)
    if not obj:
        raise RuntimeError(f"Cannot find object named {name} in {rootfile.GetName()}")
    validation_metadata_set(obj, *args, **argk)
    directoryGuard = ROOT.TDirectory.TContext(rootfile)
    obj.Write("", ROOT.TObject.kOverwrite)
    if opened:
        rootfile.Close()


class ValidationMetadataSetter(basf2.Module):
    """
    Simple module to set the valdiation metadata for a given list of objects
    automatically at the end of event processing

    Just add this module **before** any VariablesToNtuple/VariablesToHistogram
    modules and it will set the correct validation metadata at the end of processing

    Warning:
        The module needs to be before the modules creating the objects as terminate()
        functions are executed in reverse order from last to first module.  If this
        module is after the creation modules the metadata might not be set correctly
    """

    def __init__(self, variables, rootfile):
        """

        Arguments:
            variables (list(tuple(str))): List of objects to set the metadata for.
                Each entry should be the name of an object followed by the metadata
                values which will be forwarded to `validation_metadata_set`:
                ``(name, title, contact, description, check, xlabel, ylabel)``
                where ``xlabel`` and ``ylabel`` are optional
            rootfile (str): The name of the ROOT file where the objects can be found
        """
        super().__init__()
        #: Remember the metadata
        self._variables = variables
        #: And the name of the root file
        self._rootfile = rootfile

    def initialize(self):
        """Make sure we keep the file open"""
        #: Shared pointer to the root file that will be closed when the last user disconnects
        self._tfile = Belle2.RootFileCreationManager.getInstance().getFile(self._rootfile)

    def terminate(self):
        """And update the metadata at the end"""
        for name, *metadata in self._variables:
            name = Belle2.makeROOTCompatible(name)
            validation_metadata_update(self._tfile.get(), name, *metadata)
        self._tfile.reset()


def create_validation_histograms(path, rootfile, particlelist, variables_1d=None, variables_2d=None):
    """
    Create histograms for all the variables and also label them to be useful in validation plots in one go.
    This is similar to the `modularAnalysis.variablesToHistogram` function but also sets the metadata correctly to be
    used by the validation

    Arguments:
        path (basf2.Path): Path where to put the modules
        rootfile (str): Name of the output root file
        particlelist (str): Name of the particle list, can be empty for event dependent variables
        variables_1d: List of 1D histogram definitions of the form
            ``var, bins, min, max, title, contact, description, check_for [, xlabel [,ylabel]]``
        variables_2d: List of 2D histogram definitions of the form
            ``var1, bins1, min1, max1, var2, bins2, min2, max2, title, contact, description, check_for [, xlabel [,ylabel]]``
    """

    histograms_1d = []
    histograms_2d = []
    metadata = []
    if variables_1d is not None:
        for var, nbins, vmin, vmax, *data in variables_1d:
            histograms_1d.append((var, nbins, vmin, vmax))
            metadata.append([var]+data)

    if variables_2d is not None:
        for row in variables_2d:
            var1 = row[0]
            var2 = row[4]
            histograms_2d.append(row[:8])
            metadata.append([var1 + var2] + list(row[8:]))

    path.add_module(ValidationMetadataSetter(metadata, rootfile))
    path.add_module("VariablesToHistogram", particleList=particlelist, variables=histograms_1d,
                    variables_2d=histograms_2d, fileName=rootfile)
