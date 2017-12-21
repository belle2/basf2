from glob import glob
import os
import argparse

from gridcontrol_helper import write_gridcontrol_file, call_gridcontrol


def generate_events(channels, n_events, n_jobs, storage_location):
    parameters = []

    for channel in channels:
        for number in range(n_jobs):
            parameter = {}

            parameter["channel"] = channel
            parameter["random_seed"] = number + 1111

            channel_path = os.path.join(storage_location, channel)
            generated_path = os.path.join(channel_path, "generated")
            output_file = os.path.join(generated_path, f"{number}.root")
            parameter["output_file"] = output_file

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="generate.py", parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=2)


def run_reconstruction(channels, storage_location):
    parameters = []

    for channel in channels:
        parameter = {}

        channel_path = os.path.join(storage_location, channel)
        generated_path = os.path.join(channel_path, "generated")
        for filename in glob(os.path.join(generated_path, "*.root")):
            parameter = {"input_file": filename}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="reconstruct.py",
                                              parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=2)


def calculate_efficiencies(channels, storage_location):
    parameters = []

    for channel in channels:

        channel_path = os.path.join(storage_location, channel)
        reconstructed_path = os.path.join(channel_path, "reconstructed")
        for filename in glob(os.path.join(reconstructed_path, "*.root")):
            parameter = {"input_file": filename}

            parameters.append(parameter)

    gridcontrol_file = write_gridcontrol_file(steering_file="analyse.py",
                                              parameters=parameters)
    call_gridcontrol(gridcontrol_file=gridcontrol_file, retries=2)


if __name__ == "__main__":
    channels_to_study = [
        "BB_mixed",
        "BB_charged",
        # etc.
    ]

    parser = argparse.ArgumentParser()
    parser.add_argument("storage_location")

    args = parser.parse_args()

    storage_location = args.storage_location

    # Produce 5*1000 events in each channel
    generate_events(channels=channels_to_study, n_events=1000,
                    n_jobs=5, storage_location=storage_location)

    # Reconstruct each channel
    run_reconstruction(channels=channels_to_study, storage_location=storage_location)

    # Calculate file size and efficiencies for each channel
    calculate_efficiencies(channels=channels_to_study, storage_location=storage_location)
