#
# pylibssh2 - python bindings for libssh2 library
#
# Copyright (C) 2010 Wallix Inc.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation; either version 2.1 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
"""
    Installation script for the libssh2 module
"""
from distutils.core import setup
from distutils.core import Command
from distutils.core import Extension
from distutils.util import get_platform
from platform import python_version

import os, sys, glob

build_path = "build/lib.%s-%s" % (get_platform(), python_version()[:3])
sys.path.append('.')
sys.path.append('tests')
sys.path.append(build_path)
sys.path.append('libssh2')

import version as info

version = info.__version__
url = info.__url__
author = info.__author__
author_email = info.__authoremail__

long_description = '''Python binding for libssh2 library'''

classifiers = """Development Status :: 4 - Beta
License :: OSI Approved :: BSD License
Operating System :: POSIX
Programming Language :: C
Programming Language :: Python
Topic :: Security
Topic :: Software Development :: Libraries""".split('\n')

libssh2_src = glob.glob('src/*.c')
libssh2_dep = glob.glob('src/*.h')
libssh2_incdir = None
libssh2_libdir = None

if 'bsd' in sys.platform[:-1] or 'bsd' in os.uname()[0].lower():
    libssh2_incdir = ['/usr/local/include/']
    libssh2_libdir = ['/usr/local/lib/']
if 'darwin' in sys.platform:
    libssh2_incdir = ['/opt/local/include/']
    libssh2_libdir = ['/opt/local/lib/']

libssh2_lib = ['ssh2']


class Libssh2TestCommand(Command):
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        import unittest
        from test_session import SessionTest
        from test_scp import SCPTest
        from test_sftp import SFTPTest
        from test_ssh import SSHTest
        from test_session_dealloc import DeallocSessionTest
        from test_sftp_dealloc import DeallocSftpTest
        from test_sftp_sub_dealloc import DeallocSftpSubTest

        suite = unittest.TestSuite()
        suite.addTest(unittest.makeSuite(SessionTest))
        suite.addTest(unittest.makeSuite(SCPTest))
        suite.addTest(unittest.makeSuite(SFTPTest))
        suite.addTest(unittest.makeSuite(SSHTest))
        suite.addTest(unittest.makeSuite(DeallocSessionTest))
        suite.addTest(unittest.makeSuite(DeallocSftpTest))
        suite.addTest(unittest.makeSuite(DeallocSftpSubTest))

        runner = unittest.TextTestRunner(verbosity=2)
        runner.run(suite)


module = Extension('_libssh2',
                    define_macros=[
                        ('MAJOR_VERSION', version[0]),
                        ('MINOR_VERSION', version[2]),
                        ('PATCH_VERSION', version[4])
                    ],
                    sources=libssh2_src,
                    depends=libssh2_dep,
                    include_dirs=libssh2_incdir,
                    library_dirs=libssh2_libdir,
                    libraries=libssh2_lib)

setup(name='pylibssh2',
      version=version,
      packages=['libssh2'],
      package_dir={
        'libssh2': 'libssh2'
      },
      description=long_description,
      author=author,
      author_email=author_email,
      url=url,
      download_url='%s/download/pylibssh2-%s.tar.gz' % (url, version),
      ext_modules=[module],
      license='LGPL',
      platforms=['Linux', 'BSD'],
      long_description=long_description,
      classifiers=classifiers,
      cmdclass={'test': Libssh2TestCommand},
      options={'bdist_rpm': {
                    'requires': 'gcc libssh2 libssh2-devel',
                    'python': sys.executable,
                    }
               })

try:
    if os.path.exists(os.path.join(build_path + "-pydebug", "_libssh2_d.so")):
        os.symlink(os.path.join(build_path + "-pydebug", "_libssh2_d.so"), "./_libssh2.so")
    elif os.path.exists(os.path.join(build_path, "_libssh2.so")):
        os.symlink(os.path.join(build_path, "_libssh2.so"), "./_libssh2.so")
except:
    pass
