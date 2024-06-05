##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


import basf2
import yaml
import collections.abc


def _update_config_dict(d, u):
    """
    Updates the config dictionary.

    .. seealso:: Need a
    `recursive solution <https://stackoverflow.com/questions/3232943/update-value-of-a-nested-dictionary-of-varying-depth>`_
    because it's a nested dict of varying depth.

    Args:
        d (dict): Dictionary to update.
        u (dict): Dictionary of configs.

    Returns:
        dict: Updated dictionary of configs.
    """
    for k, v in u.items():
        if isinstance(v, collections.abc.Mapping):
            d[k] = _update_config_dict(d.get(k, {}), v)
        else:
            d[k] = v
    return d


def load_config(cfg_path=None, model=None, dataset=None, run_name=None, samples=None, **kwargs):
    """
    Load default configs followed by user configs and populate dataset tags.

    Args:
        cfg_path(str or Path): Path to user config yaml.
        model(str): Name of model to use (overwrites loaded config).
        dataset(int): Individual dataset to load (overwrites loaded config).
        run_name(str): Name of training run (overwrites loaded config).
        samples(int): Number of samples to train on (overwrites loaded config).

    Returns:
        dict, list: Loaded training configuration dictionary
        and list of tuples containing (tag name, dataset path, tag key).
    """

    # First load default configs
    with open(basf2.find_file('data/analysis/grafei_config.yaml')) as file:
        configs = yaml.safe_load(file)

    # Load user configs if defined, overwriting defaults
    if cfg_path is not None:
        with open(cfg_path) as file:
            # Approach if configs was not a nested dict
            # configs.update(yaml.safe_load(file))
            # Use custom update function for nested dict
            configs = _update_config_dict(configs, yaml.safe_load(file))

    # Overwrite model architecture if specified in command line
    if model is not None:
        configs['train']['model'] = model
    # Set datasets to load, or default to all if unset
    if (dataset is not None) or ('datasets' not in configs['dataset']):
        configs['dataset']['datasets'] = dataset
    # And run name
    if run_name is not None:
        configs['output']['run_name'] = run_name

    # Finally, generate the dataset tags
    tags = _generate_dataset_tags(configs, samples)

    return configs, tags


def _generate_dataset_tags(configs, samples=None):
    """
    Generate the different dataset tags and assign their file paths.
    This helps us keep track of the train/val datasets throughout training and evaluation.

    Args:
        config (dict): Training configuration dictionary.
        samples (dict): Number of training samples.

    Returns:
        list: List of tuples containing (tag name, dataset path, tag key).
    """
    # Fetch whichever data source we're loading
    source_confs = configs['dataset']

    # Set up appropriate dataset tags
    tags = [
        ("Training", source_confs['path'], 'train'),
        ("Validation", source_confs['path'], 'val'),
    ]

    # And overwrite any data source specific configs
    source_confs['config']['samples'] = samples

    return tags
