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
import hashlib
import os
import subprocess
import tempfile
from b2test_utils import run_in_subprocess


def _run_herwig_batch(work_dir, template_path, lhe_path, seed, nevents,
                      tune_content, shower_mode, herwig_path='Herwig'):
    """Run Herwig for all N events in a single Herwig subprocess invocation.

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
        'full':    '# Full shower mode (default)',
    }[shower_mode]

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


def _stage1_worker(finalstate, work_dir, seed, nevents, stage1_exp, stage1_run,
                   eventType, shower_scale, beamparametersLabel):
    """Run KKMC + HerwigLHEWriter in a forked subprocess for Stage 1.

    Global tags and BeamParameters are inherited from the parent process unless
    beamparametersLabel is explicitly set, in which case add_beamparameters() overrides the DB.
    """
    b2.set_random_seed(seed)
    kkmc_input_files = {
        'uubar': 'data/generators/kkmc/uubar_nohadronization.input.dat',
        'ddbar': 'data/generators/kkmc/ddbar_nohadronization.input.dat',
        'ssbar': 'data/generators/kkmc/ssbar_nohadronization.input.dat',
        'ccbar': 'data/generators/kkmc/ccbar_nohadronization.input.dat',
    }
    path = b2.create_path()
    if beamparametersLabel is not None:
        from beamparameters import add_beamparameters
        add_beamparameters(path, beamparametersLabel)
    path.add_module('EventInfoSetter',
                    expList=stage1_exp, runList=stage1_run, evtNumList=[nevents])
    path.add_module('KKGenInput',
                    tauinputFile=b2.find_file(kkmc_input_files[finalstate]),
                    KKdefaultFile=b2.find_file('data/generators/kkmc/KK2f_defaults.dat'),
                    taudecaytableFile='',
                    kkmcoutputfilename=os.path.join(work_dir, f'kkmc_{finalstate}.txt'),
                    eventType=eventType)
    path.add_module('HerwigLHEWriter',
                    WorkDir=work_dir,
                    ShowerScale=shower_scale)
    b2.process(path)


def add_herwig_continuum_generator(path, finalstate, nevents=None, userdecfile='', *,
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

    Stage 1 (runs in a forked subprocess): KKMC generates N quark pairs,
    HerwigLHEWriterModule writes all_events.lhe + per-event KKMC sidecars + manifest.txt.

    Stage 2 (runs via subprocess): one 'Herwig read batch.in' call
    processes all N LHE events in one Herwig invocation and writes all_events.hepmc.

    Stage 3 (added to path, zero subprocess calls per event): HerwigHepMCFragmentationModule
    reads all_events.hepmc and sidecar files, reconstructs MCParticles + truth tree,
    and applies EvtGen. Supports multiprocessing with basf2 -p K.

    Parameters:
        path (basf2.Path): path for Stage 3 module
        finalstate (str): uubar, ddbar, ssbar, ccbar
        nevents (int or None): number of events for Stages 1+2.
            Default None: auto-derived from the CLI -n/--events flag via
            Belle2.Environment.Instance().getNumberEventsOverride().
            Pass an explicit integer for programmatic use (no CLI -n flag).
            B2FATAL if both are absent or zero.
        userdecfile (str): EvtGen user decfile; '' means no user decfile
        herwigParameterFile (str): Herwig tune .dat file; '' uses herwig_belle2.dat
        work_dir (str): working directory path; None = auto-created via mkdtemp
        seed (int or None): Seed for Stage 1 (KKMC) and Stage 2 (Herwig batch).
            Default None: automatically derived from the current basf2 random seed via
            SHA-256, so a single b2.set_random_seed() call
            at the top of the steering script seeds all three stages.
            Pass an explicit integer to override the auto-derived seed.
        expList (list[int]): experiment number for the Stage 1 EventInfoSetter.
            Default None -> [0]. The standard globaltag provides Y(4S) BeamParameters
            for exp=0/run=0, so the default gives Y(4S) generation without any
            additional setup. The expList must match the steering script's EventInfoSetter
            to resolve the beam parameters correctly.
        runList (list[int]): run number for the Stage 1 EventInfoSetter.
            Default None -> [0]. Same rule as expList; the runList must match the steering script's EventInfoSetter
            to resolve the beam parameters correctly.
        beamparametersLabel (str or None): hardcode a beam-energy preset in Stage 1
            (e.g. 'Y1S', 'Y4S', 'Y4S-off', 'Y5S') via add_beamparameters().
            Caveat: add_beamparameters() is considered deprecated,
            e.g., it places the IP at (0, 0, 0).
            Default None: KKGenInput reads the BeamParameters from the DB.
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
            Note: shower_scale >= sqrt(s) is recommended to ensure SCALUP is never
            capped below the hard-process scale; violating this truncates the parton
            shower for events where the quark pair mass exceeds shower_scale.
            However, shower_scale must not be set below sqrt(s)/2 as the LHE <init> block beam-energy
            would fall below the actual per-event beam energy, causing Herwig to reject events
            (empty HepMC file and a RuntimeError in Stage 2).
            Default 11.0 GeV satisfies up to Y(5S).
        skip_on_failure (bool): Default True, skip events where Herwig failed
        eventType (str): event type label, KKMC metadata
    """
    if nevents is None:
        from ROOT import Belle2 as _Belle2
        nevents = _Belle2.Environment.Instance().getNumberEventsOverride()
    if nevents <= 0:
        b2.B2FATAL(
            'add_herwig_continuum_generator: nevents must be > 0. '
            'Use -n N on the CLI or pass nevents=N explicitly.'
        )

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

    # Validate finalstate early so the error appears before the fork
    _kkmc_input_files = {
        'uubar': 'data/generators/kkmc/uubar_nohadronization.input.dat',
        'ddbar': 'data/generators/kkmc/ddbar_nohadronization.input.dat',
        'ssbar': 'data/generators/kkmc/ssbar_nohadronization.input.dat',
        'ccbar': 'data/generators/kkmc/ccbar_nohadronization.input.dat',
    }
    if finalstate not in _kkmc_input_files:
        b2.B2FATAL(f'add_herwig_continuum_generator: finalstate must be uubar/ddbar/ssbar/ccbar, got {finalstate}')

    # Resolve tune file and decay file
    herwig_config = b2.find_file('data/generators/modules/herwigfragmentation/herwig_belle2.dat')
    if herwigParameterFile:
        herwig_config = herwigParameterFile
    decay_file = b2.find_file('decfiles/dec/DECAY_BELLE2.DEC')
    decay_user = ''
    if userdecfile:
        b2.B2INFO(f'add_herwig_continuum_generator: using custom userdecfile {userdecfile}')
        decay_user = userdecfile

    # -----------------------------------------------------------------------
    # Stage 1: run KKMC + HerwigLHEWriter in a forked subprocess.
    # -----------------------------------------------------------------------
    b2.B2INFO(f'[Stage 1] Running KKMC to generate {nevents} {finalstate} quark pairs ...')

    stage1_exp = expList if expList is not None else [0]
    stage1_run = runList if runList is not None else [0]

    exitcode = run_in_subprocess(
        target=_stage1_worker,
        finalstate=finalstate,
        work_dir=work_dir,
        seed=seed,
        nevents=nevents,
        stage1_exp=stage1_exp,
        stage1_run=stage1_run,
        eventType=eventType,
        shower_scale=shower_scale,
        beamparametersLabel=beamparametersLabel,
    )
    if exitcode != 0:
        raise RuntimeError(
            f'[Stage 1] HerwigLHEWriter subprocess failed (exit {exitcode})')

    manifest_path = os.path.join(work_dir, 'manifest.txt')
    if not os.path.exists(manifest_path):
        raise RuntimeError(f'[Stage 1] manifest.txt not created in {work_dir}')
    b2.B2INFO('[Stage 1] Complete. manifest.txt written.')

    # -----------------------------------------------------------------------
    # Stage 2: single Herwig batch invocation (subprocess call for all N events)
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
