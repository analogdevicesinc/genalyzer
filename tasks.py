# type:ignore
# flake8: noqa
import os
import sys
import glob

from invoke import task


@task
def lint(c):
    """run linting"""
    files = glob.glob("./**/CMakeLists.txt") + ["CMakeLists.txt"]
    cmd = ["cmake-format -i"] + files
    c.run(" ".join(cmd))
    c.run("clang-format -style=file -i bindings/c/include/cgenalyzer_simplified_beta.h")
    c.run("black bindings/python")

@task
def bumpversion_test(c, filename=None):
    """Bump version to {current-version}.dev.{date}
    Used for marking development releases for test-pypi
    """
    import fileinput
    import time

    if filename is None:
        filename = "bindings/python/pyproject.toml"

    if not os.path.isfile(filename):
        raise FileNotFoundError(filename)

    for line in fileinput.input(filename, inplace=True):
        if line.find("version = ") > -1:
            s = line.find("version = ")
            l = line[s+len("version = ") + 1 :].strip()
            l = l.replace('"', "")
            l = l.split(".")
            major = int(l[0])
            minor = int(l[1])
            rev = int(l[2])
            seconds = int(time.time())
            line = ' '*s+'version = "{}.{}.{}.dev.{}"\n'.format(
                major, minor, rev, seconds
            )
            ver_string = "{}.{}.{}.dev.{}".format(major, minor, rev, seconds)
        print(line, end="")

    print(f"Version bumped to {ver_string}")
