# This script contains the utility functions used for flavor tagging


def get_Belle_or_Belle2():
    """
    Gets the global ModeCode.
    """
    # imports
    import b2bii

    if b2bii.isB2BII():
        return 'Belle'
    else:
        return 'Belle2'


def read_yaml(filepath: str):
    """
    Reads the yaml file contents and returns a dictionary object.
    """
    # imports
    import yaml
    from basf2 import B2FATAL

    with open(filepath) as stream:
        try:
            data = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            B2FATAL(exc)

    return data
