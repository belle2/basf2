##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import string
import unittest


class NoReplacementField(object):

    """This class serves as a placeholder for keys in not found during lookup into
    keyword or positional arguments during a call to TolerateMissingKeyFormatter.

    It records potential item and attribute lookups, conversion and format_spec to
    reproduce original replacement_field entry into the formatted string.

    Parameters
    ----------
    field_name : str
        field_name part of the replacement_field specifier
    conversion : str
        conversion part of the replacement_field specifier
    format_spec : str%
        format_spec part of the replacement_field specifier

    Notes
    -----
    replacement_field, field_name, conversion and format_spec have the meaning as outlined in
    https://docs.python.org/2/library/string.html#format-string-syntax
    """

    def __init__(self, field_name, conversion=None, format_spec=None):
        """Constructor"""

        #: cached value of the field name
        self.field_name = field_name
        #: cached value of the conversion specifier
        self.conversion = conversion
        #: cached value of the format specifier
        self.format_spec = format_spec

    def __getattr__(self, attr):
        """Record attribute lookup"""
        return NoReplacementField(self.field_name + "." + attr)

    def __getitem__(self, attr):
        """Record item lookup"""
        return NoReplacementField(self.field_name + "[" + str(attr) + "]")

    def compose(self):
        """Compose a replacement_field string equivalent to the original replacement_field in the string formatting."""
        text_in_brackets = self.field_name

        if self.conversion is not None:
            text_in_brackets += "!" + self.conversion

        if self.format_spec is not None:
            text_in_brackets += ":" + self.format_spec

        replacement_field = "{" + text_in_brackets + "}"
        return replacement_field


class TolerateMissingKeyFormatter(string.Formatter):

    """A string formatter that implements format most equivalent to the str.format, but does not replace keys,
    that are not present in the replacements dictionary

    Example
    -------
    >>> formatter = TolerateMissingKeyFormatter()
    >>> template = "{present}_{missing}"
    >>> partially_substituted = formatter.format(template, present="replaced")
    >>> print partially_substituted
    "replaced_{missing}"

    """

    def get_value(self, key, args, kwds):
        """Retrieves the value that corresponds to the key from either the postional or
        the keyword arguments given to format

        Overrides the standard lookup such that missing keys in the keyword arguments or
        transformed in a NoReplacementField signal object.
        """

        if isinstance(key, str):
            try:
                return kwds[key]
            except KeyError:
                return NoReplacementField(key)
        else:
            return super(TolerateMissingKeyFormatter, self).get_value(key, args, kwds)

    def convert_field(self, value, conversion):
        """Applies the conversion to the value.

        Overrides the standard method such that a potential conversion is attached to the NoReplacementField
        """

        if isinstance(value, NoReplacementField):
            conversion = conversion or None
            value.conversion = conversion
            return value
        else:
            return super(TolerateMissingKeyFormatter, self).convert_field(value, conversion)

    def format_field(self, value, format_spec):
        """Applies the conversion to the value.

        Overrides the standard method such that a potential format_spec is attached to the NoReplacementField.
        Than composes the replacement_field specification to be inserted in the formatted string.
        The outcome should be equivalent to the unformatted string for missing keys.
        """

        if isinstance(value, NoReplacementField):
            format_spec = format_spec or None
            value.format_spec = format_spec
            return value.compose()
        else:
            return super(TolerateMissingKeyFormatter, self).format_field(value, format_spec)


class TolerateMissingKeyFormatterTest(unittest.TestCase):
    """Test the string formatter for cases where the keys are missing"""

    def setUp(self):
        """Prepare for the string-formatter test"""
        #: Use the custom string formatter that tolerates missing values for keywords
        self.formatter = TolerateMissingKeyFormatter()

    def test_missing_key(self):
        """Test for a missing key"""
        template = "{present}_{missing}"
        replaced = self.formatter.format(template, present="replaced")
        self.assertEqual("replaced_{missing}", replaced)

    def test_missing_key_attribute(self):
        """Test for a missing key attribute"""
        template = "{present}_{missing.field}"
        replaced = self.formatter.format(template, present="replaced")
        self.assertEqual("replaced_{missing.field}", replaced)

    def test_missing_key_item(self):
        """Test for a missing key item"""
        template = "{present}_{missing[0]}"
        replaced = self.formatter.format(template, present="replaced")
        self.assertEqual("replaced_{missing[0]}", replaced)

    def test_missing_key_with_str_conversion(self):
        """Test for a missing key item"""
        template = "{present}_{missing!s}"
        replaced = self.formatter.format(template, present="replaced")
        self.assertEqual("replaced_{missing!s}", replaced)

    def test_missing_key_with_repr_conversion(self):
        """Test for a missing key item"""
        template = "{present}_{missing!r}"
        replaced = self.formatter.format(template, present="replaced")
        self.assertEqual("replaced_{missing!r}", replaced)

    def test_missing_key_with_conversion_and_format(self):
        """Test for a missing key item"""
        template = "{present}_{missing!r:^10s}"
        replaced = self.formatter.format(template, present="replaced")
        self.assertEqual("replaced_{missing!r:^10s}", replaced)

    def test_missing_key_with_format(self):
        """Test for a missing key item"""
        template = "{present}_{missing:^10s}"
        replaced = self.formatter.format(template, present="replaced")
        self.assertEqual("replaced_{missing:^10s}", replaced)


if __name__ == '__main__':
    unittest.main()
