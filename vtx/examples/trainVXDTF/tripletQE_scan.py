#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#####################################################################
# Hyperparameter scan for optimizin maxPt and materialBudget
# parameters for tripletFit QE.
#
# Needs a simulation data file that can be generated with
# validation script software/validation/validation/VTXEvtGenSim.py
#####################################################################


import b2luigi
import basf2
import luigi
from b2luigi.basf2_helper import Basf2PathTask
import os

import tracking
from tracking import add_mc_matcher
from vtx import add_vtx_reconstruction
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule


class VXDTF2Task(Basf2PathTask):
    factor = luigi.FloatParameter()
    maxP = luigi.FloatParameter()
    nevents = luigi.FloatParameter()

    def create_path(self):
        target_path = "./fixes/"
        input_file = os.path.join("./sim_data/VTXEvtGenSim.root")

        path = basf2.create_path()

        path.add_module("RootInput", inputFileName=input_file, entrySequences=[f'0:{self.nevents}'])

        gearbox = basf2.register_module('Gearbox')
        path.add_module(gearbox)

        geometry = basf2.register_module('Geometry')
        path.add_module(geometry)

        add_vtx_reconstruction(path)
        tracking.add_vtx_track_finding_vxdtf2(
            path,
            reco_tracks="RecoTracks",
            components=["VTX"],
            materialBudgetFactor=self.factor,
            maxPt=self.maxP,
            EstimationMethod='tripletFit')

        add_mc_matcher(path, components=["VTX"], reco_tracks="RecoTracks", use_second_cdc_hits=False)

        path.add_module(CombinedTrackingValidationModule(name="vtx_comb_validation_FullReco_vxdtf2",
                                                         contact="Me",
                                                         reco_tracks_name='RecoTracks',
                                                         output_file_name=self.get_output_file_name("FoM_validation.root"),
                                                         expert_level=200))

        return path

    def output(self):
        yield self.add_to_output("FoM_validation.root")


class MasterTask(luigi.WrapperTask):
    def requires(self):
        factor_list = [
             0.1, 0.5, 1.0, 1.2, 1.4, 1.6, 1.8, 2., 2.5, 3., 3.5, 4., 4.5,
             10., 15., 20., 30, 40, 50., 60.0, 100.
        ]
        maxP_list = [0.005, 0.01, 0.03, 0.05, 0.07, 0.1, 0.2, 0.4, 0.5, 0.6, 0.8, 1.]
        nevents = 10000

        for factor in factor_list:
            for maxP in maxP_list:
                yield self.clone(VXDTF2Task, factor=factor, maxP=maxP, nevents=nevents)


if __name__ == "__main__":
    b2luigi.process(MasterTask(), workers=3)
