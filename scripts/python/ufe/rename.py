
import os

folder_path = "D:/Houdini/GFEHoudini/hdk/GeoSop/include/GeoSop/operator"
        
start = 'SOP_FeE_'
end0 = '.cpp'
end1 = '.h'
for root, dirs, files in os.walk(folder_path):
    # print(root)
    # print(dirs)
    # print(files)
    for file in files:
        fullPath = root + "/" + file
        if not file.endswith(end0) and not file.endswith(end1):
            continue
        if not file.startswith(start):
            continue
        dstPath = root + "/" + file[len(start):]
        os.rename(fullPath, dstPath)
        
        