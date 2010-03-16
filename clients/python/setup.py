#!/usr/bin/env python

"""
@file setup.py
@author Nick Gerakines
@date 3/15/2010
@brief Setuptools configuration for barbershop client
"""

version = '0.1.1'

sdict = {
    'name' : 'barbershop',
    'version' : version,
    'description' : 'Python client for interacting with Barbershop.',
    'long_description' : 'Python client for interacting with Barbershop.',
    'url': 'http://github.com/ngerakines/barbershop',
    'download_url' : 'http://cloud.github.com/downloads/ngerakines/barbershop/redis-%s.tar.gz' % version,
    'author' : 'Nick Gerakines',
    'author_email' : 'nick@gerakines.net',
    'maintainer' : 'Nick Gerakines',
    'maintainer_email' : 'nick@gerakines.net',
    'keywords' : ['barbershop', 'client'],
    'license' : 'MIT',
    'packages' : ['barbershop'],
    'test_suite' : 'tests.all_tests',
    'classifiers' : [
        'Development Status :: 4 - Beta',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python'],
}

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup
    
setup(**sdict)

