# SPDX-License-Identifier: Apache-2.0
#
# Ported from https://github.com/christiansandberg/pytest-reporter-html1
from warnings import warn


def test_warning():
    warn("This is a user warning.", UserWarning)
    warn("This is a deprecation.", DeprecationWarning)
