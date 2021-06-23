#!/usr/bin/env python3

"""
This script provides callback functions to use the recommended global tags.
"""

from basf2 import B2INFO
from basf2.version import release
from versioning import recommended_global_tags_v2


def _get_recommended_tags(base_tags, user_tags, metadata):
    """Get the recommendation from versioning and print a message if there is one"""

    recommendation = recommended_global_tags_v2(release, base_tags, user_tags, metadata)
    if 'message' in recommendation.keys():
        B2INFO('Your global tag manager says: ' + recommendation['message'])
    return recommendation['tags']


def _join_tags(*tags):
    """Return concatenated list of GTs"""
    return sum((entry for entry in tags if entry is not None), [])


def recommended_tags(base_tags, user_tags, metadata):
    """Return the the user GTs + the reccomended GTs"""

    recommendation = _get_recommended_tags(base_tags, user_tags, metadata)
    return _join_tags(user_tags, recommendation)


def recommended_analysis_tags(base_tags, user_tags, metadata):
    """Return the the user GTs + the reccomended analysis GTs + the base tags"""

    recommendation = [tag for tag in _get_recommended_tags(base_tags, user_tags, metadata) if tag.startswith('analysis')]
    return _join_tags(user_tags, recommendation, base_tags)
