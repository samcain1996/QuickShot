import os
import sys
import time

build_delay_secs = 5

cmake = False
debug = False

make = False

if len(sys.argv) >= 2:
    cmake = sys.argv[1]

if len(sys.argv) >= 3:
    debug = sys.argv[2]

if len(sys.argv) >= 4:
    make = sys.argv[3]

if cmake:
    if debug:
        os.system('build/cmake -DCMAKE_BUILD_TYPE=DEBUG ../')
    else:
        os.system('build/cmake ../')

if make:
    os.system('build/make')
    time.sleep(build_delay_secs)

for imageFile in [ file for file in os.listdir() if '.bmp' in file]:
    os.remove(imageFile)

os.system('build/QuickShotDemo')