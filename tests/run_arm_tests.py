"""Run the real Games.h logic in a Cortex-M0 CPU emulator; no peripheral claims.
Usage: python tests/run_arm_tests.py /path/to/game-tests.elf
Requires: unicorn, pyelftools. Build example in tests/README.md.
"""
import sys, struct, json
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_MODE_MCLASS, UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR
from elftools.elf.elffile import ELFFile
uc=Uc(UC_ARCH_ARM,UC_MODE_THUMB|UC_MODE_MCLASS)
uc.mem_map(0x10000000,0x200000)
uc.mem_map(0x20000000,0x80000)
with open(sys.argv[1],'rb') as f:
 elf=ELFFile(f)
 for seg in elf.iter_segments():
  if seg['p_type']=='PT_LOAD':uc.mem_write(seg['p_vaddr'],seg.data())
 symbols={s.name:s['st_value'] for s in elf.get_section_by_name('.symtab').iter_symbols()}
uc.reg_write(UC_ARM_REG_SP,0x2007fff0)
stop=0x101ff000
uc.reg_write(UC_ARM_REG_LR,stop|1)
def hook(uc,address,size,user):
 if address==stop:uc.emu_stop()
uc.hook_add(UC_HOOK_CODE,hook)
uc.emu_start(symbols['run_game_tests']|1,stop,count=10000000)
failure=struct.unpack('<I',uc.mem_read(symbols['testFailure'],4))[0]
passed=struct.unpack('<I',uc.mem_read(symbols['testsPassed'],4))[0]
assert failure==0 and passed==10,(failure,passed)
print(json.dumps({'gameLogicChecksPass':True,'checks':passed,'method':'Real C++ game logic compiled for Cortex-M0, executed with Unicorn; graphics stubbed','hardwareVerified':False}))
