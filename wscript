# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

from waflib import Utils
import os

VERSION = '0.1.0'
APPNAME = 'ndn-cledger'
GIT_TAG_PREFIX = 'cledger-'

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])
    opt.load(['default-compiler-flags', 'coverage', 'sanitizers',
              'boost', 'leveldb'],
             tooldir=['.waf-tools'])

    optgrp = opt.add_option_group('ndn-cledger Options')
    optgrp.add_option('--with-tests', action='store_true', default=False,
                      help='Build unit tests')
    optgrp.add_option('--with-benchmark', action='store_true', default=False,
                      help='Build with benchmark logger')
    
def configure(conf):
    conf.load(['compiler_cxx', 'gnu_dirs',
               'default-compiler-flags', 'boost', 'leveldb'])

    conf.env.WITH_TESTS = conf.options.with_tests
    conf.env.WITH_BENCHMARK = conf.options.with_benchmark

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'], uselib_store='NDN_CXX',
                   pkg_config_path=os.environ.get('PKG_CONFIG_PATH', '%s/pkgconfig' % conf.env.LIBDIR))
    conf.check_cfg(package='libndn-svs', args=['--cflags', '--libs'], uselib_store='NDN_SVS',
                   pkg_config_path=os.environ.get('PKG_CONFIG_PATH', '%s/pkgconfig' % conf.env.LIBDIR))
    
    boost_libs = ['system', 'program_options', 'filesystem']
    if conf.env.WITH_TESTS:
        boost_libs.append('unit_test_framework')

    conf.check_leveldb()
    conf.check_boost(lib=boost_libs, mt=True)
    if conf.env.BOOST_VERSION_NUMBER < 107100:
        conf.fatal('The minimum supported version of Boost is 1.71.0.\n'
                   'Please upgrade your distribution or manually install a newer version of Boost.\n'
                   'For more information, see https://redmine.named-data.net/projects/nfd/wiki/Boost')

    conf.check_compiler_flags()

    # Loading "late" to prevent tests from being compiled with profiling flags
    conf.load('coverage')
    conf.load('sanitizers')

    # If there happens to be a static library, waf will put the corresponding -L flags
    # before dynamic library flags.  This can result in compilation failure when the
    # system has a different version of the ndncert library installed.
    conf.env.prepend_value('STLIBPATH', ['.'])

    conf.define_cond('HAVE_TESTS', conf.env.WITH_TESTS)
    conf.define_cond('WITH_BENCHMARK', conf.env.WITH_BENCHMARK)
    conf.define('SYSCONFDIR', conf.env.SYSCONFDIR)
    # The config header will contain all defines that were added using conf.define()
    # or conf.define_cond().  Everything that was added directly to conf.env.DEFINES
    # will not appear in the config header, but will instead be passed directly to the
    # compiler on the command line.
    conf.write_config_header('src/cledger-config.hpp', define_prefix='CLEDGER_')

def build(bld):
    bld.shlib(target='ndn-cledger',
              vnum=VERSION,
              cnum=VERSION,
              source=bld.path.ant_glob('src/**/*.cpp'),
              use='NDN_CXX NDN_SVS BOOST LEVELDB',
              includes='src',
              export_includes='src')

    bld(features='subst',
        source='libndn-cledger.pc.in',
        target='libndn-cledger.pc',
        install_path='${LIBDIR}/pkgconfig',
        VERSION=VERSION)

    bld.recurse('tools')
    bld.recurse('tests')
    bld.install_files(
        dest='${INCLUDEDIR}/cledger',
        files=bld.path.ant_glob('src/**/*.hpp'),
        cwd=bld.path.find_dir('src'),
        relative_trick=True)

    bld.install_files('${INCLUDEDIR}/cledger',
                      bld.path.find_resource('src/cledger-config.hpp'))
    bld.install_files('${SYSCONFDIR}/cledger',
                      ['ledger.conf.sample'])
    bld.install_files('${SYSCONFDIR}/cledger',
                      ['trust-schema.conf.sample'])