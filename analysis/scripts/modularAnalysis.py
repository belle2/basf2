#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import os

analysis_main = create_path()


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


def outputUdst(filename, path=analysis_main):
    """
    Save uDST (micro-Data Summary Tables) = MDST + Particles
    """

    import reconstruction
    partBranches = ['Particles', 'ParticlesToMCParticles',
                    'ParticlesToPIDLikelihoods']
    reconstruction.add_mdst_output(path, mc=True, filename=filename,
                                   additionalBranches=partBranches)


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

    evtnumbers = register_module('EventInfoSetter')
    evtnumbers.param('evtNumList', [noEvents])
    evtnumbers.param('runList', [1])
    evtnumbers.param('expList', [1])
    evtgeninput = register_module('EvtGenInput')
    if decayTable is not None:
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


def printMCParticles(onlyPrimaries=False, maxLevel=-1, path=analysis_main):
    """
    Prints all MCParticles or just primary MCParticles up to specified level. -1 means no limit.
    """

    mcparticleprinter = register_module('PrintMCParticles')
    mcparticleprinter.param('onlyPrimaries', onlyPrimaries)
    mcparticleprinter.param('maxLevel', maxLevel)
    path.add_module(mcparticleprinter)


def loadMCParticles(path=analysis_main):
    """
    THIS FUNCTION IS NO LONGER NEEDED. IT IS OBSOLETE AND IT DOES NOTHING.
    """

    print ''
    print '* ************************************************* *'
    print '* loadMCParticles function is no longer needed.     *'
    print '* IT IS OBSOLETE AND IT DOES NOTHING.               *'
    print '* Delete loadMCParticles() from your python script. *'
    print '* ************************************************* *'
    print ''


def loadReconstructedParticles(path=analysis_main):
    """
    THIS FUNCTION IS NO LONGER NEEDED. IT IS OBSOLETE AND IT DOES NOTHING.
    """

    print ''
    print '* ************************************************************ *'
    print '* loadMCParticles function is no longer needed.                *'
    print '* IT IS OBSOLETE AND IT DOES NOTHING.                          *'
    print '* Delete loadReconstructedParticles() from your python script. *'
    print '* ************************************************************ *'
    print ''


def copyList(
    outputListName,
    inputListName,
    writeOut=False,
    path=analysis_main,
    ):
    """
    Copy all Particle indices from input ParticleList to the output ParticleList.

    @param ouputListName copied ParticleList
    @param inputListName original ParticleList to be copied
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path 
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', [inputListName])
    pmanipulate.param('writeOut', writeOut)
    path.add_module(pmanipulate)


def copyLists(
    outputListName,
    inputListNames,
    writeOut=False,
    path=analysis_main,
    ):
    """
    Copy all Particle indices from all input ParticleLists to the single output ParticleList.
    
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


def cutAndCopyLists(
    outputListName,
    inputListNames,
    cut,
    writeOut=False,
    path=analysis_main,
    ):
    """
    Copy Particle indices that pass selection criteria from all 
    input ParticleLists to the single output ParticleList.
    
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
    Copy Particle indices that pass selection criteria from 
    the input ParticleList to the output ParticleList.

    @param ouputListName copied ParticleList
    @param inputListName vector of original ParticleLists to be copied
    @param cut      selection criteria given in VariableManager style that copied Particles need to fullfill
    @param writeOut      wether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCutAndCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', [inputListName])
    pmanipulate.param('cut', cut)
    pmanipulate.param('writeOut', writeOut)
    path.add_module(pmanipulate)


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


def selectParticle(
    decayString,
    cut,
    writeOut=False,
    path=analysis_main,
    ):
    """
    THIS FUNCTION IS OBSOLETE. IT WILL BE DELETED.

    USE fillParticleList INSTEAD!
    """

    print ''
    print '* ******************************************************* *'
    print '* selectParticle is obsolete.                             *'
    print '* Replace it with fillParticleList in your python script. *'
    print '* ******************************************************* *'
    print ''

    fillParticleList(decayString, cut, writeOut, path)


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


def rankByHighest(
    particleList,
    variable,
    numBest=0,
    path=analysis_main,
    ):
    """
    Ranks particles in the input list by the given variable (highest to lowest), and stores an integer rank for each Particle
    in an extra-info field '${variable}_rank' starting at 1 (best). The list is also sorted from best to worst candidate
    (each charge, e.g. B+/B-, separately).
    This can be used to perform a best candidate selection by cutting on the corresponding rank value, or by specifying
    a non-zero value for 'numBest'.

    @param particleList  The input ParticleList
    @param variable      Variable to order Particles by.
    @param numBest       If not zero, only the $numBest Particles in particleList with rank <= numBest are kept.
    @param path          modules are added to this path
    """

    bcs = register_module('BestCandidateSelection')
    bcs.set_name('BestCandidateSelection_' + particleList + '_' + variable)
    bcs.param('particleList', particleList)
    bcs.param('variable', variable)
    bcs.param('numBest', numBest)
    path.add_module(bcs)


def rankByLowest(
    particleList,
    variable,
    numBest=0,
    path=analysis_main,
    ):
    """
    Ranks particles in the input list by the given variable (lowest to highest), and stores an integer rank for each Particle
    in an extra-info field '${variable}_rank' starting at 1 (best). The list is also sorted from best to worst candidate
    (each charge, e.g. B+/B-, separately).
    This can be used to perform a best candidate selection by cutting on the corresponding rank value, or by specifying
    a non-zero value for 'numBest'.

    @param particleList  The input ParticleList
    @param variable      Variable to order Particles by.
    @param numBest       If not zero, only the $numBest Particles in particleList with rank <= numBest are kept.
    @param path          modules are added to this path
    """

    bcs = register_module('BestCandidateSelection')
    bcs.set_name('BestCandidateSelection_' + particleList + '_' + variable)
    bcs.param('particleList', particleList)
    bcs.param('variable', variable)
    bcs.param('numBest', numBest)
    bcs.param('selectLowest', True)
    path.add_module(bcs)


def trainTMVAMethod(
    decayString,
    variables,
    methods=[('FastBDT', 'Plugin',
             '!H:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3'
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

    teacher = register_module('TMVATeacher')
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
    signalProbabilityName='isSignal',
    signalFraction=-1,
    signalClass=1,
    prefix='TMVA',
    workingDirectory='.',
    path=analysis_main,
    ):
    """
    Applies a trained TMVA method to a particle list
    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param method name of the TMVA method
    @param target name which is used to store signalProbability in extra info of the particle
    @param signalFraction to calculate probability, -1 if no transformation, -2 to use training signal/background
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
    expert.param('signalProbabilityName', signalProbabilityName)
    expert.param('signalFraction', signalFraction)
    expert.param('signalClass', signalClass)
    path.add_module(expert)


def isTMVAMethodAvailable(prefix='TMVA'):
    """
    True of a TMVA method with the given prefix was trained
    @param prefix prefix which is used to identify the weight files created by TMVA
    """

    return os.path.isfile(prefix + '.config')


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
        B2INFO('fitVertex: Geometry already in path')
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


def vertexRave(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=analysis_main,
    ):
    """
    Perform vertex fit using the RAVE for each Particle in the given ParticleList.

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
        'rave',
        'vertex',
        constraint,
        path,
        )


def massVertexRave(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
    ):
    """
    Perform mass-constrained vertex fit using the RAVE for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'rave',
        'massvertex',
        '',
        path,
        )


def massRave(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
    ):
    """
    Perform mass fit using the RAVE for each Particle in the given ParticleList.
    7x7 error matrix of the mother particle must be defined

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'rave',
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
    MCassociation='',
    confidenceLevel=0.001,
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
        B2INFO('TagV: Geometry already in path')
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


def removeParticlesNotInLists(lists_to_keep, path=analysis_main):
    """
    Description: Removes all Particles that are not in a given list of ParticleLists (or daughters of those). All relations from/to Particles, daughter indices, and other ParticleLists are fixed.

    @param lists_to_keep Keep the Particles and their daughters in these ParticleLists.
    @param path      modules are added to this path
    """

    mod = register_module('RemoveParticlesNotInLists')
    mod.param('particleLists', lists_to_keep)
    path.add_module(mod)


