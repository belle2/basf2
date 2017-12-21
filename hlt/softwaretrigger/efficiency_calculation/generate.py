import basf2
import os


def main():
    # Get all parameters for this calculation
    channel = os.environ.get("channel")
    output_file = os.environ.get("output_file")
    random_seed = os.environ.get("random_seed")

    # Create output directory
    output_dir = os.path.dirname(output_file)

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Do the real calculation
    # TODO: Fake
    with open(output_file, "w") as f:
        f.write(channel)
        f.write(random_seed)


if __name__ == "__main__":
    main()
