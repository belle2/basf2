#!/usr/bin/env python3

import os
import hashlib
import json

folder_name_results = "results"
folder_name_html_static = "html_static"
folder_name_html = "html"
folder_name_html_plots = "plots"

# the plots subfolder in the html folder
folder_name_plots = "plots"
folder_name_general = "__general__"
file_name_results_json = "revision.json"
file_name_comparison_json = "comparison.json"
file_name_runtimes_dat = "runtimes.dat"


def get_basepath():
    return {
        "local": os.environ.get("BELLE2_LOCAL_DIR", None),
        "central": os.environ.get("BELLE2_RELEASE_DIR", None),
    }


class TagFolderRainbowTable(dict):
    """ Because of issues of too long filenames if the number of revision
    is growing, the simple recipe of creating folder names by concatenating
    the names of the revisions does not work.
    Therefore, we use a hashing algorithm. At the same time we want to be
    able to look up the folder content easily, so we create a rainbow table,
    that is a simple text file that contains hash <> revisions.
    """

    def update_from_json(self, path: str) -> None:
        """ Read a json file which was produced by the ``to_json`` method and
        update this dictionary. If the path does not exist, do nothing. """
        if os.path.exists(path):
            with open(path) as infile:
                self.update(json.load(infile))

    def to_json(self, path: str) -> None:
        """ Write out this dictionary as a json file. """
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w") as outfile:
            json.dump(self, outfile, indent=4, sort_keys=True)

    def update_to_json(self, path: str) -> None:
        """ Read json file (if exists) for anything the dictionary
        doesn't contain yet and write everything back. """
        self.update_from_json(path)
        self.to_json(path)


# Note that it is enough to have one object, even in the (unexpected) case that
# there is more than one output_base_dir (at the cost of having some unneeded
# entries there, perhaps)
RAINBOW_TABLE = TagFolderRainbowTable()


def get_results_runtime_file(output_base_dir):
    """!
    Return the absolute path to the runtimes.dat file
    As there is only the runtimes file of the last iteration stored, this is
    located in the topmost work folder
    """
    return os.path.join(output_base_dir, file_name_runtimes_dat)


def get_results_folder(output_base_dir):
    """!
    Return the absolute path to the results folder
    """
    return os.path.join(output_base_dir, folder_name_results)


def get_html_folder(output_base_dir):
    """!
    Return the absolute path to the results folder
    """
    return os.path.join(output_base_dir, folder_name_html)


def get_html_plots_folder(output_base_dir):
    """!
    Return the absolute path to generated plots in the html folder
    """
    return os.path.join(get_html_folder(output_base_dir), folder_name_plots)


def get_html_plots_tag_comparison_folder(output_base_dir, tags):
    """!
    Return the absolute path to the results folder
    """
    string = ",".join(sorted(tags))
    tag_folder = hashlib.sha1(string.encode("utf8")).hexdigest()[:10]
    if tag_folder not in RAINBOW_TABLE:
        RAINBOW_TABLE[tag_folder] = sorted(tags)
        RAINBOW_TABLE.update_to_json(
            os.path.join(get_html_plots_folder(output_base_dir), "rainbow.json")
        )
    return os.path.join(get_html_plots_folder(output_base_dir), tag_folder)


def get_html_plots_tag_comparison_json(output_base_dir, tags):
    """!
    Return the absolute path json file with the comparison file
    """
    return os.path.join(
        get_html_plots_tag_comparison_folder(output_base_dir, tags),
        file_name_comparison_json,
    )


def get_results_tag_folder(output_base_dir, tag):
    """!
    Return the absolute path to the results folder for one specific tag
    """
    return os.path.join(get_results_folder(output_base_dir), tag)


def get_results_tag_general_folder(output_base_dir, tag):
    """!
    Return the absolute path to the results folder for one specific
    tag. In this general folder, the common log files will be placed
    """
    return os.path.join(
        get_results_tag_folder(output_base_dir, tag), folder_name_general
    )


def get_results_tag_revision_file(output_base_dir, tag):
    """!
    Return the absolute path to the revision.json file for one tag folder
    """
    return os.path.join(
        get_results_tag_folder(output_base_dir, tag), file_name_results_json
    )


def get_results_tag_package_folder(output_base_dir, tag, package):
    """!
    Returns the absolute path for a tag and package. This folder will contain
    the ROOT files plots which will be displayed on the validation website
    """
    return os.path.join(get_results_tag_folder(output_base_dir, tag), package)
