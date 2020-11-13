#!/usr/bin/env python3

"""
setup.py file for installing module
"""

from distutils.core import setup, Extension

geofun_module = Extension(
    '_geofun',
    sources=['geofun.i', 'geofun.cpp'],
    swig_opts=['-c++'],
)

setup (
    name = 'geofun',
    version = '0.1',
    author = "J.R. Versteegh <j.r.versteegh@gmail.com>",
    description = """Geographic objects and utilities""",
    ext_modules = [geofun_module,],
    py_modules = ["geofun",],
)

