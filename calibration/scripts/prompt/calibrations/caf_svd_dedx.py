#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
Script to perform the SVD dE/dx calibration
'''
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
import basf2 as b2
from ROOT.Belle2 import SVDdEdxCalibrationAlgorithm, SVDdEdxValidationAlgorithm

import modularAnalysis as ma
import vertex as vx
import reconstruction as re

settings = CalibrationSettings(
    name="caf_svd_dedx",
    expert_username="lisovsky",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["hadron_calib"],
    input_data_filters={"hadron_calib": [INPUT_DATA_FILTERS["Data Tag"]["hadron_calib"],
                                         INPUT_DATA_FILTERS["Beam Energy"]["4S"],
                                         INPUT_DATA_FILTERS["Beam Energy"]["Continuum"],
                                         INPUT_DATA_FILTERS["Run Type"]["physics"],
                                         INPUT_DATA_FILTERS["Magnet"]["On"]]},

    expert_config={
        "isMC": False,
        "listOfMutedCalibrations": [],  # dEdxCalibration, dEdxValidation
        "rerun_reco": False,  # need to rerun reconstruction for calibration?
        "rerun_reco_val": True,  # need to rerun reconstruction for validation?
        "validation_mode": "basic",  # full or basic; full also produces global PID performance plots
        "MaxFilesPerRun": 10,  # 15,
        "MaxFilesPerRunValidation": 6,  # be careful in MC to not exclude certain event types
        "MinEvtsPerFile": 1,
        "MaxEvtsPerFile": 20000,  # only if rerun the reco, to prevent jobs >10h
        "MinEvtsPerTree": 100,
        "NBinsP": 69,
        "NBinsdEdx": 100,
        "dedxCutoff": 5.e6,
        "NumROCpoints": 175,
        "MinROCMomentum": 0.,
        "MaxROCMomentum": 2.5,
        "NumEffBins": 30,
        "MaxEffMomentum": 2.5
        },
    depends_on=[])


def create_path(rerun_reco, isMC, expert_config):
    rec_path = b2.Path()

    # expert_config = kwargs.get("expert_config")
    max_events_per_file = expert_config["MaxEvtsPerFile"]

    if rerun_reco:
        rec_path.add_module(
            'RootInput',
            branchNames=[
                'RawARICHs',
                'RawCDCs',
                'RawECLs',
                'RawFTSWs',
                'RawKLMs',
                'RawPXDs',
                'RawSVDs',
                'RawTOPs',
                'RawTRGs',
                'RawDataBlock',
                'RawCOPPER'],
            entrySequences=[f'0:{max_events_per_file - 1}'],
            logLevel=b2.LogLevel.ERROR)
        if not isMC:
            re.add_unpackers(path=rec_path)
        else:
            rec_path.add_module("Gearbox")
            rec_path.add_module("Geometry")

        re.add_reconstruction(path=rec_path, pruneTracks=False)
        rec_path.add_module('VXDDedxPID')
    else:
        rec_path.add_module('RootInput')

    # Fill particle lists
    ma.fillParticleList("pi+:all", "", path=rec_path)
    ma.fillParticleList("pi+:lambda", "nCDCHits > 0", path=rec_path)  # pi without track quality for reconstructing lambda
    ma.fillParticleList("pi+:cut", "abs(dr) < 0.5 and abs(dz) < 2 and pValue > 0.00001 and nSVDHits > 1",
                        path=rec_path)  # pions for reconstructing D and Dstar

    ma.fillParticleList('K-:cut', cut='abs(dr) < 0.5 and abs(dz) < 2 and pValue > 0.00001 and nSVDHits > 1', path=rec_path)  # kaon
    ma.fillParticleList('e+:cut', cut='nSVDHits > 0', path=rec_path)  # electron
    # proton. In data, we only see background at p<0.25 GeV which motivates adding this cut.
    ma.fillParticleList('p+:lambda', cut='nCDCHits > 0 and nSVDHits > 0 and p > 0.25', path=rec_path)
    # ----------------------------------------------------------------------------
    # Reconstruct D*(D0->K-pi+)pi+ and cc.
    ma.reconstructDecay(decayString='D0:kpi -> K-:cut pi+:cut', cut='1.7 < M < 2.', path=rec_path)
    ma.reconstructDecay(
        decayString='D*+:myDstar -> D0:kpi pi+:all',
        cut='1.95 < M <2.05 and massDifference(0) < 0.16',
        path=rec_path)

    # Reconstruct Lambda->p+pi- and cc.
    ma.reconstructDecay('Lambda0:myLambda -> p+:lambda pi-:lambda', '1.1 < M < 1.3', path=rec_path)

    # Reconstruct gamma->e+e- (photon conversion)
    ma.reconstructDecay('gamma:myGamma -> e+:cut e-:cut', '0.0 < M < 0.5', path=rec_path)

    # ----------------------------------------------------------------------------
    # vertex fits
    vx.treeFit(list_name='D*+:myDstar', conf_level=0, ipConstraint=True, updateAllDaughters=True, path=rec_path)
    vx.treeFit(list_name='Lambda0:myLambda', conf_level=0, ipConstraint=True, updateAllDaughters=True, path=rec_path)
    vx.treeFit(list_name='gamma:myGamma', conf_level=0, path=rec_path)

    # ----------------------------------------------------------------------------
    # Final selections on Lambda:
    # - a tighter InvM range
    # - a good vertex quality and a displacement requirement
    # - a kinematic requirement p(p)>p(pi) which should always be satisfied in a true Lambda decay
    # - a veto on the misidentified converted photons using the convertedPhotonInvariantMass tool, m(ee)>0.02 GeV
    # - a veto on the misidentified K0S->pipi decays, vetoeing the region 0.488<m(pipi)<0.513 GeV

    ma.cutAndCopyList(
        outputListName='Lambda0:cut',
        inputListName='Lambda0:myLambda',
        cut=(
            "1.10 < InvM < 1.13 and chiProb > 0.001 and distance>1.0 and "
            "formula(daughter(0,p)) > formula(daughter(1,p)) and convertedPhotonInvariantMass(0,1) > 0.02 and "
            "[[formula((((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+"
            "daughter(1, E))*((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+"
            "daughter(1, E))-(daughter(0, px)+daughter(1, px))*(daughter(0, px)+daughter(1, px))-(daughter(0, py)+"
            "daughter(1, py))*(daughter(0, py)+daughter(1, py))-(daughter(0, pz)+daughter(1, pz))*(daughter(0, pz)+"
            "daughter(1, pz)))**0.5) < 0.488]"
            "or [formula((((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+"
            "daughter(1, E))*((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+"
            "daughter(1, E))-(daughter(0, px)+daughter(1, px))*(daughter(0, px)+daughter(1, px))-(daughter(0, py)+"
            "daughter(1, py))*(daughter(0, py)+daughter(1, py))-(daughter(0, pz)+daughter(1, pz))*(daughter(0, pz)+"
            "daughter(1, pz)))**0.5) > 0.513]]"
        ),
        path=rec_path)

    # ----------------------------------------------------------------------------
    # Selections on Dstar
    # - a tighter InvM and deltaM range
    # - a good vertex quality

    ma.cutAndCopyList(
        outputListName='D*+:cut',
        inputListName='D*+:myDstar',
        cut='massDifference(0) < 0.151 and 1.85 < daughter(0, InvM) < 1.88 and 1.95 < InvM < 2.05 and chiProb > 0.001',
        path=rec_path)

    # ----------------------------------------------------------------------------
    # Selections on gamma
    # - a good vertex quality
    # - the dr cut on the origin vertex coordinate of the photon conversion, that excludes the beam background
    # - a tighter range on both the invariant mass and the convertedPhotonInvariantMass
    # - geometric cuts on proximity of e+ and e- with convertedPhotonDelR and convertedPhotonDelZ

    ma.cutAndCopyList(
        outputListName='gamma:cut',
        inputListName='gamma:myGamma',
        cut=('chiProb > 0.001 and 1 < dr < 12 and InvM < 0.01'
                      'and convertedPhotonInvariantMass(0,1) < 0.005'
                      'and -0.05 < convertedPhotonDelR(0,1) < 0.15'
                      'and -0.05 < convertedPhotonDelZ(0,1) < 0.05'
             ),
        path=rec_path)
    return rec_path


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["requested_iov"] and kwargs["expert_config"] are used.

        "requested_iov" is the IoV range of the bucket and your payloads should correspond to this range.
        However your highest payload IoV should be open ended e.g. IoV(3,4,-1,-1)

        "expert_config" is the input configuration. It takes default values from your `CalibrationSettings` but these are
        overwritten by values from the 'expert_config' key in your input `caf_config.json` file when running ``b2caf-prompt-run``.

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    import basf2
    # Set up config options

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_hadron_calib = input_data["hadron_calib"]

    expert_config = kwargs.get("expert_config")

    isMC = expert_config["isMC"]
    listOfMutedCalibrations = expert_config["listOfMutedCalibrations"]
    rerun_reco = expert_config["rerun_reco"]
    rerun_reco_val = expert_config["rerun_reco_val"]
    max_files_per_run = expert_config["MaxFilesPerRun"]
    max_files_per_run_validation = expert_config["MaxFilesPerRunValidation"]

    # Choose between the basic (default) or the full validation (produces more plots but depends on the global PID)
    validation_mode = 1 if expert_config["validation_mode"] == "full" else 0

    # If you are using Raw data there's a chance that input files could have zero events.
    # This causes a B2FATAL in basf2 RootInput so the collector job will fail.
    # Currently we don't have a good way of filtering this on the automated side, so we can check here.
    min_events_per_file = expert_config["MinEvtsPerFile"]

    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_hadron_calib = filter_by_max_files_per_run(file_to_iov_hadron_calib, max_files_per_run, min_events_per_file)
    input_files_hadron_calib = list(reduced_file_to_iov_hadron_calib.keys())
    basf2.B2INFO(f"Total number of files actually used as input for calibration = {len(input_files_hadron_calib)}")

    if "dEdxValidation" not in listOfMutedCalibrations:
        reduced_file_to_iov_hadron_validation = filter_by_max_files_per_run(
            file_to_iov_hadron_calib, max_files_per_run_validation, min_events_per_file)
        input_files_hadron_validation = list(reduced_file_to_iov_hadron_validation.keys())
        basf2.B2INFO(f"Total number of files actually used as input for validation = {len(input_files_hadron_validation)}")
    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    algo = SVDdEdxCalibrationAlgorithm()
    algo.setMonitoringPlots(True)
    algo.setNumPBins(expert_config['NBinsP'])
    algo.setNumDEdxBins(expert_config['NBinsdEdx'])
    algo.setDEdxCutoff(expert_config['dedxCutoff'])
    algo.setMinEvtsPerTree(expert_config['MinEvtsPerTree'])

    if "dEdxValidation" not in listOfMutedCalibrations:
        algo_val = SVDdEdxValidationAlgorithm()
        algo_val.setMonitoringPlots(True)
        algo_val.setMinEvtsPerTree(expert_config['MinEvtsPerTree'])
        algo_val.setNumROCpoints(expert_config['NumROCpoints'])
        algo_val.setMinROCMomentum(expert_config['MinROCMomentum'])
        algo_val.setMaxROCMomentum(expert_config['MaxROCMomentum'])
        algo_val.setNumEffBins(expert_config['NumEffBins'])
        algo_val.setMaxEffMomentum(expert_config['MaxEffMomentum'])
        algo_val.validationMode(validation_mode)

    ###################################################
    # Calibration setup

    from caf.framework import Calibration

    rec_path = create_path(rerun_reco, isMC, expert_config)
    rec_path_validation = create_path(rerun_reco_val, isMC, expert_config)

    dedx_calibration = Calibration("SVDdEdxCalibration",
                                   collector="SVDdEdxCollector",
                                   algorithms=[algo],
                                   input_files=input_files_hadron_calib,
                                   pre_collector_path=rec_path)

    if "dEdxValidation" not in listOfMutedCalibrations:
        dedx_validation = Calibration("SVDdEdxValidation",
                                      collector="SVDdEdxValidationCollector",
                                      algorithms=[algo_val],
                                      backend_args={"queue": "l"},
                                      input_files=input_files_hadron_validation,
                                      pre_collector_path=rec_path_validation)
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in dedx_calibration.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    if "dEdxValidation" not in listOfMutedCalibrations:
        dedx_validation.depends_on(dedx_calibration)
    # You must return all calibrations you want to run in the prompt process, even if it's only one
    list_of_calibrations = []
    if "dEdxCalibration" not in listOfMutedCalibrations:
        list_of_calibrations.append(dedx_calibration)
    if "dEdxValidation" not in listOfMutedCalibrations:
        list_of_calibrations.append(dedx_validation)

    return list_of_calibrations

##############################
