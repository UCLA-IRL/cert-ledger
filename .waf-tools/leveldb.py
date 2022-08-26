
#! /usr/bin/env python
# encoding: utf-8

from waflib.Configure import conf

def options(opt):
    opt.add_option('--with-leveldb', type='string', default=None, dest='leveldb_dir',
                   help='directory where LevelDB is installed, e.g., /usr/local')

@conf
def check_leveldb(self, *k, **kw):
    root = k and k[0] or kw.get('path', self.options.leveldb_dir)
    mandatory = kw.get('mandatory', True)
    var = kw.get('uselib_store', 'LEVELDB')

    if root:
        self.check_cxx(lib='leveldb',
                       msg='Checking for LevelDB library',
                       define_name='HAVE_%s' % var,
                       uselib_store=var,
                       mandatory=mandatory,
                       includes='%s/include' % root,
                       libpath='%s/lib' % root)
    else:
        try:
            self.check_cfg(package='leveldb',
                           args=['--cflags', '--libs'],
                           global_define=True,
                           define_name='HAVE_%s' % var,
                           uselib_store='LEVELDB',
                           mandatory=True)
        except:
            self.check_cxx(lib='leveldb',
                           msg='Checking for LevelDB library',
                           define_name='HAVE_%s' % var,
                           uselib_store=var,
                           mandatory=mandatory)