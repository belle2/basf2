#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

analysis_main = create_path()


def inputMdst(filename, path=analysis_main):
    """
    Loads the specified ROOT (DST/mDST/muDST) file with the RootInput module.

    @param filename the name of the file to be loaded
    @param modules are added to this path
    """

    roinput = register_module('RootInput')
    roinput.param('inputFileName', filename)
    path.add_module(roinput)
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    progress = register_module('Progress')
    path.add_module(progress)


def inputMdstList(filelist, path=analysis_main):
    """
    Loads the specified ROOT (DST/mDST/muDST) files with the RootInput module.

    @param filelist the filename list of files to be loaded
    @param modules are added to this path
    """

    roinput = register_module('RootInput')
    roinput.param('inputFileNames', filelist)
    path.add_module(roinput)
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    progress = register_module('Progress')
    path.add_module(progress)


def outputMdst(filename, path=analysis_main):
    """
    Saves mDST (mini-Data Summary Tables) to the output root file.
    """

    import reconstruction
    reconstruction.add_mdst_output(path, mc=True, filename=filename)


def outputUdst(filename, path=analysis_main):
    """
    Save uDST (micro-Data Summary Tables) = MDST + Particles
    """

    import reconstruction
    partBranches = ['Particles', 'ParticlesToMCParticles',
                    'ParticlesToPIDLikelihoods']
    reconstruction.add_mdst_output(path, mc=True, filename=filename,
                                   additionalBranches=partBranches)


def generateY4S(noEvents, decayTable, path=analysis_main):
    """
    Generated e+e- -> Y(4S) events with EvtGen event generator.
    The Y(4S) decays according to the user specifed decay table.

    The experiment and run numbers are set to 1.

    If the simulation and reconstruction is not performed in the sam job,
    then the Gearbox needs to be loaded. Use loadGearbox(path) function
    for this purpose.

    @param noEvents   number of events to be generated
    @param decayTable file name of the decay table to be used
    @param path       modules are added to this path
    """

    evtnumbers = register_module('EventInfoSetter')
    evtnumbers.param('evtNumList', [noEvents])
    evtnumbers.param('runList', [1])
    evtnumbers.param('expList', [1])
    evtgeninput = register_module('EvtGenInput')
    evtgeninput.param('userDECFile', decayTable)
    evtgeninput.param('boost2LAB', True)
    path.add_module(evtnumbers)
    path.add_module(evtgeninput)


def generateContinuum(
    noEvents,
    inclusiveP,
    decayTable,
    inclusiveT=2,
    path=analysis_main,
    ):
    """
    Generated e+e- -> gamma* -> qq-bar where light quarks hadronize
    and decay in user specified way (via specified decay table).

    The experiment and run numbers are set to 1.

    If the simulation and reconstruction is not performed in the sam job,
    then the Gearbox needs to be loaded. Use loadGearbox(path) function
    for this purpose.

    @param noEvents   number of events to be generated
    @param inclusiveP each event will contain this particle
    @param decayTable file name of the decay table to be used
    @param inclusiveT whether (2) or not (1) charge conjugated inclusive Particles should be included
    @param path       modules are added to this path
    """

    evtnumbers = register_module('EventInfoSetter')
    evtnumbers.param('evtNumList', [noEvents])
    evtnumbers.param('runList', [1])
    evtnumbers.param('expList', [1])
    evtgeninput = register_module('EvtGenInput')
    evtgeninput.param('userDECFile', decayTable)
    evtgeninput.param('ParentParticle', 'vpho')
    evtgeninput.param('InclusiveParticle', inclusiveP)
    evtgeninput.param('InclusiveType', inclusiveT)
    evtgeninput.param('boost2LAB', True)
    path.add_module(evtnumbers)
    path.add_module(evtgeninput)


def loadGearbox(path=analysis_main):
    """
    Loads Gearbox module to the path.

    This is neccessary in a job with event generation only
    (without reconstruction and reconstruction).

    @param path modules are added to this path
    """

    paramloader = register_module('Gearbox')
    path.add_module(paramloader)


def printPrimaryMCParticles(path=analysis_main):
    """
    Prints all primary MCParticles.
    """

    mcparticleprinter = register_module('PrintMCParticles')
    path.add_module(mcparticleprinter)


def loadMCParticles(path=analysis_main):
    """
    Loads all Final state MCParticles (e/mu/pi/K/p/gamma and Klongs) as Particles.
    """

    ploader = register_module('ParticleLoader')
    ploader.param('useMCParticles', True)
    path.add_module(ploader)


def loadReconstructedParticles(path=analysis_main):
    """
    Loads mDST data objects (Tracks/ECLClusters/KLMClusters) as Particles.
    In particular:
     - each Track is loaded as e/mu/pi/K/p Particles
     - each neutral ECLCluster is loaded as gamma Particle
     - each neutral KLMCluster is loaded as Klong Particle

    In all the cases no selection criteria are applied.
    """

    ploader = register_module('ParticleLoader')
    ploader.param('useMCParticles', False)
    path.add_module(ploader)


def copyList(
    outputListName,
    inputListName,
    persistent=False,
    path=analysis_main,
    ):
    """
    Copy all Particle indices from input ParticleList to the output ParticleList.

    @param ouputListName copied ParticleList
    @param inputListName original ParticleList to be copied
    @param persistent    toggle newly created particle list btw. transient/persistent
    @param path          modules are added to this path 
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', [inputListName])
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def copyLists(
    outputListName,
    inputListNames,
    persistent=False,
    path=analysis_main,
    ):
    """
    Copy all Particle indices from all input ParticleLists to the single output ParticleList.
    
    @param ouputListName copied ParticleList
    @param inputListName vector of original ParticleLists to be copied
    @param persistent    toggle newly created particle list btw. transient/persistent
    @param path          modules are added to this path
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def cutAndCopyLists(
    outputListName,
    inputListNames,
    cut,
    persistent=False,
    path=analysis_main,
    ):
    """
    Copy Particle indices that pass selection criteria from all 
    input ParticleLists to the single output ParticleList.
    
    @param ouputListName copied ParticleList
    @param inputListName vector of original ParticleLists to be copied
    @param cut      selection criteria given in VariableManager style that copied Particles need to fullfill
    @param persistent    toggle newly created particle list btw. transient/persistent
    @param path          modules are added to this path
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCutAndCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('cut', cut)
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def cutAndCopyList(
    outputListName,
    inputListName,
    cut,
    persistent=False,
    path=analysis_main,
    ):
    """
    Copy Particle indices that pass selection criteria from 
    the input ParticleList to the output ParticleList.

    @param ouputListName copied ParticleList
    @param inputListName vector of original ParticleLists to be copied
    @param cut      selection criteria given in VariableManager style that copied Particles need to fullfill
    @param persistent    toggle newly created particle list btw. transient/persistent
    @param path          modules are added to this path
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCutAndCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', [inputListName])
    pmanipulate.param('cut', cut)
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def fillParticleList(
    decayString,
    cut,
    persistent=False,
    path=analysis_main,
    ):
    """
    Creates and fills ParticleList with StoreArray<Particle> 
    indices of Particles of desired type.
    
    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut      Particles need to pass these selection criteria to be added to the ParticleList
    @param persistent    toggle newly created particle list btw. transient/persistent
    @param path          modules are added to this path 
    """

    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_' + decayString)
    pselect.param('decayString', decayString)
    pselect.param('cut', cut)
    pselect.param('persistent', persistent)
    path.add_module(pselect)


def selectParticle(
    decayString,
    cut='',
    persistent=False,
    path=analysis_main,
    ):
    """
    Creates and fills ParticleList with StoreArray<Particle> indices of Particles of desired type.
    
    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut      Particles need to pass these selection criteria to be added to the ParticleList
    @param persistent    toggle newly created particle list btw. transient/persistent
    @param path          modules are added to this path
    """

    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_' + decayString)
    pselect.param('decayString', decayString)
    pselect.param('cut', cut)
    pselect.param('persistent', persistent)
    path.add_module(pselect)


def applyCuts(list_name, cut, path=analysis_main):
    """
    Removes StoreArray<PArticle> indices of Particles from given ParticleList 
    that do not pass the given selection criteria (given in ParticleSelector style).

    @param list_name input ParticleList name
    @param cut  Particles that do not pass these selection criteria are removed from the ParticleList
    @param path      modules are added to this path 
    """

    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_applyCuts_' + list_name)
    pselect.param('decayString', list_name)
    pselect.param('cut', cut)
    path.add_module(pselect)


def reconDecay(
    decayString,
    cut,
    persistent=False,
    path=analysis_main,
    ):
    """
    Creates new Particles by making combinations of existing Particles - it reconstructs unstable particles via 
    their specified decay mode, e.g. in form of a DecayString: D0 -> K- pi+; B+ -> anti-D0 pi+, .... All 
    possible combinations are created (overlaps are forbidden) and combinations that pass the specified selection
    criteria are saved to a newly created (mother) ParticleList. By default the charge conjugated decay is
    reconstructed as well (meaning that the charge conjugated mother list is created as well).
    
    @param decayString DecayString specifying what kind of the decay should be reconstructed 
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut        created (mother) Particles are added to the mother ParticleList if they 
                       pass give cuts (in VariableManager style) and rejected otherwise
    @param persistent  toggle newly created particle list btw. transient/persistent
    @param path        modules are added to this path     
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('persistent', persistent)
    path.add_module(pmake)


def makeParticle(
    decayString,
    cut,
    persistent=False,
    path=analysis_main,
    ):
    """
    Creates new Particles by making combinations of existing Particles - it reconstructs unstable particles via
    their specified decay mode, e.g. in form of a DecayString: D0 -> K- pi+; B+ -> anti-D0 pi+, .... All
    possible combinations are created (overlaps are forbidden) and combinations that pass the specifed selection
    criteria are saved to a newly created (mother) ParticleList. By default the charge conjugated decay is
    reconstructed as well (meaning that the charge conjugated mother list is created as well).

    @param decayString DecayString specifying what kind of the decay should be reconstructed 
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut        created (mother) Particles are added to the mother ParticleList if they 
                       pass give cuts (in VariableManager style) and rejected otherwise
    @param persistent  toggle newly created particle list btw. transient/persistent
    @param path        modules are added to this path
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('persistent', persistent)
    path.add_module(pmake)


def fitVertex(
    list_name,
    conf_level,
    decay_string='',
    fitter='rave',
    fit_type='vertex',
    constraint='',
    path=analysis_main,
    ):
    """
    Perform the specified kinematic fit for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param decay_string select particles used for the vertex fit
    @param fitter       rave or kfitter
    @param fit_type     type of the kinematic fit (valid options are vertex/massvertex/mass)
    @param constraint   type of additional constraints (valid options are empty string/ipprofile/iptube)
    @param path         modules are added to this path
    """

    if 'Geometry' in [m.name() for m in path.modules()]:
        print '[INFO] fitVertex: Geometry already in path'
    else:
        geometry = register_module('Geometry')
        geometry.param('components', ['MagneticField'])
        path.add_module(geometry)

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + list_name)
    pvfit.param('listName', list_name)
    pvfit.param('confidenceLevel', conf_level)
    pvfit.param('vertexFitter', fitter)
    pvfit.param('fitType', fit_type)
    pvfit.param('withConstraint', constraint)
    pvfit.param('decayString', decay_string)
    path.add_module(pvfit)


def vertexKFit(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=analysis_main,
    ):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'vertex',
        constraint,
        path,
        )


def massVertexKFit(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
    ):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'massvertex',
        '',
        path,
        )


def massKFit(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
    ):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'mass',
        '',
        path,
        )


def printDataStore(path=analysis_main):
    """
    Prints the contents of DataStore in each event,
    listing all objects and arrays (including size).

    @param path   modules are added to this path
    """

    printDS = register_module('PrintCollections')
    path.add_module(printDS)


def printVariableValues(list_name, var_names, path=analysis_main):
    """
    Prints out values of specified variables of all Particles included in given ParticleList. For debugging purposes.

    @param list_name input ParticleList name
    @param var_names vector of variable names to be printed
    @param path         modules are added to this path 
    """

    prlist = register_module('ParticlePrinter')
    prlist.set_name('ParticlePrinter_' + list_name)
    prlist.param('listName', list_name)
    prlist.param('fullPrint', False)
    prlist.param('variables', var_names)
    path.add_module(prlist)


def printList(list_name, full, path=analysis_main):
    """
    Prints the size and executes Particle->print() (if full=True) 
    method for all Particles in given ParticleList. For debugging purposes.
    
    @param list_name input ParticleList name
    @param full      execute Particle->print() method for all Particles
    @param path      modules are added to this path
    """

    prlist = register_module('ParticlePrinter')
    prlist.set_name('ParticlePrinter_' + list_name)
    prlist.param('listName', list_name)
    prlist.param('fullPrint', full)
    path.add_module(prlist)


def ntupleFile(file_name, path=analysis_main):
    """
    Creates new ROOT file to which the flat ntuples will be saved.
    
    @param file_name file name of the output root file
    @param path      modules are added to this path
    """

    ntmaker = register_module('NtupleMaker')
    ntmaker.set_name('NtupleMaker_ntupleFile_' + file_name)
    ntmaker.param('fileName', file_name)
    path.add_module(ntmaker)


def ntupleTree(
    tree_name,
    list_name,
    tools,
    path=analysis_main,
    ):
    """
    Creates and fills flat ntuple (TTree) with the specified Ntuple tools.

    @param tree_name output nutple (TTree) name
    @param list_name input ParticleList name
    @param tools     list of Ntuple tools to be included
    """

    ntmaker = register_module('NtupleMaker')
    ntmaker.set_name('NtupleMaker_ntupleTree_' + list_name)
    ntmaker.param('treeName', tree_name)
    ntmaker.param('listName', list_name)
    ntmaker.param('tools', tools)
    path.add_module(ntmaker)


def findMCDecay(
    list_name,
    decay,
    persistent=False,
    path=analysis_main,
    ):
    """
    The MCDecayFinder module is buggy at the moment. Not to be used.
    """

    decayfinder = register_module('MCDecayFinder')
    decayfinder.set_name('MCDecayFinder_' + list_name)
    decayfinder.param('listName', list_name)
    decayfinder.param('decayString', decay)
    decayfinder.param('persistent', persistent)
    path.add_module(decayfinder)


def summaryOfLists(particleLists, path=analysis_main):
    """
    Prints out Particle statistics at the end of the job: number of events with at 
    least one candidate, average number of candidates per event, etc.

    @param particleLists list of input ParticleLists
    """

    particleStats = register_module('ParticleStats')
    particleStats.param('particleLists', particleLists)
    path.add_module(particleStats)


def matchMCTruth(list_name, path=analysis_main):
    """
    Performs MC matching (sets relation Particle<->MCParticle) for
    all particles (and its (grand)^N-daughter particles) in the specified
    ParticleList.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    mcMatch = register_module('MCMatching')
    mcMatch.set_name('MCMatching_' + list_name)
    mcMatch.param('listName', list_name)
    path.add_module(mcMatch)


def buildRestOfEvent(list_name, only_good_ecl=True, path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    dataobject and makes BASF2 relation between them.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('ROEBuilder_' + list_name)
    roeBuilder.param('particleList', list_name)
    roeBuilder.param('onlyGoodECLClusters', only_good_ecl)
    path.add_module(roeBuilder)


def TagV(
    list_name,
    confidenceLevel,
    MCassociation=True,
    useConstraint='boostcut',
    path=analysis_main,
    ):
    """
    For each Particle in the given Breco ParticleList:
    perform the fit of tag side using the track list from the RestOfEvent dataobject
    save the MC Btag in case of signal MC

    @param list_name name of the input Breco ParticleList
    @param confidenceLevel minimum value of the ConfidenceLevel to accept the fit
    @param MCassociation: use standard MC association or the internal one
    @param useConstraint: choose constraint for the tag vertes fit
    @param path      modules are added to this path
    """

    if 'Geometry' in [m.name() for m in path.modules()]:
        print '[INFO] TAgV: Geometry already in path'
    else:
        geometry = register_module('Geometry')
        geometry.param('components', ['MagneticField'])
        path.add_module(geometry)

    tvfit = register_module('TagVertex')
    tvfit.set_name('TagVertex_' + list_name)
    tvfit.param('listName', list_name)
    tvfit.param('confidenceLevel', confidenceLevel)
    tvfit.param('MCAssociation', MCassociation)
    tvfit.param('useConstraint', useConstraint)
    path.add_module(tvfit)


def buildContinuumSuppression(list_name, path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a ContinuumSuppression
    dataobject and makes BASF2 relation between them.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    qqBuilder = register_module('ContinuumSuppressionBuilder')
    qqBuilder.set_name('QQBuilder_' + list_name)
    qqBuilder.param('particleList', list_name)
    path.add_module(qqBuilder)


def FlavTag(list_name, path=analysis_main):
    """
    For each Particle in the given Breco ParticleList:
    Tag the flavour of the tag side using the Track, the ECLCluster and the KLMCluster list from the RestOfEvent dataobject
    The flavour is predicted by trained Neural Networks
    Module under development (not ready for users)

    @param list_name name of the input Breco ParticleList
    @param path      modules are added to this path
    """

    flavtag = register_module('FlavorTagging')
    flavtag.set_name('FlavorTagging_' + list_name)
    flavtag.param('usingMode', mode)
    flavtag.param('listName', list_name)
    path.add_module(flavtag)


def calibratePhotonEnergy(list_name, path=analysis_main):
    """
    Performs energy calibration for photons given in the input ParticleList.
    @param list_name name of the input photon ParticleList
    @param path      modules are added to this path
    """

    gammacal = register_module('GammaEnergyCalibration')
    gammacal.param('gammaListName', list_name)
    path.add_module(gammacal)


