import argparse

import basf2
from ROOT import Belle2 as B2  # noqa: N811


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description='Copy a payload from a global tag into a local database.',
    )
    parser.add_argument(
        '--gt', required=True,
        help='Global tag to process'
    )
    parser.add_argument(
        '--input-payload', '-i', required=True,
        help='Name of the payload to read from the global tag'
    )
    parser.add_argument(
        '--output-payload', '-o', required=True,
        help='Name under which to store the payload locally'
    )
    parser.add_argument(
        '--exp', type=int, required=True,
        help='Experiment number to use when looking up the payload',
    )
    parser.add_argument(
        '--run', type=int, required=True,
        help='Run number to use when looking up the payload',
    )
    return parser.parse_args()


def make_event_meta_data(exp: int, run: int, event: int = 0) -> None:
    """Register and populate a minimal EventMetaData so PyDBObj works."""
    B2.DataStore.Instance().setInitializeActive(True)
    event_meta_data = B2.PyStoreObj('EventMetaData')
    event_meta_data.registerInDataStore()
    event_meta_data.assign(B2.EventMetaData(event, run, exp), True)
    B2.DataStore.Instance().setInitializeActive(False)


def load_payload(name: str):
    """Fetch a payload by name for the exp/run set in EventMetaData, or B2FATAL."""
    payload = B2.PyDBObj(name)
    if not payload.isValid():
        basf2.B2FATAL(f'{name} is not valid for the requested exp/run')
    return payload.obj(), payload.getIoV()


def main() -> None:
    args = parse_args()

    basf2.conditions.override_globaltags([args.gt])

    make_event_meta_data(args.exp, args.run)

    payload_obj, iov = load_payload(args.input_payload)

    db = B2.Database.Instance()
    ok = db.storeData(args.output_payload, payload_obj, iov)
    if not ok:
        basf2.B2FATAL(f'Failed to store payload as {args.output_payload!r}')

    basf2.B2INFO(
        f'Stored payload {args.input_payload!r} (from {args.gt!r}, '
        f'exp={args.exp}, run={args.run}) as {args.output_payload!r}'
    )


if __name__ == '__main__':
    main()
