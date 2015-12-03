#!/usr/bin/env python3
# -*- coding: utf-8 -*-


class MetaOptionParser:

    def __init__(self, meta_option_list):
        self.mo = meta_option_list

    def pvalue_warn(self):
        return self.float_value("pvalue-warn")

    def pvalue_error(self):
        return self.float_value("pvalue-error")

    def float_value(self, key):
        v = self.parse_key_value(key)
        if v is None:
            return None
        try:
            return float(v)
        except ValueError:
            return None

    def parse_key_value(self, key):
        it = [s for s in self.mo if s.startswith(key + "=")]
        if len(it) == 0:
            return None

        key_value_pair = it[0].split("=")

        if len(key_value_pair) < 2:
            return None

        return key_value_pair[1]
