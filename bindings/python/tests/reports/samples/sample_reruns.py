# SPDX-License-Identifier: Apache-2.0
#
# Ported from https://github.com/christiansandberg/pytest-reporter-html1
import pytest


count = 0


@pytest.mark.flaky(reruns=2)
def test_rerun():
    global count

    count += 1
    print("Rerun #%d" % count)
    assert False
