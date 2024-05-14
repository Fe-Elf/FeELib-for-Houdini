
import hou
import os
import fee_HDA

fee_HDA.checkHideFeENode(keepHide = True, detectName = False, detectPath = True)








'''
sopNodeTypeNameTuple = fee_HDA.readDeprecatedNode()

for sopNodeTypeName in sopNodeTypeNameTuple:
    nodetype = hou.nodeType(hou.sopNodeTypeCategory(), sopNodeTypeName)
    if nodetype is not None:
        nodetype.setHidden(True)
'''







__FeE_FeELib_PATH = hou.getenv('FeELib')#### 复制脚本需要修改这个
# print(__FeE_FeELib_PATH)
__FeE_FeELib_PATH = os.path.normpath(__FeE_FeELib_PATH)
__FeE_FeELib_PATH = __FeE_FeELib_PATH.replace('\\', '/')
# print(__FeE_FeELib_PATH)

__FeE_HOUDINI_SCRIPT_PATH = hou.getenv('HOUDINI_SCRIPT_PATH')
__FeE_HOUDINI_SCRIPT_PATH_split = __FeE_HOUDINI_SCRIPT_PATH.split(';')

try:
    __FeE_startScriptNum = 1 + __FeE_HOUDINI_SCRIPT_PATH_split.index(__FeE_FeELib_PATH + '/scripts')
except:
    __FeE_startScriptNum = 9999999

for idx in range(__FeE_startScriptNum, len(__FeE_HOUDINI_SCRIPT_PATH_split)):
    __FeE_nextStartupPy = __FeE_HOUDINI_SCRIPT_PATH_split[idx] + '/456.py'
    if os.path.exists(__FeE_nextStartupPy):
        with open(__FeE_nextStartupPy, "r") as __FeE_open:
            __FeE_code = __FeE_open.read()
            exec(__FeE_code)
        break

