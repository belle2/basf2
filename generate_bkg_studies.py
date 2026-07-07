#!/usr/bin/env python3

import shlex
import shutil
import subprocess
import sys
from pathlib import Path
import argparse

##########################################################################
#
##########################################################################

REPO_ROOT = Path(__file__).resolve().parent

MAKE_BGO_SCRIPT = REPO_ROOT / "background" / "steering_tools" / "makeBGOverlayMCFileUp.py"
GEN_SCRIPT = REPO_ROOT / "ecl" / "examples" / "generate_single_gammaUp.py"

BASF2_EXE = "basf2"
PYTHON_EXE = sys.executable

SCENARIOS = ["S1", "S2", "S3"]

ENERGIES_GEV = ["0.1", "0.2", "0.3", "0.4", "0.5"]

OVERLAY_EVENTS = 100
PHOTON_EVENTS = 1000

RUN_UPGRADE = True
RUN_CURRENT = True

RUN_WITH_NOISE = False
RUN_WITHOUT_NOISE = True

DRY_RUN = False

parser = argparse.ArgumentParser()
parser.add_argument(
    "-phase",
    default=3,
    type=int,
    help="Experiment phase for the background simulation. ",
    choices=["2", "3", "31"],
)
parser.add_argument(
    "-scenario",
    type=str,
    default=None,
    help=(
        "Background scenario to be considered. "
        f"If not passed, run over {', '.join(SCENARIOS)}. "
    ),
)
parser.add_argument(
    "-enrg",
    default=None,
    type=float,
    help=(
        "Generated photon energy for particle gun (in GeV). "
        f"If not passed, scan over {', '.join(ENERGIES_GEV)}. "
    ),
)
parser.add_argument(
    "-nBgo",
    default=OVERLAY_EVENTS,
    type=int,
    help=(
        "Number of background overlay events "
        "to be generated from background mixing. "
    ),
)
parser.add_argument(
    "-nEvt",
    default=PHOTON_EVENTS,
    type=int,
    help="Number of events to be generated for particle gun",
)
parser.add_argument(
    "--runUpgrade",
    default=RUN_UPGRADE,
    action="store_true",
    help="Simulate reconstruction with ECL upgraded electronics. ",
)
parser.add_argument(
    "--runCurrent",
    default=RUN_CURRENT,
    action="store_true",
    help="Simulate reconstruction with current ECL electronics. ",
)
parser.add_argument(
    '--runNoise',
    default=RUN_WITH_NOISE,
    action='store_true',
    help=(
        "Run with enabled electronic noise in the ECL digitizer "
        "when simulating the waveform digitization. "
    ),
)
parser.add_argument(
    '--runNoNoise',
    default=RUN_WITHOUT_NOISE,
    action='store_true',
    help=(
        "Run with disabled electronic noise in the ECL digitizer "
        "when simulating the waveform digitization. "
    ),
)
parser.add_argument(
    '--dryRun',
    default=DRY_RUN,
    action='store_true',
    help="Dry run for testing submission. ",
)

args = parser.parse_args()

PHASE = f"phase{args.phase}"

if args.scenario:
    SCENARIOS = [args.scenario]

if args.enrg:
    ENERGIES_GEV = [args.enrg]

OVERLAY_EVENTS = args.nBgo
PHOTON_EVENTS = args.nEvt

RUN_UPGRADE = args.runUpgrade
RUN_CURRENT = args.runCurrent

RUN_WITH_NOISE = args.runNoise
RUN_WITHOUT_NOISE = args.runNoNoise

RESULTS_DIR = REPO_ROOT / "scan_outputs"

OVERWRITE = False
DRY_RUN = args.dryRun

# ============================================================
# Utility
# ============================================================


def run(cmd, cwd):
    cmd_str = " ".join(shlex.quote(str(x)) for x in cmd)
    print(f"\n[RUN] {cmd_str}")
    print(f"[CWD] {cwd}")

    if DRY_RUN:
        return

    subprocess.run([str(x) for x in cmd], cwd=str(cwd), check=True)


def ensure_environment():
    if shutil.which(BASF2_EXE) is None:
        raise SystemExit(f"Errore: '{BASF2_EXE}' not found in PATH. Did you run b2setup?")

    if not MAKE_BGO_SCRIPT.exists():
        raise SystemExit(f"Script not found: {MAKE_BGO_SCRIPT}")

    if not GEN_SCRIPT.exists():
        raise SystemExit(f"Script not found: {GEN_SCRIPT}")


def overlay_filename(phase, scenario, no_upgrade, no_noise):
    ecl_mode_label = "current" if no_upgrade else "new"
    elec_label = "noNoise" if no_noise else "noise"
    return f"BGforOverlay_{phase}_{scenario}_{ecl_mode_label}_{elec_label}.root"


def photon_output_filename(energy_str, no_upgrade, no_noise):
    elec_noise_str = "_NoElecNoise" if no_noise else ""
    if not no_upgrade:
        return f"ParticleGunOutput_photon_{energy_str}GeV_bgMCbgo_Up{elec_noise_str}_WF.root"
    return f"ParticleGunOutput_photon_{energy_str}GeV_bgMCbgo{elec_noise_str}_WF.root"


def scenario_dir_name(scenario, no_upgrade, no_noise):
    ecl_label = "current" if no_upgrade else "upgrade"
    noise_label = "noNoise" if no_noise else "noise"
    return f"{scenario}_{ecl_label}_{noise_label}"


def build_overlay_command(phase, scenario, no_upgrade, no_noise, overlay_events):
    ecl_mode_arg = "current" if no_upgrade else "newDSP"
    noise_arg = "noNoise" if no_noise else "noise"

    return [
        BASF2_EXE,
        MAKE_BGO_SCRIPT,
        phase,
        scenario,
        ecl_mode_arg,
        noise_arg,
        str(overlay_events),
    ]


def build_photon_command(energy_str, photon_events, no_upgrade, no_noise, bgo_path):
    cmd = [
        PYTHON_EXE,
        GEN_SCRIPT,
        "-nevt", str(photon_events),
        "-bg", "MCbgo",
        "-enrg", energy_str,
        "-bgoPath", str(bgo_path),
    ]

    if no_upgrade:
        cmd.append("--NoUpgrade")

    if no_noise:
        cmd.append("--NoElectronicNoise")

    return cmd


# ============================================================
# Main
# ============================================================

def main():
    ensure_environment()
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)

    electronics_modes = []
    if RUN_UPGRADE:
        electronics_modes.append(False)   # no_upgrade = False
    if RUN_CURRENT:
        electronics_modes.append(True)    # no_upgrade = True

    noise_modes = []
    if RUN_WITH_NOISE:
        noise_modes.append(False)         # no_noise = False
    if RUN_WITHOUT_NOISE:
        noise_modes.append(True)          # no_noise = True

    total_overlay_jobs = 0
    total_photon_jobs = 0

    for scenario in SCENARIOS:
        for no_upgrade in electronics_modes:
            for no_noise in noise_modes:
                scenario_name = scenario_dir_name(scenario, no_upgrade, no_noise)
                scenario_dir = RESULTS_DIR / PHASE / scenario_name
                scenario_dir.mkdir(parents=True, exist_ok=True)

                overlay_file = scenario_dir / overlay_filename(PHASE, scenario, no_upgrade, no_noise)

                # 1) Generate custom BGO sample
                if OVERWRITE or not overlay_file.exists():
                    overlay_cmd = build_overlay_command(
                        phase=PHASE,
                        scenario=scenario,
                        no_upgrade=no_upgrade,
                        no_noise=no_noise,
                        overlay_events=OVERLAY_EVENTS,
                    )
                    run(overlay_cmd, cwd=scenario_dir)

                    if not DRY_RUN and not overlay_file.exists():
                        raise RuntimeError(f"Overlay not found after generation: {overlay_file}")
                else:
                    print(f"\n[SKIP] Overlay already exists: {overlay_file}")

                total_overlay_jobs += 1

                # 2) For each overlay, generate all the photon samples
                for energy_str in ENERGIES_GEV:
                    photon_out = scenario_dir / photon_output_filename(
                        energy_str=energy_str,
                        no_upgrade=no_upgrade,
                        no_noise=no_noise,
                    )

                    if OVERWRITE or not photon_out.exists():
                        photon_cmd = build_photon_command(
                            energy_str=energy_str,
                            photon_events=PHOTON_EVENTS,
                            no_upgrade=no_upgrade,
                            no_noise=no_noise,
                            bgo_path=overlay_file.resolve(),
                        )
                        run(photon_cmd, cwd=scenario_dir)
                    else:
                        print(f"[SKIP] Output already exists: {photon_out}")

                    total_photon_jobs += 1

    print("\n========================================")
    print("Done.")
    print(f"Overlay scenarios processed: {total_overlay_jobs}")
    print(f"Jobs photon processed:       {total_photon_jobs}")
    print(f"Output directory:             {RESULTS_DIR}")
    print("========================================")


if __name__ == "__main__":
    main()
