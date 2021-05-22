# !/usr/bin/env python3

"""
This module defines wrapper functions around the analysis modules.
"""

from basf2 import register_module, create_path
from basf2 import B2INFO, B2WARNING, B2ERROR, B2FATAL
import basf2
import subprocess


def setAnalysisConfigParams(configParametersAndValues, path):
    """
    Sets analysis configuration parameters.

    These are:

    - 'tupleStyle': 'Default' (default) or 'Laconic'
      o) defines the style of the branch name in the ntuple

    - 'mcMatchingVersion': Specifies what version of mc matching algorithm is going to be used:

          - 'MC5' - analysis of BelleII MC5
          - 'Belle' - analysis of Belle MC
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


def inputMdst(environmentType, filename, path, skipNEvents=0, entrySequence=None, *, parentLevel=0):
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


def inputMdstList(environmentType, filelist, path, skipNEvents=0, entrySequences=None, *, parentLevel=0):
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
        from basf2 import conditions
        conditions.globaltags += ["Legacy_MagneticField_MC8_MC9_MC10"]
    elif environmentType == 'None':
        B2INFO('No magnetic field is loaded. This is OK, if generator level information only is studied.')
    else:
        environments = ' '.join(list(environToMagneticField.keys()) + ["MC8", "MC9", "MC10"])
        B2FATAL('Incorrect environment type provided: ' + environmentType + '! Please use one of the following:' + environments)

    # set the correct MCMatching algorithm for MC5 and Belle MC
    if environmentType == 'Belle':
        setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)
        import b2bii
        b2bii.setB2BII()
    if environmentType == 'MC5':
        setAnalysisConfigParams({'mcMatchingVersion': 'MC5'}, path)

    # fixECLCluster for MC5/MC6/MC7
    if fixECLClusters.get(environmentType) is True:
        fixECL = register_module('FixECLClusters')
        path.add_module(fixECL)


def outputMdst(filename, path):
    """
    Saves mDST (mini-Data Summary Tables) to the output root file.

    .. warning::

        This functon is kept for backward-compatibility.
        Better to use `mdst.add_mdst_output` directly.

    """

    import mdst
    mdst.add_mdst_output(path, mc=True, filename=filename)


def outputUdst(filename, particleLists=None, includeArrays=None, path=None, dataDescription=None):
    """
    Save uDST (user-defined Data Summary Tables) = MDST + Particles + ParticleLists
    The charge-conjugate lists of those given in particleLists are also stored.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    Note:
        This does not reduce the amount of Particle objects saved,
        see `udst.add_skimmed_udst_output` for a function that does.

    """
    import udst
    udst.add_udst_output(
        path=path, filename=filename, particleLists=particleLists,
        additionalBranches=includeArrays, dataDescription=dataDescription)


def outputIndex(filename, path, includeArrays=None, keepParents=False, mc=True):
    """
    Write out all particle lists as an index file to be reprocessed using parentLevel flag.
    Additional branches necessary for file to be read are automatically included.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    @param str filename the name of the output index file
    @param str path modules are added to this path
    @param list(str) includeArrays: datastore arrays/objects to write to the output
        file in addition to particle lists and related information
    @param bool keepParents whether the parents of the input event will be saved as the parents of the same event
        in the output index file. Useful if you are only adding more information to another index file
    @param bool mc whether the input data is MC or not
    """
    if includeArrays is None:
        includeArrays = []

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


def setupEventInfo(noEvents, path):
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


def loadGearbox(path, silence_warning=False):
    """
    Loads Gearbox module to the path.

    Warning:
        Should be used in a job with *cosmic event generation only*

    Needed for scripts which only generate cosmic events in order to
    load the geometry.

    @param path modules are added to this path
    @param silence_warning stops a verbose warning message if you know you want to use this function
    """

    if not silence_warning:
        B2WARNING("""You are overwriting the geometry from the database with Gearbox.
          This is fine if you're generating cosmic events. But in most other cases you probably don't want this.

          If you're really sure you know what you're doing you can suppress this message with:

          >>> loadGearbox(silence_warning=True)

                  """)

    paramloader = register_module('Gearbox')
    path.add_module(paramloader)


def printPrimaryMCParticles(path, **kwargs):
    """
    Prints all primary MCParticles, that is particles from
    the physics generator and not particles created by the simulation

    This is equivalent to `printMCParticles(onlyPrimaries=True, path=path) <printMCParticles>` and additional
    keyword arguments are just forwarded to that function
    """
    return printMCParticles(onlyPrimaries=True, path=path, **kwargs)


def printMCParticles(onlyPrimaries=False, maxLevel=-1, path=None, *,
                     showProperties=False, showMomenta=False, showVertices=False, showStatus=False):
    """
    Prints all MCParticles or just primary MCParticles up to specified level. -1 means no limit.

    By default this will print a tree of just the particle names and their pdg
    codes in the event, for example ::

        [INFO] Content of MCParticle list
        ╰── Upsilon(4S) (300553)
            ├── B+ (521)
            │   ├── anti-D_0*0 (-10421)
            │   │   ├── D- (-411)
            │   │   │   ├── K*- (-323)
            │   │   │   │   ├── anti-K0 (-311)
            │   │   │   │   │   ╰── K_S0 (310)
            │   │   │   │   │       ├── pi+ (211)
            │   │   │   │   │       │   ╰╶╶ p+ (2212)
            │   │   │   │   │       ╰── pi- (-211)
            │   │   │   │   │           ├╶╶ e- (11)
            │   │   │   │   │           ├╶╶ n0 (2112)
            │   │   │   │   │           ├╶╶ n0 (2112)
            │   │   │   │   │           ╰╶╶ n0 (2112)
            │   │   │   │   ╰── pi- (-211)
            │   │   │   │       ├╶╶ anti-nu_mu (-14)
            │   │   │   │       ╰╶╶ mu- (13)
            │   │   │   │           ├╶╶ nu_mu (14)
            │   │   │   │           ├╶╶ anti-nu_e (-12)
            │   │   │   │           ╰╶╶ e- (11)
            │   │   │   ╰── K_S0 (310)
            │   │   │       ├── pi0 (111)
            │   │   │       │   ├── gamma (22)
            │   │   │       │   ╰── gamma (22)
            │   │   │       ╰── pi0 (111)
            │   │   │           ├── gamma (22)
            │   │   │           ╰── gamma (22)
            │   │   ╰── pi+ (211)
            │   ├── mu+ (-13)
            │   │   ├╶╶ anti-nu_mu (-14)
            │   │   ├╶╶ nu_e (12)
            │   │   ╰╶╶ e+ (-11)
            │   ├── nu_mu (14)
            │   ╰── gamma (22)
            ...


    There's a distinction between primary and secondary particles. Primary
    particles are the ones created by the physics generator while secondary
    particles are ones generated by the simulation of the detector interaction.

    Secondaries are indicated with a dashed line leading to the particle name
    and if the output is to the terminal they will be printed in red. If
    ``onlyPrimaries`` is True they will not be included in the tree.

    On demand, extra information on all the particles can be displayed by
    enabling any of the ``showProperties``, ``showMomenta``, ``showVertices``
    and ``showStatus`` flags. Enabling all of them will look like
    this::

        ...
        ╰── pi- (-211)
            │ mass=0.14 energy=0.445 charge=-1 lifetime=6.36
            │ p=(0.257, -0.335, 0.0238) |p|=0.423
            │ production vertex=(0.113, -0.0531, 0.0156), time=0.00589
            │ status flags=PrimaryParticle, StableInGenerator, StoppedInDetector
            │ list index=48
            │
            ╰╶╶ n0 (2112)
                mass=0.94 energy=0.94 charge=0 lifetime=5.28e+03
                p=(-0.000238, -0.0127, 0.0116) |p|=0.0172
                production vertex=(144, 21.9, -1.29), time=39
                status flags=StoppedInDetector
                creation process=HadronInelastic
                list index=66

    The first line of extra information is enabled by ``showProperties``, the
    second line by ``showMomenta``, the third line by ``showVertices`` and the
    last two lines by ``showStatus``. Note that all values are given in Belle II
    standard units, that is GeV, centimeter and nanoseconds.

    The depth of the tree can be limited with the ``maxLevel`` argument: If it's
    bigger than zero it will limit the tree to the given number of generations.
    A visual indicator will be added after each particle which would have
    additional daughters that are skipped due to this limit. An example event
    with ``maxLevel=3`` is given below. In this case only the tau neutrino and
    the pion don't have additional daughters. ::

        [INFO] Content of MCParticle list
        ╰── Upsilon(4S) (300553)
            ├── B+ (521)
            │   ├── anti-D*0 (-423) → …
            │   ├── tau+ (-15) → …
            │   ╰── nu_tau (16)
            ╰── B- (-521)
                ├── D*0 (423) → …
                ├── K*- (-323) → …
                ├── K*+ (323) → …
                ╰── pi- (-211)


    Parameters:
        onlyPrimaries (bool): If True show only primary particles, that is particles coming from
            the generator and not created by the simulation.
        maxLevel (int): If 0 or less print the whole tree, otherwise stop after n generations
        showProperties (bool): If True show mass, energy and charge of the particles
        showMomenta (bool): if True show the momenta of the particles
        showVertices (bool): if True show production vertex and production time of all particles
        showStatus (bool): if True show some status information on the particles.
            For secondary particles this includes creation process.
    """

    return path.add_module(
        "PrintMCParticles",
        onlyPrimaries=onlyPrimaries,
        maxLevel=maxLevel,
        showProperties=showProperties,
        showMomenta=showMomenta,
        showVertices=showVertices,
        showStatus=showStatus,
    )


def correctBrems(outputList,
                 inputList,
                 gammaList,
                 maximumAcceptance=3.0,
                 multiplePhotons=False,
                 usePhotonOnlyOnce=True,
                 writeOut=False,
                 path=None):
    """
    For each particle in the given ``inputList``, copies it to the ``outputList`` and adds the
    4-vector of the photon(s) in the ``gammaList`` which has(have) a weighted named relation to
    the particle's track, set by the ``ECLTrackBremFinder`` module during reconstruction.

    Warning:
        This can only work if the mdst file contains the *Bremsstrahlung* named relation. Official MC samples
        up to and including MC12 and proc9 **do not** contain this. Newer production campaigns (from proc10 and MC13) do.
        However, studies by the tau WG revealed that the cuts applied by the ``ECLTrackBremFinder`` module are too tight.
        These will be loosened but this will only have effect with proc13 and MC15.
        If your analysis is very sensitive to the Bremsstrahlung corrections, it is advised to use `correctBremsBelle`.

    Information:
        A detailed description of how the weights are set can be found directly at the documentation of the
        `BremsFinder` module.

        Please note that a new particle is always generated, with the old particle and -if found- one or more
        photons as daughters.

        The ``inputList`` should contain particles with associated tracks. Otherwise, the module will exit with an error.

        The ``gammaList`` should contain photons. Otherwise, the module will exit with an error.

    @param outputList   The output particle list name containing the corrected particles
    @param inputList    The initial particle list name containing the particles to correct. *It should already exist.*
    @param gammaList    The photon list containing possibly bremsstrahlung photons; *It should already exist.*
    @param maximumAcceptance Maximum value of the relation weight. Should be a number between [0,3)
    @param multiplePhotons Whether to use only one photon (the one with the smallest acceptance) or as many as possible
    @param usePhotonOnlyOnce If true, each brems candidate is used to correct only the track with the smallest relation weight
    @param writeOut      Whether `RootOutput` module should save the created ``outputList``
    @param path          The module is added to this path
    """

    bremscorrector = register_module('BremsFinder')
    bremscorrector.set_name('bremsCorrector_' + outputList)
    bremscorrector.param('inputList', inputList)
    bremscorrector.param('outputList', outputList)
    bremscorrector.param('gammaList', gammaList)
    bremscorrector.param('maximumAcceptance', maximumAcceptance)
    bremscorrector.param('multiplePhotons', multiplePhotons)
    bremscorrector.param('usePhotonOnlyOnce', usePhotonOnlyOnce)
    bremscorrector.param('writeOut', writeOut)
    path.add_module(bremscorrector)


def copyList(outputListName, inputListName, writeOut=False, path=None):
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


def correctBremsBelle(outputListName,
                      inputListName,
                      gammaListName,
                      multiplePhotons=True,
                      angleThreshold=0.05,
                      writeOut=False,
                      path=None):
    """
    Run the Belle - like brems finding on the ``inputListName`` of charged particles.
    Adds all photons in ``gammaListName`` to a copy of the charged particle that are within
    ``angleThreshold``.

    Tip:
        Studies by the tau WG show that using a rather wide opening angle (up to
        0.2 rad) and rather low energetic photons results in good correction.
        However, this should only serve as a starting point for your own studies
        because the optimal criteria are likely mode-dependent

    Parameters:
       outputListName (str): The output charged particle list containing the corrected charged particles
       inputListName (str): The initial charged particle list containing the charged particles to correct.
       gammaListName (str): The gammas list containing possibly radiative gammas, should already exist.
       multiplePhotons (bool): How many photons should be added to the charged particle? nearest one -> False,
             add all the photons within the cone -> True
       angleThreshold (float): The maximum angle in radians between the charged particle and the (radiative)
              gamma to be accepted.
       writeOut (bool): whether RootOutput module should save the created ParticleList
       path (basf2.Path): modules are added to this path
    """

    fsrcorrector = register_module('BelleBremRecovery')
    fsrcorrector.set_name('BelleFSRCorrection_' + outputListName)
    fsrcorrector.param('inputListName', inputListName)
    fsrcorrector.param('outputListName', outputListName)
    fsrcorrector.param('gammaListName', gammaListName)
    fsrcorrector.param('multiplePhotons', multiplePhotons)
    fsrcorrector.param('angleThreshold', angleThreshold)
    fsrcorrector.param('writeOut', writeOut)
    path.add_module(fsrcorrector)


def copyLists(outputListName, inputListNames, writeOut=False, path=None):
    """
    Copy all Particle indices from all input ParticleLists to the
    single output ParticleList.
    Note that the Particles themselves are not copied.
    The original and copied ParticleLists will point to the same Particles.

    Duplicates are removed based on the first-come, first-served principle.
    Therefore, the order of the input ParticleLists matters.

    .. seealso::
        If you want to select the best duplicate based on another criterion, have
        a look at the function `mergeListsWithBestDuplicate`.

    .. note::
        Two particles that differ only by the order of their daughters are
        considered duplicates and one of them will be removed.

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


def copyParticles(outputListName, inputListName, writeOut=False, path=None):
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


def cutAndCopyLists(outputListName, inputListNames, cut, writeOut=False, path=None):
    """
    Copy candidates from all lists in ``inputListNames`` to
    ``outputListName`` if they pass ``cut`` (given selection criteria).

    Note:
        Note that the Particles themselves are not copied.
        The original and copied ParticleLists will point to the same Particles.

    Example:
        Require energetic pions safely inside the cdc

        >>> cutAndCopyLists("pi+:energeticPions", ["pi+:good", "pi+:loose"], "[E > 2] and [0.3 < theta < 2.6]", path=mypath)

    Warning:
        You must use square braces ``[`` and ``]`` for conditional statements.

    Parameters:
        outputListName (str): the new ParticleList name
        inputListName (list(str)): list of input ParticleList names
        cut (str): Candidates that do not pass these selection criteria are removed from the ParticleList
        writeOut (bool): whether RootOutput module should save the created ParticleList
        path (basf2.Path): modules are added to this path
    """
    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCutAndCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('cut', cut)
    pmanipulate.param('writeOut', writeOut)
    path.add_module(pmanipulate)


def cutAndCopyList(outputListName, inputListName, cut, writeOut=False, path=None):
    """
    Copy candidates from ``inputListName`` to ``outputListName`` if they pass
    ``cut`` (given selection criteria).

    Note:
        Note the Particles themselves are not copied.
        The original and copied ParticleLists will point to the same Particles.

    Example:
        require energetic pions safely inside the cdc

        >>> cutAndCopyLists("pi+:energeticPions", "pi+:loose", "[E > 2] and [0.3 < theta < 2.6]", path=mypath)

    Warning:
        You must use square braces ``[`` and ``]`` for conditional statements.

    Parameters:
        outputListName (str): the new ParticleList name
        inputListName (str): input ParticleList name
        cut (str): Candidates that do not pass these selection criteria are removed from the ParticleList
        writeOut (bool): whether RootOutput module should save the created ParticleList
        path (basf2.Path): modules are added to this path
    """
    cutAndCopyLists(outputListName, [inputListName], cut, writeOut, path)


def removeTracksForTrackingEfficiencyCalculation(inputListNames, fraction, path=None):
    """
    Randomly remove tracks from the provided particle lists to estimate the tracking efficiency.
    Takes care of the duplicates, if any.

    Parameters:
        inputListNames (list(str)): input particle list names
        fraction (float): fraction of particles to be removed randomly
        path (basf2.Path): module is added to this path
    """

    trackingefficiency = register_module('TrackingEfficiency')
    trackingefficiency.param('particleLists', inputListNames)
    trackingefficiency.param('frac', fraction)
    path.add_module(trackingefficiency)


def scaleTrackMomenta(inputListNames, scale, path=None):
    """

    Scale momenta of the particles according to the scaling factor scale.
    If the particle list contains composite particles, the momenta of the track-based daughters are scaled.
    Subsequently, the momentum of the mother particle is updated as well.

    Parameters:
        inputListNames (list(str)): input particle list names
        scale (float): scaling factor (1.0 -- no scaling)
        path (basf2.Path): module is added to this path
    """

    trackingmomentum = register_module('TrackingMomentum')
    trackingmomentum.param('particleLists', inputListNames)
    trackingmomentum.param('scale', scale)
    path.add_module(trackingmomentum)


def mergeListsWithBestDuplicate(outputListName,
                                inputListNames,
                                variable,
                                preferLowest=True,
                                writeOut=False,
                                path=None):
    """
    Merge input ParticleLists into one output ParticleList. Only the best
    among duplicates is kept. The lowest or highest value (configurable via
    preferLowest) of the provided variable determines which duplicate is the
    best.

    @param ouputListName name of merged ParticleList
    @param inputListName vector of original ParticleLists to be merged
    @param variable      variable to determine best duplicate
    @param preferLowest  whether lowest or highest value of variable should be preferred
    @param writeOut      whether RootOutput module should save the created ParticleList
    @param path          modules are added to this path
    """

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListMerger_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('variable', variable)
    pmanipulate.param('preferLowest', preferLowest)
    pmanipulate.param('writeOut', writeOut)
    path.add_module(pmanipulate)


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


def fillParticleLists(decayStringsWithCuts, writeOut=False, path=None, enforceFitHypothesis=False,
                      loadPhotonsFromKLM=False, loadPhotonBeamBackgroundMVA=True):
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
        Gammas can also be loaded from KLMClusters by explicitly setting the
        parameter ``loadPhotonsFromKLM`` to True. However, this should only be
        done in selected use-cases and the effect should be studied carefully.

    Tip:
        For "K_L0" it is now possible to load from ECLClusters, to revert to
        the old (Belle) behavior, you can require ``'isFromKLM > 0'``.

    .. code-block:: python

        klongs = ('K_L0', 'isFromKLM > 0')
        fillParticleLists([kaons, pions, klongs], path=mypath)


    Parameters:
        decayStringsWithCuts (list): A list of python ntuples of (decayString, cut).
                                     The decay string determines the type of Particle
                                     and the name of the ParticleList.
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
        loadPhotonsFromKLM (bool):   If true, photon candidates will be created from KLMClusters as well.
        loadPhotonBeamBackgroundMVA (bool):   If true, photon candidates will be assigned a beam background probability.
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + 'PLists')
    pload.param('decayStrings', [decayString for decayString, cut in decayStringsWithCuts])
    pload.param('writeOut', writeOut)
    pload.param("enforceFitHypothesis", enforceFitHypothesis)
    path.add_module(pload)

    from ROOT import Belle2
    decayDescriptor = Belle2.DecayDescriptor()
    for decayString, cut in decayStringsWithCuts:
        if not decayDescriptor.init(decayString):
            raise ValueError("Invalid decay string")
        # need to check some logic to unpack possible scenarios
        if decayDescriptor.getNDaughters() > 0:
            # ... then we have an actual decay in the decay string which must be a V0
            # the particle loader automatically calls this "V0" so we have to copy over
            # the list to name/format that user wants
            if decayDescriptor.getMother().getLabel() != 'V0':
                copyList(decayDescriptor.getMother().getFullName(), decayDescriptor.getMother().getName() + ':V0', writeOut, path)
        elif decayDescriptor.getMother().getLabel() != 'all':
            # then we have a non-V0 particle which the particle loader automatically calls "all"
            # as with the special V0 case we have to copy over the list to the name/format requested
            copyList(decayString, decayDescriptor.getMother().getName() + ':all', writeOut, path)

        # optionally apply a cut
        if cut != "":
            applyCuts(decayDescriptor.getMother().getFullName(), cut, path)

        if decayString.startswith("gamma"):
            # keep KLM-source photons as a experts-only for now: they are loaded by the particle loader,
            # but the user has to explicitly request them.
            if not loadPhotonsFromKLM:
                applyCuts(decayString, 'isFromECL', path)

            # if the user asked for the beam background MVA to be added, then also provide this
            # (populates the variable named beamBackgroundProbabilityMVA)
            if loadPhotonBeamBackgroundMVA:
                getBeamBackgroundProbabilityMVA(decayString, path)


def fillParticleList(decayString, cut, writeOut=False, path=None, enforceFitHypothesis=False,
                     loadPhotonsFromKLM=False, loadPhotonBeamBackgroundMVA=True):
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
        Gammas can also be loaded from KLMClusters by explicitly setting the
        parameter ``loadPhotonsFromKLM`` to True. However, this should only be
        done in selected use-cases and the effect should be studied carefully.

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
        loadPhotonsFromKLM (bool):   If true, photon candidates will be created from KLMClusters as well.

        loadPhotonBeamBackgroundMVA (bool):   If true, photon candidates will be assigned a beam background probability.
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStrings', [decayString])
    pload.param('writeOut', writeOut)
    pload.param("enforceFitHypothesis", enforceFitHypothesis)
    path.add_module(pload)

    # need to check some logic to unpack possible scenarios
    from ROOT import Belle2
    decayDescriptor = Belle2.DecayDescriptor()
    if not decayDescriptor.init(decayString):
        raise ValueError("Invalid decay string")
    if decayDescriptor.getNDaughters() > 0:
        # ... then we have an actual decay in the decay string which must be a V0
        # the particle loader automatically calls this "V0" so we have to copy over
        # the list to name/format that user wants
        if decayDescriptor.getMother().getLabel() != 'V0':
            copyList(decayDescriptor.getMother().getFullName(), decayDescriptor.getMother().getName() + ':V0', writeOut, path)
    elif decayDescriptor.getMother().getLabel() != 'all':
        # then we have a non-V0 particle which the particle loader automatically calls "all"
        # as with the special V0 case we have to copy over the list to the name/format requested
        copyList(decayString, decayDescriptor.getMother().getName() + ':all', writeOut, path)

    # optionally apply a cut
    if cut != "":
        applyCuts(decayDescriptor.getMother().getFullName(), cut, path)

    if decayString.startswith("gamma"):
        # keep KLM-source photons as a experts-only for now: they are loaded by the particle loader,
        # but the user has to explicitly request them.
        if not loadPhotonsFromKLM:
            applyCuts(decayString, 'isFromECL', path)

        # if the user asked for the beam background MVA to be added, then also provide this
        # (populates the variable named beamBackgroundProbabilityMVA)
        if loadPhotonBeamBackgroundMVA:
            getBeamBackgroundProbabilityMVA(decayString, path)


def fillParticleListWithTrackHypothesis(decayString,
                                        cut,
                                        hypothesis,
                                        writeOut=False,
                                        enforceFitHypothesis=False,
                                        path=None):
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
    pload.param('decayStrings', [decayString])
    pload.param('trackHypothesis', hypothesis)
    pload.param('writeOut', writeOut)
    pload.param("enforceFitHypothesis", enforceFitHypothesis)
    path.add_module(pload)

    from ROOT import Belle2
    decayDescriptor = Belle2.DecayDescriptor()
    if not decayDescriptor.init(decayString):
        raise ValueError("Invalid decay string")
    if decayDescriptor.getMother().getLabel() != 'all':
        # the particle loader automatically calls particle lists of charged FSPs "all"
        # so we have to copy over the list to the name/format requested
        copyList(decayString, decayDescriptor.getMother().getName() + ':all', writeOut, path)

    # apply a cut if a non-empty cut string is provided
    if cut != "":
        applyCuts(decayString, cut, path)


def fillConvertedPhotonsList(decayString, cut, writeOut=False, path=None):
    """
    Creates photon Particle object for each e+e- combination in the V0 StoreArray.

    Note:
        You must specify the daughter ordering.

    .. code-block:: python

        fillConvertedPhotonsList('gamma:converted -> e+ e-', '')

    Parameters:
        decayString (str): Must be gamma to an e+e- pair. You must specify the daughter ordering.
                           Will also determine the name of the particleList.
        cut (str):         Particles need to pass these selection criteria to be added to the ParticleList
        writeOut (bool):   whether RootOutput module should save the created ParticleList
        path (basf2.Path): modules are added to this path

    """
    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStrings', [decayString])
    pload.param('addDaughters', True)
    pload.param('writeOut', writeOut)
    path.add_module(pload)

    from ROOT import Belle2
    decayDescriptor = Belle2.DecayDescriptor()
    if not decayDescriptor.init(decayString):
        raise ValueError("Invalid decay string")
    if decayDescriptor.getMother().getLabel() != 'V0':
        # the particle loader automatically calls converted photons "V0" so we have to copy over
        # the list to name/format that user wants
        copyList(decayDescriptor.getMother().getFullName(), decayDescriptor.getMother().getName() + ':V0', writeOut, path)

    # apply a cut if a non-empty cut string is provided
    if cut != "":
        applyCuts(decayDescriptor.getMother().getFullName(), cut, path)


def fillParticleListFromROE(decayString,
                            cut,
                            maskName='',
                            sourceParticleListName='',
                            useMissing=False,
                            writeOut=False,
                            path=None):
    """
    Creates Particle object for each ROE of the desired type found in the
    StoreArray<RestOfEvent>, loads them to the StoreArray<Particle>
    and fills the ParticleList. If useMissing is True, then the missing
    momentum is used instead of ROE.

    The type of the particles to be loaded is specified via the decayString module parameter.

    @param decayString             specifies type of Particles and determines the name of the ParticleList.
                                   Source ROEs can be taken as a daughter list, for example:
                                   'B0:tagFromROE -> B0:signal'
    @param cut                     Particles need to pass these selection criteria to be added to the ParticleList
    @param maskName                Name of the ROE mask to use
    @param sourceParticleListName  Use related ROEs to this particle list as a source
    @param useMissing              Use missing momentum instead of ROE momentum
    @param writeOut                whether RootOutput module should save the created ParticleList
    @param path                    modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStrings', [decayString])
    pload.param('writeOut', writeOut)
    pload.param('roeMaskName', maskName)
    pload.param('useMissing', useMissing)
    pload.param('sourceParticleListName', sourceParticleListName)
    pload.param('useROEs', True)
    path.add_module(pload)

    from ROOT import Belle2
    decayDescriptor = Belle2.DecayDescriptor()
    if not decayDescriptor.init(decayString):
        raise ValueError("Invalid decay string")
    if decayDescriptor.getMother().getLabel() != 'ROE':
        # the particle loader automatically uses the label "ROE" for particles built from the ROE
        # so we have to copy over the list to name/format that user wants
        copyList(decayDescriptor.getMother().getFullName(), decayDescriptor.getMother().getName() + ':ROE', writeOut, path)

    # apply a cut if a non-empty cut string is provided
    if cut != "":
        applyCuts(decayDescriptor.getMother().getFullName(), cut, path)


def fillParticleListFromMC(decayString,
                           cut,
                           addDaughters=False,
                           skipNonPrimaryDaughters=False,
                           writeOut=False,
                           path=None):
    """
    Creates Particle object for each MCParticle of the desired type found in the StoreArray<MCParticle>,
    loads them to the StoreArray<Particle> and fills the ParticleList.

    The type of the particles to be loaded is specified via the decayString module parameter.

    @param decayString             specifies type of Particles and determines the name of the ParticleList
    @param cut                     Particles need to pass these selection criteria to be added to the ParticleList
    @param addDaughters            adds the bottom part of the decay chain of the particle to the datastore and
                                   sets mother-daughter relations
    @param skipNonPrimaryDaughters if true, skip non primary daughters, useful to study final state daughter particles
    @param writeOut                whether RootOutput module should save the created ParticleList
    @param path                    modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + decayString)
    pload.param('decayStrings', [decayString])
    pload.param('addDaughters', addDaughters)
    pload.param('skipNonPrimaryDaughters', skipNonPrimaryDaughters)
    pload.param('writeOut', writeOut)
    pload.param('useMCParticles', True)
    path.add_module(pload)

    from ROOT import Belle2
    decayDescriptor = Belle2.DecayDescriptor()
    if not decayDescriptor.init(decayString):
        raise ValueError("Invalid decay string")
    if decayDescriptor.getMother().getLabel() != 'MC':
        # the particle loader automatically uses the label "MC" for particles built from MCParticles
        # so we have to copy over the list to name/format that user wants
        copyList(decayString, decayDescriptor.getMother().getName() + ':MC', writeOut, path)

    # apply a cut if a non-empty cut string is provided
    if cut != "":
        applyCuts(decayString, cut, path)


def fillParticleListsFromMC(decayStringsWithCuts,
                            addDaughters=False,
                            skipNonPrimaryDaughters=False,
                            writeOut=False,
                            path=None):
    """
    Creates Particle object for each MCParticle of the desired type found in the StoreArray<MCParticle>,
    loads them to the StoreArray<Particle> and fills the ParticleLists.

    The types of the particles to be loaded are specified via the (decayString, cut) tuples given in a list.
    For example:
    kaons = ('K+:gen', '')
    pions = ('pi+:gen', 'pionID>0.1')
    fillParticleListsFromMC([kaons, pions])

    @param decayString             specifies type of Particles and determines the name of the ParticleList
    @param cut                     Particles need to pass these selection criteria to be added to the ParticleList
    @param addDaughters            adds the bottom part of the decay chain of the particle to the datastore and
                                   sets mother-daughter relations
    @param skipNonPrimaryDaughters if true, skip non primary daughters, useful to study final state daughter particles
    @param writeOut                whether RootOutput module should save the created ParticleList
    @param path                    modules are added to this path
    """

    pload = register_module('ParticleLoader')
    pload.set_name('ParticleLoader_' + 'PLists')
    pload.param('decayStrings', [decayString for decayString, cut in decayStringsWithCuts])
    pload.param('addDaughters', addDaughters)
    pload.param('skipNonPrimaryDaughters', skipNonPrimaryDaughters)
    pload.param('writeOut', writeOut)
    pload.param('useMCParticles', True)
    path.add_module(pload)

    from ROOT import Belle2
    decayDescriptor = Belle2.DecayDescriptor()
    for decayString, cut in decayStringsWithCuts:
        if not decayDescriptor.init(decayString):
            raise ValueError("Invalid decay string")
        if decayDescriptor.getMother().getLabel() != 'MC':
            # the particle loader automatically uses the label "MC" for particles built from MCParticles
            # so we have to copy over the list to name/format that user wants
            copyList(decayString, decayDescriptor.getMother().getName() + ':MC', writeOut, path)

        # apply a cut if a non-empty cut string is provided
        if cut != "":
            applyCuts(decayString, cut, path)


def applyCuts(list_name, cut, path):
    """
    Removes particle candidates from ``list_name`` that do not pass ``cut``
    (given selection criteria).

    Example:
        require energetic pions safely inside the cdc

        >>> applyCuts("pi+:mypions", "[E > 2] and [0.3 < theta < 2.6]", path=mypath)

    Warning:
        You must use square braces ``[`` and ``]`` for conditional statements.

    Parameters:
        list_name (str): input ParticleList name
        cut (str): Candidates that do not pass these selection criteria are removed from the ParticleList
        path (basf2.Path): modules are added to this path
    """

    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_applyCuts_' + list_name)
    pselect.param('decayString', list_name)
    pselect.param('cut', cut)
    path.add_module(pselect)


def applyEventCuts(cut, path):
    """
    Removes events that do not pass the ``cut`` (given selection criteria).

    Example:
        continuum events (in mc only) with more than 5 tracks

        >>> applyEventCuts("[nTracks > 5] and [isContinuumEvent], path=mypath)

    Warning:
        You must use square braces ``[`` and ``]`` for conditional statements.

    Parameters:
        cut (str): Events that do not pass these selection criteria are skipped
        path (basf2.Path): modules are added to this path
    """

    eselect = register_module('VariableToReturnValue')
    eselect.param('variable', 'passesEventCut(' + cut + ')')
    path.add_module(eselect)
    empty_path = create_path()
    eselect.if_value('<1', empty_path)


def reconstructDecay(decayString,
                     cut,
                     dmID=0,
                     writeOut=False,
                     path=None,
                     candidate_limit=None,
                     ignoreIfTooManyCandidates=True,
                     chargeConjugation=True,
                     allowChargeViolation=False):
    r"""
    Creates new Particles by making combinations of existing Particles - it reconstructs unstable particles via their specified
    decay mode, e.g. in form of a :ref:`DecayString`: :code:`D0 -> K- pi+` or :code:`B+ -> anti-D0 pi+`, ... All possible
    combinations are created (particles are used only once per candidate) and combinations that pass the specified selection
    criteria are saved to a newly created (mother) ParticleList. By default the charge conjugated decay is reconstructed as well
    (meaning that the charge conjugated mother list is created as well) but this can be deactivated.

    One can use an ``@``-sign to mark a particle as unspecified for inclusive analyses,
    e.g. in a DecayString: :code:`'@Xsd -> K+ pi-'`.

    .. seealso:: :ref:`Marker_of_unspecified_particle`

    .. warning::
        The input ParticleLists are typically ordered according to the upstream reconstruction algorithm.
        Therefore, if you combine two or more identical particles in the decay chain you should not expect to see the same
        distribution for the daughter kinematics as they may be sorted by geometry, momentum etc.

        For example, in the decay :code:`D0 -> pi0 pi0` the momentum distributions of the two ``pi0`` s are not identical.
        This can be solved by manually randomising the lists before combining.

    See Also:

        * `Particle combiner how does it work? <https://questions.belle2.org/question/4318/particle-combiner-how-does-it-work/>`_
        * `Identical particles in decay chain <https://questions.belle2.org/question/5724/identical-particles-in-decay-chain/>`_

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
    @param chargeConjugation boolean to decide whether charge conjugated mode should be reconstructed as well (on by default)
    @param allowChargeViolation whether the decay string needs to conserve the electric charge
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
    pmake.param('chargeConjugation', chargeConjugation)
    pmake.param("allowChargeViolation", allowChargeViolation)
    path.add_module(pmake)


def combineAllParticles(inputParticleLists, outputList, cut='', writeOut=False, path=None):
    """
    Creates a new Particle as the combination of all Particles from all
    provided inputParticleLists. However, each particle is used only once
    (even if duplicates are provided) and the combination has to pass the
    specified selection criteria to be saved in the newly created (mother)
    ParticleList.

    @param inputParticleLists List of input particle lists which are combined to the new Particle
    @param outputList         Name of the particle combination created with this module
    @param cut                created (mother) Particle is added to the mother ParticleList if it passes
                              these given cuts (in VariableManager style) and is rejected otherwise
    @param writeOut           whether RootOutput module should save the created ParticleList
    @param path               module is added to this path
    """

    pmake = register_module('AllParticleCombiner')
    pmake.set_name('AllParticleCombiner_' + outputList)
    pmake.param('inputListNames', inputParticleLists)
    pmake.param('outputListName', outputList)
    pmake.param('cut', cut)
    pmake.param('writeOut', writeOut)
    path.add_module(pmake)


def reconstructMissingKlongDecayExpert(decayString,
                                       cut,
                                       dmID=0,
                                       writeOut=False,
                                       path=None,
                                       recoList="_reco"):
    """
    Creates a list of K_L0's with their momentum determined from kinematic constraints of B->K_L0 + something else.

    @param decayString DecayString specifying what kind of the decay should be reconstructed
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut         Particles are added to the K_L0 ParticleList if they
                       pass the given cuts (in VariableManager style) and rejected otherwise
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
    path.add_module(pcalc)

    rmake = register_module('KlongDecayReconstructorExpert')
    rmake.set_name('KlongDecayReconstructorExpert_' + decayString)
    rmake.param('decayString', decayString)
    rmake.param('cut', cut)
    rmake.param('decayMode', dmID)
    rmake.param('writeOut', writeOut)
    rmake.param('recoList', recoList)
    path.add_module(rmake)


def replaceMass(replacerName, particleLists=None, pdgCode=22, path=None):
    """
    replaces the mass of the particles inside the given particleLists
    with the invariant mass of the particle corresponding to the given pdgCode.

    @param particleLists new ParticleList filled with copied Particles
    @param pdgCode PDG   code for mass reference
    @param path          modules are added to this path
    """
    if particleLists is None:
        particleLists = []

    # first copy original particles to the new ParticleList
    pmassupdater = register_module('ParticleMassUpdater')
    pmassupdater.set_name('ParticleMassUpdater_' + replacerName)
    pmassupdater.param('particleLists', particleLists)
    pmassupdater.param('pdgCode', pdgCode)
    path.add_module(pmassupdater)


def reconstructRecoil(decayString,
                      cut,
                      dmID=0,
                      writeOut=False,
                      path=None,
                      candidate_limit=None,
                      allowChargeViolation=False):
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
    @param allowChargeViolation whether the decay string needs to conserve the electric charge
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
    pmake.param('allowChargeViolation', allowChargeViolation)
    path.add_module(pmake)


def reconstructRecoilDaughter(decayString,
                              cut,
                              dmID=0,
                              writeOut=False,
                              path=None,
                              candidate_limit=None,
                              allowChargeViolation=False):
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
    @param allowChargeViolation whether the decay string needs to conserve the electric charge taking into account that the first
                       daughter is actually the mother
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
    pmake.param('allowChargeViolation', allowChargeViolation)
    path.add_module(pmake)


def rankByHighest(particleList,
                  variable,
                  numBest=0,
                  outputVariable='',
                  allowMultiRank=False,
                  cut='',
                  path=None):
    """
    Ranks particles in the input list by the given variable (highest to lowest), and stores an integer rank for each Particle
    in an :b2:var:`extraInfo` field ``${variable}_rank`` starting at 1 (best).
    The list is also sorted from best to worst candidate
    (each charge, e.g. B+/B-, separately).
    This can be used to perform a best candidate selection by cutting on the corresponding rank value, or by specifying
    a non-zero value for 'numBest'.

    .. tip::
        Extra-info fields can be accessed by the :b2:var:`extraInfo` metavariable.
        These variable names can become clunky, so it's probably a good idea to set an alias.
        For example if you rank your B candidates by momentum,

        .. code:: python

            rankByHighest("B0:myCandidates", "p", path=mypath)
            vm.addAlias("momentumRank", "extraInfo(p_rank)")


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


def rankByLowest(particleList,
                 variable,
                 numBest=0,
                 outputVariable='',
                 allowMultiRank=False,
                 cut='',
                 path=None):
    """
    Ranks particles in the input list by the given variable (lowest to highest), and stores an integer rank for each Particle
    in an :b2:var:`extraInfo` field ``${variable}_rank`` starting at 1 (best).
    The list is also sorted from best to worst candidate
    (each charge, e.g. B+/B-, separately).
    This can be used to perform a best candidate selection by cutting on the corresponding rank value, or by specifying
    a non-zero value for 'numBest'.

    .. tip::
        Extra-info fields can be accessed by the :b2:var:`extraInfo` metavariable.
        These variable names can become clunky, so it's probably a good idea to set an alias.
        For example if you rank your B candidates by :b2:var:`dM`,

        .. code:: python

            rankByLowest("B0:myCandidates", "dM", path=mypath)
            vm.addAlias("massDifferenceRank", "extraInfo(dM_rank)")


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


def applyRandomCandidateSelection(particleList, path=None):
    """
    If there are multiple candidates in the provided particleList, all but one of them are removed randomly.
    This is done on a event-by-event basis.

    @param particleList     ParticleList for which the random candidate selection should be applied
    @param path             module is added to this path
    """

    rcs = register_module('BestCandidateSelection')
    rcs.set_name('RandomCandidateSelection_' + particleList)
    rcs.param('particleList', particleList)
    rcs.param('variable', 'random')
    rcs.param('selectLowest', False)
    rcs.param('allowMultiRank', False)
    rcs.param('numBest', 1)
    rcs.param('cut', '')
    rcs.param('outputVariable', '')
    path.add_module(rcs)


def printDataStore(eventNumber=-1, path=None):
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


def printVariableValues(list_name, var_names, path):
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


def printList(list_name, full, path):
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


def variablesToNtuple(decayString, variables, treename='variables', filename='ntuple.root', path=None):
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


def variablesToHistogram(decayString,
                         variables,
                         variables_2d=None,
                         filename='ntuple.root',
                         path=None, *,
                         directory=None,
                         prefixDecayString=False):
    """
    Creates and fills a flat ntuple with the specified variables from the VariableManager

    Parameters:
        decayString (str): specifies type of Particles and determines the name of the ParticleList
        variables (list(tuple))): variables + binning which must be registered in the VariableManager
        variables_2d (list(tuple)): pair of variables + binning for each which must be registered in the VariableManager
        filename (str): which is used to store the variables
        path (basf2.Path): the basf2 path where the analysis is processed
        directory (str): directory inside the output file where the histograms should be saved.
            Useful if you want to have different histograms in the same file to separate them.
        prefixDecayString (bool): If True the decayString will be prepended to the directory name to allow for more
            programmatic naming of the structure in the file.
    """
    if variables_2d is None:
        variables_2d = []
    output = register_module('VariablesToHistogram')
    output.set_name('VariablesToHistogram_' + decayString)
    output.param('particleList', decayString)
    output.param('variables', variables)
    output.param('variables_2d', variables_2d)
    output.param('fileName', filename)
    if directory is not None or prefixDecayString:
        if directory is None:
            directory = ""
        if prefixDecayString:
            directory = decayString + "_" + directory
        output.param("directory", directory)
    path.add_module(output)


def variablesToExtraInfo(particleList, variables, option=0, path=None):
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


def variablesToDaughterExtraInfo(particleList, decayString, variables, option=0, path=None):
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


def variablesToEventExtraInfo(particleList, variables, option=0, path=None):
    """
    For each particle in the input list the selected variables are saved in an event-extra-info field with the given name,
    Can be used to save MC truth information, for example, in a ntuple of reconstructed particles.

    An existing extra info with the same name will be overwritten if the new
    value is lower / will never be overwritten / will be overwritten if the
    new value is higher / will always be overwritten (-1/0/1/2).

    @param particleList  The input ParticleList
    @param variables     Dictionary of Variables and extraInfo names.
    @param path          modules are added to this path
    """

    mod = register_module('VariablesToEventExtraInfo')
    mod.set_name('VariablesToEventExtraInfo_' + particleList)
    mod.param('particleList', particleList)
    mod.param('variables', variables)
    mod.param('overwrite', option)
    path.add_module(mod)


def variableToSignalSideExtraInfo(particleList, varToExtraInfo, path):
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


def signalRegion(particleList, cut, path=None, name="isSignalRegion", blind_data=True):
    """
    Define and blind a signal region.
    Per default, the defined signal region is cut out if ran on data.
    This function will provide a new variable 'isSignalRegion' as default, which is either 0 or 1 depending on the cut
    provided.

    Example:
        >>> ma.reconstructDecay("B+:sig -> D+ pi0", "Mbc>5.2", path=path)
        >>> ma.signalRegion("B+:sig",
        >>>                  "Mbc>5.27 and abs(deltaE)<0.2",
        >>>                  blind_data=True,
        >>>                  path=path)
        >>> ma.variablesToNtuples("B+:sig", ["isSignalRegion"], path=path)

    Parameters:
        particleList (str):     The input ParticleList
        cut (str):              Cut string describing the signal region
        path (basf2.Path)::     Modules are added to this path
        name (str):             Name of the Signal region in the variable manager
        blind_data (bool):      Automatically exclude signal region from data

    """

    from variables import variables
    mod = register_module('VariablesToExtraInfo')
    mod.set_name(f'{name}_' + particleList)
    mod.param('particleList', particleList)
    mod.param('variables', {f"passesCut({cut})": name})
    variables.addAlias(name, f"extraInfo({name})")
    path.add_module(mod)

    # Check if we run on Data
    if blind_data:
        applyCuts(particleList, f"{name}==0 or isMC==1", path=path)


def removeExtraInfo(particleLists=None, removeEventExtraInfo=False, path=None):
    """
    Removes the ExtraInfo of the given particleLists. If specified (removeEventExtraInfo = True) also the EventExtraInfo is removed.
    """
    if particleLists is None:
        particleLists = []
    mod = register_module('ExtraInfoRemover')
    mod.param('particleLists', particleLists)
    mod.param('removeEventExtraInfo', removeEventExtraInfo)
    path.add_module(mod)


def signalSideParticleFilter(particleList, selection, roe_path, deadEndPath):
    """
    Checks if the current ROE object in the for_each roe path (argument roe_path) is related
    to the particle from the input ParticleList. Additional selection criteria can be applied.
    If ROE is not related to any of the Particles from ParticleList or the Particle doesn't
    meet the selection criteria the execution of deadEndPath is started. This path, as the name
    suggests should be empty and its purpose is to end the execution of for_each roe path for
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


def signalSideParticleListsFilter(particleLists, selection, roe_path, deadEndPath):
    """
    Checks if the current ROE object in the for_each roe path (argument roe_path) is related
    to the particle from the input ParticleList. Additional selection criteria can be applied.
    If ROE is not related to any of the Particles from ParticleList or the Particle doesn't
    meet the selection criteria the execution of deadEndPath is started. This path, as the name
    suggests should be empty and its purpose is to end the execution of for_each roe path for
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


def reconstructMCDecay(
    decayString,
    cut,
    dmID=0,
    writeOut=False,
    path=None,
    chargeConjugation=True,
):
    r"""
    Finds and creates a ``ParticleList`` from given decay string.
    ``ParticleList`` of daughters with sub-decay is created.

    Only signal particle, which means :b2:var:`isSignal` is equal to 1, is stored. One can use the decay string grammar
    to change the behavior of :b2:var:`isSignal`. One can find detailed information in :ref:`DecayString`.

    .. tip::
        If one uses same sub-decay twice, same particles are registered to a ``ParticleList``. For example,
        ``K_S0:pi0pi0 =direct=> [pi0:gg =direct=> gamma:MC gamma:MC] [pi0:gg =direct=> gamma:MC gamma:MC]``.
        One can skip the second sub-decay, ``K_S0:pi0pi0 =direct=> [pi0:gg =direct=> gamma:MC gamma:MC] pi0:gg``.


    @param decayString :ref:`DecayString` specifying what kind of the decay should be reconstructed
                       (from the DecayString the mother and daughter ParticleLists are determined)
    @param cut         created (mother) Particles are added to the mother ParticleList if they
                       pass given cuts (in VariableManager style) and rejected otherwise
                       isSignal==1 is always required by default.
    @param dmID        user specified decay mode identifier
    @param writeOut    whether RootOutput module should save the created ParticleList
    @param path        modules are added to this path
    @param chargeConjugation boolean to decide whether charge conjugated mode should be reconstructed as well (on by default)
    """

    pmake = register_module('ParticleCombinerFromMC')
    pmake.set_name('ParticleCombinerFromMC_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cut', cut)
    pmake.param('decayMode', dmID)
    pmake.param('writeOut', writeOut)
    pmake.param('chargeConjugation', chargeConjugation)
    path.add_module(pmake)


def findMCDecay(
    list_name,
    decay,
    writeOut=False,
    path=None,
):
    """
    .. warning::
        This function is not fully tested and maintained.
        Please consider to use reconstructMCDecay() instead.

    Finds and creates a ``ParticleList`` for all ``MCParticle`` decays matching a given :ref:`DecayString`.
    The decay string is required to describe correctly what you want.
    In the case of inclusive decays, you can use :ref:`Grammar_for_custom_MCMatching`

    @param list_name The output particle list name
    @param decay     The decay string which you want
    @param writeOut  Whether `RootOutput` module should save the created ``outputList``
    @param path      modules are added to this path
    """
    B2WARNING("This function is not fully tested and maintained."
              "Please consider to use reconstructMCDecay() instead.")

    decayfinder = register_module('MCDecayFinder')
    decayfinder.set_name('MCDecayFinder_' + list_name)
    decayfinder.param('listName', list_name)
    decayfinder.param('decayString', decay)
    decayfinder.param('writeOut', writeOut)
    path.add_module(decayfinder)


def summaryOfLists(particleLists, outputFile=None, path=None):
    """
    Prints out Particle statistics at the end of the job: number of events with at
    least one candidate, average number of candidates per event, etc.
    If an output file name is provided the statistics is also dumped into a json file with that name.

    @param particleLists list of input ParticleLists
    @param outputFile output file name (not created by default)
    """

    particleStats = register_module('ParticleStats')
    particleStats.param('particleLists', particleLists)
    if outputFile is not None:
        particleStats.param('outputFile', outputFile)
    path.add_module(particleStats)


def matchMCTruth(list_name, path):
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


def looseMCTruth(list_name, path):
    """
    Performs loose MC matching for all particles in the specified
    ParticleList.
    The difference between loose and normal mc matching algorithm is that
    the loose algorithm will find the common mother of the majority of daughter
    particles while the normal algorithm finds the common mother of all daughters.
    The results of loose mc matching algorithm are stored to the following extraInfo
    items:

      - looseMCMotherPDG: PDG code of most common mother
      - looseMCMotherIndex: 1-based StoreArray<MCParticle> index of most common mother
      - looseMCWrongDaughterN: number of daughters that don't originate from the most
                               common mother
      - looseMCWrongDaughterPDG: PDG code of the daughter that doesn't originate from
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


def buildRestOfEvent(target_list_name, inputParticlelists=None,
                     fillWithMostLikely=True,
                     chargedPIDPriors=None, path=None):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    dataobject and makes BASF2 relation between them. User can provide additional
    particle lists with a different particle hypothesis like ['K+:good, e+:good'], etc.

    @param target_list_name   name of the input ParticleList
    @param inputParticlelists list of user-defined input particle list names, which serve
                              as source of particles to build the ROE, the FSP particles from
                              target_list_name are automatically excluded from the ROE object
    @param fillWithMostLikely By default the module uses the most likely particle mass hypothesis for charged particles
                              based on the PID likelihood. Turn this behavior off if you want to configure your own
                              input particle lists.
    @param chargedPIDPriors   The prior PID fractions, that are used to regulate the
                              amount of certain charged particle species, should be a list of
                              six floats if not None. The order of particle types is
                              the following: [e-, mu-, pi-, K-, p+, d+]
    @param path               modules are added to this path
    """
    if inputParticlelists is None:
        inputParticlelists = []
    fillParticleList('pi+:all', '', path=path)
    if fillWithMostLikely:
        from stdCharged import stdMostLikely
        stdMostLikely(chargedPIDPriors, '_roe', path=path)
        inputParticlelists = ['%s:mostlikely_roe' % ptype for ptype in ['K+', 'p+', 'e+', 'mu+']]
    import b2bii
    if not b2bii.isB2BII():
        fillParticleList('gamma:all', '', path=path)
        fillParticleList('K_L0:roe_default', 'isFromKLM > 0', path=path)
        inputParticlelists += ['pi+:all', 'gamma:all', 'K_L0:roe_default']
    else:
        inputParticlelists += ['pi+:all', 'gamma:mdst']
    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('ROEBuilder_' + target_list_name)
    roeBuilder.param('particleList', target_list_name)
    roeBuilder.param('particleListsInput', inputParticlelists)
    roeBuilder.param('mostLikely', fillWithMostLikely)
    path.add_module(roeBuilder)


def buildNestedRestOfEvent(target_list_name, maskName='', path=None):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    @param target_list_name      name of the input ParticleList
    @param mask_name             name of the ROEMask to be used
    @param path                  modules are added to this path
    """
    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('NestedROEBuilder_' + target_list_name)
    roeBuilder.param('particleList', target_list_name)
    roeBuilder.param('nestedROEMask', maskName)
    roeBuilder.param('createNestedROE', True)
    path.add_module(roeBuilder)


def buildRestOfEventFromMC(target_list_name, inputParticlelists=None, path=None):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    @param target_list_name   name of the input ParticleList
    @param inputParticlelists list of input particle list names, which serve
                              as a source of particles to build ROE, the FSP particles from
                              target_list_name are excluded from ROE object
    @param path               modules are added to this path
    """
    if inputParticlelists is None:
        inputParticlelists = []
    if (len(inputParticlelists) == 0):
        # Type of particles to use for ROEBuilder
        # K_S0 and Lambda0 are added here because some of them have interacted
        # with the detector material
        types = ['gamma', 'e+', 'mu+', 'pi+', 'K+', 'p+', 'K_L0',
                 'n0', 'nu_e', 'nu_mu', 'nu_tau',
                 'K_S0', 'Lambda0']
        for t in types:
            fillParticleListFromMC("%s:roe_default_gen" % t, 'mcPrimary > 0 and nDaughters == 0',
                                   True, True, path=path)
            inputParticlelists += ["%s:roe_default_gen" % t]
    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('MCROEBuilder_' + target_list_name)
    roeBuilder.param('particleList', target_list_name)
    roeBuilder.param('particleListsInput', inputParticlelists)
    roeBuilder.param('fromMC', True)
    path.add_module(roeBuilder)


def appendROEMask(list_name,
                  mask_name,
                  trackSelection,
                  eclClusterSelection,
                  klmClusterSelection='',
                  path=None):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for tracks and eclClusters which will be used by variables in ROEVariables.cc.

    - append a ROE mask with all tracks in ROE coming from the IP region

       >>> appendROEMask('B+:sig', 'IPtracks', '[dr < 2] and [abs(dz) < 5]', '')

    - append a ROE mask with only ECL-based particles that pass as good photon candidates

       >>> goodPhotons = 'inCDCAcceptance and clusterErrorTiming < 1e6 and [clusterE1E9 > 0.4 or E > 0.075]'
       >>> appendROEMask('B+:sig', 'goodROEGamma', '', goodPhotons)


    @param list_name             name of the input ParticleList
    @param mask_name             name of the appended ROEMask
    @param trackSelection        decay string for the track-based particles in ROE
    @param eclClusterSelection   decay string for the ECL-based particles in ROE
    @param klmClusterSelection   decay string for the KLM-based particles in ROE
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + mask_name)
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasks', [(mask_name, trackSelection, eclClusterSelection, klmClusterSelection)])
    path.add_module(roeMask)


def appendROEMasks(list_name, mask_tuples, path=None):
    """
    Loads the ROE object of a particle and creates a ROE mask with a specific name. It applies
    selection criteria for track-, ECL- and KLM-based particles which will be used by ROE variables.

    The multiple ROE masks with their own selection criteria are specified
    via list of tuples (mask_name, trackParticleSelection, eclParticleSelection, klmParticleSelection) or
    (mask_name, trackSelection, eclClusterSelection) in case with fractions.

    - Example for two tuples, one with and one without fractions

       >>> ipTracks     = ('IPtracks', '[dr < 2] and [abs(dz) < 5]', '', '')
       >>> goodPhotons = 'inCDCAcceptance and [clusterErrorTiming < 1e6] and [clusterE1E9 > 0.4 or E > 0.075]'
       >>> goodROEGamma = ('ROESel', '[dr < 2] and [abs(dz) < 5]', goodPhotons, '')
       >>> goodROEKLM     = ('IPtracks', '[dr < 2] and [abs(dz) < 5]', '', 'nKLMClusterTrackMatches == 0')
       >>> appendROEMasks('B+:sig', [ipTracks, goodROEGamma, goodROEKLM])

    @param list_name             name of the input ParticleList
    @param mask_tuples           array of ROEMask list tuples to be appended
    @param path                  modules are added to this path
    """
    compatible_masks = []
    for mask in mask_tuples:
        # add empty KLM-based selection if it's absent:
        if len(mask) == 3:
            compatible_masks += [(*mask, '')]
        else:
            compatible_masks += [mask]
    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + 'MaskList')
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasks', compatible_masks)
    path.add_module(roeMask)


def updateROEMask(list_name,
                  mask_name,
                  trackSelection,
                  eclClusterSelection='',
                  klmClusterSelection='',
                  path=None):
    """
    Update an existing ROE mask by applying additional selection cuts for
    tracks and/or clusters.

    See function `appendROEMask`!

    @param list_name             name of the input ParticleList
    @param mask_name             name of the ROEMask to update
    @param trackSelection        decay string for the track-based particles in ROE
    @param eclClusterSelection   decay string for the ECL-based particles in ROE
    @param klmClusterSelection   decay string for the KLM-based particles in ROE
    @param path                  modules are added to this path
    """

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + mask_name)
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasks', [(mask_name, trackSelection, eclClusterSelection, klmClusterSelection)])
    roeMask.param('update', True)
    path.add_module(roeMask)


def updateROEMasks(list_name, mask_tuples, path):
    """
    Update existing ROE masks by applying additional selection cuts for tracks
    and/or clusters.

    The multiple ROE masks with their own selection criteria are specified
    via list tuples (mask_name, trackSelection, eclClusterSelection, klmClusterSelection)

    See function `appendROEMasks`!

    @param list_name             name of the input ParticleList
    @param mask_tuples           array of ROEMask list tuples to be appended
    @param path                  modules are added to this path
    """
    compatible_masks = []
    for mask in mask_tuples:
        # add empty KLM-based selection if it's absent:
        if len(mask) == 3:
            compatible_masks += [(*mask, '')]
        else:
            compatible_masks += [mask]

    roeMask = register_module('RestOfEventInterpreter')
    roeMask.set_name('RestOfEventInterpreter_' + list_name + '_' + 'MaskList')
    roeMask.param('particleList', list_name)
    roeMask.param('ROEMasks', compatible_masks)
    roeMask.param('update', True)
    path.add_module(roeMask)


def keepInROEMasks(list_name, mask_names, cut_string, path=None):
    """
    This function is used to apply particle list specific cuts on one or more ROE masks (track or eclCluster).
    With this function one can KEEP the tracks/eclclusters used in particles from provided particle list.
    This function should be executed only in the for_each roe path for the current ROE object.

    To avoid unnecessary computation, the input particle list should only contain particles from ROE
    (use cut 'isInRestOfEvent == 1'). To update the ECLCluster masks, the input particle list should be a photon
    particle list (e.g. 'gamma:someLabel'). To update the Track masks, the input particle list should be a charged
    pion particle list (e.g. 'pi+:someLabel').

    Updating a non-existing mask will create a new one.

    - keep only those tracks that were used in provided particle list

       >>> keepInROEMasks('pi+:goodTracks', 'mask', '')

    - keep only those clusters that were used in provided particle list and pass a cut, apply to several masks

       >>> keepInROEMasks('gamma:goodClusters', ['mask1', 'mask2'], 'E > 0.1')


    @param list_name    name of the input ParticleList
    @param mask_names   array of ROEMasks to be updated
    @param cut_string   decay string with which the mask will be updated
    @param path         modules are added to this path
    """

    updateMask = register_module('RestOfEventUpdater')
    updateMask.set_name('RestOfEventUpdater_' + list_name + '_masks')
    updateMask.param('particleList', list_name)
    updateMask.param('updateMasks', mask_names)
    updateMask.param('cutString', cut_string)
    updateMask.param('discard', False)
    path.add_module(updateMask)


def discardFromROEMasks(list_name, mask_names, cut_string, path=None):
    """
    This function is used to apply particle list specific cuts on one or more ROE masks (track or eclCluster).
    With this function one can DISCARD the tracks/eclclusters used in particles from provided particle list.
    This function should be executed only in the for_each roe path for the current ROE object.

    To avoid unnecessary computation, the input particle list should only contain particles from ROE
    (use cut 'isInRestOfEvent == 1'). To update the ECLCluster masks, the input particle list should be a photon
    particle list (e.g. 'gamma:someLabel'). To update the Track masks, the input particle list should be a charged
    pion particle list (e.g. 'pi+:someLabel').

    Updating a non-existing mask will create a new one.

    - discard tracks that were used in provided particle list

       >>> discardFromROEMasks('pi+:badTracks', 'mask', '')

    - discard clusters that were used in provided particle list and pass a cut, apply to several masks

       >>> discardFromROEMasks('gamma:badClusters', ['mask1', 'mask2'], 'E < 0.1')


    @param list_name    name of the input ParticleList
    @param mask_names   array of ROEMasks to be updated
    @param cut_string   decay string with which the mask will be updated
    @param path         modules are added to this path
    """

    updateMask = register_module('RestOfEventUpdater')
    updateMask.set_name('RestOfEventUpdater_' + list_name + '_masks')
    updateMask.param('particleList', list_name)
    updateMask.param('updateMasks', mask_names)
    updateMask.param('cutString', cut_string)
    updateMask.param('discard', True)
    path.add_module(updateMask)


def optimizeROEWithV0(list_name, mask_names, cut_string, path=None):
    """
    This function is used to apply particle list specific cuts on one or more ROE masks for Tracks.
    It is possible to optimize the ROE selection by treating tracks from V0's separately, meaning,
    taking V0's 4-momentum into account instead of 4-momenta of tracks. A cut for only specific V0's
    passing it can be applied.

    The input particle list should be a V0 particle list: K_S0 ('K_S0:someLabel', ''),
    Lambda ('Lambda:someLabel', '') or converted photons ('gamma:someLabel').

    Updating a non-existing mask will create a new one.

    - treat tracks from K_S0 inside mass window separately, replace track momenta with K_S0 momentum

       >>> optimizeROEWithV0('K_S0:opt', 'mask', '0.450 < M < 0.550')

    @param list_name    name of the input ParticleList
    @param mask_names   array of ROEMasks to be updated
    @param cut_string   decay string with which the mask will be updated
    @param path         modules are added to this path
    """

    updateMask = register_module('RestOfEventUpdater')
    updateMask.set_name('RestOfEventUpdater_' + list_name + '_masks')
    updateMask.param('particleList', list_name)
    updateMask.param('updateMasks', mask_names)
    updateMask.param('cutString', cut_string)
    path.add_module(updateMask)


def updateROEUsingV0Lists(target_particle_list, mask_names, default_cleanup=True, selection_cuts=None,
                          apply_mass_fit=False, fitter='treefit', path=None):
    """
    This function creates V0 particle lists (photons, :math:`K^0_S` and :math:`\\Lambda^0`)
    and it uses V0 candidates to update the Rest Of Event, which is associated to the target particle list.
    It is possible to apply a standard or customized selection and mass fit to the V0 candidates.


    @param target_particle_list  name of the input ParticleList
    @param mask_names            array of ROE masks to be applied
    @param default_cleanup       if True, predefined cuts will be applied on the V0 lists
    @param selection_cuts        a single string of selection cuts or tuple of three strings (photon_cuts, K_S0_cuts, Lambda0_cuts),
                                 which will be applied to the V0 lists. These cuts will have a priority over the default ones.
    @param apply_mass_fit        if True, a mass fit will be applied to the V0 particles
    @param fitter                string, that represent a fitter choice: "treefit" for TreeFitter and "kfit" for KFit
    @param path                  modules are added to this path
    """
    roe_path = create_path()
    deadEndPath = create_path()
    signalSideParticleFilter(target_particle_list, '', roe_path, deadEndPath)

    if (default_cleanup and selection_cuts is None):
        B2INFO("Using default cleanup in updateROEUsingV0Lists.")
        selection_cuts = 'abs(dM) < 0.1 '
        selection_cuts += 'and daughter(0,particleID) > 0.2 and daughter(1,particleID) > 0.2 '
        selection_cuts += 'and daughter(0,thetaInCDCAcceptance) and daughter(1,thetaInCDCAcceptance)'
    if (selection_cuts is None or selection_cuts == ''):
        B2INFO("No cleanup in updateROEUsingV0Lists.")
        selection_cuts = ('True', 'True', 'True')
    if (isinstance(selection_cuts, str)):
        selection_cuts = (selection_cuts, selection_cuts, selection_cuts)
    # The isInRestOfEvent variable will be applied on FSPs of composite particles automatically:
    roe_cuts = 'isInRestOfEvent > 0'
    fillConvertedPhotonsList('gamma:v0_roe -> e+ e-', f'{selection_cuts[0]} and {roe_cuts}',
                             path=roe_path)
    fillParticleList('K_S0:v0_roe -> pi+ pi-', f'{selection_cuts[1]} and {roe_cuts}',
                     path=roe_path)
    fillParticleList('Lambda0:v0_roe -> p+ pi-', f'{selection_cuts[2]} and {roe_cuts}',
                     path=roe_path)
    fitter = fitter.lower()
    if (fitter != 'treefit' and fitter != 'kfit'):
        B2WARNING('Argument "fitter" in updateROEUsingV0Lists has only "treefit" and "kfit" options, '
                  f'but "{fitter}" was provided! TreeFitter will be used instead.')
        fitter = 'treefit'
    from vertex import kFit, treeFit
    for v0 in ['gamma:v0_roe', 'K_S0:v0_roe', 'Lambda0:v0_roe']:
        if (apply_mass_fit and fitter == 'kfit'):
            kFit(v0, conf_level=0.0, fit_type='massvertex', path=roe_path)
        if (apply_mass_fit and fitter == 'treefit'):
            treeFit(v0, conf_level=0.0, massConstraint=[v0.split(':')[0]], path=roe_path)
        optimizeROEWithV0(v0, mask_names, '', path=roe_path)
    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)


def printROEInfo(mask_names=None, full_print=False,
                 unpackComposites=True, path=None):
    """
    This function prints out the information for the current ROE, so it should only be used in the for_each path.
    It prints out basic ROE object info.

    If mask names are provided, specific information for those masks will be printed out.

    It is also possible to print out all particles in a given mask if the
    'full_print' is set to True.

    @param mask_names         array of ROEMask names for printing out info
    @param unpackComposites   if true, replace composite particles by their daughters
    @param full_print         print out particles in mask
    @param path               modules are added to this path
    """
    if mask_names is None:
        mask_names = []
    printMask = register_module('RestOfEventPrinter')
    printMask.set_name('RestOfEventPrinter')
    printMask.param('maskNames', mask_names)
    printMask.param('fullPrint', full_print)
    printMask.param('unpackComposites', unpackComposites)
    path.add_module(printMask)


def buildContinuumSuppression(list_name, roe_mask, path):
    """
    Creates for each Particle in the given ParticleList a ContinuumSuppression
    dataobject and makes BASF2 relation between them.

    :param list_name: name of the input ParticleList
    :param roe_mask: name of the ROE mask
    :param path: modules are added to this path
    """

    qqBuilder = register_module('ContinuumSuppressionBuilder')
    qqBuilder.set_name('QQBuilder_' + list_name)
    qqBuilder.param('particleList', list_name)
    qqBuilder.param('ROEMask', roe_mask)
    path.add_module(qqBuilder)


def removeParticlesNotInLists(lists_to_keep, path):
    """
    Removes all Particles that are not in a given list of ParticleLists (or daughters of those).
    All relations from/to Particles, daughter indices, and other ParticleLists are fixed.

    @param lists_to_keep Keep the Particles and their daughters in these ParticleLists.
    @param path      modules are added to this path
    """

    mod = register_module('RemoveParticlesNotInLists')
    mod.param('particleLists', lists_to_keep)
    path.add_module(mod)


def inclusiveBtagReconstruction(upsilon_list_name, bsig_list_name, btag_list_name, input_lists_names, path):
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


def selectDaughters(particle_list_name, decay_string, path):
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


def markDuplicate(particleList, prioritiseV0, path):
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


PI0ETAVETO_COUNTER = 0


def oldwritePi0EtaVeto(
    particleList,
    decayString,
    workingDirectory='.',
    pi0vetoname='Pi0_Prob',
    etavetoname='Eta_Prob',
    downloadFlag=True,
    selection='',
    path=None
):
    """
    Give pi0/eta probability for hard photon.

    In the default weight files a value of 1.4 GeV is set as the lower limit for the hard photon energy in the CMS frame.

    The current default weight files are optimised using MC9.
    The input variables are as below. Aliases are set to some variables during training.

    * M: pi0/eta candidates Invariant mass
    * lowE: soft photon energy in lab frame
    * cTheta: soft photon ECL cluster's polar angle
    * Zmva: soft photon output of MVA using Zernike moments of the cluster
    * minC2Hdist: soft photon distance from eclCluster to nearest point on nearest Helix at the ECL cylindrical radius

    If you don't have weight files in your workingDirectory,
    these files are downloaded from database to your workingDirectory automatically.
    Please refer to analysis/examples/tutorials/B2A306-B02RhoGamma-withPi0EtaVeto.py
    about how to use this function.

    NOTE:
      Please don't use following ParticleList names elsewhere:

      ``gamma:HARDPHOTON``, ``pi0:PI0VETO``, ``eta:ETAVETO``,
      ``gamma:PI0SOFT + str(PI0ETAVETO_COUNTER)``, ``gamma:ETASOFT + str(PI0ETAVETO_COUNTER)``

      Please don't use ``lowE``, ``cTheta``, ``Zmva``, ``minC2Hdist`` as alias elsewhere.

    @param particleList     The input ParticleList
    @param decayString specify Particle to be added to the ParticleList
    @param workingDirectory The weight file directory
    @param downloadFlag whether download default weight files or not
    @param pi0vetoname extraInfo name of pi0 probability
    @param etavetoname extraInfo name of eta probability
    @param selection Selection criteria that Particle needs meet in order for for_each ROE path to continue
    @param path       modules are added to this path
    """

    import os
    import basf2_mva

    global PI0ETAVETO_COUNTER

    if PI0ETAVETO_COUNTER == 0:
        from variables import variables
        variables.addAlias('lowE', 'daughter(1,E)')
        variables.addAlias('cTheta', 'daughter(1,clusterTheta)')
        variables.addAlias('Zmva', 'daughter(1,clusterZernikeMVA)')
        variables.addAlias('minC2Tdist', 'daughter(1,minC2TDist)')
        variables.addAlias('cluNHits', 'daughter(1,clusterNHits)')
        variables.addAlias('E9E21', 'daughter(1,clusterE9E21)')

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
        B2INFO('oldwritePi0EtaVeto: ' + workingDirectory + ' has been created as workingDirectory.')

    if not os.path.isfile(workingDirectory + '/pi0veto.root'):
        if downloadFlag:
            basf2_mva.download('Pi0VetoIdentifier', workingDirectory + '/pi0veto.root')
            B2INFO('oldwritePi0EtaVeto: pi0veto.root has been downloaded from database to workingDirectory.')

    if not os.path.isfile(workingDirectory + '/etaveto.root'):
        if downloadFlag:
            basf2_mva.download('EtaVetoIdentifier', workingDirectory + '/etaveto.root')
            B2INFO('oldwritePi0EtaVeto: etaveto.root has been downloaded from database to workingDirectory.')

    roe_path.add_module('MVAExpert', listNames=['pi0:PI0VETO'], extraInfoName='Pi0Veto',
                        identifier=workingDirectory + '/pi0veto.root')
    roe_path.add_module('MVAExpert', listNames=['eta:ETAVETO'], extraInfoName='EtaVeto',
                        identifier=workingDirectory + '/etaveto.root')

    rankByHighest('pi0:PI0VETO', 'extraInfo(Pi0Veto)', numBest=1, path=roe_path)
    rankByHighest('eta:ETAVETO', 'extraInfo(EtaVeto)', numBest=1, path=roe_path)

    variableToSignalSideExtraInfo('pi0:PI0VETO', {'extraInfo(Pi0Veto)': pi0vetoname}, path=roe_path)
    variableToSignalSideExtraInfo('eta:ETAVETO', {'extraInfo(EtaVeto)': etavetoname}, path=roe_path)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)


def writePi0EtaVeto(
    particleList,
    decayString,
    mode='standard',
    selection='',
    path=None,
    suffix='',
    hardParticle='gamma',
    pi0PayloadNameOverride=None,
    pi0SoftPhotonCutOverride=None,
    etaPayloadNameOverride=None,
    etaSoftPhotonCutOverride=None
):
    """
    Give pi0/eta probability for hard photon.

    In the default weight files a value of 1.4 GeV is set as the lower limit for the hard photon energy in the CMS frame.

    The current default weight files are optimised using MC12.

    The input variables of the mva training are:

    * M: pi0/eta candidates Invariant mass
    * daughter(1,E): soft photon energy in lab frame
    * daughter(1,clusterTheta): soft photon ECL cluster's polar angle
    * daughter(1,minC2TDist): soft photon distance from eclCluster to nearest point on nearest Helix at the ECL cylindrical radius
    * daughter(1,clusterZernikeMVA): soft photon output of MVA using Zernike moments of the cluster
    * daughter(1,clusterNHits): soft photon total crystal weights sum(w_i) with w_i<=1
    * daughter(1,clusterE9E21): soft photon ratio of energies in inner 3x3 crystals and 5x5 crystals without corners
    * cosHelicityAngleMomentum: pi0/eta candidates cosHelicityAngleMomentum

    The following strings are available for mode:

    * standard: loose energy cut and no clusterNHits cut are applied to soft photon
    * tight: tight energy cut and no clusterNHits cut are applied to soft photon
    * cluster: loose energy cut and clusterNHits cut are applied to soft photon
    * both: tight energy cut and clusterNHits cut are applied to soft photon

    The final probability of the pi0/eta veto is stored as an extraInfo. If no suffix is set it can be obtained from the variables
    `pi0Prob`/`etaProb`. Otherwise, it is available as '{Pi0, Eta}ProbOrigin', '{Pi0, Eta}ProbTightEnergyThreshold', '{Pi0,
    Eta}ProbLargeClusterSize', or '{Pi0, Eta}ProbTightEnergyThresholdAndLargeClusterSize'} for the four modes described above, with
    the chosen suffix appended.

    NOTE:
      Please don't use following ParticleList names elsewhere:

      ``gamma:HardPhoton``,
      ``gamma:Pi0Soft + ListName + '_' + particleList.replace(':', '_')``,
      ``gamma:EtaSoft + ListName + '_' + particleList.replace(':', '_')``,
      ``pi0:EtaVeto + ListName``,
      ``eta:EtaVeto + ListName``

    @param particleList     the input ParticleList
    @param decayString 		specify Particle to be added to the ParticleList
    @param mode				choose one mode out of 'standard', 'tight', 'cluster' and 'both'
    @param selection 		selection criteria that Particle needs meet in order for for_each ROE path to continue
    @param path       		modules are added to this path
    @param suffix           optional suffix to be appended to the usual extraInfo name
    @param hardParticle           particle name which is used to calculate the pi0/eta probability (default is gamma)
    @param pi0PayloadNameOverride  specify the payload name of pi0 veto only if one wants to use non-default one. (default is None)
    @param pi0SoftPhotonCutOverride specify the soft photon selection criteria of pi0 veto only if one wants to use non-default one.
                                    (default is None)
    @param etaPayloadNameOverride  specify the payload name of eta veto only if one wants to use non-default one. (default is None)
    @param etaSoftPhotonCutOverride specify the soft photon selection criteria of eta veto only if one wants to use non-default one.
                                    (default is None)
    """

    renameSuffix = False

    for module in path.modules():
        if module.type() == "SubEvent" and not renameSuffix:
            for subpath in [p.values for p in module.available_params() if p.name == "path"]:
                if renameSuffix:
                    break
                for submodule in subpath.modules():
                    print(submodule.name())
                    if f'{hardParticle}:HardPhoton{suffix}' in submodule.name():
                        suffix += '_0'
                        B2WARNING("Same extension already used in writePi0EtaVeto, append '_0'")
                        renameSuffix = True
                        break

    roe_path = create_path()
    deadEndPath = create_path()
    signalSideParticleFilter(particleList, selection, roe_path, deadEndPath)
    fillSignalSideParticleList(f'{hardParticle}:HardPhoton{suffix}', decayString, path=roe_path)

    dictListName = {'standard': 'Origin',
                    'tight': 'TightEnergyThreshold',
                    'cluster': 'LargeClusterSize',
                    'both': 'TightEnrgyThresholdAndLargeClusterSize'}

    dictPi0EnergyCut = {'standard': '[[clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.02] or [clusterReg==3 and E>0.02]]',
                        'tight': '[[clusterReg==1 and E>0.03] or [clusterReg==2 and E>0.03] or [clusterReg==3 and E>0.04]]',
                        'cluster': '[[clusterReg==1 and E>0.025] or [clusterReg==2 and E>0.02] or [clusterReg==3 and E>0.02]]',
                        'both': '[[clusterReg==1 and E>0.03] or [clusterReg==2 and E>0.03] or [clusterReg==3 and E>0.04]]'}

    dictEtaEnergyCut = {'standard': '[clusterReg==1 and E>0.035] or [clusterReg==2 and E>0.03] or [clusterReg==3 and E>0.03]',
                        'tight': '[clusterReg==1 and E>0.06] or [clusterReg==2 and E>0.06] or [clusterReg==3 and E>0.06]',
                        'cluster': '[clusterReg==1 and E>0.035] or [clusterReg==2 and E>0.03] or [clusterReg==3 and E>0.03]',
                        'both': '[clusterReg==1 and E>0.06] or [clusterReg==2 and E>0.06] or [clusterReg==3 and E>0.06]'}

    dictTimingAndNHitsCut = {'standard': 'abs(clusterTiming)<clusterErrorTiming',
                             'tight': 'abs(clusterTiming)<clusterErrorTiming',
                             'cluster': 'abs(clusterTiming)<clusterErrorTiming and clusterNHits >= 2',
                             'both': 'abs(clusterTiming)<clusterErrorTiming and clusterNHits >= 2'}

    dictPi0PayloadName = {'standard': 'Pi0VetoIdentifierStandard',
                          'tight': 'Pi0VetoIdentifierWithHigherEnergyThreshold',
                          'cluster': 'Pi0VetoIdentifierWithLargerClusterSize',
                          'both': 'Pi0VetoIdentifierWithHigherEnergyThresholdAndLargerClusterSize'}

    dictEtaPayloadName = {'standard': 'EtaVetoIdentifierStandard',
                          'tight': 'EtaVetoIdentifierWithHigherEnergyThreshold',
                          'cluster': 'EtaVetoIdentifierWithLargerClusterSize',
                          'both': 'EtaVetoIdentifierWithHigherEnergyThresholdAndLargerClusterSize'}

    dictPi0ExtraInfoName = {'standard': 'Pi0ProbOrigin',
                            'tight': 'Pi0ProbTightEnergyThreshold',
                            'cluster': 'Pi0ProbLargeClusterSize',
                            'both': 'Pi0ProbTightEnergyThresholdAndLargeClusterSize'}

    dictEtaExtraInfoName = {'standard': 'EtaProbOrigin',
                            'tight': 'EtaProbTightEnergyThreshold',
                            'cluster': 'EtaProbLargeClusterSize',
                            'both': 'EtaProbTightEnergyThresholdAndLargeClusterSize'}

    ListName = dictListName[mode]
    Pi0EnergyCut = dictPi0EnergyCut[mode]
    EtaEnergyCut = dictEtaEnergyCut[mode]
    TimingAndNHitsCut = dictTimingAndNHitsCut[mode]
    Pi0PayloadName = dictPi0PayloadName[mode]
    EtaPayloadName = dictEtaPayloadName[mode]
    Pi0ExtraInfoName = dictPi0ExtraInfoName[mode]
    EtaExtraInfoName = dictEtaExtraInfoName[mode]

    """
    pi0 veto
    """
    if pi0PayloadNameOverride is not None:
        Pi0PayloadName = pi0PayloadNameOverride
    if pi0SoftPhotonCutOverride is None:
        Pi0SoftPhotonCut = Pi0EnergyCut + ' and ' + TimingAndNHitsCut
    else:
        Pi0SoftPhotonCut = pi0SoftPhotonCutOverride

    # define the particleList name for soft photon
    pi0soft = f'gamma:Pi0Soft{suffix}' + ListName + '_' + particleList.replace(':', '_')
    # fill the particleList for soft photon with energy, timing and clusterNHits cuts
    fillParticleList(pi0soft, Pi0SoftPhotonCut, path=roe_path)
    # reconstruct pi0
    reconstructDecay('pi0:Pi0Veto' + ListName + f' -> {hardParticle}:HardPhoton{suffix} ' + pi0soft, '',
                     allowChargeViolation=True, path=roe_path)
    # MVA training is conducted.
    roe_path.add_module('MVAExpert', listNames=['pi0:Pi0Veto' + ListName],
                        extraInfoName=Pi0ExtraInfoName, identifier=Pi0PayloadName)
    # Pick up only one pi0/eta candidate with the highest pi0/eta probability.
    rankByHighest('pi0:Pi0Veto' + ListName, 'extraInfo(' + Pi0ExtraInfoName + ')', numBest=1, path=roe_path)
    # 'extraInfo(Pi0Veto)' is labeled 'Pi0_Prob'
    variableToSignalSideExtraInfo('pi0:Pi0Veto' + ListName,
                                  {'extraInfo(' + Pi0ExtraInfoName + ')': Pi0ExtraInfoName + suffix}, path=roe_path)

    """
    eta veto
    """
    if etaPayloadNameOverride is not None:
        EtaPayloadName = etaPayloadNameOverride
    if etaSoftPhotonCutOverride is None:
        EtaSoftPhotonCut = EtaEnergyCut + ' and ' + TimingAndNHitsCut
    else:
        EtaSoftPhotonCut = etaSoftPhotonCutOverride

    etasoft = f'gamma:EtaSoft{suffix}' + ListName + '_' + particleList.replace(':', '_')
    fillParticleList(etasoft, EtaSoftPhotonCut, path=roe_path)
    reconstructDecay('eta:EtaVeto' + ListName + f' -> {hardParticle}:HardPhoton{suffix} ' + etasoft, '',
                     allowChargeViolation=True, path=roe_path)
    roe_path.add_module('MVAExpert', listNames=['eta:EtaVeto' + ListName],
                        extraInfoName=EtaExtraInfoName, identifier=EtaPayloadName)
    rankByHighest('eta:EtaVeto' + ListName, 'extraInfo(' + EtaExtraInfoName + ')', numBest=1, path=roe_path)
    variableToSignalSideExtraInfo('eta:EtaVeto' + ListName,
                                  {'extraInfo(' + EtaExtraInfoName + ')': EtaExtraInfoName + suffix}, path=roe_path)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)


def getBeamBackgroundProbabilityMVA(
    particleList,
    path=None,
):
    """
    Assign a probability to each ECL cluster as being background like (0) or signal like (1)
    @param particleList     The input ParticleList, must be a photon list
    @param path       modules are added to this path
    """

    basf2.conditions.prepend_globaltag(getAnalysisGlobaltag())
    path.add_module(
        'MVAExpert',
        listNames=particleList,
        extraInfoName='beamBackgroundProbabilityMVA',
        identifier='BeamBackgroundMVA')


def buildEventKinematics(inputListNames=None, default_cleanup=True, custom_cuts=None,
                         chargedPIDPriors=None, fillWithMostLikely=False, path=None):
    """
    Calculates the global kinematics of the event (visible energy, missing momentum, missing mass...)
    using ParticleLists provided. If no ParticleList is provided, default ParticleLists are used
    (all track and all hits in ECL without associated track).

    The visible energy missing values are
    stored in a EventKinematics dataobject.

    @param inputListNames     list of ParticleLists used to calculate the global event kinematics.
                              If the list is empty, default ParticleLists pi+:evtkin and gamma:evtkin are filled.
    @param fillWithMostLikely if True, the module uses the most likely particle mass hypothesis for charged particles
                              according to the PID likelihood and the option inputListNames will be ignored.
    @param chargedPIDPriors   The prior PID fractions, that are used to regulate
                              amount of certain charged particle species, should be a list of
                              six floats if not None. The order of particle types is
                              the following: [e-, mu-, pi-, K-, p+, d+]
    @param default_cleanup    if True and either inputListNames empty or fillWithMostLikely True, default clean up cuts are applied
    @param custom_cuts        tuple of selection cut strings of form (trackCuts, photonCuts), default is None,
                              which would result in a standard predefined selection cuts
    @param path               modules are added to this path
    """
    if inputListNames is None:
        inputListNames = []
    trackCuts = 'pt > 0.1'
    trackCuts += ' and thetaInCDCAcceptance'
    trackCuts += ' and abs(dz) < 3'
    trackCuts += ' and dr < 0.5'

    gammaCuts = 'E > 0.05'
    gammaCuts += ' and thetaInCDCAcceptance'
    if (custom_cuts is not None):
        trackCuts, gammaCuts = custom_cuts

    if fillWithMostLikely:
        from stdCharged import stdMostLikely
        stdMostLikely(chargedPIDPriors, '_evtkin', path=path)
        inputListNames = ['%s:mostlikely_evtkin' % ptype for ptype in ['K+', 'p+', 'e+', 'mu+', 'pi+']]
        fillParticleList('gamma:evtkin', '', path=path)
        inputListNames += ['gamma:evtkin']
        if default_cleanup:
            B2INFO("Using default cleanup in EventKinematics module.")
            for ptype in ['K+', 'p+', 'e+', 'mu+', 'pi+']:
                applyCuts(f'{ptype}:mostlikely_evtkin', trackCuts, path=path)
            applyCuts('gamma:evtkin', gammaCuts, path=path)
        else:
            B2INFO("No cleanup in EventKinematics module.")
    if not inputListNames:
        B2INFO("Creating particle lists pi+:evtkin and gamma:evtkin to get the global kinematics of the event.")
        fillParticleList('pi+:evtkin', '', path=path)
        fillParticleList('gamma:evtkin', '', path=path)
        particleLists = ['pi+:evtkin', 'gamma:evtkin']
        if default_cleanup:
            if (custom_cuts is not None):
                B2INFO("Using default cleanup in EventKinematics module.")
            applyCuts('pi+:evtkin', trackCuts, path=path)
            applyCuts('gamma:evtkin', gammaCuts, path=path)
        else:
            B2INFO("No cleanup in EventKinematics module.")
    else:
        particleLists = inputListNames

    eventKinematicsModule = register_module('EventKinematics')
    eventKinematicsModule.set_name('EventKinematics_reco')
    eventKinematicsModule.param('particleLists', particleLists)
    path.add_module(eventKinematicsModule)


def buildEventKinematicsFromMC(inputListNames=None, selectionCut='', path=None):
    """
    Calculates the global kinematics of the event (visible energy, missing momentum, missing mass...)
    using generated particles. If no ParticleList is provided, default generated ParticleLists are used.

    @param inputListNames     list of ParticleLists used to calculate the global event kinematics.
                              If the list is empty, default ParticleLists are filled.
    @param selectionCut       optional selection cuts
    @param path               Path to append the eventKinematics module to.
    """
    if inputListNames is None:
        inputListNames = []
    if (len(inputListNames) == 0):
        # Type of particles to use for EventKinematics
        # K_S0 and Lambda0 are added here because some of them have interacted
        # with the detector material
        types = ['gamma', 'e+', 'mu+', 'pi+', 'K+', 'p+',
                 'K_S0', 'Lambda0']
        for t in types:
            fillParticleListFromMC("%s:evtkin_default_gen" % t, 'mcPrimary > 0 and nDaughters == 0',
                                   True, True, path=path)
            if (selectionCut != ''):
                applyCuts("%s:evtkin_default_gen" % t, selectionCut, path=path)
            inputListNames += ["%s:evtkin_default_gen" % t]

    eventKinematicsModule = register_module('EventKinematics')
    eventKinematicsModule.set_name('EventKinematics_gen')
    eventKinematicsModule.param('particleLists', inputListNames)
    eventKinematicsModule.param('usingMC', True)
    path.add_module(eventKinematicsModule)


def buildEventShape(inputListNames=None,
                    default_cleanup=True,
                    custom_cuts=None,
                    allMoments=False,
                    cleoCones=True,
                    collisionAxis=True,
                    foxWolfram=True,
                    harmonicMoments=True,
                    jets=True,
                    sphericity=True,
                    thrust=True,
                    checkForDuplicates=False,
                    path=None):
    """
    Calculates the event-level shape quantities (thrust, sphericity, Fox-Wolfram moments...)
    using the particles in the lists provided by the user. If no particle list is provided,
    the function will internally create a list of good tracks and a list of good photons
    with (optionally) minimal quality cuts.


    The results of the calculation are then stored into the EventShapeContainer dataobject,
    and are accessible using the variables of the EventShape group.

    The user can switch the calculation of certain quantities on or off to save computing
    time. By default the calculation of the high-order moments (5-8) is turned off.
    Switching off an option will make the corresponding variables not available.

    Warning:
       The user can provide as many particle lists
       as needed, using also combined particles, but the function will always assume that
       the lists are independent.
       If the lists provided by the user contain several times the same track (either with
       different mass hypothesis, or once as an independent particle and once as daughter of a
       combined particle) the results won't be reliable.
       A basic check for duplicates is available setting the checkForDuplicate flags,
       but is usually quite time consuming.


    @param inputListNames     List of ParticleLists used to calculate the
                              event shape variables. If the list is empty the default
                              particleLists pi+:evtshape and gamma:evtshape are filled.
    @param default_cleanup    If True, applies standard cuts on pt and cosTheta when
                              defining the internal lists. This option is ignored if the
                              particleLists are provided by the user.
    @param custom_cuts        tuple of selection cut strings of form (trackCuts, photonCuts), default is None,
                              which would result in a standard predefined selection cuts
    @param path               Path to append the eventShape modules to.
    @param thrust             Enables the calculation of thrust-related quantities (CLEO
                              cones, Harmonic moments, jets).
    @param collisionAxis      Enables the calculation of the  quantities related to the
                              collision axis .
    @param foxWolfram         Enables the calculation of the Fox-Wolfram moments.
    @param harmonicMoments    Enables the calculation of the Harmonic moments with respect
                              to both the thrust axis and, if collisionAxis = True, the collision axis.
    @param allMoments         If True, calculates also the  FW and harmonic moments from order
                              5 to 8 instead of the low-order ones only.
    @param cleoCones          Enables the calculation of the CLEO cones with respect to both the thrust
                              axis and, if collisionAxis = True, the collision axis.
    @param jets               Enables the calculation of the hemisphere momenta and masses.
                              Requires thrust = True.
    @param sphericity         Enables the calculation of the sphericity-related quantities.
    @param checkForDuplicates Perform a check for duplicate particles before adding them. This option
                              is quite time consuming, instead of using it consider sanitizing
                              the lists you are passing to the function.
    """
    if inputListNames is None:
        inputListNames = []
    trackCuts = 'pt > 0.1'
    trackCuts += ' and thetaInCDCAcceptance'
    trackCuts += ' and abs(dz) < 3.0'
    trackCuts += ' and dr < 0.5'

    gammaCuts = 'E > 0.05'
    gammaCuts += ' and thetaInCDCAcceptance'
    if (custom_cuts is not None):
        trackCuts, gammaCuts = custom_cuts

    if not inputListNames:
        B2INFO("Creating particle lists pi+:evtshape and gamma:evtshape to get the event shape variables.")
        fillParticleList('pi+:evtshape', '', path=path)
        fillParticleList('gamma:evtshape', '', path=path)
        particleLists = ['pi+:evtshape', 'gamma:evtshape']

        if default_cleanup:
            if (custom_cuts is not None):
                B2INFO("Applying standard cuts")
            applyCuts('pi+:evtshape', trackCuts, path=path)

            applyCuts('gamma:evtshape', gammaCuts, path=path)
        else:
            B2WARNING("Creating the default lists with no cleanup.")
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


def labelTauPairMC(printDecayInfo=False, path=None, TauolaBelle=False, mapping_minus=None, mapping_plus=None):
    """
    Search tau leptons into the MC information of the event. If confirms it's a generated tau pair decay,
    labels the decay generated of the positive and negative leptons using the ID of KKMC tau decay table.

    @param printDecayInfo:  If true, prints ID and prong of each tau lepton in the event.
    @param path:        module is added to this path
    @param TauolaBelle: if False, TauDecayMarker is set. If True, TauDecayMode is set.
    @param mapping_minus: if None, the map is the default one, else the path for the map is given by the user for tau-
    @param mapping_plus: if None, the map is the default one, else the path for the map is given by the user for tau+
    """
    from basf2 import find_file
    if not TauolaBelle:

        if printDecayInfo:
            m_printmode = 'all'
        else:
            m_printmode = 'default'

        if mapping_minus is None:
            mp_file_minus = find_file('data/analysis/modules/TauDecayMode/map_tauminus.txt')
        else:
            mp_file_minus = mapping_minus

        if mapping_plus is None:
            mp_file_plus = find_file('data/analysis/modules/TauDecayMode/map_tauplus.txt')
        else:
            mp_file_plus = mapping_plus

        path.add_module('TauDecayMode', printmode=m_printmode, file_minus=mp_file_minus, file_plus=mp_file_plus)

    else:
        tauDecayMarker = register_module('TauDecayMarker')
        tauDecayMarker.set_name('TauDecayMarker_')

        path.add_module(tauDecayMarker, printDecayInfo=printDecayInfo)


def tagCurlTracks(particleLists,
                  mcTruth=False,
                  responseCut=0.324,
                  selectorType='cut',
                  ptCut=0.6,
                  train=False,
                  path=None):
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

    import b2bii
    belle = b2bii.isB2BII()

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


def applyChargedPidMVA(particleLists, path, trainingMode, binaryHypoPDGCodes=(0, 0)):
    """
    Use an MVA to perform particle identification for charged stable particles, using the `ChargedPidMVA` module.

    The module decorates Particle objects in the input ParticleList(s) with variables
    containing the appropriate MVA score, which can be used to select candidates by placing a cut on it.

    Note:
        The MVA algorithm used is a gradient boosted decision tree (**TMVA 4.2.1**, **ROOT 6.14/06**).

    The module can perform either 'binary' PID between input S, B particle mass hypotheses according to the following scheme:

    - e (11) vs. pi (211)
    - mu (13) vs. pi (211)
    - pi (211) vs. K (321)
    - K (321) vs. pi (211)
    - p (2212) vs. pi (211)
    - d (1000010020) vs pi (211)

    , or 'global' PID, namely "one-vs-others" separation. The latter makes use of an MVA algorithm trained in multi-class mode,
    and it's the default behaviour.

    Note:
        Currently the MVA is charge-agnostic, i.e. the training is not done independently for +/- charged particles.

    Parameters:
        particleLists (list(str)): list of names of ParticleList objects for charged stable particles.
                                   The charge-conjugate ParticleLists will be also processed automatically.
        path (basf2.Path): the module is added to this path.
        trainingMode (``Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode``): enum identifier of the training mode.
          Needed to pick up the correct payload from the DB. Available choices:

          * c_Classification=0
          * c_Multiclass=1
          * c_ECL_Classification=2
          * c_ECL_Multiclass=3
          * c_PSD_Classification=4
          * c_PSD_Multiclass=5
          * c_ECL_PSD_Classification=6
          * c_ECL_PSD_Multiclass=7

        binaryHypoPDGCodes (tuple(int, int), ``optional``): the pdgIds of the signal, background mass hypothesis.
          Required only for binary PID mode.
    """

    from ROOT import Belle2

    plSet = set(particleLists)

    # Map the training mode enum value to the actual name of the payload in the GT.
    payloadNames = {
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_Classification:
        {"mode": "Classification", "detector": "ALL"},
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_Multiclass:
        {"mode": "Multiclass", "detector": "ALL"},
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_ECL_Classification:
        {"mode": "ECL_Classification", "detector": "ECL"},
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_ECL_Multiclass:
        {"mode": "ECL_Multiclass", "detector": "ECL"},
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_PSD_Classification:
        {"mode": "PSD_Classification", "detector": "ALL"},
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_PSD_Multiclass:
        {"mode": "PSD_Multiclass", "detector": "ALL"},
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_ECL_PSD_Classification:
        {"mode": "ECL_PSD_Classification", "detector": "ECL"},
        Belle2.ChargedPidMVAWeights.ChargedPidMVATrainingMode.c_ECL_PSD_Multiclass:
        {"mode": "ECL_PSD_Multiclass", "detector": "ECL"},
    }

    if payloadNames.get(trainingMode) is None:
        B2FATAL("The chosen training mode integer identifier:\n", trainingMode,
                "\nis not supported. Please choose among the following:\n",
                "\n".join(f"{key}:{val.get('mode')}" for key, val in sorted(payloadNames.items())))

    mode = payloadNames.get(trainingMode).get("mode")
    detector = payloadNames.get(trainingMode).get("detector")

    payloadName = f"ChargedPidMVAWeights_{mode}"

    if binaryHypoPDGCodes == (0, 0):
        # MULTI-CLASS training mode.

        chargedpid = register_module("ChargedPidMVAMulticlass")
        chargedpid.set_name(f"ChargedPidMVAMulticlass_{mode}")

    else:
        # BINARY training mode.

        # In binary mode, enforce check on input S, B hypotheses compatibility.
        binaryOpts = [
            (Belle2.Const.electron.getPDGCode(), Belle2.Const.pion.getPDGCode()),
            (Belle2.Const.muon.getPDGCode(), Belle2.Const.pion.getPDGCode()),
            (Belle2.Const.pion.getPDGCode(), Belle2.Const.kaon.getPDGCode()),
            (Belle2.Const.kaon.getPDGCode(), Belle2.Const.pion.getPDGCode()),
            (Belle2.Const.proton.getPDGCode(), Belle2.Const.pion.getPDGCode()),
            (Belle2.Const.deuteron.getPDGCode(), Belle2.Const.pion.getPDGCode())
        ]

        if binaryHypoPDGCodes not in binaryOpts:
            B2FATAL("No charged pid MVA was trained to separate ", binaryHypoPDGCodes[0], " vs. ", binaryHypoPDGCodes[1],
                    ". Please choose among the following pairs:\n",
                    "\n".join(f"{opt[0]} vs. {opt[1]}" for opt in binaryOpts))

        chargedpid = register_module("ChargedPidMVA")
        chargedpid.set_name(f"ChargedPidMVA_{binaryHypoPDGCodes[0]}_vs_{binaryHypoPDGCodes[1]}_{mode}")
        chargedpid.param("sigHypoPDGCode", binaryHypoPDGCodes[0])
        chargedpid.param("bkgHypoPDGCode", binaryHypoPDGCodes[1])

    chargedpid.param("particleLists", list(plSet))

    chargedpid.param("payloadName", payloadName)

    # Ensure the module knows whether we are using ECL-only training mode.
    if detector == "ECL":
        chargedpid.param("useECLOnlyTraining", True)

    path.add_module(chargedpid)


def calculateTrackIsolation(list_name, path, *detectors, use2DRhoPhiDist=False, alias=None):
    """
    Given a list of charged stable particles, compute variables that quantify "isolation" of the associated tracks.

    Currently, a proxy for isolation is defined as the 3D distance (or optionally, a 2D distance projecting on r-phi)
    of each particle's track to its closest neighbour at a given detector entry surface.

    Parameters:
        list_name (str): name of the input ParticleList.
                         It must be a list of charged stable particles as defined in ``Const::chargedStableSet``.
                         The charge-conjugate ParticleList will be also processed automatically.
        path (basf2.Path): the module is added to this path.
        use2DRhoPhiDist (Optional[bool]): if true, will calculate the pair-wise track distance
                                          as the cord length on the (rho, phi) projection.
                                          By default, a 3D distance is calculated.
        alias (Optional[str]): An alias to the extraInfo variable computed by the `TrackIsoCalculator` module.
                               Please note, for each input detector a variable is calculated,
                               and the detector's name is appended to the alias to distinguish them.
        *detectors: detectors at whose entry surface track isolation variables will be calculated.
                    Choose among: "CDC", "PID", "ECL", "KLM" (NB: 'PID' indicates TOP+ARICH entry surface.)

    """

    from variables import variables

    det_choices = ("CDC", "PID", "ECL", "KLM")
    if any(d not in det_choices for d in detectors):
        B2ERROR("Your input detector list: ", detectors, " contains an invalid choice. Please select among: ", det_choices)

    for det in detectors:
        path.add_module("TrackIsoCalculator",
                        particleList=list_name,
                        detectorInnerSurface=det,
                        use2DRhoPhiDist=use2DRhoPhiDist)
        if isinstance(alias, str):
            if not use2DRhoPhiDist:
                variables.addAlias(f"{alias}{det}", f"extraInfo(dist3DToClosestTrkAt{det}Surface)")
            else:
                variables.addAlias(f"{alias}{det}", f"extraInfo(dist2DRhoPhiToClosestTrkAt{det}Surface)")


def calculateDistance(list_name, decay_string, mode='vertextrack', path=None):
    """
    Calculates distance between two vertices, distance of closest approach between a vertex and a track,\
    distance of closest approach between a vertex and btube. For track, this calculation ignores track curvature,\
    it's negligible for small distances.The user should use extraInfo(CalculatedDistance)\
    to get it. A full example steering file is at analysis/tests/test_DistanceCalculator.py

    Example:
      >>> from modularAnalysis import calculateDistance
      >>>calculateDistance('list_name', 'decay_string', "mode", path=user_path)

    @param list_name              name of the input ParticleList
    @param decay_string           select particles between the distance of closest approch will be calculated
    @param mode                   Specifies how the distance is calculated
                                  vertextrack: calculate the distance of closest appreach between a track and a\
                                   vertex, taking the first candidate as vertex, default
                                  trackvertex: calculate the distance of closest appreach between a track and a\
                                   vertex, taking the first candidate as track
                                  2tracks: calculates the distance of closest appreach between two tracks
                                  2vertices: calculates the distance between two vertices
                                  vertexbtube: calculates the distance of closest appreach between a vertex and btube
                                  trackbtube: calculates the distance of closest appreach between a track and btube
    @param path                   modules are added to this path

    """

    dist_mod = register_module('DistanceCalculator')

    dist_mod.set_name('DistanceCalculator_' + list_name)
    dist_mod.param('listName', list_name)
    dist_mod.param('decayString', decay_string)
    dist_mod.param('mode', mode)
    path.add_module(dist_mod)


def addInclusiveDstarReconstruction(decayString, slowPionCut, DstarCut, path):
    """
    Adds the InclusiveDstarReconstruction module to the given path.
    This module creates a D* particle list by estimating the D* four momenta
    from slow pions, specified by a given cut. The D* energy is approximated
    as  E(D*) = m(D*)/(m(D*) - m(D)) * E(pi). The absolute value of the D*
    momentum is calculated using the D* PDG mass and the direction is collinear
    to the slow pion direction. The charge of the given pion list has to be consistent
    with the D* charge

    @param decayString Decay string, must be of form ``D* -> pi``
    @param slowPionCut Cut applied to the input pion list to identify slow pions
    @param DstarCut Cut applied to the output D* list
    @param path the module is added to this path
    """
    incl_dstar = register_module("InclusiveDstarReconstruction")
    incl_dstar.param("decayString", decayString)
    incl_dstar.param("slowPionCut", slowPionCut)
    incl_dstar.param("DstarCut", DstarCut)
    path.add_module(incl_dstar)


def scaleError(outputListName, inputListName,
               scaleFactors=[1.17, 1.12, 1.16, 1.15, 1.13],
               d0Resolution=[12.2e-4, 14.1e-4],
               z0Resolution=[13.4e-4, 15.3e-4],
               path=None):
    '''
    This module creates a new charged particle list.
    The helix errors of the new particles are scaled by constant factors.
    These scale factors are defined for each helix parameter (d0, phi0, omega, z0, tanlambda).
    The impact parameter resolution can be defined in a pseudo-momentum dependent form,
    which limits the d0 and z0 errors so that they do not shrink below the resolution.
    This module is supposed to be used for low-momentum (0-3 GeV/c) tracks in BBbar events.
    Details will be documented in a Belle II note by the Belle II Japan ICPV group.

    @param inputListName Name of input charged particle list to be scaled
    @param outputListName Name of output charged particle list with scaled error
    @param scaleFactors List of five constants to be multiplied to each of helix errors
    @param d0Resolution List of two parameters, (a [cm], b [cm/(GeV/c)]),
                        defining d0 resolution as sqrt{ a**2 + (b / (p*beta*sinTheta**1.5))**2 }
    @param z0Resolution List of two parameters, (a [cm], b [cm/(GeV/c)]),
                        defining z0 resolution as sqrt{ a**2 + (b / (p*beta*sinTheta**2.5))**2 }
    '''
    scale_error = register_module("HelixErrorScaler")
    scale_error.set_name('ScaleError_' + inputListName)
    scale_error.param('inputListName', inputListName)
    scale_error.param('outputListName', outputListName)
    scale_error.param('scaleFactors', scaleFactors)
    scale_error.param('d0ResolutionParameters', d0Resolution)
    scale_error.param('z0ResolutionParameters', z0Resolution)
    path.add_module(scale_error)


def correctEnergyBias(inputListNames, tableName, path=None):
    """
    Scale energy of the particles according to the scaling factor.
    If the particle list contains composite particles, the energy of the daughters are scaled.
    Subsequently, the energy of the mother particle is updated as well.

    Parameters:
        inputListNames (list(str)): input particle list names
        tableName : stored in localdb and created using ParticleWeightingLookUpCreator
        path (basf2.Path): module is added to this path
    """

    correctenergybias = register_module('EnergyBiasCorrection')
    correctenergybias.param('particleLists', inputListNames)
    correctenergybias.param('tableName', tableName)
    path.add_module(correctenergybias)


def getAnalysisGlobaltag():
    """
    Returns a string containing the name of the latest and recommended analysis globaltag.
    """
    # b2conditionsdb-recommend relies on a different repository, so it's better to protect
    # this function against potential failures of check_output.
    try:
        tags = subprocess.check_output(['b2conditionsdb-recommend', '--oneline'],
                                       timeout=60).decode('UTF-8').rstrip().split(' ')
        analysis_tag = ''
        for tag in tags:
            if tag.startswith('analysis_tools'):
                analysis_tag = tag
        return analysis_tag
    # In case of issues with git, b2conditionsdb-recommend may take too much time.
    except TimeoutExpired as te:
        B2FATAL(f'A {te} exception was raised during the call of getAnalysisGlobalTag(). '
                'The function took too much time to retrieve the requested information '
                'from the versioning repository.\n'
                'Plase try to re-run your job. In case of persistent failures, there may '
                'be issues with the DESY collaborative services, so please contact the experts.')
    except CalledProcessError as ce:
        B2FATAL(f'A {ce} exception was raised during the call of getAnalysisGlobalTag(). '
                'Please try to re-run your job. In case of persistent failures, please contact '
                'the experts.')


if __name__ == '__main__':
    from basf2.utils import pretty_print_module
    pretty_print_module(__name__, "modularAnalysis")
