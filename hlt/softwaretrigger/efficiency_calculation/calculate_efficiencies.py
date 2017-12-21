from glob import glob
import os
import argparse

from extract import extract_efficiencies, extract_file_sizes
from gridcontrol_helper import write_gridcontrol_file, call_gridcontrol


def generate_events(channels, n_events, n_jobs, storage_location):
    parameters = []

    for channel in channels:
        for number in range(n_jobs):
            channel_path = os.path.join(storage_location, channel)
            generated_path = os.path.join(channel_path, "generated")
            output_file = os.path.join(generated_path, f"{number}.root")

            parameter = {"channel": channel, "random_seed": number + 1111, "output_file": output_file,
                         "n_events": n_events}

            # Create output directory
            output_dir = os.path.dirname(output_file)

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="generate.py", parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=2)


def run_reconstruction(channels, storage_location):
    parameters = []

    for channel in channels:
        channel_path = os.path.join(storage_location, channel)
        generated_path = os.path.join(channel_path, "generated")
        for input_file in glob(os.path.join(generated_path, "*.root")):
            # Create output directory
            output_file = input_file.replace("/generated/", "/reconstructed/")
            output_dir = os.path.dirname(output_file)

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameter = {"input_file": input_file, "output_file": output_file}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="reconstruct.py", parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=2)


def calculate_efficiencies(channels, storage_location):
    parameters = []

    for channel in channels:

        channel_path = os.path.join(storage_location, channel)
        reconstructed_path = os.path.join(channel_path, "reconstructed")
        for input_file in glob(os.path.join(reconstructed_path, "[0-9].root")):
            # Create output directory
            output_file = input_file.replace("/reconstructed/", "/analysed/")
            output_dir = os.path.dirname(output_file)

            if not os.path.exists(output_dir):
                os.makedirs(output_dir)

            parameter = {"input_file": input_file, "output_file": output_file}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="analyse.py", parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=2)

    extract_efficiencies(channels=channels, storage_location=storage_location)
    extract_file_sizes(channels=channels, storage_location=storage_location)


if __name__ == "__main__":
    channels_to_study = [
        "background_only",

        "ee",
        "eemumu",

        "tau",
        # "tau_to_1_prong_1_prong",
        # "tau_to_e_gamma",
        # "tau_to_mu_gamma",
        # "tau_to_e_nu",
        # "tau_to_mu_nu",

        "mumu",
        "mumu_kkgen",

        "ee",
        "gg",

        "continuum_ccbar",
        "continuum_uubar",
        "continuum_ddbar",
        "continuum_ssbar",

        "BB_charged",
        "BB_mixed",
        # "B_to_nu_nu",
        # "B_to_pi0_pi0",
        # "B_to_rho0_gamma",
        # "B_to_JpsiKS_e_e",

        "pipi",
        "pipipi",
    ]

    parser = argparse.ArgumentParser()
    parser.add_argument("storage_location")

    args = parser.parse_args()

    # Produce 5*1000 events in each channel
    generate_events(channels=channels_to_study, n_events=1000,
                    n_jobs=5, storage_location=args.storage_location)

    # Reconstruct each channel
    run_reconstruction(channels=channels_to_study, storage_location=args.storage_location)

    # Calculate file size and efficiencies for each channel
    calculate_efficiencies(channels=channels_to_study, storage_location=args.storage_location)
