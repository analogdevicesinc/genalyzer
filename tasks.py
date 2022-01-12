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
    c.run("clang-format -style=file -i bindings/c/include/cgenalyzer.h")
    c.run("black bindings/python")
