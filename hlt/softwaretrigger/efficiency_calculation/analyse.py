import basf2
import os


def main():
    # Get all parameters for this calculation
    input_file = os.environ.get("input_file")
    output_file = input_file.replace("/reconstructed/", "/analysed/")

    # Create output directory
    output_dir = os.path.dirname(output_file)

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Do the real calculation
    # TODO: Fake
    with open(output_file, "w") as output_f:
        with open(input_file, "r") as input_f:
            output_f.write(input_f.read())


if __name__ == "__main__":
    main()
