# -*- coding: utf-8 -*-
import copy

import numbers
import collections

import array
import numpy as np

import basf2
import ROOT
from ROOT import Belle2  # make Belle2 namespace available

from tracking.modules import BrowseFileOnTerminateModule
from tracking.validation.utilities import root_cd, coroutine
from tracking.validation.refiners import Refiner

import logging


def get_logger():
    """Getter for the logger instance of thsi file."""
    return logging.getLogger(__name__)


class AttributeDict(dict):

    """Class that enables access to the dictionaries items by attribute lookup in addition to normal item lookup."""

    def __getattr__(self, name):
        """If the name is not an attribute of the object, try to look it up as an item instead."""
        return self[name]


def harvest(foreach="", pick=None, name=None, output_file_name=None):
    """Decorator to turn a function into a HarvestingModule instance.

    The decorated function becomes the peel method of the module.
    The function is invoked once for each element in the foreach storearray and should return
    a mapping of names and values extracting the relevant variables from the object in question.

    Parameters
    ----------
    foreach : string
        Name of the StoreArray or StoreObjPtr
    pick : function(obj) -> bool
        Function that gets invoked with each object in the foreach StoreArray.
        It can return a False value, if the object should not be investigated further.
    name : string
        Name used in tree abd histrogram names produced in this harvest.
    output_file_name : string
        Name of the ROOT output file to be produced.

    Returns
    -------
    function decorator
        A decorator that turns a function into HarvestingModule instance,
        which peel function is replaced by the decorated function.
        """

    def harvest_decorator(peel_func):
        name_or_default = name or peel_func.__name__
        output_file_name_or_default = output_file_name or "%s.root" % name
        harvesting_module = HarvestingModule(foreach=foreach,
                                             output_file_name=output_file_name_or_default,
                                             name=name_or_default)
        harvesting_module.peel = peel_func
        if pick:
            harvesting_module.pick = pick
        return harvesting_module
    return harvest_decorator


class HarvestingModule(basf2.Module):

    """Python module to generate summary figures of merits, plots and/or trees
    from on StoreArray.

    It runs as a proper module in the main path and examines each object in a StoreArray
    in each event. Each


    Notes
    -----
    Methods to be overwritten
    ``prepare``
        Method called at the start of each event, that may prepare things (e.g. setup lookup tables or precomputed list)
        used in the following methods.
    ``pick``
        Method called with each object in the StoreArray. Returns a False value if the object should be skipped.
    ``peel``
        Method called with each object in the StoreArray. Extractes the parts relevant for analysis and
        returns them as MutableMapping (e.g. a dict) of part_name and values. Currently only float values
        or values convertable to floats are supported. If requested that can change in the future.

    On termination all the collected values are recasted to numpy arrays and the whole ``crops`` of the harvest
    are casted to MutableMapping of numpy.array with the same part_name and the same MutableMapping class
    as returned from peel.

    Also in the termination phase refiners a invoked with the aggregated crops.
    Refiners can be given in two ways.

    First way is as a class methods marked as refiners like

        @refiners.Refiner
        def plot(self, crops, tdirectory, **kwds):
            ...

    where self is the module instance, crops is the MutableMapping of numpy arrays and tdirectory is the current tdirectory to which
    the current output shall be written. The additional kwds leave room for future additional arguments.

    Second way is to define the refiner method (like plot) out of line and add it the the harvesting module instance refiners list like
    harvesting_module.refiners.append(plot).


    Other specialised decorators to mark a function as a Refiner such as

    * refiners.filter
    * refiners.select
    * refiners.groupby

    exist.

    Predefined refiner function exist in the refiners python module.
    For instance

        save_tree = refiners.save_tree()

    is a predefined method to output the MutableMapping of numpy arrays as a TTree.

    Examples
    --------
    You find examples of concrete HarvestingModules in
    * tracking/scripts/tracking/validation/mc_side_module.py
    * tracking/scripts/tracking/validation/pr_side_module.py
    * tracking/scripts/tracking/validation/eventwise_side_module.py
    * tracking/trackFindingCDC/scripts/trackfindingcdc/validation/segmentFitValidation.py
    * tracking/trackFindingCDC/scripts/trackfindingcdc/validation/segmentPairFitValidation.py


    Parameters
    ----------
    foreach : string
        Name of a StoreArray, which objects should be investigated
    output_file_name : string
        Name of the ROOT file to which the results should be written
    name : string, optional
        Name of the harvest that is used in the names of ROOT plots and trees. Defaults to the class name.
    title : string, optional
        Name of the harvest that is used in the title of ROOT plots and trees. Defaults to the name.
    contact : string, optional
        Contract email adress to be used in the validation plots contract. Defaults to None.
    expert_level : int, optional
        Expert level that can be used to switch on more plots. Generally the higher the more detailed to analysis.
        Meaning depends entirely on the subclass implementing a certain policy. Defaults to default_expert_level.

    Attributes
    ----------
    refiners : list of Refiners.
        A list of additional refiners of the instance to be invoked on termination of the module.
    """

    default_expert_level = 1

    def __init__(self,
                 foreach,
                 output_file_name,
                 name=None,
                 title=None,
                 contact=None,
                 expert_level=None):

        super(HarvestingModule, self).__init__()
        self.foreach = foreach

        self.output_file_name = output_file_name
        if not isinstance(self.output_file_name, (ROOT.TFile, basestring)):
            raise TypeError("output_file_name is allowed to be a string or a ROOT.TFile object")

        self.name = name or self.__class__.__name__
        self.title = title or self.name
        self.contact = contact
        self.expert_level = self.default_expert_level if expert_level is None else expert_level

        self.refiners = []

    def run(self,
            input_root_file,
            gearbox=False,
            geometry=False,
            components=False,
            show=True):

        main_path = basf2.create_path()

        root_input_module = basf2.register_module('RootInput')
        root_input_module.param({
            'inputFileName': input_root_file,
        })
        main_path.add_module(root_input_module)

        progress_module = basf2.register_module('Progress')
        main_path.add_module(progress_module)

        if gearbox:
            gearbox_module = basf2.register_module('Gearbox')
            main_path.add_module(gearbox_module)
            if geometry:
                geometry_module = basf2.register_module('Geometry')
                if components:
                    geometry_module.param('components', components)
                main_path.add_module(geometry_module)

        # Display the root file on terminate
        if show and self.output_file_name:
            main_path.add_module(BrowseFileOnTerminateModule(self.output_file_name))

        main_path.add_module(self)
        get_logger().info("Processing...")
        basf2.process(main_path)

    def initialize(self):
        if isinstance(self.output_file_name, basestring) and self.output_file_name.startswith("datastore://"):
            # Check if the tfile was registered on the datastore before
            datastore_output_file_name = self.output_file_name[len("datastore://"):]
            persistent = 1
            stored_tfile = Belle2.PyStoreObj(datastore_output_file_name, persistent)
            if not stored_tfile:
                raise KeyError("No TFile with name %s created on the DataStore." % datastore_output_file_name)

        # prepare the barn to receive the harvested crops
        self.stash = self.barn()

    def event(self):
        self.prepare()
        stash = self.stash.send
        pick = self.pick
        peel = self.peel
        for crop in self.gather():
            if pick(crop):
                crop = peel(crop)
                stash(crop)

    def terminate(self):
        self.stash.close()
        del self.stash
        self.refine(self.crops)

    @staticmethod
    def create_crop_part_collection():
        return array.array("d")

    @coroutine
    def barn(self):
        crop = (yield)
        raw_crops = copy.copy(crop)
        crops = copy.copy(crop)

        if isinstance(crop, numbers.Number):
            raw_crops = self.create_crop_part_collection()
            try:
                while True:
                    raw_crops.append(crop)
                    # next crop
                    crop = (yield)
            except GeneratorExit:
                crops = np.array(raw_crops)

        elif isinstance(crop, collections.MutableMapping):
            for part_name in crop:
                raw_crops[part_name] = self.create_crop_part_collection()

            try:
                while True:
                    for part_name, part in crop.items():
                        raw_crops[part_name].append(part)
                    # next crop
                    crop = (yield)
            except GeneratorExit:
                for part_name, parts in raw_crops.items():
                    crops[part_name] = np.array(parts)

        else:
            raise ValueError("Unrecognised crop %s of type %s" % (crop, type(crop)))

        self.raw_crops = raw_crops
        self.crops = crops

    def gather(self):
        registered_store_arrays = Belle2.PyStoreArray.list()
        registered_store_objs = Belle2.PyStoreObj.list()

        foreach = self.foreach
        foreach_is_store_obj = foreach in registered_store_objs
        foreach_is_store_array = foreach in registered_store_arrays

        if foreach is not None:
            if foreach_is_store_array:
                store_array = Belle2.PyStoreArray(self.foreach)
                for crop in store_array:
                    yield crop

            elif foreach_is_store_obj:
                store_obj = Belle2.PyStoreObj(self.foreach)
                try:
                    for crop in self.iter_store_obj(store_obj):
                        yield crop
                except TypeError:
                    # Cannot iter the store object. Yield it instead.
                    yield store_obj.obj()

            else:
                raise KeyError("Name %s does not refer to a valid object on the data store" % self.foreach)
        else:
            yield None

    def prepare(self):
        return

    def peel(self, crop):
        return crop

    def pick(self, crop):
        return True

    def refine(self, crops):
        kwds = {}
        if self.output_file_name:
            # Save everything to a ROOT file
            if isinstance(self.output_file_name, ROOT.TFile):
                output_tdirectory = self.output_file_name
            elif self.output_file_name.startswith("datastore://"):
                # Check if the tfile was registered on the datastore before
                datastore_output_file_name = self.output_file_name[len("datastore://"):]
                persistent = 1
                stored_tfile = Belle2.PyStoreObj(datastore_output_file_name, persistent)
                if not stored_tfile:
                    raise KeyError("No TFile with name %s created on the DataStore." % datastore_output_file_name)
                output_tdirectory = stored_tfile.obj()
            else:
                output_tfile = ROOT.TFile(self.output_file_name, 'recreate')
                output_tdirectory = output_tfile

        else:
            output_tdirectory = None

        try:
            with root_cd(output_tdirectory) as tdirectory:
                for refiner in self.refiners:
                    refiner(self, crops, tdirectory=output_tdirectory, **kwds)

                # Get the methods marked as refiners from the class
                cls = type(self)
                for name in dir(cls):
                    if isinstance(getattr(cls, name), Refiner):
                        refiner = getattr(self, name)
                        # Getattr already binds self
                        refiner(crops, tdirectory=output_tdirectory, **kwds)

        finally:
            # If we opened the TFile ourself, close it again
            if self.output_file_name:
                if isinstance(self.output_file_name, basestring) and not self.output_file_name.startswith("datastore://"):
                    output_tfile.Close()

    @staticmethod
    def iter_store_obj(store_obj):
        iterable = store_obj.obj()
        last_iterable = None
        while iterable is not last_iterable:
            if hasattr(iterable, "__iter__"):
                iterable, last_iterable = iterable.__iter__(), iterable
            else:
                iterable, last_iterable = iter(iterable), iterable
        return iterable


def test():
    """Test a quick analysis of the MCParticles in generic events."""
    from tracking.validation.utilities import is_primary, is_stable_in_generator
    from tracking.validation.refiners import save_histograms, save_tree, save_fom

    # Proposed syntax for quick generation of overview plots
    @save_fom(aggregation=np.mean, select=["energy", "pt"], name="physics", key="mean_{part_name}")
    @save_histograms(outlier_z_score=5.0, allow_discrete=True, filter=lambda xs: xs != 0.0, filter_on="is_secondary", select=["pt", "is_secondary"], folder_name="secondary_pt")
    @save_histograms(outlier_z_score=5.0, allow_discrete=True, groupby="status", select=["is_secondary", "pt"])
    @save_histograms(outlier_z_score=5.0, allow_discrete=True, select=["is_secondary", "pt"], stackby="is_secondary", folder_name="pt_stackby_is_secondary")
    @save_histograms(outlier_z_score=5.0, allow_discrete=True)
    @save_tree()
    @harvest(foreach="MCParticles", pick=lambda mc_particle: not mc_particle.hasStatus(Belle2.MCParticle.c_IsVirtual), output_file_name="MCParticleOverview.root")
    def MCParticleOverview(mc_particle):
        momentum_tvector3 = mc_particle.getMomentum()
        pdg_code = mc_particle.getPDG()
        secondary_process = mc_particle.getSecondaryPhysicsProcess()

        return AttributeDict(
            tan_lambda=np.divide(1.0, np.tan(momentum_tvector3.Theta())),  # Save divide not throwing an ZeroDivisionError
            pt=momentum_tvector3.Pt(),
            secondary_process=secondary_process,
            is_secondary=secondary_process != 0,
            mass=mc_particle.getMass(),
            status=mc_particle.getStatus(),
            pdg_mass=ROOT.TDatabasePDG.Instance().GetParticle(pdg_code).Mass(),
            energy=mc_particle.getEnergy(),
            pdg_code=pdg_code,
        )

    MCParticleOverview.run("generic1000.root")


if __name__ == "__main__":
    test()
