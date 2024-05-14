
import os
from shutil import copyfile

def genCMakeLists():
    for houdini_vertion in ("18.5", "19.0", "19.5"):
        target_path = "../CMakeLists" + houdini_vertion + "/CMakeLists.txt"
        copyfile("../cmake/CMakeLists.txt", target_path)

        file_data = ""
        with open(target_path, "r") as f:
            for line in f:
                line = line.replace(r"""list( APPEND CMAKE_PREFIX_PATH "$ENV{HFSLatest}/toolkit/cmake" )""", r"""list( APPEND CMAKE_PREFIX_PATH "$ENV{HFS""" + houdini_vertion + r"""}/toolkit/cmake" )""")
                # line = line.replace(r'''./include''', r'''../include''')
                # line = line.replace(r'''./src/''', r'''../src/''')
                # line = line.replace(r'''./../dso/''', r'''../../dso/''')

                file_data += line
        with open(target_path, "w") as f:
            f.write(file_data)
