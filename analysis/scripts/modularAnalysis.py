# !/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import os
import sys
import inspect
from beamparameters import add_beamparameters
from vertex import *
from analysisPath import *


def inputMdst(filename, path=analysis_main):
    """
    Loads the specified ROOT (DST/mDST/muDST) file with the RootInput module.

    @param filename the name of the file to be loaded
    @param modules are added to this path
    """

    inputMdstList([filename], path)


def inputMdstList(filelist, path=analysis_main):
    """
    Loads the specified ROOT (DST/mDST/muDST) files with the RootInput module.

    @param filelist the filename list of files to be loaded
    @param modules are added to this path
    """

    roinput = register_module('RootInput')
    roinput.param('inputFileNames', filelist)
    path.add_module(roinput)
    progress = register_module('Progress')
    path.add_module(progress)

    gearbox = register_module('Gearbox')
    path.add_module(gearbox)


def outputMdst(filename, path=analysis_main):
    """
    Saves mDST (mini-Data Summary Tables) to the output root file.
    """

    import reconstruction
    reconstruction.add_mdst_output(path, mc=True, filename=filename)


def outputUdst(filename, particleLists=[], path=analysis_main):
    """
    Save uDST (micro-Data Summary Tables) = MDST + Particles + ParticleLists
    The charge-conjugate lists of those given in particleLists are also stored.
    """

    import reconstruction
    import pdg
    # also add anti-particle lists
    plSet = set(particleLists)
    for List in particleLists:
        name, label = List.split(':')
        plSet.add(pdg.conjugate(name) + ':' + label)

    partBranches = ['Particles', 'ParticlesToMCParticles',
                    'ParticlesToPIDLikelihoods', 'ParticleExtraInfoMap',
                    'EventExtraInfo'] + list(plSet)
    reconstruction.add_mdst_output(path, mc=True, filename=filename,
                                   additionalBranches=partBranches)


def skimOutputUdst(skimname, particleLists=[], path=analysis_main):
    """
    Create a new path for events that contain a non-empty particle list.
    Write the accepted events as a udst file.
    Currently mdst are also written. This is for testing purposes
    and will be removed in the future.
    """

    skimfilter = register_module('SkimFilter')
    skimfilter.set_name('SkimFilter_' + skimname)
    skimfilter.param('particleLists', particleLists)
    analysis_main.add_module(skimfilter)
    skim_path = create_path()
    skimfilter.if_value('=1', skim_path, AfterConditionPath.CONTINUE)
    outputUdst(skimname + '.udst.root', particleLists, path=skim_path)
    outputMdst(skimname + '.mdst.root', path=skim_path)


def generateY4S(noEvents, decayTable=None, path=analysis_main):
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

    add_beamparameters(path, "Y4S")
    evtnumbers = register_module('EventInfoSetter')
    evtnumbers.param('evtNumList', [noEvents])
    evtnumbers.param('runList', [1])
    evtnumbers.param('expList', [1])
    evtgeninput = register_module('EvtGenInput')
    if decayTable is not None:
        if os.path.exists(decayTable):
            evtgeninput.param('userDECFile', decayTable)
        else:
            B2ERROR('The specifed decay table file does not exist:' + decayTable)
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
    if os.path.exists(decayTable):
        evtgeninput.param('userDECFile', decayTable)
    else:
        B2ERROR('The specifed decay table file does not exist:' + decayTable)
    evtgeninput.param('ParentParticle', 'vpho')
    evtgeninput.param('InclusiveParticle', inclusiveP)
    evtgeninput.param('InclusiveType', inclusiveT)
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


def printMCParticles(onlyPrimaries=False, maxLevel=-1, path=analysis_main):
    """
    Prints all MCParticles or just primary MCParticles up to specified level. -1 means no limit.
    """

    mcparticleprinter = register_module('PrintMCParticles')
    mcparticleprinter.param('onlyPrimaries', onlyPrimaries)
    mcparticleprinter.param('maxLevel', maxLevel)
    path.add_module(mcparticleprinter)


def copyList(
    outputListName,
    inputListName,
    writeOut=False,
    path=analysis_main,
):
    """
    Copy all Particle indices from input ParticleList to the output ParticleList.
    Note that the Particles themselves are not copied. The original and copied
    ParticleLists will point to the same Particles.

    @param ouputListName copied ParticleList
    @param inputListName original ParticleList to be copied
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    copyLists(outputListName, [inputListName], writeOut, path)


def copyLists(
    outputListName,
    inputListNames,
    writeOut=False,
    path=analysis_main,
):
    """
    Copy all Particle indices from all input ParticleLists to the single output ParticleList.
    Note that the Particles themselves are not copied.The original and copied
    ParticleLists will point to the same Particles.

    @param ouputListName copied ParticleList
    @param inputListName vector of original ParticleLists to be copied
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('writeOut', writeOut)
    path.add_module(pmanipulate)


def copyParticles(
    outputListName,
    inputListName,
    writeOut=False,
    path=analysis_main,
):
    """
    Create copies of Particles given in the input ParticleList and add them to the output ParticleList.

    The existing relations of the original Particle (or it's (grand-)^n-daughters)
    are copied as well. Note that only the relation is copied and that the related
    object is not. Copied particles are therefore related to the *same* object as
    the original ones.

    @param ouputListName new ParticleList filled with copied Particles
    @param inputListName input ParticleList with original Particles
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    # first copy original particles to the new ParticleList
    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', [inputListName])
    pmanipulate.param('writeOut', writeOut)
    path.add_module(pmanipulate)

    # now replace original particles with their copies
    pcopy = register_module('ParticleCopier')
    pcopy.param('inputListNames', [outputListName])
    path.add_module(pcopy)


def cutAndCopyLists(
    outputListName,
    inputListNames,
    cut,
    writeOut=False,
    path=analysis_main,
):
    """
    Copy Particle indices that pass selection criteria from all input ParticleLists to
    the single output ParticleList.
    Note that the Particles themselves are not copied.The original and copied
    ParticleLists will point to the same Particles.

    @param ouputListName copied ParticleList
    @param inputListName vector of original ParticleLists to be copied
    @param cut      selection criteria given in VariableManager style that copied Particles need to fullfill
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCutAndCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('cut', cut)
    pmanipulate.param('writeOut', writeOut)
    path.add_module(pmanipulate)


def cutAndCopyList(
    outputListName,
    inputListName,
    cut,
    writeOut=False,
    path=analysis_main,
):
    """
    Copy Particle indices that pass selection criteria from the input ParticleList to
    the output ParticleList.
    Note that the Particles themselves are not copied.The original and copied
    ParticleLists will point to the same Particles.

    @param ouputListName copied ParticleList
    @param inputListName vector of original ParticleLists to be copied
    @param cut      selection criteria given in VariableManager style that copied Particles need to fullfill
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    cutAndCopyLists(outputListName, [inputListName], cut, writeOut, path)


def fillSignalSideParticleList(outputListName, decayString, path):
    """
    This function should only be used in the ROE path, that is a path
    that is executed for each ROE object in the DataStore.

    Example: fillSignalSideParticleList('gamma:sig','B0 -> K*0 ^gamma', roe_path)

    Function will create a ParticleList with name 'gamma:sig' which will be filled
    with the existing photon Particle, being the second daughter of the B0 candidate
    to which the ROE object has to be related.

    @param ouputListName name of the created ParticleList
    @param decayString specify Particle to be added to the ParticleList
    """

    pload = register_module('SignalSideParticleListCreator')
    pload.set_name('SSParticleList_' + outputListName)
    pload.param('particleListName', outputListName)
    pload.param('decayString', decayString)
    path.add_module(pload)


def fillParticleLists(decayStringsWithCuts, writeOut=False,
                      path=analysis_main):
    """
    Creates Particles of the desired types from the corresponding MDST dataobjects,
    loads them to the StoreArray<Particle> and fills the ParticleLists.

    The multiple ParticleLists with their own selection criteria are specified
    via list tuples (decayString, cut), like for example
    kaons = ('K+:std', 'Kid>0.1')
    pions = ('pi+:std', 'piid>0.1')
    fillParticleLists([kaons, pions])

    The type of the particles to be loaded is specified via the decayString module parameter.
    The type of the MDST dataobject that is used as an input is determined from the type of
    the particle. The following types of the particles can be loaded:

    o) charged final state particles (input MDST type = Tracks)
       - e+, mu+, pi+, K+, p, deuteron (and charge conjugated particles)

    o) neutral final state particles
       - gamma         (input MDST type = ECLCluster)
       - K_S0, Lambda0 (input MDST type = V0)
       - K_L0          (input MDST type = KLMCluster)

    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut           Particles need to pass these selection criteria to be added to the ParticleList
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + 'PLists')
    pload.param('decayStringsWithCuts', decayStringsWithCuts)
    pload.param('writeOut', writeOut)
    path.add_module(pload)


def fillParticleList(
    decayString,
    cut,
    writeOut=False,
    path=analysis_main,
):
    """
    Creates Particles of the desired type from the corresponding MDST dataobjects,
    loads them to the StoreArray<Particle> and fills the ParticleList.

    The type of the particles to be loaded is specified via the decayString module parameter.
    The type of the MDST dataobject that is used as an input is determined from the type of
    the particle. The following types of the particles can be loaded:

    o) charged final state particles (input MDST type = Tracks)
       - e+, mu+, pi+, K+, p, deuteron (and charge conjugated particles)

    o) neutral final state particles
       - gamma         (input MDST type = ECLCluster)
       - K_S0, Lambda0 (input MDST type = V0)
       - K_L0          (input MDST type = KLMCluster)

    Use 'fillConvertedPhotonsList' function to load converted photons from the V0 StoreArray.

    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut           Particles need to pass these selection criteria to be added to the ParticleList
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStringsWithCuts', [(decayString, cut)])
    pload.param('writeOut', writeOut)
    path.add_module(pload)


def fillConvertedPhotonsList(
    decayString,
    cut,
    writeOut=False,
    path=analysis_main,
):
    """
    Creates photon Particle object for each e+e- combination in the V0 StoreArray.

    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut           Particles need to pass these selection criteria to be added to the ParticleList
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """
    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStringsWithCuts', [(decayString, cut)])
    pload.param('addDaughters', True)
    pload.param('writeOut', writeOut)
    path.add_module(pload)


def fillParticleListFromMC(
    decayString,
    cut,
    addDaughters=False,
    writeOut=False,
    path=analysis_main,
):
    """
    Creates Particle object for each MCParticle of the desired type found in the StoreArray<MCParticle>,
    loads them to the StoreArray<Particle> and fills the ParticleList.

    The type of the particles to be loaded is specified via the decayString module parameter.

    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut           Particles need to pass these selection criteria to be added to the ParticleList
    @param addDaughters  adds the bottom part of the decay chain of the particle to the datastore and sets mother-daughter relations
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStringsWithCuts', [(decayString, cut)])
    pload.param('addDaughters', addDaughters)
    pload.param('writeOut', writeOut)
    pload.param('useMCParticles', True)
    path.add_module(pload)


def fillParticleListsFromMC(
    decayStringsWithCuts,
    addDaughters=False,
    writeOut=False,
    path=analysis_main,
):
    """
    Creates Particle object for each MCParticle of the desired type found in the StoreArray<MCParticle>,
    loads them to the StoreArray<Particle> and fills the ParticleLists.

    The types of the particles to be loaded are specified via the (decayString, cut) tuples given in a list.
    For example:
    kaons = ('K+:gen', '')
    pions = ('pi+:gen', 'piid>0.1')
    fillParticleListsFromMC([kaons, pions])

    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut           Particles need to pass these selection criteria to be added to the ParticleList
    @param addDaughters  adds the bottom part of the decay chain of the particle to the datastore and sets mother-daughter relations
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + 'PLists')
    pload.param('decayStringsWithCuts', decayStringsWithCuts)
    pload.param('addDaughters', addDaughters)
    pload.param('writeOut', writeOut)
    pload.param('useMCParticles', True)
    path.add_module(pload)


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


def reconstructDecay(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=analysis_main
):
    """
    Creates new Particles by making combinations of existing Particles - it reconstructs unstable particles via
    their specified decay mode, e.g. in form of a DecayString: D0 -> K- pi+; B+ -> anti-D0 pi+, .... All
    possible combinations are created (overlaps are forbidden) and combinations that pass the specified selection
    criteria are saved to a newly created (mother) ParticleList. By default the charge conjugated decay is
    reconstructed as well (meaning that the charge conjugated mother list is created as well).

    @param decayString DecayString specifying what kind of the decay should be reconstructed
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut         created (mother) Particles are added to the mother ParticleList if they
                       pass give cuts (in VariableManager style) and rejected otherwise
    @oaram dmID        user specified decay mode identifier
    @param writeOut    wether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('decayMode', dmID)
    pmake.param('writeOut', writeOut)
    path.add_module(pmake)


def reconstructRecoil(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=analysis_main,
):
    """
    Creates new Particles that recoil against the input particles.

    For example the decay string M -> D1 D2 D3 will:
     - create mother Particle M for each unique combination of D1, D2, D3 Particles
     - Particles D1, D2, D3 will be appended as daughters to M
     - the 4-momentum of the mother Particle M is given by
         p(M) = p(HER) + p(LER) - Sum_i p(Di)

    @param decayString DecayString specifying what kind of the decay should be reconstructed
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut         created (mother) Particles are added to the mother ParticleList if they
                       pass give cuts (in VariableManager style) and rejected otherwise
    @oaram dmID        user specified decay mode identifier
    @param writeOut    wether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('decayMode', dmID)
    pmake.param('writeOut', writeOut)
    pmake.param('recoilParticleType', 1)
    path.add_module(pmake)


def reconstructRecoilDaughter(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=analysis_main,
):
    """
    Creates new Particles that are daughters of the particle reconstructed in the recoil (always assumed to be the first daughter).

    For example the decay string M -> D1 D2 D3 will:
     - create mother Particle M for each unique combination of D1, D2, D3 Particles
     - Particles D1, D2, D3 will be appended as daughters to M
     - the 4-momentum of the mother Particle M is given by
         p(M) = p(D1) - Sum_i p(Di), where i>1

    @param decayString DecayString specifying what kind of the decay should be reconstructed
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut         created (mother) Particles are added to the mother ParticleList if they
                       pass give cuts (in VariableManager style) and rejected otherwise
    @oaram dmID        user specified decay mode identifier
    @param writeOut    wether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('decayMode', dmID)
    pmake.param('writeOut', writeOut)
    pmake.param('recoilParticleType', 2)
    path.add_module(pmake)


def rankByHighest(
    particleList,
    variable,
    numBest=0,
    outputVariable='',
    path=analysis_main,
):
    """
    Ranks particles in the input list by the given variable (highest to lowest), and stores an integer rank for each Particle
    in an extra-info field '${variable}_rank' starting at 1 (best). The list is also sorted from best to worst candidate
    (each charge, e.g. B+/B-, separately).
    This can be used to perform a best candidate selection by cutting on the corresponding rank value, or by specifying
    a non-zero value for 'numBest'.

    @param particleList     The input ParticleList
    @param variable         Variable to order Particles by.
    @param numBest          If not zero, only the $numBest Particles in particleList with rank <= numBest are kept.
    @param outputVariable   Name for the variable that will be created which contains the rank, Default is '${variable}_rank'.
    @param path             modules are added to this path
    """

    bcs = register_module('BestCandidateSelection')
    bcs.set_name('BestCandidateSelection_' + particleList + '_' + variable)
    bcs.param('particleList', particleList)
    bcs.param('variable', variable)
    bcs.param('numBest', numBest)
    bcs.param('outputVariable', outputVariable)
    path.add_module(bcs)


def rankByLowest(
    particleList,
    variable,
    numBest=0,
    outputVariable='',
    path=analysis_main,
):
    """
    Ranks particles in the input list by the given variable (lowest to highest), and stores an integer rank for each Particle
    in an extra-info field '${variable}_rank' starting at 1 (best). The list is also sorted from best to worst candidate
    (each charge, e.g. B+/B-, separately).
    This can be used to perform a best candidate selection by cutting on the corresponding rank value, or by specifying
    a non-zero value for 'numBest'.

    @param particleList     The input ParticleList
    @param variable         Variable to order Particles by.
    @param numBest          If not zero, only the $numBest Particles in particleList with rank <= numBest are kept.
    @param outputVariable   Name for the variable that will be created which contains the rank, Default is '${variable}_rank'.
    @param path             modules are added to this path
    """

    bcs = register_module('BestCandidateSelection')
    bcs.set_name('BestCandidateSelection_' + particleList + '_' + variable)
    bcs.param('particleList', particleList)
    bcs.param('variable', variable)
    bcs.param('numBest', numBest)
    bcs.param('selectLowest', True)
    bcs.param('outputVariable', outputVariable)
    path.add_module(bcs)


def trainTMVAMethod(
    decayString,
    variables,
    methods=[('FastBDT', 'Plugin',
              '!H:!V:CreateMVAPdfs:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
              )],
    target='isSignal',
    prefix='TMVA',
    workingDirectory='.',
    path=analysis_main,
):
    """
    Trains a TMVA Method
    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param variables list of variables which are registered in the VariableManager
    @param methods list of tuples (name, type, config) of the TMVA methods
    @param target variable registered in VariableManager which is used as target
    @param prefix prefix which is used to identify the weight files created by TMVA
    @param workingDirectory in which the config file and the weight file directory are created
    @param path         modules are added to this path
    """

    teacher = register_module('TMVAOnTheFlyTeacher')
    teacher.param('prefix', prefix)
    teacher.param('methods', methods)
    teacher.param('variables', variables)
    teacher.param('target', target)
    teacher.param('workingDirectory', workingDirectory)
    teacher.param('listNames', decayString)
    path.add_module(teacher)


def applyTMVAMethod(
    decayString,
    method='FastBDT',
    expertOutputName='isSignal',
    signalFraction=-1,
    signalClass=1,
    prefix='TMVA',
    transformToProbability=True,
    sPlotPrior='',
    workingDirectory='.',
    path=analysis_main,
):
    """
    Applies a trained TMVA method to a particle list
    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param method name of the TMVA method
    @param expertOutputName extra-info name which is used to store the classifier output in the particle
    @param signalFraction to calculate probability, -1 for training fraction
    @param signalClass is the cluster to calculate the probability of beeing signal
    @param prefix prefix which is used to identify the weight files created by TMVA
    @param workingDirectory in which the expert finds the config file and the weight file directory
    @param path         modules are added to this path
    """

    expert = register_module('TMVAExpert')
    expert.param('prefix', prefix)
    expert.param('method', method)
    expert.param('workingDirectory', workingDirectory)
    expert.param('listNames', decayString)
    expert.param('expertOutputName', expertOutputName)
    expert.param('signalFraction', signalFraction)
    expert.param('transformToProbability', transformToProbability)
    expert.param('sPlotPrior', sPlotPrior)
    expert.param('signalClass', signalClass)
    path.add_module(expert)


def isTMVAMethodAvailable(prefix='TMVA'):
    """
    True of a TMVA method with the given prefix was trained
    @param prefix which is used to identify the weight files created by TMVA
    """

    return os.path.isfile(prefix + '_1.config')


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


def variablesToNTuple(
    decayString,
    variables,
    treename='variables',
    filename='ntuple.root',
    path=analysis_main,
):
    """
    Creates and fills a flat ntuple with the specified variables from the VariableManager
    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param variables variables which must be registered in the VariableManager
    @param treename name of the ntuple tree
    @param filename which is used to store the variables
    @param path basf2 path
    """

    output = register_module('VariablesToNtuple')
    output.param('particleList', decayString)
    output.param('variables', variables)
    output.param('fileName', filename)
    output.param('treeName', treename)
    path.add_module(output)


def variablesToExtraInfo(
    particleList,
    variables,
    path=analysis_main,
):
    """
    For each particle in the input list the selected variables are saved in an extra-info field witht he given name.
    Can be used when wanting to save variables before modifying them, e.g. when performing vertex fits.

    @param particleList  The input ParticleList
    @param variables     Dictionary of Variables and extraInfo names.
    @param path          modules are added to this path
    """

    mod = register_module('VariablesToExtraInfo')
    mod.set_name('VariablesToExtraInfo_' + particleList)
    mod.param('particleList', particleList)
    mod.param('variables', variables)
    path.add_module(mod)


def variablesToDaughterExtraInfo(
    particleList,
    decayString,
    variables,
    path=analysis_main,
):
    """
    For each daughter particle specified via decay string the selected variables (estimated for the mother particle)
    are saved in an extra-info field with the given name. In other words, the property of mother is saved as extra-info
    to specified daughter particle.
    Should only be used in ROE path, that is path executed for each ROE object in an event.

    @param particleList  The input ParticleList
    @param decayString   Decay string that specifiec to which daughter the extra infor should be appended
    @param variables     Dictionary of Variables and extraInfo names.
    @param path          modules are added to this path
    """

    mod = register_module('VariablesToExtraInfo')
    mod.set_name('VariablesToDaughterExtraInfo_' + particleList)
    mod.param('particleList', particleList)
    mod.param('decayString', decayString)
    mod.param('variables', variables)
    path.add_module(mod)


def findMCDecay(
    list_name,
    decay,
    writeOut=False,
    path=analysis_main,
):
    """
    The MCDecayFinder module is buggy at the moment. Not to be used.
    """

    decayfinder = register_module('MCDecayFinder')
    decayfinder.set_name('MCDecayFinder_' + list_name)
    decayfinder.param('listName', list_name)
    decayfinder.param('decayString', decay)
    decayfinder.param('writeOut', writeOut)
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
    Performs MC matching (sets relation Particle->MCParticle) for
    all particles (and its (grand)^N-daughter particles) in the specified
    ParticleList.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    mcMatch = register_module('MCMatcherParticles')
    mcMatch.set_name('MCMatch_' + list_name)
    mcMatch.param('listName', list_name)
    path.add_module(mcMatch)


def buildRestOfEvent(list_name, path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    dataobject and makes BASF2 relation between them.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('ROEBuilder_' + list_name)
    roeBuilder.param('particleList', list_name)
    path.add_module(roeBuilder)


def appendROEMasks(list_name, ROEMasks, path=analysis_main):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for tracks and eclClusters which will be used by variables in ROEVariables.cc.
    Default ChargedStable fractions are used in this case (pion mass hypothesis always, 0,0,1,0,0,0]).

    The multiple ROE masks with their own selection criteria are specified
    via list tuples (mask_name, trackSelection, eclClusterSelection)

    o) Example for two tuples
       ipTracks     = ('IPtracks', 'abs(d0) < 0.05 and abs(z0) < 0.1', '')
       goodROEGamma = ('goodROEGamma', '', 'goodGamma == 1')
       appendROEMasks('B+:sig', [ipTracks, goodROEGamma])

    @param list_name             name of the input ParticleList
    @param mask_name             name of the appended ROEMask
    @param trackSelection        decay string for the tracks in ROE
    @param eclClusterSelection   decay string for the tracks in ROE
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + 'MaskList')
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasks', ROEMasks)
    path.add_module(roeMask)


def appendROEMask(
    list_name,
    mask_name,
    trackSelection,
    eclClusterSelection,
    path=analysis_main,
):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for tracks and eclClusters which will be used by variables in ROEVariables.cc.
    Default ChargedStable fractions are used in this case (pion mass hypothesis always, 0,0,1,0,0,0]).

    o) append a ROE mask with all tracks in ROE coming from the IP region
       - appendROEMask('B+:sig', 'IPtracks', 'abs(d0) < 0.05 and abs(z0) < 0.1', '')

    o) append a ROE mask with only ECLClusters that pass as good photon candidates
       - appendROEMask('B+:sig', 'goodROEGamma', '', 'goodGamma == 1')

    @param list_name             name of the input ParticleList
    @param mask_name             name of the appended ROEMask
    @param trackSelection        decay string for the tracks in ROE
    @param eclClusterSelection   decay string for the tracks in ROE
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + mask_name)
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasks', [(mask_name, trackSelection, eclClusterSelection)])
    path.add_module(roeMask)


def appendROEMasksWithFractions(list_name, ROEMasksWithFractions, path=analysis_main):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for tracks and eclClusters which will be used by variables in ROEVariables.cc.
    Specific ChargedStable fractions can be provided. If fractions = [-1], MC truth will be used, if available.

    The multiple ROE masks with their own selection criteria are specified
    via list tuples (mask_name, trackSelection, eclClusterSelection)

    o) Example for two tuples
       IPtracksEqualFractions  = ('IPtracksEqualFractions', 'abs(d0) < 0.05 and abs(z0) < 0.1', '', [1,1,1,1,1,1])
       IPtracksMCHypothesis    = ('IPtracksMCHypothesis', 'abs(d0) < 0.05 and abs(z0) < 0.1', '', [-1])
       appendROEMasks('B+:sig', [IPtracksEqualFractions, IPtracksMCHypothesis])

    @param list_name             name of the input ParticleList
    @param mask_name             name of the appended ROEMask
    @param trackSelection        decay string for the tracks in ROE
    @param eclClusterSelection   decay string for the tracks in ROE
    @param fractions             chargedStable particle fractions
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + 'MaskListWithFractions')
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasksWithFractions', ROEMasksWithFractions)
    path.add_module(roeMask)


def appendROEMaskWithFractions(
    list_name,
    mask_name,
    trackSelection,
    eclClusterSelection,
    fractions,
    path=analysis_main,
):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for tracks and eclClusters which will be used by variables in ROEVariables.cc.
    Specific ChargedStable fractions can be provided. If fractions = [-1], MC truth will be used, if available.

    o) append a ROE mask with all tracks in ROE coming from the IP region
       - appendROEMask('B+:sig', 'IPtracksEqualFractions', 'abs(d0) < 0.05 and abs(z0) < 0.1', '', [1,1,1,1,1,1])

    o) append a ROE mask with only ECLClusters that pass as good photon candidates
       - appendROEMask('B+:sig', 'IPtracksMCHypothesis', 'abs(d0) < 0.05 and abs(z0) < 0.1', '', [-1])

    @param list_name             name of the input ParticleList
    @param mask_name             name of the appended ROEMask
    @param trackSelection        decay string for the tracks in ROE
    @param eclClusterSelection   decay string for the tracks in ROE
    @param fractions             chargedStable particle fractions
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + mask_name)
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasksWithFractions', [(mask_name, trackSelection, eclClusterSelection, fractions)])
    path.add_module(roeMask)


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


def calibratePhotonEnergy(list_name, energy_bias=0.030, path=analysis_main):
    """
    Performs energy calibration for photons given in the input ParticleList.
    @param list_name name of the input photon ParticleList
    @param energy_bias energy bias in GeV
    @param path      modules are added to this path
    """

    gammacal = register_module('GammaEnergyCalibration')
    gammacal.param('gammaListName', list_name)
    gammacal.param('energyBias', energy_bias)
    path.add_module(gammacal)


def removeParticlesNotInLists(lists_to_keep, path=analysis_main):
    """
    Removes all Particles that are not in a given list of ParticleLists (or daughters of those).
    All relations from/to Particles, daughter indices, and other ParticleLists are fixed.

    @param lists_to_keep Keep the Particles and their daughters in these ParticleLists.
    @param path      modules are added to this path
    """

    mod = register_module('RemoveParticlesNotInLists')
    mod.param('particleLists', lists_to_keep)
    path.add_module(mod)


def inclusiveBtagReconstruction(upsilon_list_name, bsig_list_name, btag_list_name, input_lists_names, path=analysis_main):
    """
    Reconstructs Btag from particles in given ParticleLists which do not share any final state particles (mdstSource) with Bsig.

    @param upsilon_list_name Name of the ParticleList to be filled with `Upsilon(4S) -> B:sig anti-B:tag`
    @param bsig_list_name Name of the Bsig ParticleList
    @param btag_list_name Name of the Bsig ParticleList
    @param input_lists_names List of names of the ParticleLists which are used to reconstruct Btag from
    """
    btag = register_module('InclusiveBtagReconstruction')
    btag.set_name('InclusiveBtagReconstruction_' + bsig_list_name)
    btag.param('upsilonListName', upsilon_list_name)
    btag.param('bsigListName', bsig_list_name)
    btag.param('btagListName', btag_list_name)
    btag.param('inputListsNames', input_lists_names)
    path.add_module(btag)


if __name__ == '__main__':
    desc_list = []
    for function_name in sorted(list_functions(sys.modules[__name__])):
        function = globals()[function_name]
        signature = inspect.formatargspec(*inspect.getargspec(function))
        signature = signature.replace(repr(analysis_main), 'analysis_main')
        desc_list.append((function.__name__, signature + '\n' + function.__doc__))
    pretty_print_description_list(desc_list)
