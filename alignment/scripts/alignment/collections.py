import basf2
import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana
import vertex as vtx


def make_collection(name, path, **argk):
    """
    Handy function to make a collection configuration
    to be passed in 'collections' argument of the create(...) function

    Parameters
    ----------
    name : str
      Collection name
    path : basf2.Path
      pre-collector path
    argk : dict
      Dictionary of collector parameters specific for collection

    Returns
    -------
    tuple(str, basf2.Path, dict)

    """
    return (name, path, argk)


def physicsTracks(name="physicsTracks", add_unpackers=True, klm=False, prescale=1.):
    """
    Standard collection of all RecoTracks with standard reconstruction

    Parameters
    ----------
    name : str
      Collection name
    add_unpackers : bool
      Whether to add unpacking (set to False for MC)
    klm : bool
      Whether to add muid hits to the track fit
    prescale : float
      Process only 'prescale' fraction of events
    """
    path = basf2.create_path()

    path.add_module('Progress')
    path.add_module('RootInput')
    if prescale != 1.:
        path.add_module('Prescale', prescale=prescale).if_false(basf2.Path(), basf2.AfterConditionPath.END)
    path.add_module('Gearbox')
    path.add_module('Geometry')

    if add_unpackers:
        raw.add_unpackers(path)

    reco.add_reconstruction(path, pruneTracks=False, add_muid_hits=klm)

    # teporary (?) fix : TODO: still needed?
    tmp = basf2.create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            m.param('CriticalErrorMask', 0)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path = tmp
    path.add_module('DAFRecoFitter')

    return make_collection(name, path=path, tracks=['RecoTracks'])


def cosmicTracks(name="cosmicTracks",
                 add_unpackers=True,
                 skim_hlt_cosmic=False,
                 cut='[z0 <= 57. or abs(d0) >= 26.5] and abs(dz) > 0.4 and nTracks == 1',
                 klm=False,
                 prescale=1.):
    """
    Standard collection of all RecoTracks with cosmic reconstruction

    Parameters
    ----------
    name : str
      Collection name
    add_unpackers : bool
      Whether to add unpacking (set to False for MC)
    skim_hlt_cosmic : bool
      Whether to add TriggerSkim module and process only events with cosmic TRG
    cut : str
      Cut string to select GOOD events. By default set to avoid region of poorly described magnetic
      field around QCS magnets + remove the 'background' from physics around IP
    klm : bool
      Whether to add muid hits to the track fit
    prescale : float
      Process only 'prescale' fraction of events
    """
    path = basf2.create_path()

    path.add_module('Progress')
    path.add_module('RootInput')
    if prescale != 1.:
        path.add_module('Prescale', prescale=prescale).if_false(basf2.Path(), basf2.AfterConditionPath.END)

    if skim_hlt_cosmic:
        path.add_module(
            "TriggerSkim",
            triggerLines=["software_trigger_cut&filter&cosmic"]).if_value(
            "==0",
            basf2.Path(),
            basf2.AfterConditionPath.END)

    path.add_module('Gearbox')
    path.add_module('Geometry')

    if add_unpackers:
        raw.add_unpackers(path)

    path.add_module('SetupGenfitExtrapolation')
    reco.add_cosmics_reconstruction(
        path,
        pruneTracks=False,
        skipGeometryAdding=True,
        data_taking_period='early_phase3',
        merge_tracks=True,
        add_muid_hits=klm
    )

    # teporary (?) fix : TODO: still needed?
    tmp = basf2.create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            m.param('CriticalErrorMask', 0)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path = tmp

    path.add_module('SetRecoTrackMomentum', automatic=True)
    path.add_module('DAFRecoFitter', resortHits=True, pdgCodesToUseForFitting=[13])

    if cut is not None:
        ana.fillParticleList('mu+:good_cosmics', cut, path=path)
        # ana.variablesToNtuple('mu+:good_cosmics', variables=track_variables, filename='analysis_cosmics.root', path=path)
        path.add_module('SkimFilter', particleLists=['mu+:good_cosmics']).if_false(basf2.create_path())

    return make_collection(name, path=path, tracks=['RecoTracks'])


def diMuonsIP(
        name="diMuonsIP",
        add_unpackers=True,
        skim_mumu_2trk=False,
        muon_cut='p > 1.0 and abs(dz) < 2.0 and dr < 0.5',
        dimuon_cut='',
        klm=False,
        prescale=1.):
    """
    Di-muons with vertex+beam constraint collection

    Parameters
    ----------
    name : str
      Collection name
    add_unpackers : bool
      Whether to add unpacking (set to False for MC)
    skim_hlt_cosmic : bool
      Whether to add TriggerSkim module and process only events with accept_mumu_2trk TRG
    muon_cut : str
      Cut string to select daughter muons
    dimuon_cut : str
      Cut string to apply for reconstructed di-muon decay
    klm : bool
      Whether to add muid hits to the track fit
    prescale : float
      Process only 'prescale' fraction of events
    """
    path = basf2.create_path()
    path.add_module('Progress')
    path.add_module('RootInput')
    if prescale != 1.:
        path.add_module('Prescale', prescale=prescale).if_false(basf2.Path(), basf2.AfterConditionPath.END)

    path.add_module('Gearbox')
    path.add_module('Geometry')

    if skim_mumu_2trk:
        path.add_module(
            "TriggerSkim",
            triggerLines=["software_trigger_cut&skim&accept_mumu_2trk"]).if_value(
            "==0",
            basf2.Path(),
            basf2.AfterConditionPath.END)

    if add_unpackers:
        raw.add_unpackers(path)

    reco.add_reconstruction(path, pruneTracks=False, add_muid_hits=klm)

    # teporary (?) fix : TODO: still needed?
    tmp = basf2.create_path()
    for m in path.modules():
        if m.name() == "PXDPostErrorChecker":
            m.param('CriticalErrorMask', 0)
        if m.name() == "SVDSpacePointCreator":
            m.param("MinClusterTime", -999)
        tmp.add_module(m)
    path = tmp

    path.add_module('DAFRecoFitter', pdgCodesToUseForFitting=[13])

    ana.fillParticleList(f"mu+:{name}", muon_cut, path=path)
    ana.reconstructDecay(f"Upsilon(4S):{name} -> mu+:{name} mu-:{name}", dimuon_cut, path=path)

    vtx.raveFit(f"Upsilon(4S):{name}", 0.001, daughtersUpdate=True, silence_warning=True, path=path)

    return make_collection(name, path, primaryVertices=[f"Upsilon(4S):{name}"])
