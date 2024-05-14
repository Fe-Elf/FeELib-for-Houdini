
import os
import shutil

inputVSVersion = input('WARNING! this is very Danger Operation Are you sure you want to do this? 0 check')
inputVSVersion = input('WARNING! this is very Danger Operation Are you sure you want to do this? 1 check')
inputVSVersion = input('WARNING! this is very Danger Operation Are you sure you want to do this? 2 check')


for fileRootPath in ("./../src/SOP/", "./../src/DM/"):
    fileRootPath = os.path.realpath(fileRootPath) + '/'

    for folderName in os.listdir(fileRootPath):
        relFilePath = fileRootPath + folderName
        absFilePath = os.path.realpath(relFilePath)
        if os.path.isfile(absFilePath):
            continue
        
        absBuildPath = "{0}/build".format(absFilePath).replace('\\', '/')

        doDel = False
        if os.path.exists(absBuildPath):
            # print(os.listdir(absBuildPath))
            listDir = os.listdir(absBuildPath)
            if not listDir:
                continue
            for slnFileName in listDir:
                _fileName, extName = os.path.splitext(slnFileName)
                # print(extName)
                if _fileName == "CMakeFiles" or _fileName == "CMakeCache" or extName == '.sln':
                    doDel = True
                    break
        else:
            continue

        if doDel:
            print(absBuildPath)
            shutil.rmtree(absBuildPath)

