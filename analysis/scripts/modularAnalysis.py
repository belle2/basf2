# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
This module defines wrapper functions around the analysis modules. An overview can
be found at https://confluence.desy.de/display/BI/Physics+AnalysisSteering
"""

from basf2 import *
import os
import sys
import inspect
from vertex import *
from kinfit import *
from analysisPath import analysis_main
from variables import variables
import basf2_mva
import pdg


def setAnalysisConfigParams(configParametersAndValues, path=analysis_main):
    """
    Sets analysis configuration parameters.

    These are:

    - 'tupleStyle': 'Default' (default) or 'Laconic'
      o) defines the style of the branch name in the ntuple

    - 'mcMatchingVersion': Specifies what version of mc matching algorithm is going to be used:

          - 'MC5' - analysis of BelleII MC5
          - 'Belle' - analaysis of Belle MC
          - 'BelleII' (default) - all other cases

    @param configParametersAndValues dictionary of parameters and their values of the form {param1: value, param2: value, ...)
    @param modules are added to this path
    """

    conf = register_module('AnalysisConfiguration')

    allParameters = ['tupleStyle', 'mcMatchingVersion']

    keys = configParametersAndValues.keys()
    for key in keys:
        if key not in allParameters:
            allParametersString = ', '.join(allParameters)
            B2ERROR('Invalid analysis configuration parameter: ' + key + '.\n'
                    'Please use one of the following: ' + allParametersString)

    for param in allParameters:
        if param in configParametersAndValues:
            conf.param(param, configParametersAndValues.get(param))

    path.add_module(conf)


def inputMdst(environmentType, filename, path=analysis_main, skipNEvents=0, entrySequence=None, *, parentLevel=0):
    """
    Loads the specified ROOT (DST/mDST/muDST) file with the RootInput module.

    The correct environment (e.g. magnetic field settings) are determined from
    the specified environment type. For the possible values please see
    `inputMdstList()`

    Parameters:
        environmentType (str): type of the environment to be loaded
        filename (str): the name of the file to be loaded
        path (basf2.Path): modules are added to this path
        skipNEvents (int): N events of the input file are skipped
        entrySequence (str): The number sequences (e.g. 23:42,101) defining the entries which are processed.
        parentLevel (int): Number of generations of parent files (files used as input when creating a file) to be read
    """
    if entrySequence is not None:
        entrySequence = [entrySequence]

    inputMdstList(environmentType, [filename], path, skipNEvents, entrySequence, parentLevel=parentLevel)


def inputMdstList(environmentType, filelist, path=analysis_main, skipNEvents=0, entrySequences=None, *, parentLevel=0):
    """
    Loads the specified ROOT (DST/mDST/muDST) files with the RootInput module.

    The correct environment (e.g. magnetic field settings) are determined from the specified environment type.
    The currently available environments are:

    - 'MC5': for analysis of Belle II MC samples produced with releases prior to build-2016-05-01.
      This environment sets the constant magnetic field (B = 1.5 T)
    - 'MC6': for analysis of Belle II MC samples produced with build-2016-05-01 or newer but prior to release-00-08-00
    - 'MC7': for analysis of Belle II MC samples produced with build-2016-05-01 or newer but prior to release-00-08-00
    - 'MC8', for analysis of Belle II MC samples produced with release-00-08-00 or newer but prior to release-02-00-00
    - 'MC9', for analysis of Belle II MC samples produced with release-00-08-00 or newer but prior to release-02-00-00
    - 'MC10', for analysis of Belle II MC samples produced with release-00-08-00 or newer but prior to release-02-00-00
    - 'default': for analysis of Belle II MC samples produced with releases with release-02-00-00 or newer.
      This environment sets the default magnetic field (see geometry settings)
    - 'Belle': for analysis of converted (or during of conversion of) Belle MC/DATA samples
    - 'None': for analysis of generator level information or during simulation/reconstruction of
      previously generated events

    Note that there is no difference between MC6 and MC7. Both are given for sake of completion.
    The same is true for MC8, MC9 and MC10

    Parameters:
        environmentType (str): type of the environment to be loaded
        filelist (list(str)): the filename list of files to be loaded
        path (basf2.Path): modules are added to this path
        skipNEvents (int): N events of the input files are skipped
        entrySequences (list(str)): The number sequences (e.g. 23:42,101) defining
            the entries which are processed for each inputFileName.
        parentLevel (int): Number of generations of parent files (files used as input when creating a file) to be read
    """

    roinput = register_module('RootInput')
    roinput.param('inputFileNames', filelist)
    roinput.param('skipNEvents', skipNEvents)
    if entrySequences is not None:
        roinput.param('entrySequences', entrySequences)
    roinput.param('parentLevel', parentLevel)

    path.add_module(roinput)
    progress = register_module('ProgressBar')
    path.add_module(progress)

    # None means don't create custom magnetic field, use whatever comes from the
    # DB
    environToMagneticField = {'MC5': 'MagneticFieldConstant',
                              'MC6': None,
                              'MC7': None,
                              'default': None,
                              'Belle': 'MagneticFieldConstantBelle'}

    fixECLClusters = {'MC5': True,
                      'MC6': True,
                      'MC7': True,
                      'default': False,
                      'Belle': False}

    if environmentType in environToMagneticField:
        fieldType = environToMagneticField[environmentType]
        if fieldType is not None:
            from ROOT import Belle2  # reduced scope of potentially-misbehaving import
            field = Belle2.MagneticField()
            field.addComponent(Belle2.MagneticFieldComponentConstant(Belle2.B2Vector3D(0, 0, 1.5 * Belle2.Unit.T)))
            Belle2.DBStore.Instance().addConstantOverride("MagneticField", field, False)
    elif environmentType in ["MC8", "MC9", "MC10"]:
        # make sure the last database setup is the magnetic field for MC8-10
        use_database_chain()
        use_central_database("Legacy_MagneticField_MC8_MC9_MC10", "", "", "centraldb",
                             loglevel=LogLevel.INFO, invertLogging=True)
    elif environmentType is 'None':
        B2INFO('No magnetic field is loaded. This is OK, if generator level information only is studied.')
    else:
        environments = ' '.join(list(environToMagneticField.keys()) + ["MC8", "MC9", "MC10"])
        B2FATAL('Incorrect environment type provided: ' + environmentType + '! Please use one of the following:' + environments)

    # set the correct MCMatching algorithm for MC5 and Belle MC
    if environmentType is 'Belle':
        setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)
    if environmentType is 'MC5':
        setAnalysisConfigParams({'mcMatchingVersion': 'MC5'}, path)

    # fixECLCluster for MC5/MC6/MC7
    if fixECLClusters.get(environmentType) is True:
        fixECL = register_module('FixECLClusters')
        path.add_module(fixECL)


def outputMdst(filename, path=analysis_main):
    """
    Saves mDST (mini-Data Summary Tables) to the output root file.
    """

    import mdst
    mdst.add_mdst_output(path, mc=True, filename=filename)


def outputUdst(filename, particleLists=[], includeArrays=[], path=analysis_main, dataDescription=None):
    """
    Save uDST (micro-Data Summary Tables) = MDST + Particles + ParticleLists
    The charge-conjugate lists of those given in particleLists are also stored.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    Note that this does not reduce the amount of Particle objects saved,
    see skimOutputUdst() for a function that does.
    """

    import mdst
    import pdg
    # also add anti-particle lists
    plSet = set(particleLists)
    for List in particleLists:
        name, label = List.split(':')
        plSet.add(pdg.conjugate(name) + ':' + label)

    partBranches = ['Particles', 'ParticlesToMCParticles',
                    'ParticlesToPIDLikelihoods', 'ParticleExtraInfoMap',
                    'EventExtraInfo'] + includeArrays + list(plSet)

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.setdefault("dataLevel", "udst")

    return mdst.add_mdst_output(path, mc=True, filename=filename,
                                additionalBranches=partBranches,
                                dataDescription=dataDescription)


def skimOutputUdst(skimDecayMode, skimParticleLists=[], outputParticleLists=[], includeArrays=[], path=analysis_main, *,
                   outputFile=None, dataDescription=None):
    """
    Create a new path for events that contain a non-empty particle list specified via skimParticleLists.
    Write the accepted events as a udst file, saving only particles from skimParticleLists
    and from outputParticleLists.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    :param str skimDecayMode: Name of the skim. If no outputFile is given this is
        also the name of the output filename. This name will be added to the
        FileMetaData as an extra data description "skimDecayMode"
    :param list(str) skimParticleLists: Names of the particle lists to skim for.
        An event will be accepted if at least one of the particle lists is not empty
    :param list(str) outputParticleLists: Names of the particle lists to store in
        the output in addition to the ones in skimParticleLists
    :param list(str) includeArrays: datastore arrays/objects to write to the output
        file in addition to mdst and particle information
    :param basf2.Path path: Path to add the skim output to. Defaults to the default analysis path
    :param str outputFile: Name of the output file if different from the skim name
    :param dict dataDescription: Additional data descriptions to add to the output file. For example {"mcEventType":"mixed"}
    """

    # if no outputfile is specified, set it to the skim name
    if outputFile is None:
        outputFile = skimDecayMode

    # make sure the output filename has the correct extension
    if not outputFile.endswith(".udst.root"):
        outputFile += ".udst.root"

    skimfilter = register_module('SkimFilter')
    skimfilter.set_name('SkimFilter_' + skimDecayMode)
    skimfilter.param('particleLists', skimParticleLists)
    path.add_module(skimfilter)
    filter_path = create_path()
    skimfilter.if_value('=1', filter_path, AfterConditionPath.CONTINUE)

    # add_independent_path() is rather expensive, only do this for skimmed events
    skim_path = create_path()
    saveParticleLists = skimParticleLists + outputParticleLists
    removeParticlesNotInLists(saveParticleLists, path=skim_path)

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.setdefault("skimDecayMode", skimDecayMode)
    outputUdst(outputFile, saveParticleLists, includeArrays, path=skim_path,
               dataDescription=dataDescription)
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)


def outputIndex(filename, path, includeArrays=[], keepParents=False, mc=True):
    """
    Write out all particle lists as an index file to be reprocessed using parentLevel flag.
    Additional branches necessary for file to be read are automatically included.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    @param str filename the name of the output index file
    @param str path modules are added to this path
    @param list(str) includeArrays: datastore arrays/objects to write to the output
        file in addition to particl lists and related information
    @param bool keepParents whether the parents of the input event will be saved as the parents of the same event
        in the output index file. Useful if you are only adding more information to another index file
    @param bool mc whether the input data is MC or not
    """

    # Module to mark all branches to not be saved except particle lists
    onlyPLists = register_module('OnlyWriteOutParticleLists')
    path.add_module(onlyPLists)

    # Set up list of all other branches we need to make index file complete
    partBranches = [
        'Particles',
        'ParticlesToMCParticles',
        'ParticlesToPIDLikelihoods',
        'ParticleExtraInfoMap',
        'EventExtraInfo'
    ]
    branches = ['EventMetaData']
    persistentBranches = ['FileMetaData']
    if mc:
        branches += []
        # persistentBranches += ['BackgroundInfos']
    branches += partBranches
    branches += includeArrays

    r1 = register_module('RootOutput')
    r1.param('outputFileName', filename)
    r1.param('additionalBranchNames', branches)
    r1.param('branchNamesPersistent', persistentBranches)
    r1.param('keepParents', keepParents)
    path.add_module(r1)


def setupEventInfo(noEvents, path=analysis_main):
    """
    Prepare to generate events. This function sets up the EventInfoSetter.
    You should call this before adding a generator from generators.
    The experiment and run numbers are set to 0 (run independent generic MC in phase 3).
    https://confluence.desy.de/display/BI/Experiment+numbering

    Parameters:
        noEvents (int): number of events to be generated
        path (basf2.Path): modules are added to this path
    """
    evtnumbers = register_module('EventInfoSetter')
    evtnumbers.param('evtNumList', [noEvents])
    evtnumbers.param('runList', [0])
    evtnumbers.param('expList', [0])
    path.add_module(evtnumbers)


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
    @param writeOut      whether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    copyLists(outputListName, [inputListName], writeOut, path)


def correctFSR(
    outputListName,
    inputListName,
    gammaListName,
    angleThreshold=5.0,
    energyThreshold=1.0,
    writeOut=False,
    path=analysis_main,
):
    """
    Takes the particles from the given lepton list copies them to the output list and adds the
    4-vector of the closest photon (considered as radiative) to the lepton, if the given
    criteria for maximal angle and energy are fulfilled.
    Please note, a new lepton is generated, with the old electron and -if found- a gamma as daughters.
    Information attached to the track is only available for the old lepton, accessable via the daughter
    metavariable, e.g. <daughter(0, eid)>.

    @param outputListName The output lepton list containing the corrected leptons.
    @param inputListName The initial lepton list containing the leptons to correct, should already exists.
    @param gammaListName The gammas list containing possibly radiative gammas, should already exist..
    @param angleThreshold The maximum angle (in degrees) between the lepton and the (radiative) gamma to be accepted.
    @param energyThreshold The maximum energy of the (radiative) gamma to be accepted.
    @param writeOut      whether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    fsrcorrector = register_module('FSRCorrection')
    fsrcorrector.set_name('FSRCorrection_' + outputListName)
    fsrcorrector.param('inputListName', inputListName)
    fsrcorrector.param('outputListName', outputListName)
    fsrcorrector.param('gammaListName', gammaListName)
    fsrcorrector.param('angleThreshold', angleThreshold)
    fsrcorrector.param('energyThreshold', energyThreshold)
    fsrcorrector.param('writeOut', writeOut)
    path.add_module(fsrcorrector)


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
    @param writeOut      whether RootOutput module should save the created ParticleList
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
    @param writeOut      whether RootOutput module should save the created ParticleList
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
    @param writeOut      whether RootOutput module should save the created ParticleList
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
    @param writeOut      whether RootOutput module should save the created ParticleList
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
                      path=analysis_main,
                      enforceFitHypothesis=False):
    """
    Creates Particles of the desired types from the corresponding ``mdst`` dataobjects,
    loads them to the ``StoreArray<Particle>`` and fills the ParticleLists.

    The multiple ParticleLists with their own selection criteria are specified
    via list tuples (decayString, cut), for example

    .. code-block:: python

        kaons = ('K+:mykaons', 'kaonID>0.1')
        pions = ('pi+:mypions','pionID>0.1')
        fillParticleLists([kaons, pions])

    If you are unsure what selection you want, you might like to see the
    :doc:`StandardParticles` functions.

    The type of the particles to be loaded is specified via the decayString module parameter.
    The type of the ``mdst`` dataobject that is used as an input is determined from the type of
    the particle. The following types of the particles can be loaded:

    * charged final state particles (input ``mdst`` type = Tracks)
        - e+, mu+, pi+, K+, p, deuteron (and charge conjugated particles)

    * neutral final state particles
        - "gamma"           (input ``mdst`` type = ECLCluster)
        - "K_S0", "Lambda0" (input ``mdst`` type = V0)
        - "K_L0"            (input ``mdst`` type = KLMCluster or ECLCluster)

    Note:
        For "K_S0" and "Lambda0" you must specify the daughter ordering.

    For example, to load V0s as :math:`\\Lambda^0\\to p^+\\pi^-` decays from V0s:

    .. code-block:: python

        v0lambdas = ('Lambda0 -> p+ pi-', '0.9 < M < 1.3')
        fillParticleLists([kaons, pions, v0lambdas], path=mypath)

    Tip:
        For "K_L0" it is now possible to load from ECLClusters, to revert to
        the old (Belle) behavior, you can require ``'isFromKLM > 0'``.

    .. code-block:: python

        klongs = ('K_L0', 'isFromKLM > 0')
        fillParticleLists([kaons, pions, klongs], path=mypath)


    Parameters:
        decayStringsWithCuts (list): A list of python ntuples of (decayString, cut).
                                     The decay string determines the type of Particle
                                     and determines the of the ParticleList.
                                     If the input MDST type is V0 the whole
                                     decay chain needs to be specified, so that
                                     the user decides and controls the daughters
                                     ' order (e.g. ``K_S0 -> pi+ pi-``)
                                     The cut is the selection criteria
                                     to be added to the ParticleList. It can be an empty string.
        writeOut (bool):             whether RootOutput module should save the created ParticleList
        path (basf2.Path):           modules are added to this path
        enforceFitHypothesis (bool): If true, Particles will be created only for the tracks which have been fitted
                                     using a mass hypothesis of the exact type passed to fillParticleLists().
                                     If enforceFitHypothesis is False (the default) the next closest fit hypothesis
                                     in terms of mass difference will be used if the fit using exact particle
                                     type is not available.
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + 'PLists')
    pload.param('decayStringsWithCuts', decayStringsWithCuts)
    pload.param('writeOut', writeOut)
    pload.param("enforceFitHypothesis", enforceFitHypothesis)
    path.add_module(pload)


def fillParticleList(
    decayString,
    cut,
    writeOut=False,
    path=analysis_main,
    enforceFitHypothesis=False
):
    """
    Creates Particles of the desired type from the corresponding ``mdst`` dataobjects,
    loads them to the StoreArray<Particle> and fills the ParticleList.

    See also:
        the :doc:`StandardParticles` functions.

    The type of the particles to be loaded is specified via the decayString module parameter.
    The type of the ``mdst`` dataobject that is used as an input is determined from the type of
    the particle. The following types of the particles can be loaded:

    * charged final state particles (input ``mdst`` type = Tracks)
        - e+, mu+, pi+, K+, p, deuteron (and charge conjugated particles)

    * neutral final state particles
        - "gamma"           (input ``mdst`` type = ECLCluster)
        - "K_S0", "Lambda0" (input ``mdst`` type = V0)
        - "K_L0"            (input ``mdst`` type = KLMCluster or ECLCluster)

    Note:
        For "K_S0" and "Lambda0" you must specify the daughter ordering.

    For example, to load V0s as :math:`\\Lambda^0\\to p^+\\pi^-` decays from V0ss:

    .. code-block:: python

        fillParticleList('Lambda0 -> p+ pi-', '0.9 < M < 1.3', path=mypath)

    Tip:
        For "K_L0" it is now possible to load from ECLClusters, to revert to
        the old (Belle) behavior, you can require ``'isFromKLM > 0'``.

    .. code-block:: python

        fillParticleList('K_L0', 'isFromKLM > 0', path=mypath)

    Parameters:
        decayString (str):           Type of Particle and determines the name of the ParticleList.
                                     If the input MDST type is V0 the whole decay chain needs to be specified, so that
                                     the user decides and controls the daughters' order (e.g. ``K_S0 -> pi+ pi-``)
        cut (str):                   Particles need to pass these selection criteria to be added to the ParticleList
        writeOut (bool):             whether RootOutput module should save the created ParticleList
        path (basf2.Path):           modules are added to this path
        enforceFitHypothesis (bool): If true, Particles will be created only for the tracks which have been fitted
                                     using a mass hypothesis of the exact type passed to fillParticleLists().
                                     If enforceFitHypothesis is False (the default) the next closest fit hypothesis
                                     in terms of mass difference will be used if the fit using exact particle
                                     type is not available.
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStringsWithCuts', [(decayString, cut)])
    pload.param('writeOut', writeOut)
    pload.param("enforceFitHypothesis", enforceFitHypothesis)
    path.add_module(pload)


def fillParticleListWithTrackHypothesis(
    decayString,
    cut,
    hypothesis,
    writeOut=False,
    enforceFitHypothesis=False,
    path=analysis_main,
):
    """
    As fillParticleList, but if used for a charged FSP, loads the particle with the requested hypothesis if available

    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut           Particles need to pass these selection criteria to be added to the ParticleList
    @param hypothesis    the PDG code of the desired track hypothesis
    @param writeOut      whether RootOutput module should save the created ParticleList
    @param enforceFitHypothesis If true, Particles will be created only for the tracks which have been fitted
                                using a mass hypothesis of the exact type passed to fillParticleLists().
                                If enforceFitHypothesis is False (the default) the next closest fit hypothesis
                                in terms of mass difference will be used if the fit using exact particle
                                type is not available.
    @param path          modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStringsWithCuts', [(decayString, cut)])
    pload.param('trackHypothesis', hypothesis)
    pload.param('writeOut', writeOut)
    pload.param("enforceFitHypothesis", enforceFitHypothesis)
    path.add_module(pload)


def fillConvertedPhotonsList(
    decayString,
    cut,
    writeOut=False,
    path=analysis_main,
):
    """
    Creates photon Particle object for each e+e- combination in the V0 StoreArray.

    Note:
        You must specify the daughter ordering.

    .. code-block:: python

        fillConvertedPhotonsList('gamma:converted -> e+ e-', '')

    Parameters:
        decayString (str): Must be gamma to an e+e- pair. You muse specify the daughter ordering.
                           Will also determine the name of the particleList.
        cut (str):         Particles need to pass these selection criteria to be added to the ParticleList
        writeOut (bool):   whether RootOutput module should save the created ParticleList
        path (basf2.Path): modules are added to this path

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
    @param writeOut      whether RootOutput module should save the created ParticleList
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
    pions = ('pi+:gen', 'pionID>0.1')
    fillParticleListsFromMC([kaons, pions])

    @param decayString   specifies type of Particles and determines the name of the ParticleList
    @param cut           Particles need to pass these selection criteria to be added to the ParticleList
    @param addDaughters  adds the bottom part of the decay chain of the particle to the datastore and sets mother-daughter relations
    @param writeOut      whether RootOutput module should save the created ParticleList
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
    Removes StoreArray<Particle> indices of Particles from given ParticleList
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


def applyEventCuts(cut, path=analysis_main):
    """
    Removes events that do not pass the given selection criteria (given in ParticleSelector style).

    @param cut  Events that do not pass these selection criteria are skipped
    @param path      modules are added to this path
    """

    eselect = register_module('VariableToReturnValue')
    eselect.param('variable', 'passesEventCut(' + cut + ')')
    path.add_module(eselect)
    empty_path = create_path()
    eselect.if_value('<1', empty_path)


def reconstructDecay(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=analysis_main,
    candidate_limit=None,
    ignoreIfTooManyCandidates=True,
):
    """
    Creates new Particles by making combinations of existing Particles - it reconstructs unstable particles via
    their specified decay mode, e.g. in form of a DecayString: D0 -> K- pi+; B+ -> anti-D0 pi+, .... All
    possible combinations are created (overlaps are forbidden) and combinations that pass the specified selection
    criteria are saved to a newly created (mother) ParticleList. By default the charge conjugated decay is
    reconstructed as well (meaning that the charge conjugated mother list is created as well). One cay use an
    at-sign '@' to mark a particle as inclusive, e.g. in form of a DecayString: '@Xsd -> K+ pi-'. If a particle
    is marked as inclusive, it does not fire the flag of c_AddedWrongParticle. In the sum-of-exclusive analysis,
    one can use a inclusive particle with some different resonance with a same code.

    @param decayString :ref:`DecayString` specifying what kind of the decay should be reconstructed
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut         created (mother) Particles are added to the mother ParticleList if they
                       pass give cuts (in VariableManager style) and rejected otherwise
    @param dmID        user specified decay mode identifier
    @param writeOut    whether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    @param candidate_limit Maximum amount of candidates to be reconstructed. If
                       the number of candidates is exceeded a Warning will be
                       printed.
                       By default, all these candidates will be removed and event will be ignored.
                       This behaviour can be changed by \'ignoreIfTooManyCandidates\' flag.
                       If no value is given the amount is limited to a sensible
                       default. A value <=0 will disable this limit and can
                       cause huge memory amounts so be careful.
    @param ignoreIfTooManyCandidates whether event should be ignored or not if number of reconstructed
                       candidates reaches limit. If event is ignored, no candidates are reconstructed,
                       otherwise, number of candidates in candidate_limit is reconstructed.
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('decayMode', dmID)
    pmake.param('writeOut', writeOut)
    if candidate_limit is not None:
        pmake.param("maximumNumberOfCandidates", candidate_limit)
    pmake.param("ignoreIfTooManyCandidates", ignoreIfTooManyCandidates)
    path.add_module(pmake)


def reconstructMissingKlongDecayExpert(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=analysis_main,
    recoList="_reco",
):
    """
    Creates mother particle accounting for missing momentum.

    @param decayString DecayString specifying what kind of the decay should be reconstructed
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut         created (mother) Particles are added to the mother ParticleList if they
                       pass give cuts (in VariableManager style) and rejected otherwise
    @param dmID        user specified decay mode identifier
    @param writeOut    whether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    @param recoList    suffix appended to original K_L0 ParticleList that identifies the newly created K_L0 list
    """

    pcalc = register_module('KlongMomentumCalculatorExpert')
    pcalc.set_name('KlongMomentumCalculatorExpert_' + decayString)
    pcalc.param('decayString', decayString)
    pcalc.param('cut', cut)
    pcalc.param('decayMode', dmID)
    pcalc.param('writeOut', writeOut)
    pcalc.param('recoList', recoList)
    analysis_main.add_module(pcalc)

    rmake = register_module('KlongDecayReconstructorExpert')
    rmake.set_name('KlongDecayReconstructorExpert_' + decayString)
    rmake.param('decayString', decayString)
    rmake.param('cut', cut)
    rmake.param('decayMode', dmID)
    rmake.param('writeOut', writeOut)
    rmake.param('recoList', recoList)
    analysis_main.add_module(rmake)


def replaceMass(
    replacerName,
    particleLists=[],
    pdgCode=22,
    path=analysis_main,
):
    """
    replaces the mass of the particles inside the given particleLists
    with the invariant mass of the particle corresponding to the given pdgCode.

    @param particleLists new ParticleList filled with copied Particles
    @param pdgCode PDG   code for mass reference
    @param path          modules are added to this path
    """

    # first copy original particles to the new ParticleList
    pmassupdater = register_module('ParticleMassUpdater')
    pmassupdater.set_name('ParticleMassUpdater_' + replacerName)
    pmassupdater.param('particleLists', particleLists)
    pmassupdater.param('pdgCode', pdgCode)
    path.add_module(pmassupdater)


def reconstructRecoil(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=analysis_main,
    candidate_limit=None,
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
    @param dmID        user specified decay mode identifier
    @param writeOut    whether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    @param candidate_limit Maximum amount of candidates to be reconstructed. If
                       the number of candidates is exceeded no candidate will be
                       reconstructed for that event and a Warning will be
                       printed.
                       If no value is given the amount is limited to a sensible
                       default. A value <=0 will disable this limit and can
                       cause huge memory amounts so be careful.
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('decayMode', dmID)
    pmake.param('writeOut', writeOut)
    pmake.param('recoilParticleType', 1)
    if candidate_limit is not None:
        pmake.param("maximumNumberOfCandidates", candidate_limit)
    path.add_module(pmake)


def reconstructRecoilDaughter(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=analysis_main,
    candidate_limit=None,
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
    @param dmID        user specified decay mode identifier
    @param writeOut    whether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    @param candidate_limit Maximum amount of candidates to be reconstructed. If
                       the number of candidates is exceeded no candidate will be
                       reconstructed for that event and a Warning will be
                       printed.
                       If no value is given the amount is limited to a sensible
                       default. A value <=0 will disable this limit and can
                       cause huge memory amounts so be careful.
    """

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('decayMode', dmID)
    pmake.param('writeOut', writeOut)
    pmake.param('recoilParticleType', 2)
    if candidate_limit is not None:
        pmake.param("maximumNumberOfCandidates", candidate_limit)
    path.add_module(pmake)


def rankByHighest(
    particleList,
    variable,
    numBest=0,
    outputVariable='',
    allowMultiRank=False,
    cut='',
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
    @param allowMultiRank   If true, candidates with the same value will get the same rank.
    @param cut              Only candidates passing the cut will be ranked. The others will have rank -1
    @param path             modules are added to this path
    """

    bcs = register_module('BestCandidateSelection')
    bcs.set_name('BestCandidateSelection_' + particleList + '_' + variable)
    bcs.param('particleList', particleList)
    bcs.param('variable', variable)
    bcs.param('numBest', numBest)
    bcs.param('outputVariable', outputVariable)
    bcs.param('allowMultiRank', allowMultiRank)
    bcs.param('cut', cut)
    path.add_module(bcs)


def rankByLowest(
    particleList,
    variable,
    numBest=0,
    outputVariable='',
    allowMultiRank=False,
    cut='',
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
    @param allowMultiRank   If true, candidates with the same value will get the same rank.
    @param cut              Only candidates passing the cut will be ranked. The others will have rank -1
    @param path             modules are added to this path
    """

    bcs = register_module('BestCandidateSelection')
    bcs.set_name('BestCandidateSelection_' + particleList + '_' + variable)
    bcs.param('particleList', particleList)
    bcs.param('variable', variable)
    bcs.param('numBest', numBest)
    bcs.param('selectLowest', True)
    bcs.param('allowMultiRank', allowMultiRank)
    bcs.param('outputVariable', outputVariable)
    bcs.param('cut', cut)
    path.add_module(bcs)


def printDataStore(eventNumber=-1, path=analysis_main):
    """
    Prints the contents of DataStore in the first event (or a specific event number or all events).
    Will list all objects and arrays (including size).

    See also:
        The command line tool: ``b2file-size``.

    Parameters:
        eventNumber (int): Print the datastore only for this event. The default
            (-1) prints only the first event, 0 means print for all events (can produce large output)
        path (basf2.Path): the PrintCollections module is added to this path

    Warning:
        This will print a lot of output if you print it for all events and process many events.

    """

    printDS = register_module('PrintCollections')
    printDS.param('printForEvent', eventNumber)
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
    Warning:
        NtupleTools are going to be deprecated from release-03 and we aim to
        remove them from release-04.  Please see
        `modularAnalysis.variablesToNtuple` for the recommended alternative.

    Creates new ROOT file to which the flat ntuples will be saved.

    Parameters:
        file_name (str): file name of the output root file
        path (basf2.Path): modules are added to this path
    """

    message = (
        "NtupleTools are going to be deprecated from release-03 and we"
        " aim to remove them from release-04.\nPlease see "
        "modularAnalysis.variablesToNtuple for the recommended alternative."
    )
    B2WARNING(message)

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
    Warning:
        NtupleTools are going to be deprecated from release-03 and we aim to
        remove them from release-04.  Please see
        `modularAnalysis.variablesToNtuple` for the recommended alternative.

    Creates and fills flat ntuple (TTree) with the specified Ntuple tools.

    Parameters:
        tree_name (str): the output nutple (TTree) name
        list_name (str): input ParticleList name
        tools (list(str)): list of Ntuple tools to be included, tool-decaystring pairs.
    """

    message = (
        "NtupleTools are going to be deprecated from release-03 and we"
        " aim to remove them from release-04.\nPlease see "
        "modularAnalysis.variablesToNtuple for the recommended alternative."
    )
    B2WARNING(message)

    ntmaker = register_module('NtupleMaker')
    ntmaker.set_name('NtupleMaker_ntupleTree_' + list_name)
    ntmaker.param('treeName', tree_name)
    ntmaker.param('listName', list_name)
    ntmaker.param('tools', tools)
    path.add_module(ntmaker)


def variablesToNtuple(
    decayString,
    variables,
    treename='variables',
    filename='ntuple.root',
    path=analysis_main,
):
    """
    Creates and fills a flat ntuple with the specified variables from the VariableManager.
    If a decayString is provided, then there will be one entry per candidate (for particle in list of candidates).
    If an empty decayString is provided, there will be one entry per event (useful for trigger studies, etc).

    Parameters:
        decayString (str): specifies type of Particles and determines the name of the ParticleList
        variables (list(str)): the list of variables (which must be registered in the VariableManager)
        treename (str): name of the ntuple tree
        filename (str): which is used to store the variables
        path (basf2.Path): the basf2 path where the analysis is processed
    """

    output = register_module('VariablesToNtuple')
    output.set_name('VariablesToNtuple_' + decayString)
    output.param('particleList', decayString)
    output.param('variables', variables)
    output.param('fileName', filename)
    output.param('treeName', treename)
    path.add_module(output)


def variablesToHistogram(
    decayString,
    variables,
    variables_2d=[],
    filename='ntuple.root',
    path=analysis_main,
):
    """
    Creates and fills a flat ntuple with the specified variables from the VariableManager

    Parameters:
        decayString (str): specifies type of Particles and determines the name of the ParticleList
        variables (list(tuple))): variables + binning which must be registered in the VariableManager
        variables_2d (list(tuple)): pair of variables + binning for each which must be registered in the VariableManager
        filename (str): which is used to store the variables
        path (basf2.Path): the basf2 path where the analysis is processed
    """

    output = register_module('VariablesToHistogram')
    output.set_name('VariablesToHistogram_' + decayString)
    output.param('particleList', decayString)
    output.param('variables', variables)
    output.param('variables_2d', variables_2d)
    output.param('fileName', filename)
    path.add_module(output)


def variablesToExtraInfo(
    particleList,
    variables,
    option=0,
    path=analysis_main,
):
    """
    For each particle in the input list the selected variables are saved in an extra-info field with the given name.
    Can be used when wanting to save variables before modifying them, e.g. when performing vertex fits.

    An existing extra info with the same name will be overwritten if the new
    value is lower / will never be overwritten / will be overwritten if the
    new value is higher / will always be overwritten (-1/0/1/2).

    @param particleList  The input ParticleList
    @param variables     Dictionary of Variables and extraInfo names.
    @param path          modules are added to this path
    """

    mod = register_module('VariablesToExtraInfo')
    mod.set_name('VariablesToExtraInfo_' + particleList)
    mod.param('particleList', particleList)
    mod.param('variables', variables)
    mod.param('overwrite', option)
    path.add_module(mod)


def variablesToDaughterExtraInfo(
    particleList,
    decayString,
    variables,
    option=0,
    path=analysis_main,
):
    """
    For each daughter particle specified via decay string the selected variables (estimated for the mother particle)
    are saved in an extra-info field with the given name. In other words, the property of mother is saved as extra-info
    to specified daughter particle.

    An existing extra info with the same name will be overwritten if the new
    value is lower / will never be overwritten / will be overwritten if the
    new value is higher / will always be overwritten (-1/0/1/2).

    @param particleList  The input ParticleList
    @param decayString   Decay string that specifies to which daughter the extra info should be appended
    @param variables     Dictionary of Variables and extraInfo names.
    @param option        Various options for overwriting
    @param path          modules are added to this path
    """

    mod = register_module('VariablesToExtraInfo')
    mod.set_name('VariablesToDaughterExtraInfo_' + particleList)
    mod.param('particleList', particleList)
    mod.param('decayString', decayString)
    mod.param('variables', variables)
    mod.param('overwrite', option)
    path.add_module(mod)


def variableToSignalSideExtraInfo(
    particleList,
    varToExtraInfo,
    path,
):
    """
    Write the value of specified variables estimated for the single particle in the input list (has to contain exactly 1
    particle) as an extra info to the particle related to current ROE.
    Should be used only in the for_each roe path.

    @param particleList  The input ParticleList
    @param varToExtraInfo Dictionary of Variables and extraInfo names.
    @param path          modules are added to this path
    """
    mod = register_module('SignalSideVariablesToExtraInfo')
    mod.set_name('SigSideVarToExtraInfo_' + particleList)
    mod.param('particleListName', particleList)
    mod.param('variableToExtraInfo', varToExtraInfo)
    path.add_module(mod)


def removeExtraInfo(particleLists=[], removeEventExtraInfo=False, path=analysis_main):
    """
    Removes the ExtraInfo of the given particleLists. If specified (removeEventExtraInfo = True) also the EventExtraInfo is removed.
    """

    mod = register_module('ExtraInfoRemover')
    mod.param('particleLists', particleLists)
    mod.param('removeEventExtraInfo', removeEventExtraInfo)
    path.add_module(mod)


def signalSideParticleFilter(
    particleList,
    selection,
    roe_path,
    deadEndPath
):
    """
    Checks if the current ROE object in the for_each roe path (argument roe_path) is related
    to the particle from the input ParticleList. Additional selection criteria can be applied.
    If ROE is not related to any of the Particles from ParticleList or the Particle doesn't
    meet the selection criteria the execution of deadEndPath is started. This path, as the name
    sugest should be empty and its purpose is to end the execution of for_each roe path for
    the current ROE object.

    @param particleList  The input ParticleList
    @param selection Selection criteria that Particle needs meet in order for for_each ROE path to continue
    @param for_each roe path in which this filter is executed
    @param deadEndPath empty path that ends execution of or_each roe path for the current ROE object.
    """
    mod = register_module('SignalSideParticleFilter')
    mod.set_name('SigSideParticleFilter_' + particleList)
    mod.param('particleLists', [particleList])
    mod.param('selection', selection)
    roe_path.add_module(mod)
    mod.if_false(deadEndPath)


def signalSideParticleListsFilter(
    particleLists,
    selection,
    roe_path,
    deadEndPath
):
    """
    Checks if the current ROE object in the for_each roe path (argument roe_path) is related
    to the particle from the input ParticleList. Additional selection criteria can be applied.
    If ROE is not related to any of the Particles from ParticleList or the Particle doesn't
    meet the selection criteria the execution of deadEndPath is started. This path, as the name
    sugest should be empty and its purpose is to end the execution of for_each roe path for
    the current ROE object.

    @param particleLists  The input ParticleLists
    @param selection Selection criteria that Particle needs meet in order for for_each ROE path to continue
    @param for_each roe path in which this filter is executed
    @param deadEndPath empty path that ends execution of or_each roe path for the current ROE object.
    """
    mod = register_module('SignalSideParticleFilter')
    mod.set_name('SigSideParticleFilter_' + particleLists[0])
    mod.param('particleLists', particleLists)
    mod.param('selection', selection)
    roe_path.add_module(mod)
    mod.if_false(deadEndPath)


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


def looseMCTruth(list_name, path=analysis_main):
    """
    Performs loose MC matching for all particles in the specified
    ParticleList.
    The difference between loose and normal mc matching algorithm is that
    the loose agorithm will find the common mother of the majority of daughter
    particles while the normal algorithm finds the common mother of all daughters.
    The results of loose mc matching algorithm are stored to the following extraInfo
    items:

      - looseMCMotherPDG: PDG code of most common mother
      - looseMCMotherIndex: 1-based StoreArray<MCParticle> index of most common mother
      - looseMCWrongDaughterN: number of daughters that don't originate from the most
                               common mother
      - looseMCWrongDaughterPDG: PDG code of the daughter that doesn't orginate from
                                 the most common mother
                                 (only if looseMCWrongDaughterN = 1)
      - looseMCWrongDaughterBiB: 1 if the wrong daughter is Beam Induced Background
                                 Particle

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    mcMatch = register_module('MCMatcherParticles')
    mcMatch.set_name('LooseMCMatch_' + list_name)
    mcMatch.param('listName', list_name)
    mcMatch.param('looseMCMatching', True)
    path.add_module(mcMatch)


def buildRestOfEvent(target_list_name, inputParticlelists=[], path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    dataobject and makes BASF2 relation between them. User can provide additional
    particle lists with a different particle hypotheses like ['K+:good, e+:good'], etc.

    @param target_list_name name of the input ParticleList
    @param inputParticlelists list of input particle list names, which serve
                              as a source of particles to build ROE, the FSP particles from
                              target_list_name are excluded from ROE object
    @param path      modules are added to this path
    """
    # if (len(inputParticlelists) < 3):
    fillParticleList('pi+:roe_default', '', path=path)
    fillParticleList('gamma:roe_default', '', path=path)
    fillParticleList('K_L0:roe_default', '', path=path)
    inputParticlelists += ['pi+:roe_default', 'gamma:roe_default', 'K_L0:roe_default']
    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('ROEBuilder_' + target_list_name)
    roeBuilder.param('particleList', target_list_name)
    roeBuilder.param('particleListsInput', inputParticlelists)
    path.add_module(roeBuilder)


def buildNestedRestOfEvent(target_list_name, maskName='', path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    """
    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('NestedROEBuilder_' + target_list_name)
    roeBuilder.param('particleList', target_list_name)
    roeBuilder.param('nestedROEMask', maskName)
    roeBuilder.param('createNestedROE', True)
    path.add_module(roeBuilder)


def appendROEMask(
    list_name,
    mask_name,
    trackSelection,
    eclClusterSelection,
    fractions=[0, 0, 1, 0, 0, 0],
    path=analysis_main
):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for tracks and eclClusters which will be used by variables in ROEVariables.cc.
    A-priori ChargedStable fractions can be provided, otherwise pion mass hypothesis is used.

    - append a ROE mask with all tracks in ROE coming from the IP region

       >>> appendROEMask('B+:sig', 'IPtracks', 'abs(d0) < 0.05 and abs(z0) < 0.1', '')

    - append a ROE mask with only ECLClusters that pass as good photon candidates

       >>> good_photons = 'theta > 0.296706 and theta < 2.61799 and clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]'
       >>> appendROEMask('B+:sig', 'goodROEGamma', '', good_photons)

    - append a ROE mask with track from IP, use equal a-priori probabilities

       >>> appendROEMask('B+:sig', 'IPAndGoodGamma', 'abs(d0) < 0.05 and abs(z0) < 0.1', good_photons, [1,1,1,1,1,1])

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


def appendROEMasks(list_name, mask_tuples, path=analysis_main):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for tracks and eclClusters which will be used by variables in ROEVariables.cc.
    A-priori ChargedStable fractions can be provided, otherwise pion mass hypothesis is used.

    The multiple ROE masks with their own selection criteria are specified
    via list of tuples (mask_name, trackSelection, eclClusterSelection) or
    (mask_name, trackSelection, eclClusterSelection, chargedStable fractions) in case with fractions.

    - Example for two tuples, one with and one without fractions

       >>> ipTracks     = ('IPtracks', 'abs(d0) < 0.05 and abs(z0) < 0.1', '')
       >>> good_photons = 'theta > 0.296706 and theta < 2.61799 and clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]'
       >>> goodROEGamma = ('ROESel', 'abs(d0) < 0.05 and abs(z0) < 0.1', good_photons, [1,1,1,1,1,1])
       >>> appendROEMasks('B+:sig', [ipTracks, goodROEGamma])

    @param list_name             name of the input ParticleList
    @param mask_tuples           array of ROEMask list tuples to be appended
    @param path                  modules are added to this path
    """

    new_tuples = []
    appendix = ([0, 0, 1, 0, 0, 0],)

    for entry in mask_tuples:
        if len(entry) == 4:
            new_tuples.append(entry)
        if len(entry) == 3:
            new_tuples.append(entry + appendix)

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + 'MaskList')
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasksWithFractions', new_tuples)
    path.add_module(roeMask)


def updateROEMask(
    list_name,
    mask_name,
    trackSelection,
    eclClusterSelection='',
    fractions=[],
    path=analysis_main
):
    """
    Update an existing ROE mask by applying additional selection cuts for tracks and/or clusters
    and change a-priori charged stable fractions. Empty string or array containers result
    in no change.

    See function `appendROEMask`!

    @param list_name             name of the input ParticleList
    @param mask_name             name of the ROEMask to update
    @param trackSelection        decay string for the tracks in ROE
    @param eclClusterSelection   decay string for the tracks in ROE
    @param fractions             chargedStable particle fractions
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + mask_name)
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasksWithFractions', [(mask_name, trackSelection, eclClusterSelection, fractions)])
    roeMask.param('update', True)
    path.add_module(roeMask)


def updateROEFractions(
    list_name,
    mask_name,
    fractions,
    path=analysis_main
):
    """
    Update chargedStable fractions for an existing ROE mask.

    @param list_name             name of the input ParticleList
    @param mask_name             name of the ROEMask to update
    @param fractions             chargedStable particle fractions
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + mask_name)
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasksWithFractions', [(mask_name, '', '', fractions)])
    roeMask.param('update', True)
    path.add_module(roeMask)


def updateROEMasks(
    list_name,
    mask_tuples,
    path=analysis_main
):
    """
    Update existing ROE masks by applying additional selection cuts for tracks and/or clusters
    and change a-priori charged stable fractions. Empty string or array containers result
    in no change.

    The multiple ROE masks with their own selection criteria are specified
    via list tuples (mask_name, trackSelection, eclClusterSelection) or
    (mask_name, trackSelection, eclClusterSelection, chargedStable fractions) in case with fractions.

    See function `appendROEMasks`!

    @param list_name             name of the input ParticleList
    @param mask_tuples           array of ROEMask list tuples to be appended
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + 'MaskList')
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasksWithFractions', mask_tuples)
    roeMask.param('update', True)
    path.add_module(roeMask)


def keepInROEMasks(
    list_name,
    mask_names,
    cut_string,
    fractions=[],
    path=analysis_main
):
    """
    This function is used to apply particle list specific cuts on one or more ROE masks (track or eclCluster).
    With this funciton one can KEEP the tracks/eclclusters used in particles from provided particle list.
    This function should be executed only in the for_each roe path for the current ROE object.

    To avoid unnecessary computation, the input particle list should only contain particles from ROE
    (use cut 'isInRestOfEvent == 1'). To update the ECLCluster masks, the input particle list should be a photon
    particle list (e.g. 'gamma:someLabel'). To update the Track masks, the input particle list should be a charged
    pion particle list (e.g. 'pi+:someLabel').

    It is possible to update a-priori fractions by providing them (see `appendROEMask()` and `updateROEFractions()`).
    Empty array will result in no change.

    Updating a non-existing mask will create a new one. If a-priori fractions for ChargedStable particles are not provided,
    pion-mass hypothesis will be used as default.

    - keep only those tracks that were used in provided particle list

       >>> keepInROEMasks('pi+:goodTracks', 'mask', '')

    - keep only those clusters that were used in provided particle list and pass a cut, apply to several masks

       >>> keepInROEMasks('gamma:goodClusters', ['mask1', 'mask2'], 'E > 0.1')

    - create a ROE mask on-the-fly with some fractions and with tracks used in provided particle list

       >>> keepInROEMasks('pi+:trueTracks', 'newMask', 'mcPrimary == 1', [1,1,1,1,1,1])
       >>> # - or use [-1] fractions to use true MC mass hypothesis


    @param list_name    name of the input ParticleList
    @param mask_names   array of ROEMasks to be updated
    @param cut_string   decay string with which the mask will be updated
    @param fractions    chargedStable particle fractions
    @param path         modules are added to this path
    """

    updateMask = register_module('RestOfEventUpdater')
    updateMask.set_name('RestOfEventUpdater_' + list_name + '_masks')
    updateMask.param('particleList', list_name)
    updateMask.param('updateMasks', mask_names)
    updateMask.param('cutString', cut_string)
    updateMask.param('fractions', fractions)
    updateMask.param('discard', False)
    path.add_module(updateMask)


def discardFromROEMasks(
    list_name,
    mask_names,
    cut_string,
    fractions=[],
    path=analysis_main
):
    """
    This function is used to apply particle list specific cuts on one or more ROE masks (track or eclCluster).
    With this funciton one can DISCARD the tracks/eclclusters used in particles from provided particle list.
    This function should be executed only in the for_each roe path for the current ROE object.

    To avoid unnecessary computation, the input particle list should only contain particles from ROE
    (use cut 'isInRestOfEvent == 1'). To update the ECLCluster masks, the input particle list should be a photon
    particle list (e.g. 'gamma:someLabel'). To update the Track masks, the input particle list should be a charged
    pion particle list (e.g. 'pi+:someLabel').

    It is possible to update a-priori fractions by providing them (see appendROEMask or updateROEFractions).
    Empty array will result in no change.

    Updating a non-existing mask will create a new one. If a-priori fractions for ChargedStable particles are not provided,
    pion-mass hypothesis will be used as default.

    - discard tracks that were used in provided particle list

       >>> discardFromROEMasks('pi+:badTracks', 'mask', '')

    - discard clusters that were used in provided particle list and pass a cut, apply to several masks

       >>> discardFromROEMasks('gamma:badClusters', ['mask1', 'mask2'], 'E < 0.1')

    - create a ROE mask on-the-fly with some fractions and with tracks NOT used in provided particle list

       >>> discardFromROEMasks('pi+:badTracks', 'newMask', 'mcPrimary != 1', [1,1,1,1,1,1])
       >>> # or use [-1] fractions to use true MC mass hypothesis

    @param list_name    name of the input ParticleList
    @param mask_names   array of ROEMasks to be updated
    @param cut_string   decay string with which the mask will be updated
    @param fractions    chargedStable particle fractions
    @param path         modules are added to this path
    """

    updateMask = register_module('RestOfEventUpdater')
    updateMask.set_name('RestOfEventUpdater_' + list_name + '_masks')
    updateMask.param('particleList', list_name)
    updateMask.param('updateMasks', mask_names)
    updateMask.param('cutString', cut_string)
    updateMask.param('fractions', fractions)
    updateMask.param('discard', True)
    path.add_module(updateMask)


def optimizeROEWithV0(
    list_name,
    mask_names,
    cut_string,
    fractions=[],
    path=analysis_main
):
    """
    This function is used to apply particle list specific cuts on one or more ROE masks for Tracks.
    It is possible to optimize the ROE selection by treating tracks from V0's separately, meaning,
    taking V0's 4-momentum into account instead of 4-momenta of tracks. A cut for only specific V0's
    passing it can be applied.

    The input particle list should be a V0 particle list: K_S0 ('K_S0:someLabel', ''),
    Lambda ('Lambda:someLabel', '') or converted photons ('gamma:someLabel')

    It is possible to update a-priori fractions by providing them (see appendROEMask and updateROEFractions).
    Empty array will result in no change.

    Updating a non-existing mask will create a new one. If a-priori fractions for ChargedStable particles are not provided,
    pion-mass hypothesis will be used as default.

    - treat tracks from K_S0 inside mass window separately, replace track momenta with K_S0 momentum

       >>> optimizeROEWithV0('K_S0:opt', 'mask', '0.450 < M < 0.550')

    @param list_name    name of the input ParticleList
    @param mask_names   array of ROEMasks to be updated
    @param cut_string   decay string with which the mask will be updated
    @param fractions    chargedStable particle fractions
    @param path         modules are added to this path
    """

    updateMask = register_module('RestOfEventUpdater')
    updateMask.set_name('RestOfEventUpdater_' + list_name + '_masks')
    updateMask.param('particleList', list_name)
    updateMask.param('updateMasks', mask_names)
    updateMask.param('cutString', cut_string)
    updateMask.param('fractions', fractions)
    path.add_module(updateMask)


def printROEInfo(
    mask_names=[],
    which_mask='both',
    full_print=False,
    path=None
):
    """
    This function prints out the information for the current ROE, so it should only be used in the for_each path.
    It prints out basic ROE object info.

    If mask names are provided, specific information for those masks will be printed out. By default, basic
    ECLCluster and Track mask info will be printed out, but it is possible to do this only for one, if needed.

    It is also possible to print out the specific mask values for each Track and ECLCluster by setting the 'full_print'
    option to True.

    @param mask_names   array of ROEMask names for printing out info
    @param which_mask   print out info for Tracks ('track'), ECLClusters ('cluster') or ('both')
    @param full_print   print out mask values for each Track/ECLCLuster in mask
    @param path         modules are added to this path
    """
    if not isinstance(path, basf2.Path):
        B2FATAL("Error from printROEInfo, please add this to the for_each path")

    printMask = register_module('RestOfEventPrinter')
    printMask.set_name('RestOfEventPrinter')
    printMask.param('maskNames', mask_names)
    printMask.param('whichMask', which_mask)
    printMask.param('fullPrint', full_print)
    path.add_module(printMask)


def buildContinuumSuppression(list_name, roe_mask, path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a ContinuumSuppression
    dataobject and makes BASF2 relation between them.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    qqBuilder = register_module('ContinuumSuppressionBuilder')
    qqBuilder.set_name('QQBuilder_' + list_name)
    qqBuilder.param('particleList', list_name)
    qqBuilder.param('ROEMask', roe_mask)
    path.add_module(qqBuilder)


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

    @param upsilon_list_name Name of the ParticleList to be filled with 'Upsilon(4S) -> B:sig anti-B:tag'
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


def selectDaughters(particle_list_name, decay_string, path=analysis_main):
    """
    Redefine the Daughters of a particle: select from decayString

    @param particle_list_name input particle list
    @param decay_string  for selecting the Daughters to be preserved
    """
    seld = register_module('SelectDaughters')
    seld.set_name('SelectDaughters_' + particle_list_name)
    seld.param('listName', particle_list_name)
    seld.param('decayString', decay_string)
    path.add_module(seld)


def markDuplicate(particleList, prioritiseV0, path=analysis_main):
    """
    Call DuplicateVertexMarker to find duplicate particles in a list and
    flag the ones that should be kept

    @param particleList input particle list
    @param prioritiseV0 if true, give V0s a higher priority
    """
    markdup = register_module('DuplicateVertexMarker')
    markdup.param('particleList', particleList)
    markdup.param('prioritiseV0', prioritiseV0)
    path.add_module(markdup)


def V0ListMerger(firstList, secondList, prioritiseV0, path=analysis_main):
    """
    Merge two particle lists, vertex them and trim duplicates

    @param firstList first particle list to merge
    @param secondList second particle list to merge
    @param prioritiseV0 if true, give V0s a higher priority
    """
    listName = firstList.split(':')[0]
    if (listName == secondList.split(':')[0]):
        outList = listName + ':merged'
        copyLists(outList, [firstList, secondList], False, path)
        vertexKFit(outList, 0.0, '', '', path)
        markDuplicate(outList, prioritiseV0, path)
        applyCuts(outList, 'extraInfo(highQualityVertex)', path)
    else:
        B2ERROR("Lists to be merged contain different particles")


PI0ETAVETO_COUNTER = 0


def writePi0EtaVeto(
    particleList,
    decayString,
    workingDirectory='.',
    pi0vetoname='Pi0_Prob',
    etavetoname='Eta_Prob',
    downloadFlag=True,
    selection='',
    path=analysis_main,
):
    """
    Give pi0/eta probability for hard photon.

    default weight files are set 1.4 GeV as the lower limit of hard photon energy in CMS Frame when mva training for pi0etaveto.
    current default weight files are optimised by MC9.
    The Input Variables are as below. Aliases are set to some variables when training.
    M : pi0/eta candidates Invariant mass
    lowE : soft photon energy in lab frame
    cTheta : soft photon ECL cluster's polar angle
    Zmva : soft photon output of MVA using Zernike moments of the cluster
    minC2Hdist : soft photon distance from eclCluster to nearest point on nearest Helix at the ECL cylindrical radius

    If you don't have weight files in your workingDirectory,
    these files are downloaded from database to your workingDirectory automatically.
    Please refer to analysis/examples/tutorials/B2A306-B02RhoGamma-withPi0EtaVeto.py
    about how to use this function.

    NOTE for debug
    Please don't use following ParticleList names elsewhere.
    'gamma:HARDPHOTON', pi0:PI0VETO, eta:ETAVETO,
    'gamma:PI0SOFT' + str(PI0ETAVETO_COUNTER), 'gamma:ETASOFT' + str(PI0ETAVETO_COUNTER)
    Please don't use "lowE", "cTheta", "Zmva", "minC2Hdist" as alias elsewhere.

    @param particleList     The input ParticleList
    @param decayString specify Particle to be added to the ParticleList
    @param workingDirectory The weight file directory
    @param downloadFlag whether download default weight files or not
    @param pi0vetoname extraInfo name of pi0 probability
    @param etavetoname extraInfo name of eta probability
    @param selection Selection criteria that Particle needs meet in order for for_each ROE path to continue
    @param path       modules are added to this path
    """
    global PI0ETAVETO_COUNTER

    if PI0ETAVETO_COUNTER == 0:
        variables.addAlias('lowE', 'daughter(1,E)')
        variables.addAlias('cTheta', 'daughter(1,clusterTheta)')
        variables.addAlias('Zmva', 'daughter(1,clusterZernikeMVA)')
        variables.addAlias('minC2Hdist', 'daughter(1,minC2TDist)')

    PI0ETAVETO_COUNTER = PI0ETAVETO_COUNTER + 1

    roe_path = create_path()

    deadEndPath = create_path()

    signalSideParticleFilter(particleList, selection, roe_path, deadEndPath)

    fillSignalSideParticleList('gamma:HARDPHOTON', decayString, path=roe_path)

    pi0softname = 'gamma:PI0SOFT'
    etasoftname = 'gamma:ETASOFT'
    softphoton1 = pi0softname + str(PI0ETAVETO_COUNTER)
    softphoton2 = etasoftname + str(PI0ETAVETO_COUNTER)

    fillParticleList(
        softphoton1,
        '[clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.02] or [clusterReg==3 and E>0.02]',
        path=roe_path)
    applyCuts(softphoton1, 'abs(clusterTiming)<120', path=roe_path)
    fillParticleList(
        softphoton2,
        '[clusterReg==1 and E>0.035] or [clusterReg==2 and E>0.03] or [clusterReg==3 and E>0.03]',
        path=roe_path)
    applyCuts(softphoton2, 'abs(clusterTiming)<120', path=roe_path)

    reconstructDecay('pi0:PI0VETO -> gamma:HARDPHOTON ' + softphoton1, '', path=roe_path)
    reconstructDecay('eta:ETAVETO -> gamma:HARDPHOTON ' + softphoton2, '', path=roe_path)

    if not os.path.isdir(workingDirectory):
        os.mkdir(workingDirectory)
        B2INFO('writePi0EtaVeto: ' + workingDirectory + ' has been created as workingDirectory.')

    if not os.path.isfile(workingDirectory + '/pi0veto.root'):
        if downloadFlag:
            use_central_database('development')
            basf2_mva.download('Pi0VetoIdentifier', workingDirectory + '/pi0veto.root')
            B2INFO('writePi0EtaVeto: pi0veto.root has been downloaded from database to workingDirectory.')

    if not os.path.isfile(workingDirectory + '/etaveto.root'):
        if downloadFlag:
            use_central_database('development')
            basf2_mva.download('EtaVetoIdentifier', workingDirectory + '/etaveto.root')
            B2INFO('writePi0EtaVeto: etaveto.root has been downloaded from database to workingDirectory.')

    roe_path.add_module('MVAExpert', listNames=['pi0:PI0VETO'], extraInfoName='Pi0Veto',
                        identifier=workingDirectory + '/pi0veto.root')
    roe_path.add_module('MVAExpert', listNames=['eta:ETAVETO'], extraInfoName='EtaVeto',
                        identifier=workingDirectory + '/etaveto.root')

    rankByHighest('pi0:PI0VETO', 'extraInfo(Pi0Veto)', numBest=1, path=roe_path)
    rankByHighest('eta:ETAVETO', 'extraInfo(EtaVeto)', numBest=1, path=roe_path)

    variableToSignalSideExtraInfo('pi0:PI0VETO', {'extraInfo(Pi0Veto)': pi0vetoname}, path=roe_path)
    variableToSignalSideExtraInfo('eta:ETAVETO', {'extraInfo(EtaVeto)': etavetoname}, path=roe_path)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)


def buildEventKinematics(inputListNames=[], default_cleanup=True, path=analysis_main):
    """
    Calculates the global kinematics of the event (visible energy, missing momentum, missing mass...)
    using ParticleLists provided. If no ParticleList is provided, default ParticleLists are used
    (all track and all hits in ECL without associated track).

    The visible energy missing values are
    stored in a EventKinematics dataobject.

    @param inputListNames   list of ParticleLists used to calculate the global event kinematics.
                            If the list is empty, default ParticleLists pi+:evtkin and gamma:evtkin are filled.
    @param default_cleanup  if True, apply default clean up cuts to default
                            ParticleLists pi+:evtkin and gamma:evtkin.
    @param path             modules are added to this path
    """
    if not inputListNames:
        B2INFO("Creating particle lists pi+:evtkin and gamma:evtkin to get the global kinematics of the event.")
        fillParticleList('pi+:evtkin', '', path=path)
        fillParticleList('gamma:evtkin', '', path=path)
        particleLists = ['pi+:evtkin', 'gamma:evtkin']

        if default_cleanup:
            B2INFO("Using default cleanup in EventKinematics module.")
            trackCuts = 'pt > 0.1'
            trackCuts += ' and -0.8660 < cosTheta < 0.9535'
            trackCuts += ' and -3.0 < dz < 3.0'
            trackCuts += ' and -0.5 < dr < 0.5'
            applyCuts('pi+:evtkin', trackCuts, path=path)

            gammaCuts = 'E > 0.05'
            gammaCuts += ' and -0.8660 < cosTheta < 0.9535'
            applyCuts('gamma:evtkin', gammaCuts, path=path)
        else:
            B2INFO("No cleanup in EventKinematics module.")
    else:
        particleLists = inputListNames

    eventKinematicsModule = register_module('EventKinematics')
    eventKinematicsModule.set_name('EventKinematics_')
    eventKinematicsModule.param('particleLists', particleLists)
    path.add_module(eventKinematicsModule)


def buildEventShape(inputListNames=[],
                    default_cleanup=True,
                    allMoments=False,
                    cleoCones=True,
                    collisionAxis=True,
                    foxWolfram=True,
                    harmonicMoments=True,
                    jets=True,
                    sphericity=True,
                    thrust=True,
                    checkForDuplicates=False,
                    path=analysis_main):
    """
    Calculates the event shape quantities (thrust, sphericity, Fox-Wolfram moments...) using the
    particles in the lists provided by the user.
    The results of the calculation are then store in the EventShapeContainer dataobject, and are accessible
    byt the variabels of the EventShape group.

    @param inputListNames   list of ParticleLists used to calculate the global event kinematics.
                            If the list is empty, default ParticleLists pi+:evtkin and gamma:evtkin are filled.
    @param default_cleanup  if True,  applyes some very standard cuts on pt and costTheta when defines the interanl lists.
    @param path             modules are added to this path
    @param allMoments  Enables the calculation of FW and harmonic moments from 5 to 8
    @param cleoCones  Enables the calculation of the CLEO cones.
    @param collisionAxis  Enables the calculation of the  quantities related to the collision axis.
    @param foxWolfram    Enables the calculation of the Fox-Wolfram moments.
    @param jets   Enables the calculation of jet-related quantities.
    @param harmonicMoments   Enables the calculation of the Harmonic moments.
    @param sphericity  Enables the calculation of the sphericity-related quantities.
    @param thrust  Enables the calculation of thust-related quantities.
    @param checkForDuplicates Perform a check for duplicate particles before adding them.

    """
    if not inputListNames:
        B2INFO("Creating particle lists pi+:evtshape and gamma:evtshape to get the event shape variables.")
        fillParticleList('pi+:evtshape', '', path=path)
        fillParticleList('gamma:evtshape', '', path=path)
        particleLists = ['pi+:evtshape', 'gamma:evtshape']

        if default_cleanup:
            B2INFO("Using the default lists for the EventShape module.")
            trackCuts = 'pt > 0.1'
            trackCuts += ' and -0.8660 < cosTheta < 0.9535'
            trackCuts += ' and -3.0 < dz < 3.0'
            trackCuts += ' and -0.5 < dr < 0.5'
            applyCuts('pi+:evtshape', trackCuts, path=path)

            gammaCuts = 'E > 0.05'
            gammaCuts += ' and -0.8660 < cosTheta < 0.9535'
            applyCuts('gamma:evtshape', gammaCuts, path=path)
        else:
            B2WARNING("Creating the default lists with no cleanup. This can be potentially dangerous")
    else:
        particleLists = inputListNames

    eventShapeModule = register_module('EventShapeCalculator')
    eventShapeModule.set_name('EventShape')
    eventShapeModule.param('particleListNames', particleLists)
    eventShapeModule.param('enableAllMoments', allMoments)
    eventShapeModule.param('enableCleoCones', cleoCones)
    eventShapeModule.param('enableCollisionAxis', collisionAxis)
    eventShapeModule.param('enableFoxWolfram', foxWolfram)
    eventShapeModule.param('enableJets', jets)
    eventShapeModule.param('enableHarmonicMoments', harmonicMoments)
    eventShapeModule.param('enableSphericity', sphericity)
    eventShapeModule.param('enableThrust', thrust)
    eventShapeModule.param('checkForDuplicates', checkForDuplicates)

    path.add_module(eventShapeModule)


def labelTauPairMC(printDecayInfo=False, path=analysis_main):
    """
    Search tau leptons into the MC information of the event. If confirms it's a generated tau pair decay,
    labels the decay generated of the positive and negative leptons using the ID of KKMC tau decay table.

    @param printDecayInfo:  If true, prints ID and prong of each tau lepton in the event.
    @param path:        module is added to this path
    """
    tauDecayMarker = register_module('TauDecayMarker')
    tauDecayMarker.set_name('TauDecayMarker_')

    path.add_module(tauDecayMarker, printDecayInfo=printDecayInfo)


def tagCurlTracks(particleLists,
                  belle=False,
                  mcTruth=False,
                  responseCut=0.324,
                  selectorType='cUt',
                  ptCut=0.6,
                  train=False,
                  path=analysis_main):
    """
    Warning:
        The cut selector is not calibrated with Belle II data and should not be used without extensive study.

    Identifies curl tracks and tags them with extraInfo(isCurl=1) for later removal.
    For Belle data with a `b2bii` analysis the available cut based selection is described in `BN1079`_.

      .. _BN1079: https://belle.kek.jp/secured/belle_note/gn1079/bn1079.pdf


    The module loops over all particles in a given list that meet the preselection **ptCut** and assigns them to
    bundles based on the response of the chosen **selector** and the required minimum response set by the
    **responseCut**. Once all particles are assigned they are ranked by 25dr^2+dz^2. All but the lowest are tagged
    with extraInfo(isCurl=1) to allow for later removal  by cutting the list or removing these from ROE as
    applicable.


    @param particleLists: list of particle lists to check for curls.
    @param belle:         bool flag for Belle or Belle II data/mc.
    @param mcTruth:       bool flag to additionally assign particles with extraInfo(isTruthCurl) and
                          extraInfo(truthBundleSize). To calculate these particles are assigned to bundles by their
                          genParticleIndex then ranked and tagged as normal.
    @param responseCut:   float min classifier response that considers two tracks to come from the same particle.
                          Note 'cut' selector is binary 0/1.
    @param selectorType:  string name of selector to use. The available options are 'cut' and 'mva'.
                          It is strongly recommended to used the 'mva' selection. The 'cut' selection
                          is based on BN1079 and is only calibrated for Belle data.
    @param ptCut:         pre-selection cut on transverse momentum.
    @param train:         flag to set training mode if selector has a training mode (mva).
    @param path:          module is added to this path.
    """

    if (not isinstance(particleLists, list)):
        particleLists = [particleLists]  # in case user inputs a particle list as string

    curlTagger = register_module('CurlTagger')
    curlTagger.set_name('CurlTagger_')
    curlTagger.param('particleLists', particleLists)
    curlTagger.param('belle', belle)
    curlTagger.param('mcTruth', mcTruth)
    curlTagger.param('responseCut', responseCut)
    curlTagger.param('selectorType', selectorType)
    curlTagger.param('ptCut', ptCut)
    curlTagger.param('train', train)

    path.add_module(curlTagger)


def applyChargedPidMVA(sigHypoPDGCode, bkgHypoPDGCode, particleLists, path=analysis_main):
    """
    Apply an MVA (BDT) to perform particle identification for charged stable particles using `ChargedPidMVA` module.

    Note:
        Currently, the MVA is trained including only **ECL-based inputs**.
        Hence, it is mostly suited for **electron (muon) identification**.
        For the future, this approach could however be extended to include info from other subdetetctors,
        as long as they are made available in the Mdst format...

    The module decorates Particle objects in the input ParticleList(s) w/ variables
    containing the appropriate MVA score, which can be used to select candidates.

    Note:
        Currently, particle identification works as 'binary' separation between
        input S, B particle mass hypotheses according to the following scheme:

        - e (11) vs. pi (211)

        - mu (13) vs. pi (211)

        - pi (211) vs K (321)

        - K (321) vs pi (211)

        - p (2212) vs pi (211)

        - d : NOT AVAILABLE

        The MVA is charge-agnostic, i.e. the training is not done independently for +/- charged particles.

    The MVA algorithm used is a gradient boosted decision tree (`TMVA 4.2.1`, `ROOT 6.14/06`).

    Parameters:
        sigHypoPDGCode (int): the pdgId of the signal mass hypothesis.
        bkgHypoPDGCode (int): the pdgId of the background mass hypothesis.
        particleLists list(str): list of names of ParticleList objects for charged stable particles.
                                 The charge-conjugate ParticleLists will be also stored automatically.
        path (basf2.Path): the module is added to this path.

    """

    # Enforce check on input S, B hypotheses compatibility.
    binaryOpts = [(11, 211), (13, 211), (211, 321), (321, 211), (2212, 211)]

    if (sigHypoPDGCode, bkgHypoPDGCode) not in binaryOpts:
        B2FATAL("No charged pid MVA was trained to separate ", sigHypoPDGCode, " vs. ", bkgHypoPDGCode,
                ". Please choose among the following pairs:\n",
                "\n".join(f"{opt[0]} vs. {opt[1]}" for opt in binaryOpts))

    plSet = set(particleLists)
    for pList in particleLists:
        name, label = pList.split(':')
        plSet.add(f"{pdg.conjugate(name)}:{label}")

    chargedpid = register_module("ChargedPidMVA")
    chargedpid.set_name(f"ChargedPidMVA_{sigHypoPDGCode}_vs_{bkgHypoPDGCode}")
    chargedpid.param("sigHypoPDGCode", sigHypoPDGCode)
    chargedpid.param("bkgHypoPDGCode", bkgHypoPDGCode)
    chargedpid.param("particleLists", list(plSet))

    path.add_module(chargedpid)


if __name__ == '__main__':
    from basf2.utils import pretty_print_module
    pretty_print_module(__name__, "modularAnalysis", {
        repr(analysis_main): "analysis_main",
    })
