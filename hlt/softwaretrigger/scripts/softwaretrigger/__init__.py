import modularAnalysis
import stdFSParticles
import vertex

SOFTWARE_TRIGGER_GLOBAL_TAG_NAME = "software_trigger_test"

FAST_RECO_CUTS = ["reject_ee", "accept_ee", "reject_bkg"]

HLT_CUTS = ["accept_hadron", "accept_bhabha", "accept_tau_tau", "accept_2_tracks", "accept_1_track1_cluster",
            "accept_mu_mu", "accept_gamma_gamma"]

CALIB_CUTS = ["accept_ee", "accept_gee", "accept_mumu", "accept_gmumu", "accept_gg_ee", "accept_gg_4pi",
              "accept_D0_Kpi", "accept_Dstar", "accept_Xi_piLambda", "accept_test",
              "accept_dqm_D0", "accept_dqm_Dplus", "accept_dqm_Dstar",
              "accept_dqm_Jpsiee", "accept_dqm_Jpsimumu"]


def add_fast_reco_software_trigger(path, store_array_debug_prescale=None):
    """
    Add the SoftwareTrigger for the fast reco cuts to the given path.

    Only the calculation of the cuts is implemented here - the cut logic has to be done
    using the module return value.

    :param path: The path to which the module should be added.
    :param store_array_debug_prescale: When not None, store each N events the content of the variables needed for the
     cut calculations in the data store.
    :return: the software trigger module
    """
    fast_reco_cut_module = path.add_module("SoftwareTrigger", baseIdentifier="fast_reco",
                                           cutIdentifiers=FAST_RECO_CUTS,
                                           acceptOverridesReject=True)
    if store_array_debug_prescale is not None:
        fast_reco_cut_module.param("preScaleStoreDebugOutputToDataStore", store_array_debug_prescale)

    return fast_reco_cut_module


def add_hlt_software_trigger(path, store_array_debug_prescale=None):
    """
    Add the SoftwareTrigger for the HLT cuts to the given path.

    Only the calculation of the cuts is implemented here - the cut logic has to be done
    using the module return value.

    :param path: The path to which the module should be added.
    :param store_array_debug_prescale: When not None, store each N events the content of the variables needed for the
     cut calculations in the data store.
    :return: the software trigger module
    """
    modularAnalysis.fillParticleList("pi+:HLT", 'pt>0.2', path=path)
    modularAnalysis.fillParticleList("gamma:HLT", 'E>0.1', path=path)

    # Add fast reco cuts
    hlt_cut_module = path.add_module("SoftwareTrigger", baseIdentifier="hlt",
                                     cutIdentifiers=HLT_CUTS)

    if store_array_debug_prescale is not None:
        hlt_cut_module.param("preScaleStoreDebugOutputToDataStore", store_array_debug_prescale)

    return hlt_cut_module


def add_calibration_software_trigger(path, store_array_debug_prescale=None):
    """
    Add the SoftwareTrigger for the calibration (after HLT) to the given path.

    Only the calculation of the cuts is implemented here - the cut logic has to be done

    :param path: The path to which the module should be added.
    :param store_array_debug_prescale: When not None, store each N events the content of the variables needed for the
     cut calculations in the data store.
    :return: the software trigger module
    """
    trackcut = 'abs(dz)<4.0 and dr<2.0 and chiProb>0.001 and cosTheta>-0.866 and cosTheta<0.956'
    modularAnalysis.fillParticleList("pi+:calib", trackcut, path=path)
    modularAnalysis.fillParticleList("K+:calib", trackcut, path=path)
    modularAnalysis.fillParticleList("p+:calib", trackcut, path=path)
    modularAnalysis.fillParticleList("gamma:calib", 'E>0.05', path=path)

    calib_particle_list = []
    calib_extraInfo_list = []
    # rho
    modularAnalysis.reconstructDecay('rho0:calib -> pi+:calib pi-:calib', 'abs(dM)<0.5', path=path)
    modularAnalysis.rankByLowest('rho0:calib', 'abs(dM)', 1, path=path)
    modularAnalysis.variablesToExtraInfo('rho0:calib', {'abs(dM)': 'rho0_dM'}, path=path)
    calib_particle_list.append('rho0:calib')
    calib_extraInfo_list.append('rho0_dM')

    # reconstruct intermediate state
    # D0->Kpi, D*->D0(Kpi) pi
    modularAnalysis.reconstructDecay('D0:calib -> pi+:calib K-:calib', 'abs(dM)<0.5', path=path)

    modularAnalysis.reconstructDecay('D*+:calib -> D0:calib pi+:calib', 'abs(dM)<0.5', path=path)
    modularAnalysis.rankByLowest('D0:calib', 'abs(dM)', 1, path=path)
    modularAnalysis.variablesToExtraInfo('D0:calib', {'abs(dM)': 'D0_dM'}, path=path)
    modularAnalysis.rankByLowest('D*+:calib', 'abs(dQ)', 1, path=path)
    modularAnalysis.variablesToExtraInfo('D*+:calib', {'abs(dQ)': 'Dstar_dQ'}, path=path)
    calib_particle_list.append('D0:calib')
    calib_extraInfo_list.append('D0_dM')
    calib_particle_list.append('D*+:calib')
    calib_extraInfo_list.append('Dstar_dQ')

    # Lambda0->p pi-, Xi-->Lambda0 pi-
    modularAnalysis.reconstructDecay('Lambda0:calib -> pi-:calib p+:calib', '', path=path)
    vertex.fitVertex('Lambda0:calib', 0.001, fitter='kfitter', path=path)
    modularAnalysis.rankByHighest('Lambda0:calib', 'chiProb', 1, path=path)
    modularAnalysis.variablesToExtraInfo('Lambda0:calib', {'chiProb': 'Lambda0_chiProb'}, path=path)

    modularAnalysis.reconstructDecay('Xi-:calib -> Lambda0:calib pi-:calib', '', path=path)
    vertex.fitVertex('Xi-:calib', 0.001, fitter='kfitter', path=path)
    modularAnalysis.rankByHighest('Xi-:calib', 'chiProb', 1, path=path)
    modularAnalysis.variablesToExtraInfo('Xi-:calib', {'chiProb': 'Xi_chiProb'}, path=path)
    calib_particle_list.append('Xi-:calib')
    calib_extraInfo_list.append('Xi_chiProb')

    # Reconstruct D0(Kpi), D+(Kpipi), D*+(D0pi), B+(D0pi+), J/psi(ee/mumu) for hlt-dqm display
    modularAnalysis.fillParticleList("pi+:dqm", 'piid > 0.5 and chiProb > 0.001', path=path)
    modularAnalysis.fillParticleList("K-:dqm", 'Kid > 0.5 and chiProb > 0.001', path=path)
    # D0->K- pi+
    modularAnalysis.reconstructDecay('D0:dqm -> K-:dqm pi+:dqm', '1.8 < M < 1.92', path=path)
    vertex.vertexKFit('D0:dqm', 0.0, path=path)
    modularAnalysis.rankByHighest('D0:dqm', 'chiProb', 1, path=path)
    modularAnalysis.variablesToExtraInfo('D0:dqm', {'M': 'dqm_D0_M'}, path=path)
    calib_particle_list.append('D0:dqm')
    calib_extraInfo_list.append('dqm_D0_M')

    # D*+->D0 pi-
    modularAnalysis.reconstructDecay('D*+:dqm -> D0:dqm pi+:dqm',
                                     '1.95 < M <2.05 and 0.0 < Q < 0.020 and 2.5 < useCMSFrame(p) < 5.5', path=path)
    vertex.vertexKFit('D*+:dqm', 0.0, path=path)
    modularAnalysis.rankByHighest('D*+:dqm', 'chiProb', 1, path=path)
    modularAnalysis.variablesToExtraInfo('D*+:dqm', {'M': 'dqm_Dstar_M'}, path=path)
    calib_particle_list.append('D*+:dqm')
    calib_extraInfo_list.append('dqm_Dstar_M')

    # D+ -> K- pi+ pi+
    modularAnalysis.reconstructDecay('D+:dqm -> K-:dqm pi+:dqm pi+:dqm', '1.8 < M < 1.92', path=path)
    vertex.vertexKFit('D+:dqm', 0.0, path=path)
    modularAnalysis.rankByHighest('D+:dqm', 'chiProb', 1, path=path)
    modularAnalysis.variablesToExtraInfo('D+:dqm', {'M': 'dqm_Dplus_M'}, path=path)
    calib_particle_list.append('D+:dqm')
    calib_extraInfo_list.append('dqm_Dplus_M')

    # Jpsi-> ee
    modularAnalysis.fillParticleList('e+:good', 'eid > 0.2 and d0 < 2 and abs(z0) < 4 ', path=path)
    modularAnalysis.reconstructDecay('J/psi:dqm_ee -> e+:good e-:good', '2.9 < M < 3.2', path=path)
    vertex.massVertexKFit('J/psi:dqm_ee', 0.0, path=path)
    modularAnalysis.rankByHighest('J/psi:dqm_ee', 'chiProb', 1, path=path)
    modularAnalysis.variablesToExtraInfo('J/psi:dqm_ee', {'M': 'dqm_Jpsiee_M'}, path=path)
    calib_particle_list.append('J/psi:dqm_ee')
    calib_extraInfo_list.append('dqm_Jpsiee_M')

    # Jpsi-> mumu
    modularAnalysis.fillParticleList('mu+:good', 'muid > 0.2 and d0 < 2 and abs(z0) < 4 ', path=path)
    modularAnalysis.reconstructDecay('J/psi:dqm_mumu -> mu+:good mu-:good', '2.9 < M < 3.2', path=path)
    vertex.massVertexKFit('J/psi:dqm_mumu', 0.0, path=path)
    modularAnalysis.rankByHighest('J/psi:dqm_mumu', 'chiProb', 1, path=path)
    modularAnalysis.variablesToExtraInfo('J/psi:dqm_mumu', {'M': 'dqm_Jpsimumu_M'}, path=path)
    calib_particle_list.append('J/psi:dqm_mumu')
    calib_extraInfo_list.append('dqm_Jpsimumu_M')
    calibration_cut_module = path.add_module("SoftwareTrigger", baseIdentifier="calib", cutIdentifiers=CALIB_CUTS,
                                             calibParticleListName=calib_particle_list, calibExtraInfoName=calib_extraInfo_list)

    if store_array_debug_prescale is not None:
        calibration_cut_module.param("preScaleStoreDebugOutputToDataStore", store_array_debug_prescale)
