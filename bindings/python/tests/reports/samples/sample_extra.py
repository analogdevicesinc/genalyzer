# SPDX-License-Identifier: Apache-2.0
#
# Ported from https://github.com/christiansandberg/pytest-reporter-html1
from base64 import b64encode

import pytest_html


def test_with_extras(extras):
    extras.append(pytest_html.extras.html("This is some <strong>HTML</strong>!"))
    extras.append(pytest_html.extras.text("<lorem ipsum> " * 100))
    extras.append(pytest_html.extras.url("http://www.example.com/"))
    extras.append(pytest_html.extras.json({"content": {"list": [1, 2, 3]}}))
