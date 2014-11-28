import Options

VERSION = '0.0.1'
APPNAME = 'block_graph'
top = '.'
out = 'build'
subdirs = 'src'

def options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.env.CXXFLAGS += ['-O3', '-g', '-Wall']

def build(bld):
  bld.recurse(subdirs)

