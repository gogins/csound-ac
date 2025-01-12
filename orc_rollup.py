'''
Roll up all .inc files into one .orc file.

If the "instrnames" list is not empty, it is a flat list of .inc files that 
will be rolled up in the order specified; otherwise, all the .inc files will 
simply be concatenated.
'''
print(__doc__)
import os
import sys

instrnames = '''FMWaterBell.inc
Harpsichord.inc
HeavyMetal.inc
Internals.inc
MasterOutput.inc
Melody.inc
Sweeper.inc
Tambourine.inc
WGPluck.inc
ZakianFlute.inc
ReverbSC.inc
MasterOutput.inc'''

template_top = '''<CsoundSynthesizer>
<CsLicense>
This piece tests combinations of instr definitions.
</CsLicense>
<CsOptions>
--m-amps=1 --m-range=1 --m-dB=1 --m-benchmarks=1 --m-warnings=0 -+msg_color=0 -d -odac -Fmisty.mid
</CsOptions>
<CsInstruments>

sr = 48000
ksmps = 128
nchnls = 2
0dbfs = 4
'''
template_bottom = '''
</CsInstruments>
<CsScore>
f 0 360
</CsScore>
<CsoundSynthesizer>
'''

from pathlib import Path
orc = {}

print("Reading all .inc files...")
for p in Path('.').glob('patches/*.inc'):
    print(f"Found: {p.name}")
    orc[p.name] = p.read_text()

print("Creating output csd...")
if len(instrnames) > 0:
   with open("rollup.csd", 'w') as output:
    output.write(template_top)
    for instrname in instrnames.split('\n'):
        print("Using: ", instrname)
        instr = orc[instrname]
        output.write(instr)
    output.write(template_bottom)
print("Finished.")

