#!/usr/bin/env python

"""
setup.py file for SWIG
"""

from distutils.core import setup, Extension


geofun_module = Extension(
    '_geofun',
    sources=['geofun_wrap.cxx',
             'geofun.cpp'],
    language='c++',
)

setup (
    name = 'geofun',
    version = '0.1',
    author      = "J.R. Versteegh <j.r.versteegh@gmail.com>",
    description = """Geographic objects and utilities""",
    ext_modules = [geofun_module],
    py_modules = ["geofun"],
)
