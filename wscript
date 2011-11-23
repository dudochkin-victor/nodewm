srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  conf.env.append_value('CPPFLAGS', ['-Du_int32_t=uint32_t', '-Du_int16_t=uint16_t'])
  conf.env.append_value('CCFLAGS', ['-O3'])

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "wmsigner"
  obj.source = """
	wmsigner-2.0.2/crypto.cpp 
	wmsigner-2.0.2/md4.cpp 
	wmsigner-2.0.2/rsalib1.cpp 
	wmsigner-2.0.2/cmdbase.cpp 
	wmsigner-2.0.2/signer.cpp 
	wmsigner-2.0.2/wmsigner.cpp
	wmsigner-2.0.2/base64.cpp
	wmlib.cc"""
  obj.includes = "wmsigner-2.0.2"