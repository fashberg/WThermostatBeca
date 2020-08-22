import os
import shutil
try:
    import configparser
except ImportError:
    import ConfigParser as configparser

Import("env", "projenv")

# Dump construction environments (for debug purpose)
#print(env.Dump())
#print(projenv.Dump())

config = configparser.ConfigParser()
config.read("platformio.ini")
appversion = config.get("common", "appversion")
config.get

OUTPUT_DIR = "build_output{}".format(os.path.sep)
#print("name-firmware.py // appversion: {}".format(appversion))

def bin_map_copy(source, target, env):
    variant = str(target[0]).split(os.path.sep)[1]
    #print("bin_map_copy: {} {} {}".format(source[0], target[0], variant))
    
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware', 'map']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # create string with location and file names based on variant
    map_file = "{}map{}{}-{}.map".format(OUTPUT_DIR, os.path.sep, variant, appversion)
    bin_file = "{}firmware{}{}-{}.bin".format(OUTPUT_DIR, os.path.sep, variant, appversion)

    # check if new target files exist and remove if necessary
    for f in [map_file, bin_file]:
        if os.path.isfile(f):
            #print("removing: {}".format(f))
            os.remove(f)

    # copy firmware.bin to firmware/<variant>.bin
    shutil.copy(str(target[0]), bin_file)


    print("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
    print("FIRMWARE BUILT: {}".format(bin_file))
    print("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")

    # copy firmware.map to map/<variant>.map
    if os.path.isfile("firmware.map"):
        shutil.move("firmware.map", map_file)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_map_copy])
