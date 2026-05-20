##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Herwig multistep pipeline API for basf2.

Provides add_herwig_continuum_generator() for e+e- -> qqbar continuum event generation
using a 3-stage batch architecture (KKMC + Herwig + EvtGen).

Example usage:
    from herwig import add_herwig_continuum_generator
    add_herwig_continuum_generator(path, finalstate='ccbar', nevents=5000)
"""

import basf2 as b2
import beamparameters
import hashlib
import os
import pdg
import subprocess
import tempfile

# Guard against repeated particle registration, set to True after the first call
_herwig_particles_registered = False


def _register_herwig_particles():
    """Register resonances absent from evt.pdl that Herwig's cluster model produces.
    Decay modes for all particles are in the default userdecfile missing_resonances.dec.
    """
    global _herwig_particles_registered
    if _herwig_particles_registered:
        return
    # Nucleon resonances J=1/2
    pdg.add_particle('N(1440)0', 12112, 1.440, 0.300, 0.0, 0.5, define_anti_particle=True)
    pdg.add_particle('N(1440)+', 12212, 1.440, 0.300, 1.0, 0.5, define_anti_particle=True)
    pdg.add_particle('N(1535)0', 22112, 1.535, 0.150, 0.0, 0.5, define_anti_particle=True)
    pdg.add_particle('N(1535)+', 22212, 1.535, 0.150, 1.0, 0.5, define_anti_particle=True)

    # K'+ / K'0 scalar kaon J=0 (PDG 100321/100311)
    pdg.add_particle("K'+", 100321, 1.460, 0.260, 1.0, 0.0, define_anti_particle=True)
    pdg.add_particle("K'0", 100311, 1.460, 0.260, 0.0, 0.0, define_anti_particle=True)

    # Sigma(1660) charged states
    pdg.add_particle('Sigma(1660)-', 13112, 1.660, 0.100, -1.0, 0.5, define_anti_particle=True)
    pdg.add_particle('Sigma(1660)+', 13222, 1.660, 0.100,  1.0, 0.5, define_anti_particle=True)

    # kappa / K0*(700) scalar kaon isodoublet J=0
    pdg.add_particle("kappa+",     9000321, 0.841, 0.618,  1.0, 0.0, define_anti_particle=True)
    pdg.add_particle("kappa0",     9000311, 0.841, 0.618,  0.0, 0.0, define_anti_particle=True)

    # eta'_2 = eta2(1870), self-conjugate J=2 meson
    pdg.add_particle("eta'_2",      10335,  1.842, 0.225,  0.0, 2.0)

    # chi_c2(2P) = chi_c2(3930), self-conjugate J=2 charmonium
    pdg.add_particle("chi_c2(2P)", 100445,  3.929, 0.029,  0.0, 2.0)

    # N(1520) J=3/2 nucleon resonances
    pdg.add_particle('N(1520)0',       1214, 1.520, 0.120,  0.0, 1.5, define_anti_particle=True)
    pdg.add_particle('N(1520)+',       2124, 1.520, 0.120,  1.0, 1.5, define_anti_particle=True)

    # Sigma(1670) charged states
    pdg.add_particle('Sigma(1670)-', 13114, 1.670, 0.060, -1.0, 1.5, define_anti_particle=True)
    pdg.add_particle('Sigma(1670)+', 13224, 1.670, 0.060,  1.0, 1.5, define_anti_particle=True)

    # Sigma(1750) charged states
    pdg.add_particle('Sigma(1750)-', 23112, 1.750, 0.090, -1.0, 0.5, define_anti_particle=True)
    pdg.add_particle('Sigma(1750)+', 23222, 1.750, 0.090,  1.0, 0.5, define_anti_particle=True)

    # Xi(1690) S=-2 cascades J=1/2
    pdg.add_particle('Xi(1690)-',   13312, 1.690, 0.010, -1.0, 0.5, define_anti_particle=True)
    pdg.add_particle('Xi(1690)0',   13322, 1.690, 0.010,  0.0, 0.5, define_anti_particle=True)

    # Xi(1820) S=-2 cascades J=3/2
    pdg.add_particle('Xi(1820)-',   13314, 1.823, 0.024, -1.0, 1.5, define_anti_particle=True)
    pdg.add_particle('Xi(1820)0',   13324, 1.823, 0.024,  0.0, 1.5, define_anti_particle=True)

    # Xi(1950) S=-2 cascades J=1/2
    pdg.add_particle('Xi(1950)-',   23312, 1.959, 0.060, -1.0, 0.5, define_anti_particle=True)
    pdg.add_particle('Xi(1950)0',   23322, 1.959, 0.060,  0.0, 0.5, define_anti_particle=True)

    # Xi_c(2790) J=1/2 charmed-strange baryons
    pdg.add_particle('Xi_c(2790)0', 14312, 2.792, 0.00113, 0.0, 0.5, define_anti_particle=True)
    pdg.add_particle('Xi_c(2790)+', 14322, 2.789, 0.00113, 1.0, 0.5, define_anti_particle=True)

    # Xi_c(2815) J=3/2 charmed-strange baryons
    pdg.add_particle('Xi_c(2815)0', 14314, 2.818, 0.00126, 0.0, 1.5, define_anti_particle=True)
    pdg.add_particle('Xi_c(2815)+', 14324, 2.817, 0.00126, 1.0, 1.5, define_anti_particle=True)

    _herwig_particles_registered = True


def _run_herwig_batch(work_dir, template_path, lhe_path, seed, nevents,
                      tune_content, shower_mode, herwig_path='Herwig'):
    """Run Herwig for all N events in a single subprocess call (1 fork for the entire batch).

    Reads herwig_fragmentation.in, substitutes template variables, and writes batch.in.
    """
    runname = 'HerwigFrag_batch'
    hepmc_file = os.path.join(work_dir, 'all_events.hepmc')
    cache_file = os.path.join(work_dir, 'all_events.lhe.cache')
    in_file = os.path.join(work_dir, 'batch.in')

    with open(template_path) as f:
        content = f.read()

    shower_config = {
        'off':     'set LHEHandler:CascadeHandler NULL',
        'minimal': 'set ShowerHandler:MaxTry 1',
    }.get(shower_mode, '# Full shower mode (default)')

    content = content.replace('{lhe_file}',      lhe_path)
    content = content.replace('{cache_file}',    cache_file)
    content = content.replace('{seed}',          str(seed))
    content = content.replace('{nevents}',       str(nevents))
    content = content.replace('{print_event}',   '0')
    content = content.replace('{output_file}',   hepmc_file)
    content = content.replace('{runname}',       runname)
    content = content.replace('{shower_config}', shower_config)
    content = content.replace('{tune_settings}', tune_content)

    with open(in_file, 'w') as f:
        f.write(content)

    log_file = os.path.join(work_dir, 'herwig_batch.log')
    with open(log_file, 'w') as log:
        subprocess.run([herwig_path, 'read', in_file],
                       cwd=work_dir, stdout=log, stderr=log)

    # Non-zero exit may still produce valid HepMC after Herwig exhausts LHE input.
    if not os.path.exists(hepmc_file) or os.path.getsize(hepmc_file) == 0:
        raise RuntimeError(
            f'Herwig batch run produced no HepMC output. See {log_file}')

    return hepmc_file


def _read_batch_seed(work_dir):
    """Read the batch seed from manifest.txt written by HerwigLHEWriterModule."""
    manifest_path = os.path.join(work_dir, 'manifest.txt')
    with open(manifest_path) as f:
        for line in f:
            line = line.strip()
            if line.startswith('batch_seed '):
                return int(line.split()[1])
    raise RuntimeError(f'batch_seed not found in {manifest_path}')


def add_herwig_continuum_generator(path, finalstate, nevents, userdecfile='', *,
                                   herwigParameterFile='',
                                   work_dir=None,
                                   seed=None,
                                   expList=None,
                                   runList=None,
                                   beamparametersLabel=None,
                                   keep_temp_files=None,
                                   shower_scale=11.0,
                                   skip_on_failure=True,
                                   eventType=''):
    """
    Three-stage Herwig continuum generator (multistep pipeline).

    Stage 1 (runs via subprocess): KKMC generates N quark pairs,
    HerwigLHEWriterModule writes all_events.lhe + per-event KKMC sidecars + manifest.txt.

    Stage 2 (runs via subprocess): one 'Herwig read batch.in' call
    processes all N LHE events in one Herwig invocation and writes all_events.hepmc.

    Stage 3 (added to path, zero subprocess calls per event): HerwigHepMCFragmentationModule
    reads all_events.hepmc and sidecar files, reconstructs MCParticles + truth tree,
    and applies EvtGen + PHOTOS. Supports multiprocessing with basf2 -p K.

    Parameters:
        path (basf2.Path): path for Stage 3 module
        finalstate (str): uubar, ddbar, ssbar, ccbar
        nevents (int): number of events - required (Stage 2 must know N before basf2 starts)
        userdecfile (str): EvtGen user decfile; '' uses missing_resonances.dec
        herwigParameterFile (str): Herwig tune .dat file; '' uses herwig_belle2.dat
        work_dir (str): working directory path; None = auto-created via mkdtemp
        seed (int or None): Seed for Stage 1 (KKMC) and Stage 2 (Herwig batch).
            Default None: automatically derived from the current basf2 random seed via
            SHA-256, so a single b2.set_random_seed() call
            at the top of the steering script seeds all three stages.
            Pass an explicit integer to override the auto-derived seed.
        expList (list[int]): experiment numbers for the Stage 1 EventInfoSetter.
            Default None -> [0].
            Regarding MCrd: expList must be set to the same value as the outer script's
            EventInfoSetter (e.g. expList=[12]). Stage 1 runs in an isolated
            subprocess and queries the conditions DB using these numbers to resolve
            BeamParameters. If left at the default [0] while beamparametersLabel=None,
            Stage 1 will query for experiment 0, find the standard Y4S payload, and
            load Y4S beam parameters.
        runList (list[int]): run numbers for the Stage 1 EventInfoSetter.
            Default None -> [1]. Same MCrd rule as expList: should match the outer
            EventInfoSetter so the conditions DB resolves the correct per-run
            BeamParameters payload.
        beamparametersLabel (str or None): controls the beam energy given to KKMC
            in the Stage 1 subprocess. Stage 1 runs in an isolated subprocess before
            b2.process() and cannot inherit beam parameters from the user's path,
            so this parameter is needed (unlike add_continuum_generator()).
            Default None: probes the conditions DB for expList[0]/runList[0].
            On success, Stage 1 loads the full
            per-run BeamParameters payload directly from the DB.
            If the DB has no payload for that experiment/run, it falls back to
            add_beamparameters(path, 'Y4S') with a B2WARNING.
            One can pass an explicit preset name ('Y4S', 'Y4S-off', 'Y5S', etc.) to bypass
            the DB query and hardcode the beam energy.
        keep_temp_files (bool or None): controls whether WorkDir and all pipeline
            files (LHE, HepMC, sidecars, manifest, Herwig log) are deleted after
            the run completes. Deletion is performed by HerwigHepMCFragmentationModule
            in terminate(), after all events are processed and sidecars are no longer
            needed.
            Default None (auto-derive):
              work_dir=None (auto mkdtemp)  -> keep_temp_files resolves to False
                (temp dir is created for this run only; deleted when done).
              work_dir='<explicit path>'    -> keep_temp_files resolves to True
                (set keep_temp_files to False explicitly to override).
        shower_scale (float): parton shower scale ceiling in GeV, written as
            SCALUP in each LHE <event> block and as the nominal beam energy in
            the LHE <init> block. Per-event SCALUP = min(shower_scale, sqrt(M2)),
            where M2 is the Lorentz-invariant mass squared of the quark pair.
            Two constraints must both be satisfied:
            (1) shower_scale >= sqrt(s)/2  - the <init> block beam energy must
                not be lower than the per-event beam energy (sE/2); violating
                this causes Herwig to reject events where the quark pair energy
                exceeds the declared beam maximum, producing an empty HepMC file
                and a RuntimeError in Stage 2.
            (2) shower_scale >= sqrt(s)    - ensures SCALUP is never capped below
                the hard-process scale; violating this truncates the parton shower
                for events where the quark pair mass exceeds shower_scale.
            Default 11.0 GeV satisfies up to Y(5S).
        skip_on_failure (bool): if True, skip events where Herwig failed
        eventType (str): event type label, KKMC metadata
    """
    if nevents <= 0:
        b2.B2FATAL(f'add_herwig_continuum_generator: nevents must be > 0, got {nevents}')

    # Register Category 1 particles before any b2.process()
    _register_herwig_particles()

    # Derive Stage 1+2 seed from the current basf2 seed when not explicitly provided.
    # basf2.get_random_seed() returns the seed string set by the
    # user's b2.set_random_seed() call; SHA-256 maps it to an integer in Herwig's
    # run. Different gbasf2 jobs have different basf2 seeds and
    # therefore get different Herwig seeds automatically.
    if seed is None:
        seed = int(hashlib.sha256(b2.get_random_seed().encode()).hexdigest(), 16) % 2000000000 + 1

    # Set up working directory; record whether the user specified it explicitly
    # (needed to derive the keep_temp_files default before work_dir is overwritten)
    user_specified_work_dir = work_dir is not None
    if work_dir is None:
        work_dir = tempfile.mkdtemp(prefix='herwig_pipeline_')
    else:
        os.makedirs(work_dir, exist_ok=True)

    # Resolve keep_temp_files: None means "auto" (files kept if user chose the path,
    # deleted if the directory was auto-created as a one-job temp dir).
    if keep_temp_files is None:
        keep_temp_files = user_specified_work_dir

    b2.B2INFO(f'add_herwig_continuum_generator: work_dir = {work_dir}')

    # Resolve KKMC input file
    kkmc_files = {
        'uubar': 'data/generators/kkmc/uubar_nohadronization.input.dat',
        'ddbar': 'data/generators/kkmc/ddbar_nohadronization.input.dat',
        'ssbar': 'data/generators/kkmc/ssbar_nohadronization.input.dat',
        'ccbar': 'data/generators/kkmc/ccbar_nohadronization.input.dat',
    }
    if finalstate not in kkmc_files:
        b2.B2FATAL(f'add_herwig_continuum_generator: finalstate must be uubar/ddbar/ssbar/ccbar, got {finalstate}')
    kkmc_inputfile = b2.find_file(kkmc_files[finalstate])
    kkmc_config = b2.find_file('data/generators/kkmc/KK2f_defaults.dat')
    kkmc_logfile = os.path.join(work_dir, f'kkmc_{finalstate}.txt')

    # Resolve tune file and decay file
    herwig_config = b2.find_file('data/generators/modules/herwigfragmentation/herwig_belle2.dat')
    if herwigParameterFile:
        herwig_config = herwigParameterFile
    decay_file = b2.find_file('decfiles/dec/DECAY_BELLE2.DEC')
    decay_user = b2.find_file(
        'data/generators/modules/herwigfragmentation/missing_resonances.dec')
    if userdecfile:
        b2.B2INFO(f'add_herwig_continuum_generator: using custom userdecfile {userdecfile}')
        decay_user = userdecfile

    # -----------------------------------------------------------------------
    # Stage 1: run KKMC + HerwigLHEWriterModule via a subprocess
    # -----------------------------------------------------------------------
    b2.B2INFO(f'[Stage 1] Running KKMC to generate {nevents} {finalstate} quark pairs ...')

    # Resolve Stage 1 exp/run (pure metadata - no physics impact)
    stage1_exp = expList if expList is not None else [0]
    stage1_run = runList if runList is not None else [1]
    # Propagate the outer script's global tags to the Stage 1 subprocess
    stage1_tags = repr(list(b2.conditions.globaltags))

    if beamparametersLabel is not None:
        bp_snippet = (
            f"from beamparameters import add_beamparameters\n"
            f"add_beamparameters(path, '{beamparametersLabel}')"
        )
    else:
        # Default (beamparametersLabel=None): mirrors whizard.py pattern.
        # Probe the DB to confirm a BeamParameters payload exists for this
        # exp/run. If yes, emit a comment so Stage 1 loads the full per-run
        # payload from the DB directly.
        # If no payload is found (local/run-independent, no MCrd global tags),
        # fall back to Y4S preset with a B2WARNING.
        try:
            cm_energy = beamparameters.get_collisions_invariant_mass(
                stage1_exp[0], stage1_run[0])
            b2.B2INFO(
                f'add_herwig_continuum_generator: beamparametersLabel=None - '
                f'DB BeamParameters confirmed for exp {stage1_exp[0]}/'
                f'run {stage1_run[0]} (sqrt(s) = {cm_energy:.6f} GeV). '
                f'Stage 1 will load from DB.'
            )
            bp_snippet = ""  # Stage 1 loads BeamParameters from DB implicitly
        except Exception:
            b2.B2WARNING(
                f'add_herwig_continuum_generator: beamparametersLabel=None but no '
                f'BeamParameters found in DB for exp {stage1_exp[0]}/'
                f'run {stage1_run[0]}. Falling back to Y4S preset. '
            )
            bp_snippet = (
                "from beamparameters import add_beamparameters\n"
                "add_beamparameters(path, 'Y4S')"
            )

    stage1_script = f"""
import basf2 as b2
b2.set_random_seed({seed})
b2.conditions.globaltags = {stage1_tags}
path = b2.create_path()
{bp_snippet}
path.add_module('EventInfoSetter', expList={repr(stage1_exp)}, runList={repr(stage1_run)}, evtNumList=[{nevents}])
path.add_module('KKGenInput',
    tauinputFile={repr(kkmc_inputfile)},
    KKdefaultFile={repr(kkmc_config)},
    taudecaytableFile='',
    kkmcoutputfilename={repr(kkmc_logfile)},
    eventType='{eventType}',
)
path.add_module('HerwigLHEWriter',
    WorkDir={repr(work_dir)},
    ShowerScale={shower_scale},
)
b2.process(path)
"""
    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
        f.write(stage1_script)
        stage1_script_path = f.name

    try:
        result = subprocess.run(['basf2', stage1_script_path], check=False)
        if result.returncode != 0:
            raise RuntimeError(
                f'[Stage 1] HerwigLHEWriter basf2 subprocess failed (exit {result.returncode})')
    finally:
        os.unlink(stage1_script_path)

    manifest_path = os.path.join(work_dir, 'manifest.txt')
    if not os.path.exists(manifest_path):
        raise RuntimeError(f'[Stage 1] manifest.txt not created in {work_dir}')
    b2.B2INFO('[Stage 1] Complete. manifest.txt written.')

    # -----------------------------------------------------------------------
    # Stage 2: single Herwig batch invocation (1 fork for all N events)
    # -----------------------------------------------------------------------
    b2.B2INFO(f'[Stage 2] Running Herwig batch read for {nevents} events ...')

    template = b2.find_file(
        'data/generators/modules/herwigfragmentation/herwig_fragmentation.in')
    batch_seed = _read_batch_seed(work_dir)
    with open(herwig_config) as f:
        tune_content = f.read()
    lhe_path = os.path.join(work_dir, 'all_events.lhe')

    _run_herwig_batch(work_dir, template, lhe_path,
                      seed=batch_seed, nevents=nevents,
                      tune_content=tune_content, shower_mode='full')

    hepmc_path = os.path.join(work_dir, 'all_events.hepmc')
    b2.B2INFO(f'[Stage 2] Complete. HepMC written to {hepmc_path}')

    # -----------------------------------------------------------------------
    # Stage 3: add HerwigHepMCFragmentationModule to the user's path
    # -----------------------------------------------------------------------
    fragmentation = path.add_module(
        'HerwigHepMCFragmentation',
        WorkDir=work_dir,
        UseEvtGen=True,
        DecFile=decay_file,
        UserDecFile=decay_user,
        CoherentMixing=True,
        KeepTempFiles=keep_temp_files,
    )
    if skip_on_failure:
        empty = b2.create_path()
        fragmentation.if_value('<1', empty)
