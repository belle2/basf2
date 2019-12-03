import basf2
import hbasf2

from zmq_daq.example_support import add_input_module, add_reco_modules, add_output_module

from argparse import ArgumentParser

if __name__ == '__main__':
    basf2.reset_database()
    basf2.use_central_database("data_reprocessing_prompt")
    basf2.set_nprocesses(2)

    parser = ArgumentParser(description="Example steering file for the ZMQ tests")
    parser.add_argument("--input", required=True, help="ZMQ Input Address")
    parser.add_argument("--output", required=True, help="ZMQ Output Address")
    parser.add_argument("--dqm", required=False, default="", help="ZMQ DQM Address (or empty for no DQM)")
    parser.add_argument("--raw", action="store_true", help="Send out data in raw format")
    parser.add_argument("--expressreco", action="store_true", help="Also initialize express reco objects")
    parser.add_argument("--mimik-startup", action="store_true", help="Mimik the geometry loading with a sleep")

    args = parser.parse_args()

    path = basf2.Path()
    reco_path = basf2.Path()

    input_module = add_input_module(path, input_address=args.input, add_expressreco_objects=args.expressreco)

    add_reco_modules(reco_path, dqm_address=args.dqm, mimik_startup=args.mimik_startup)

    input_module.if_value("==0", reco_path, basf2.AfterConditionPath.CONTINUE)

    add_output_module(path, output_address=args.output, raw=args.raw)

    if args.dqm:
        hbasf2.process(path, [args.output, args.dqm], False)
    else:
        hbasf2.process(path, [args.output], False)
