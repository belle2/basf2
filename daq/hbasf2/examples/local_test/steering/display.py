import basf2

from argparse import ArgumentParser

if __name__ == '__main__':
    basf2.reset_database()
    basf2.use_central_database("data_reprocessing_prompt")

    parser = ArgumentParser(description="Example steering file for the ZMQ tests")
    parser.add_argument("--input", required=True, help="ZMQ Input Address")

    args = parser.parse_args()

    path = basf2.Path()

    path.add_module("HLTZMQ2DsDirect", input=args.input)
    path.add_module("Gearbox")
    path.add_module("Geometry")
    path.add_module("Display")

    basf2.process(path)
