
import os
# import subprocess



for fileRootPath in ("./../src/SOP/", "./../src/DM/"):
    fileRootPath = os.path.realpath(fileRootPath) + '/'

    for folderName in os.listdir(fileRootPath):
        relFilePath = fileRootPath + folderName
        absFilePath = os.path.realpath(relFilePath)
        if os.path.isfile(absFilePath):
            continue
        
        cmdRealPath = absFilePath + ".cmd"
        # print(cmdRealPath)
        # continue
        if os.path.exists(cmdRealPath):
            continue
        
        absBuildPath = "{0}/build/".format(absFilePath).replace('\\', '/')

        if not os.path.exists(absBuildPath):
            continue
        
        for slnFileName in os.listdir(absBuildPath):
            _fileName, extName = os.path.splitext(slnFileName)
            # print(extName)
            if extName != '.sln':
                continue
            slnAbsFilePath = absBuildPath + '\\' + slnFileName
            slnAbsFilePath = os.path.realpath(slnAbsFilePath)
            break

        cmdFileCommand = r'start "" "' + os.path.relpath(slnAbsFilePath, "..\SOP").replace("/","\\") + r'"'

        cmdFile = open(cmdRealPath, 'w')
        cmdFile.write(cmdFileCommand)



    # command = ''
    # for folderName in os.listdir(fileRootPath):
    #     absFilePath = fileRootPath + folderName
    #     if os.path.isfile(absFilePath):
    #         continue

    #     # print(absFilePath)
    #     command += "\n"
    #     command += "cd \".\\SOP\\{0}\\build\"".format(folderName)
    #     command += "\n"
    #     command += "cmake -G {0} .. -DCMAKE_PREFIX_PATH={1}".format(parm_VSVersion, parm_DCMAKE_PREFIX_PATH)


    # command += '\npause'

    # print(command)
    # p = subprocess.Popen("cmd.exe /c" + "d:/start.bat", stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    # os.system(command)
