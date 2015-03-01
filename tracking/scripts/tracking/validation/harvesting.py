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
    return logging.getLogger(__name__)


def test():
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


class AttributeDict(dict):

    """Class that enables access to the dictionaries items by attribute lookup in addition to normal item lookup."""

    def __getattr__(self, name):
        return self[name]


def harvest(foreach="", pick=None, name=None, output_file_name=None):
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
    default_expert_level = 1

    def __init__(self,
                 foreach,
                 output_file_name=None,
                 name=None,
                 title=None,
                 contact=None,
                 expert_level=None):

        super(HarvestingModule, self).__init__()
        self.foreach = foreach
        self.output_file_name = output_file_name
        self.refiners = []
        self.contact = contact
        self.expert_level = self.default_expert_level if expert_level is None else expert_level

        self.name = name or self.__class__.__name__

        self.title = title or self.name

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

        elif isinstance(crop, collections.Sequence):
            for i_part, part in enumerate(crop):
                raw_crops[i_part] = self.create_crop_part_collection()

            try:
                while True:
                    for part, parts in zip(crop, raw_crops):
                        parts.append(part)
                    # next crop
                    crop = (yield)

            except GeneratorExit:
                for i_part, parts in enumerate(raw_crops):
                    crops[i_part] = np.array(parts)
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
                    for crop in iter_store_obj(store_obj):
                        yield crop
                except TypeError:
                    # Cannot iter the store object. Yield it instead.
                    yield storeobj.obj()

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
            else:
                output_tfile = ROOT.TFile(self.output_file_name, 'recreate')
                output_tdirectory = output_tfile
        else:
            output_tdirectory = None

        try:
            for refiner in self.refiners:
                refiner(self, crops, tdirectory=output_tdirectory, **kwds)

            # Get the methods marked as refiners from the class
            cls = type(self)
            for name in dir(cls):
                if isinstance(getattr(cls, name), Refiner):
                    refiner = getattr(self, name)
                    refiner(crops, tdirectory=output_tdirectory, **kwds)

        finally:
            # If we opened the TFile ourselves, close it again
            if self.output_file_name:
                if not isinstance(self.output_file_name, ROOT.TFile):
                    output_tfile.Close()


def iter_store_obj(store_obj):
    iterable = store_obj.obj()
    last_iterable = None
    while iterable is not last_iterable:
        if hasattr(iterable, "__iter__"):
            iterable, last_iterable = iterable.__iter__(), iterable
        else:
            iterable, last_iterable = iter(iterable), iterable
    return iterable


if __name__ == "__main__":
    test()
