# -*- coding: utf-8 -*-
import copy

import numbers
import collections

import array
import numpy as np
import types
import functools

import basf2
import ROOT
from ROOT import Belle2  # make Belle2 namespace available

from .refiners import Refiner
from tracking.root_utils import root_cd, root_browse

import logging


def get_logger():
    """Getter for the logger instance of this file."""
    return logging.getLogger(__name__)


def coroutine(generator_func):
    """Famous coroutine decorator.

    Starts a receiving generator function to the first yield,
    such that it can receive a send call immediatly.
    """

    @functools.wraps(generator_func)
    def start(*args, **kwargs):
        cr = generator_func(*args, **kwargs)
        next(cr)
        return cr
    return start


def harvest(foreach="", pick=None, name=None, output_file_name=None, show_results=False):
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

    Notes
    -----
    An example of the usage pattern can be found at the end of this file
    """

    def harvest_decorator(peel_func):
        name_or_default = name or peel_func.__name__
        output_file_name_or_default = output_file_name or "{}.root".format(name_or_default)
        harvesting_module = HarvestingModule(foreach=foreach,
                                             output_file_name=output_file_name_or_default,
                                             name=name_or_default,
                                             show_results=show_results)
        harvesting_module.peel = peel_func
        if pick:
            harvesting_module.pick = pick
        return harvesting_module
    return harvest_decorator


class HarvestingModule(basf2.Module):

    """Python module to generate summary figures of merits, plots and/or trees
    from on StoreArray.

    It runs as a proper module in the main path and examines each object in a StoreArray
    in each event.


    Notes
    -----
    Methods to be overwritten
    ``prepare``
        Method called at the start of each event, that may prepare things
        (e.g. setup lookup tables or precomputed list) used in the following methods.
    ``pick``
        Method called with each object in the StoreArray.
        Returns a False value if the object should be skipped.

    ``peel``
        Method called with each object in the StoreArray.
        Extractes the parts relevant for analysis and
        returns them as MutableMapping (e.g. a dict) of part_name and values.
        Currently only float values or values convertable to floats are supported.
        If requested that can change in the future.

    On termination all the collected values are recasted to numpy arrays and
    the whole ``crops`` of the harvest are casted to MutableMapping of numpy.array
    with the same part_name and the same MutableMapping class as returned from peel.

    Also in the termination phase refiners a invoked with the aggregated crops.
    Refiners can be given in two ways.

    First way is as a class methods marked as refiners like

        @refiners.Refiner
        def plot(self, crops, tdirectory, **kwds):
            ...

    where self is the module instance, crops is the MutableMapping of numpy arrays and tdirectory
    is the current tdirectory to which the current output shall be written.
    The additional kwds leave room for future additional arguments.

    Second way is to define the refiner method (like plot) out of line and add it to
    the harvesting module instance refiners list like harvesting_module.refiners.append(plot).

    Other specialised decorators to mark a function as a Refiner such as

    * refiners.filter
    * refiners.select
    * refiners.groupby

    exist.

    Predefined refiner functions exist in the refiners python module as well.
    For instance

        save_tree = refiners.save_tree()

    is a predefined method to output the MutableMapping of numpy arrays as a TTree.
    """

    default_expert_level = 1

    def __init__(self,
                 foreach,
                 output_file_name,
                 name=None,
                 title=None,
                 contact=None,
                 expert_level=None,
                 show_results=False):
        """Constructor of the harvesting module.

        Parameters
        ----------
        foreach : string
            Name of a StoreArray, which objects should be investigated
        output_file_name : string
            Name of the ROOT file to which the results should be written.
            Giving an opened ROOT file is also allowed.
            If None is given write to the current ROOT file.
        name : string, optional
            Name of the harvest that is used in the names of ROOT plots and trees.
            Defaults to the class name.
        title : string, optional
            Name of the harvest that is used in the title of ROOT plots and trees.
            Defaults to the name.
        contact : string, optional
            Contact email adress to be used in the validation plots contact. Defaults to None.
        expert_level : int, optional
            Expert level that can be used to switch on more plots.
            Generally the higher the more detailed to analysis.
            Meaning depends entirely on the subclass implementing a certain policy.
            Defaults to default_expert_level.
        show_results : bool, optional
           Indicator to show the refined results at termination of the path
           """

        super().__init__()

        #: Name of the StoreArray or iterable StoreObjPtr that contains the objects to be harvested
        self.foreach = foreach

        #: Name of the ROOT output file to be generated
        self.output_file_name = output_file_name

        if not isinstance(self.output_file_name, (ROOT.TFile, str)):
            raise TypeError("output_file_name is allowed to be a string or a ROOT.TFile object")

        #: Name of this harvest
        self.set_name(name or self.__class__.__name__)

        #: Title particle of this harvest
        self.title = title or self.name()

        #: Contact email address to be displayed on the validation page
        self.contact = contact

        #: Integer expert level that controlls to detail of plots to be generated
        self.expert_level = self.default_expert_level if expert_level is None else expert_level

        #: A list of additional refiner instances to be executed
        #: on top of the refiner methods that are members of this class
        self.refiners = []

        #: Switch to show the result ROOT file in a TBrowser on terminate
        self.show_results = show_results

    @property
    def id(self):
        """Working around that name() is a method.

        Exposing the name as a property using a different name
        """
        return self.name()

    def initialize(self):
        """Initialisation method of the module.

        Prepares the receiver stash of objects to be harvestered.
        """
        self.stash = self.barn()

    def event(self):
        """Event method of the module

        * Does invoke the prepare method before the iteration starts.
        * In each event fetch the StoreArray / iterable StoreObjPtr,
        * Iterate through all instances
        * Feed each instance to the pick method to deside it the instance is relevant
        * Forward it to the peel method that should generated a dictionary of values
        * Store each dictionary of values
        """
        self.prepare()
        stash = self.stash.send
        pick = self.pick
        peel = self.peel
        for crop in self.gather():
            if pick(crop):
                crop = peel(crop)
                if isinstance(crop, types.GeneratorType):
                    many_crops = crop
                    for crop in many_crops:
                        stash(crop)
                else:
                    stash(crop)

    def terminate(self):
        """Termination method of the module.

        Finalize the collected crops.
        Start the refinement.
        """

        self.stash.close()
        del self.stash

        try:
            self.refine(self.crops)
        except AttributeError:
            pass

    @staticmethod
    def create_crop_part_collection():
        """Create the storing objects for the crop values

        Currently a numpy.array of doubles is used to store all values in memory.
        """
        return array.array("d")

    @coroutine
    def barn(self):
        """Coroutine that receives the dictionaries of names and values from peel and store them."""
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
                    for part_name, parts in list(raw_crops.items()):
                        if part_name in crop:
                            parts.append(crop[part_name])
                        else:
                            parts.append(np.NaN)
                    # next crop
                    crop = (yield)
            except GeneratorExit:
                for part_name, parts in list(raw_crops.items()):
                    crops[part_name] = np.array(parts)

        else:
            msg = "Unrecognised crop {} of type {}".format(
                crop,
                type(crop)
            )
            raise ValueError(msg)

        self.raw_crops = raw_crops
        self.crops = crops

    def gather(self):
        """Iterator that yield the instances form the StoreArray / iterable StoreObj.

        Yields
        ------
        Object instances from the StoreArray, iterable StoreObj or the StoreObj itself
        in case it is not iterable.
        """

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
                msg = "Name {} does not refer to a valid object on the data store".format(
                    self.foreach
                )
                raise KeyError(msg)
        else:
            yield None

    def prepare(self):
        """Default implementation of prepare.

        Can be overridden by subclasses.
        """
        return

    def peel(self, crop):
        """Unpack the the instances and return and dictionary of names to values or
        a generator of those dictionaries to be saved.

        Returns
        -------
        dict(str -> float)
            Unpacked names and values
        or

        Yields
        ------
        dict(str -> float)
            Unpacked names and values

        """
        return {"name": np.nan}

    def pick(self, crop):
        """Indicate whether the instance should be forwarded to the peeling

        Returns
        -------
        bool : Indicator if the instance is valueable in the current harverst.
        """
        return True

    def refine(self, crops):
        """Receive the gathered crops and forward them to the refiners."""

        kwds = {}
        if self.output_file_name:
            # Save everything to a ROOT file
            if isinstance(self.output_file_name, ROOT.TFile):
                output_tdirectory = self.output_file_name
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
                if isinstance(self.output_file_name, str):
                    output_tfile.Close()

        if self.show_results and self.output_file_name:
            if isinstance(self.output_file_name, str):
                output_tfile = ROOT.TFile(self.output_file_name)
                root_browse(output_tfile)
                input("Press enter to close")
                output_tfile.Close()
            else:
                root_browse(self.output_file_name)
                input("Press enter to close")

    @staticmethod
    def iter_store_obj(store_obj):
        """Obtain a iterator from a StoreObj

        Repeatly calls iter(store_obj) or store_obj.__iter__()
        until the final iterator returns itself

        Returns
        -------
        iterator of the StoreObj
        """
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
    from .refiners import save_histograms, save_tree, save_fom

    def primaries_seen_in_detector(mc_particle):
        return (mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle) and
                mc_particle.hasStatus(Belle2.MCParticle.c_StableInGenerator) and
                not mc_particle.hasStatus(Belle2.MCParticle.c_IsVirtual) and
                (mc_particle.hasStatus(Belle2.MCParticle.c_LeftDetector) or
                 mc_particle.hasStatus(Belle2.MCParticle.c_StoppedInDetector)))

    # Proposed syntax for quick generation of overview plots
    @save_fom(aggregation=np.mean, select=["energy", "pt"], name="physics", key="mean_{part_name}")
    @save_histograms(outlier_z_score=5.0,
                     allow_discrete=True,
                     filter=lambda xs: xs != 0.0,
                     filter_on="is_secondary",
                     select=["pt", "is_secondary"],
                     folder_name="secondary_pt")
    @save_histograms(outlier_z_score=5.0,
                     allow_discrete=True,
                     groupby="status",
                     select=["is_secondary", "pt"])
    @save_histograms(outlier_z_score=5.0,
                     allow_discrete=True,
                     select=["is_secondary", "pt"],
                     stackby="is_secondary",
                     folder_name="pt_stackby_is_secondary/nested_test")
    @save_histograms(outlier_z_score=5.0,
                     allow_discrete=True,
                     select={'pt': '$p_t$'},
                     title="Distribution of p_{t}")
    @save_tree()
    @harvest(foreach="MCParticles",
             pick=primaries_seen_in_detector,
             output_file_name="MCParticleOverview.root")
    def MCParticleOverview(mc_particle):
        momentum_tvector3 = mc_particle.getMomentum()
        pdg_code = mc_particle.getPDG()
        secondary_process = mc_particle.getSecondaryPhysicsProcess()

        return dict(
            # Save divide not throwing an ZeroDivisionError
            tan_lambda=np.divide(1.0, np.tan(momentum_tvector3.Theta())),
            pt=momentum_tvector3.Pt(),
            secondary_process=secondary_process,
            is_secondary=secondary_process != 0,
            mass=mc_particle.getMass(),
            status=mc_particle.getStatus(),
            pdg_mass=ROOT.TDatabasePDG.Instance().GetParticle(pdg_code).Mass(),
            energy=mc_particle.getEnergy(),
            pdg_code=pdg_code,
        )

    from .run import HarvestingRun

    class ExampleHarvestingRun(HarvestingRun):
        n_events = 100

        def harvesting_module(self):
            return MCParticleOverview

    ExampleHarvestingRun().configure_and_execute_from_commandline()

if __name__ == "__main__":
    test()
