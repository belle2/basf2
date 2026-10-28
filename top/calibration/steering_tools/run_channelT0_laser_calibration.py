#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# ---------------------------------------------------------------------------------------
# CAF calibration script: channel T0 with laser
# data type: local runs with laser
#
# usage: basf2 run_channelT0_laser_calibration.py EXP RUN_LOW RUN_HIGH -- [options]
# ---------------------------------------------------------------------------------------

import os
import sys
import glob
import argparse
import yaml
import basf2
from caf import backends
from caf.framework import Calibration, CAF
from caf.strategies import SequentialRunByRun, SingleIOV, SimpleRunByRun
from ROOT.Belle2 import TOP
from basf2 import B2ERROR, B2INFO, B2WARNING

# ------------------------ CLI ------------------------


def parse_args():
    """ Parse command-line arguments."""

    p = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    # Positional arguments
    p.add_argument('exp', type=int, help='Experiment number (int)')
    p.add_argument('run_low', type=int, help='First run (inclusive)')
    p.add_argument('run_high', type=int, help='Last run (inclusive)')

    # Optional arguments
    p.add_argument('--source-dir', default='/ghi/fs01/belle2/bdata/group/detector/TOP/current_data_sroot_global',
                   help='Directory containing top.<exp>.<run>.*root files')
    p.add_argument('--localDBs', type=str,
                   default='/group/belle2/group/detector/TOP/calibration/TBC/official/TBC_2025c_Nov25/localDB/localDB.txt',
                   help='Whitespace-separated list of localDB files (priority > global tags)')
    p.add_argument('--global-tags', type=str, default='online',
                   help='Comma-separated central DB global tags, highest priority first')
    p.add_argument('--main-output-dir', default='./top_calibration',
                   help='Base output directory')
    p.add_argument('--ref-channel', default=0, type=int, help='Reference channel (pixel)')
    p.add_argument('--ref-slot', default=9, type=int, help='Reference slot')
    p.add_argument('--fit-mode', choices=['calibration', 'monitoring', 'mc'],
                   default='calibration', help='Fitter mode')
    p.add_argument('--strategy', choices=['SingleIOV', 'SequentialRunByRun', 'SimpleRunByRun'],
                   default='SingleIOV', help='CAF merge strategy')
    p.add_argument('--lookback', type=int, default=28, help='lookBackWindows for RawDigitConverter (0 uses DB default)')

    # Time-base calibration toggle (enabled by default)
    tb = p.add_mutually_exclusive_group()
    tb.add_argument('--tbc', dest='use_tbc', action='store_true', help='Enable TBC in RawDigitConverter')
    tb.add_argument('--no-tbc', dest='use_tbc', action='store_false', help='Disable TBC in RawDigitConverter')
    p.set_defaults(use_tbc=True)

    # Additional fitter options: amplitude bins, crosstalk detection
    p.add_argument('--amplitude-bins', action='store_true', help='Fit in amplitude bins')
    p.add_argument('--detect-crosstalk', action='store_true', help='Enable crosstalk detection')

    # Input data type (sroot/root)
    srt = p.add_mutually_exclusive_group()
    srt.add_argument('--sroot', action='store_true', help='Force SeqRootInput')
    srt.add_argument('--root', dest='sroot', action='store_false', help='Force RootInput (default: auto)')
    p.set_defaults(sroot=None)  # auto-detect

    # Files for fitter
    p.add_argument('--tts-file', default='/group/belle2/group/detector/TOP/calibration/MCreferences/TTSParametrizations.root',
                   help='TTS parametrization ROOT file')
    p.add_argument('--laser-mc-fit', default='/group/belle2/group/detector/TOP/calibration/MCreferences/laserMCFit.root',
                   help='Laser MC fit constraints ROOT file')

    # Pulser DeltaT
    p.add_argument('--pulser-deltaT', type=float, default=None,
                   help='Override pulser DeltaT (ns). Default auto: 21.78 (exp<14), 31.86 (exp>=14)')

    # Backend knobs
    p.add_argument('--queue', default='l', help='LSF queue name')
    p.add_argument('--max-files-per-job', type=int, default=1, help='max_files_per_collector_job')
    p.add_argument('--max-collector-jobs', type=int, default=None, help='cap number of collector jobs')

    # Misc
    p.add_argument('--calname', default='TOP_channelT0', help='Calibration name (conditions object)')
    p.add_argument('--dry-run', action='store_true', help='List files and exit without running CAF')
    p.add_argument('--log-level', choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'FATAL'], default='INFO',
                   help='basf2 log level')

    return p.parse_args()

# -------------------- Utilities ----------------------


def exp_run_strings(exp, run_low, run_high):
    """ Generate list of formatted run strings for given exp and run range."""
    exp_str = f'{exp:04d}'
    runs = [f'{exp_str}.{r:05d}' for r in range(run_low, run_high + 1)]
    return runs


def discover_input_files(source_dir, runs):
    """ Discover (s)root files in source_dir matching exp and runs."""
    patterns = [os.path.join(source_dir, f'top.{r}.*root') for r in runs]
    files = []
    for pat in patterns:
        hits = glob.glob(pat)
        files.extend(sorted(hits))
    return files


def auto_pulser_deltaT(exp, override):
    """ Auto-select pulser DeltaT based on exp, unless overridden."""
    if override is not None:
        return override
    return 21.78 if exp < 14 else 31.86


def auto_detect_sroot(files, forced):
    """ Auto-detect whether to use SeqRootInput (sroot) or RootInput (root) based on filenames, unless forced."""
    if forced is not None:
        return forced
    # Heuristic: if filenames contain ".sroot" anywhere, use SeqRootInput
    return any('.sroot' in f for f in files)


def strategy_from_name(name):
    """ Map strategy name to CAF strategy class."""
    return {'SingleIOV': SingleIOV, 'SequentialRunByRun': SequentialRunByRun, 'SimpleRunByRun': SimpleRunByRun}[name]


def ensure_file(path, label):
    """ Ensure that a file exists, else exit with error."""
    if not os.path.isfile(path):
        B2ERROR(f'{label} not found: {path}')
        sys.exit(1)


def set_log_level(level_str):
    """ Set basf2 log level."""
    level = getattr(basf2.LogLevel, level_str)
    basf2.set_log_level(level)

# --------------------- Calibration -------------------


def make_calibration(args, input_files, pulser_deltaT, use_seq_input):
    """ Build and return the channel T0 calibration object."""

    # Build the pre-collector path
    main = basf2.create_path()

    # Input
    if use_seq_input:
        main.add_module('SeqRootInput')
    else:
        main.add_module('RootInput')

    # Geometry (not needed for MC mode)
    if args.fit_mode != 'mc':
        main.add_module('TOPGeometryParInitializer')

        # TOPUnpacker
        main.add_module('TOPUnpacker')

        # TOPRawDigitConverter
        main.add_module(
            'TOPRawDigitConverter',
            useChannelT0Calibration=False,
            useSampleTimeCalibration=args.use_tbc,
            useAsicShiftCalibration=True,
            useModuleT0Calibration=False,
            useCommonT0Calibration=False,
            calpulseHeightMin=200,
            calpulseHeightMax=700,
            calpulseWidthMin=0.5,
            calpulseWidthMax=3.0,
            minPulseWidth=0.5,
            calibrationChannel=0,
            lookBackWindows=int(args.lookback)
        )

    # Collector
    collector = basf2.register_module('TOPLaserCalibratorCollector')
    collector.param('useReferencePulse', True)
    collector.param('storeMCTruth', args.fit_mode == 'mc')
    collector.param('refChannel', int(args.ref_channel))
    collector.param('refSlot', int(args.ref_slot))
    collector.param('pulserDeltaT', float(pulser_deltaT))
    # Optionally: collector.param('pulserDeltaTTolerance', 50)

    # CAF algorithm
    algo = TOP.TOPLocalCalFitter()
    algo.setFitMode(args.fit_mode)
    algo.setTTSFileName(args.tts_file)
    algo.setFitConstraintsFileName(args.laser_mc_fit)
    # NOTE: fitInAmplitudeBins method name has a typo in older basf2 versions; the correct one is:
    try:
        algo.fitInAmplitudeBins(bool(args.amplitude_bins))
    except AttributeError:
        # Fallback for older basf2 versions; keep original to be safe
        algo.fitInAmpliduteBins(bool(args.amplitude_bins))

    # Crosstalk detection algorithm
    if hasattr(algo, 'crosstalkDetection'):
        algo.crosstalkDetection(bool(args.detect_crosstalk))
    elif args.detect_crosstalk:
        B2WARNING("Crosstalk detection requested (--detect-crosstalk) "
                  "but this TOPLocalCalFitter has no 'crosstalkDetection' method. Ignoring.")

    # Calibration object
    cal = Calibration(name=args.calname, collector=collector, algorithms=algo, input_files=input_files)

    # Database priorities: local before global
    cal.reset_database()
    # Central DB global tags (highest priority first)
    global_tags = [g.strip() for g in args.global_tags.split(',') if g.strip()]
    for gt in reversed(global_tags):
        cal.use_central_database(gt)

    # Local DB files (highest priority first)
    local_dbs = args.localDBs.split()
    for ldb in reversed(local_dbs):
        cal.use_local_database(ldb)

    cal.pre_collector_path = main
    cal.max_files_per_collector_job = int(args.max_files_per_job)
    if args.max_collector_jobs is not None:
        cal.max_collector_jobs = int(args.max_collector_jobs)

    cal.strategies = strategy_from_name(args.strategy)
    return cal, global_tags, local_dbs

# ------------------------ Main -----------------------


def main():
    args = parse_args()
    set_log_level(args.log_level)

    # basf2 conditions: prepend highest-priority global tag for non-CAF modules (kept minimal)
    basf2.conditions.prepend_globaltag('online')  # harmless even if overridden in CAF

    # Validate ranges
    if args.run_high < args.run_low:
        B2ERROR(f'run_high < run_low: {args.run_high} < {args.run_low}')
        sys.exit(1)
    if args.lookback < 0:
        B2ERROR(f'lookback must be >= 0, got {args.lookback}')
        sys.exit(1)

    # Discover input
    runs = exp_run_strings(args.exp, args.run_low, args.run_high)
    input_files = discover_input_files(args.source_dir, runs)
    if not input_files:
        B2ERROR(f'No (s)root files found in {args.source_dir} for exp={args.exp} runs={args.run_low}-{args.run_high}')
        sys.exit(1)

    # Auto-detect reader flavor unless user forced it
    use_seq_input = auto_detect_sroot(input_files, args.sroot)

    # External files
    ensure_file(args.tts_file, 'TTS file')
    ensure_file(args.laser_mc_fit, 'Laser MC fit file')

    # Pulser DeltaT
    pulser_deltaT = auto_pulser_deltaT(args.exp, args.pulser_deltaT)

    # Output dirs
    expNo = f"e{args.exp:04d}"
    run_range = f"r{args.run_low:05d}-{args.run_high:05d}"
    output_dir = os.path.join(args.main_output_dir, f"channelT0-local-{expNo}-{run_range}")
    os.makedirs(output_dir, exist_ok=True)

    # Summary logging
    B2INFO(f"Found {len(input_files)} input files (showing up to 5):")
    for f in input_files[:5]:
        B2INFO(f"  {f}")
    if len(input_files) > 5:
        B2INFO("  ...")

    B2INFO(f"Reader: {'SeqRootInput' if use_seq_input else 'RootInput'} | fit-mode: {args.fit_mode} | strategy: {args.strategy}")
    B2INFO(f"TBC: {'ON' if args.use_tbc else 'OFF'} | amp-bins: {args.amplitude_bins} | crosstalk: {args.detect_crosstalk}")
    B2INFO(f"Ref slot/channel: {args.ref_slot}/{args.ref_channel} | pulser DeltaT: {pulser_deltaT:.2f} ns")
    B2INFO(f"Queue: {args.queue} | max_files_per_job: {args.max_files_per_job} | max_collector_jobs: {args.max_collector_jobs}")
    B2INFO(f"Output dir: {output_dir}")

    # Build calibration
    cal, global_tags, local_dbs = make_calibration(args, input_files, pulser_deltaT, use_seq_input)
    cal.backend_args = {"queue": args.queue}

    # Persist setup to YAML
    setup_dict = {
        'Experiment': int(args.exp),
        'Run low': int(args.run_low),
        'Run high': int(args.run_high),
        'Globaltags': global_tags,
        'Local databases': local_dbs,
        'Source directory': os.path.abspath(args.source_dir),
        'Output directory (base)': os.path.abspath(args.main_output_dir),
        'Resolved output dir': os.path.abspath(output_dir),
        'Lookback windows': int(args.lookback),
        'TTS file': os.path.abspath(args.tts_file),
        'MC fit file': os.path.abspath(args.laser_mc_fit),
        'Fit mode': args.fit_mode,
        'Input files (count)': len(input_files),
        'TBC enabled': bool(args.use_tbc),
        'Reference slot': int(args.ref_slot),
        'Reference channel': int(args.ref_channel),
        'Pulser Delta T (ns)': float(pulser_deltaT),
        'Amplitude bins': bool(args.amplitude_bins),
        'Detect crosstalk': bool(args.detect_crosstalk),
        'Reader': 'SeqRootInput' if use_seq_input else 'RootInput',
        'Strategy': args.strategy,
        'Queue': args.queue,
        'max_files_per_collector_job': int(args.max_files_per_job),
        'max_collector_jobs': None if args.max_collector_jobs is None else int(args.max_collector_jobs),
        'Command': ' '.join(sys.argv),
        'Env': {k: v for k, v in os.environ.items() if k.startswith(('BELLE2_', 'BASF2_', 'TOP_', 'PG', 'USE_'))}
    }
    with open(os.path.join(output_dir, 'setup.yml'), 'w') as f:
        yaml.dump(setup_dict, f, sort_keys=False)
    B2INFO(f"Written setup config: {os.path.join(output_dir, 'setup.yml')}")

    if args.dry_run:
        B2INFO("Dry-run requested; exiting before CAF execution.")
        return

    # CAF wiring
    cal_fw = CAF()
    cal_fw.add_calibration(cal)
    cal_fw.output_dir = output_dir
    cal_fw.backend = backends.LSF()

    # Run!
    cal_fw.run()


if __name__ == '__main__':
    main()
