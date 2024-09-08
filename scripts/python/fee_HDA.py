# This Python file uses the following encoding: utf-8
import hou
import os
import fee_Utils as fee_Utils


# from importlib import reload
# reload(fee_Utils)


# import fee_HDA
# from importlib import reload
# reload(fee_HDA)


# if 1:
#     import fee_HDA
# else:


def convertDefi(inputNodeType):
    if isinstance(inputNodeType, hou.HDADefinition):
        return inputNodeType
    elif isinstance(inputNodeType, hou.NodeType):
        return inputNodeType.definition()
    elif isinstance(inputNodeType, hou.Node):
        return inputNodeType.type().definition()
    else:
        raise TypeError('Invalid Type')


hou.Node.convertDefi = convertDefi
hou.NodeType.convertDefi = convertDefi
hou.HDADefinition.convertDefi = convertDefi


def convertNodeType(inputNodeType):
    if isinstance(inputNodeType, hou.NodeType):
        return inputNodeType
    elif isinstance(inputNodeType, hou.Node):
        return inputNodeType.type()
    elif isinstance(inputNodeType, hou.HDADefinition):
        return inputNodeType.nodeType()
    else:
        raise TypeError('Invalid Type')


hou.Node.convertNodeType = convertNodeType
hou.NodeType.convertNodeType = convertNodeType
hou.HDADefinition.convertNodeType = convertNodeType


def splitTypeNametoNameComponents(nodeTypeName) -> list:
    # FeE::attrib_fee::0.1 --> ['', 'FeE', 'attrib_fee', '0.1']
    # print(fee_HDA.splitTypeNametoNameComponents("FeE::attrib_fee::0.1"))
    if not isinstance(nodeTypeName, str):
        raise ValueError('input must be string')

    nameComponents = ['', '', '', '']
    nodeTypeName = nodeTypeName.split('::')

    listlen = len(nodeTypeName)
    if listlen == 3:
        if fee_Utils.isFloat(nodeTypeName[-1]):
            nameComponents[3] = nodeTypeName[-1]
        else:
            raise ValueError('must be float')

        if fee_Utils.isFloat(nodeTypeName[-2]):
            raise ValueError('can not be float')
        else:
            nameComponents[2] = nodeTypeName[-2]

        if fee_Utils.isFloat(nodeTypeName[-3]):
            raise ValueError('can not be float')
        else:
            nameComponents[1] = nodeTypeName[-3]

    elif listlen == 2:
        if fee_Utils.isFloat(nodeTypeName[-1]):
            nameComponents[3] = nodeTypeName[-1]
            if fee_Utils.isFloat(nodeTypeName[-2]):
                raise ValueError('can not be float')
            else:
                nameComponents[2] = nodeTypeName[-2]

        else:
            nameComponents[2] = nodeTypeName[-1]
            if fee_Utils.isFloat(nodeTypeName[-2]):
                raise ValueError('can not be float')
            else:
                nameComponents[1] = nodeTypeName[-2]


    elif listlen == 1:
        if fee_Utils.isFloat(nodeTypeName[-1]):
            raise ValueError('can not be float')
        else:
            nameComponents[2] = nodeTypeName[-1]

    else:
        raise ValueError('len too high')

    listlen = len(nodeTypeName)

    return nameComponents


def trimNodeTypeName(nodeTypeName) -> str:
    ### FeE::attrib_fee::0.1 --> attrib_fee
    if not isinstance(nodeTypeName, str):
        raise ValueError()

    nodeTypeName = nodeTypeName.split('::')
    listlen = len(nodeTypeName)
    for idx in range(listlen - 1, -1, -1):
        if fee_Utils.isFloat(nodeTypeName[idx]):
            continue
        return nodeTypeName[idx]

    raise ValueError()


def trimFeENodeName(nodeTypeName) -> str:
    ### FeE::attrib_fee::0.1 --> attrib
    ### attrib_fee::0.1 --> attrib
    if isinstance(nodeTypeName, str):
        nodeTypeName = trimNodeTypeName(nodeTypeName)
    elif isinstance(nodeTypeName, tuple):
        nodeTypeName = nodeTypeName[2]
        if not isinstance(nodeTypeName, str):
            raise ValueError()
    else:
        raise ValueError()

    trimString = '_fee'
    if nodeTypeName.endswith(trimString):
        nodeTypeName = nodeTypeName[:len(nodeTypeName) - len(trimString)]
    trimString = 'FeE_'
    if nodeTypeName.startswith(trimString):
        nodeTypeName = nodeTypeName[len(trimString):]

    return nodeTypeName


def combineNameComponents(nameComponents) -> str:
    ### ['', 'FeE', 'attrib_fee', '0.1'] --> FeE::attrib_fee::0.1
    ### print(fee_HDA.combineNameComponents(['', 'FeE', 'attrib_fee', '0.1']))
    import copy
    trimedNameComponents = copy.deepcopy(nameComponents)
    # trimedNameComponents = ['', '', '', '']
    if isinstance(trimedNameComponents, tuple):
        trimedNameComponents = list(trimedNameComponents)
    if not isinstance(trimedNameComponents, list):
        raise ValueError('must be list or tuple')
    if len(trimedNameComponents) != 4:
        raise ValueError('len of nameComponents must be 4')
    for idx in range(3, -1, -1):
        if not isinstance(trimedNameComponents[idx], str):
            raise ValueError('must be str')
        elif trimedNameComponents[idx] == '':
            del trimedNameComponents[idx]

    return '::'.join(trimedNameComponents)


def tryFindNodeType(inputNodeTypeName, nodeTypeCategory=hou.sopNodeTypeCategory()):
    # print(hou.node('/obj/'))
    if nodeTypeCategory == hou.sopNodeTypeCategory():
        tmp_NodeNetwork = hou.node('/obj/').createNode('geo', run_init_scripts=False, load_contents=False)
    else:
        raise ValueError('unsupport nodeTypeCategory')

    try:
        newNode = tmp_NodeNetwork.createNode(inputNodeTypeName, run_init_scripts=False, load_contents=False)
        outNodeType = newNode.type()
    except:
        outNodeType = None

    tmp_NodeNetwork.destroy(disable_safety_checks=True)
    return outNodeType


'''
def hasNodeType(nodeTypeName):
    nodeType = hou.nodeType(hou.sopNodeTypeCategory(), nodeTypeName)
    if nodeType is None:
        return None
    else:
        return nodeTypeName
'''


def findFeENodeType_byNameComponents(nameComponents, nodeTypeCategory=hou.sopNodeTypeCategory(), fuzzy=False):
    nodeTypeName = combineNameComponents(nameComponents)
    nodeType = tryFindNodeType(nodeTypeName) if fuzzy else hou.nodeType(nodeTypeCategory, nodeTypeName)
    return nodeType


def findFeENodeType(inputNodeTypeName, nodeTypeCategory=hou.sopNodeTypeCategory(), fuzzy=False):
    if not isinstance(inputNodeTypeName, str):
        raise TypeError('must be string')

    # nodeTypeName = inputNodeTypeName
    nameComponents = splitTypeNametoNameComponents(inputNodeTypeName)

    nameComponents[1] = 'FeE'
    if nameComponents[2].endswith('_fee'):
        nameComponents[2] = trimFeENodeName(nameComponents[2])

    nodeType = findFeENodeType_byNameComponents(nameComponents, nodeTypeCategory, fuzzy)
    if nodeType is not None:
        return nodeType

    nameComponents[2] += '_fee'
    nodeType = findFeENodeType_byNameComponents(nameComponents, nodeTypeCategory, fuzzy)
    if nodeType is not None:
        return nodeType

    nameComponents[1] = ''
    nodeType = findFeENodeType_byNameComponents(nameComponents, nodeTypeCategory, fuzzy)
    if nodeType is not None:
        return nodeType

    # nodeTypeName = combineNameComponents(nameComponents)
    # nodeType = tryFindNodeType(nodeTypeName) if fuzzy else hou.nodeType(nodeTypeCategory, nodeTypeName)
    # return hasNodeType(nodeTypeName)
    print('can not found node type' + inputNodeTypeName)
    return None


def readDeprecatedNodeSub2(deprecatedNodeList, pathLib, deprecatedFileName):
    if pathLib is not None:
        deprecatedSopDefinitionTXT = pathLib + '/scripts/deprecatedDefinition/' + (
            'deprecatedSopDefinition.txt' if deprecatedFileName is None else deprecatedFileName)
        if os.path.exists(deprecatedSopDefinitionTXT):
            with open(deprecatedSopDefinitionTXT, "r") as TXT:
                fee_Utils.readTXTAsList(deprecatedNodeList, TXT)


def readDeprecatedNodeSub(deprecatedNodeList, env, deprecatedFileName):
    readDeprecatedNodeSub2(deprecatedNodeList, hou.getenv(env), deprecatedFileName)


def readDeprecatedNode() -> list:
    deprecatedNodeList = []

    readDeprecatedNodeSub(deprecatedNodeList, 'FeELib', None)
    readDeprecatedNodeSub(deprecatedNodeList, 'FeELib', 'deprecatedSopDefinitionHDK.txt')
    readDeprecatedNodeSub(deprecatedNodeList, 'FeEProjectHoudini', None)
    readDeprecatedNodeSub(deprecatedNodeList, 'FeEworkHoudini', None)

    # pathFeELib = hou.getenv('FeELib')
    # if pathFeELib is not None:
    #     deprecatedSopDefinitionTXT = pathFeELib + '/scripts/deprecatedDefinition/deprecatedSopDefinition.txt'
    #     if os.path.exists(deprecatedSopDefinitionTXT):
    #         with open(deprecatedSopDefinitionTXT, "r") as TXT:
    #             fee_Utils.readTXTAsList(deprecatedNodeList, TXT)
    # else:
    #     print('no found env FeELib which belongs to FeELib for Houdini')

    # pathFeELib = hou.getenv('FeEProjectHoudini')
    # readDeprecatedNode(deprecatedNodeList, pathFeELib)
    # if pathFeELib is not None:
    #     deprecatedSopDefinitionTXT = pathFeELib + '/scripts/deprecatedDefinition/deprecatedSopDefinition.txt'
    #     if os.path.exists(deprecatedSopDefinitionTXT):
    #         with open(deprecatedSopDefinitionTXT, "r") as TXT:
    #             fee_Utils.readTXTAsList(deprecatedNodeList, TXT)

    # pathFeELib = hou.getenv('FeEworkHoudini')
    # readDeprecatedNode(deprecatedNodeList, pathFeELib)
    # if pathFeELib is not None:
    #     deprecatedSopDefinitionTXT = pathFeELib + '/scripts/deprecatedDefinition/deprecatedSopDefinition.txt'
    #     if os.path.exists(deprecatedSopDefinitionTXT):
    #         with open(deprecatedSopDefinitionTXT, "r") as TXT:
    #             fee_Utils.readTXTAsList(deprecatedNodeList, TXT)

    deprecatedNodeList = list(set(deprecatedNodeList))
    return deprecatedNodeList


def readNeedHideNode() -> list:
    deprecatedNodeList = []
    pathFeELib = hou.getenv('FeELib')
    if pathFeELib is not None:
        deprecatedSopDefinitionTXT = pathFeELib + '/scripts/deprecatedDefinition/needHideSopNode.txt'
        if os.path.exists(deprecatedSopDefinitionTXT):
            with open(deprecatedSopDefinitionTXT, "r") as TXT:
                fee_Utils.readTXTAsList(deprecatedNodeList, TXT)
    else:
        print('no found env FeELib which belongs to FeELib for Houdini')

    pathFeELib = hou.getenv('FeEProjectHoudini')
    if pathFeELib is not None:
        deprecatedSopDefinitionTXT = pathFeELib + '/scripts/deprecatedDefinition/needHideSopNode.txt'
        if os.path.exists(deprecatedSopDefinitionTXT):
            with open(deprecatedSopDefinitionTXT, "r") as TXT:
                fee_Utils.readTXTAsList(deprecatedNodeList, TXT)

    pathFeELib = hou.getenv('FeEworkHoudini')
    if pathFeELib is not None:
        deprecatedSopDefinitionTXT = pathFeELib + '/scripts/deprecatedDefinition/needHideSopNode.txt'
        if os.path.exists(deprecatedSopDefinitionTXT):
            with open(deprecatedSopDefinitionTXT, "r") as TXT:
                fee_Utils.readTXTAsList(deprecatedNodeList, TXT)

    deprecatedNodeList = list(set(deprecatedNodeList))
    return deprecatedNodeList


def isUnlockedHDA(node) -> bool:
    return not node.isLockedHDA() and node.type().definition() is not None


hou.Node.isUnlockedHDA = isUnlockedHDA


def isFeENode(nodeType: hou.NodeType, detectName=True, detectPath=False) -> bool:
    nodeType = convertNodeType(nodeType)

    if detectName:
        nameComponents = nodeType.nameComponents()
        if not (nameComponents[0].startswith('FeE::') or
                nameComponents[0].startswith('FeE_Debug::') or
                nameComponents[1] == 'FeE' or
                nameComponents[1] == 'ghda' or
                nameComponents[1] == 'GHDA' or
                nameComponents[1] == 'yghda' or
                nameComponents[1] == 'YGHDA' or (
                nameComponents[1] == 'FeE_Debug' or
                nameComponents[2].endswith("_fee") and nodeType.description().startswith("FeE"))):
            return False

    defi = nodeType.definition()
    if defi is None:
        return True

    if detectPath:
        libraryFilePath = defi.libraryFilePath()

        pathFeELib = hou.getenv('FeELib')
        if pathFeELib is None:
            raise AttributeError('not found env: FeELib')

        isInPathCondition = False
        if pathFeELib is not None:
            isInPathCondition = pathFeELib in libraryFilePath

        pathFeELib = hou.getenv('FeEProjectHoudini')
        if pathFeELib is not None:
            isInPathCondition = isInPathCondition or pathFeELib in libraryFilePath

        pathFeELib = hou.getenv('FeEworkHoudini')
        if pathFeELib is not None:
            isInPathCondition = isInPathCondition or pathFeELib in libraryFilePath

        if not isInPathCondition:
            return False

    return True


hou.Node.isFeENode = isFeENode
hou.NodeType.isFeENode = isFeENode
hou.HDADefinition.isFeENode = isFeENode


def isSideFXLabsNode(nodeType, detectName=True, detectPath=False) -> bool:
    nodeType = convertNodeType(nodeType)

    if detectName:
        nameComponents = nodeType.nameComponents()
        if not 'labs' in nameComponents[1]:
            return False

    if detectPath:
        defi = nodeType.definition()
        if defi is None:
            libraryFilePath = 'None'
        else:
            libraryFilePath = defi.libraryFilePath()

        pathFeELib = hou.getenv('SIDEFXLABS')
        if pathFeELib is None:
            raise AttributeError('not found env: SIDEFXLABS')

        isInPathCondition = pathFeELib in libraryFilePath

        if not isInPathCondition:
            return False

    return True


hou.Node.isSideFXLabsNode = isSideFXLabsNode
hou.NodeType.isSideFXLabsNode = isSideFXLabsNode
hou.HDADefinition.isSideFXLabsNode = isSideFXLabsNode


def isSideFXNode(inputNodeType, ignoreDSO=True) -> bool:
    HFS = hou.getenv('HFS')
    if HFS is None:
        raise AttributeError('dont have HFS env')

    nodeType = convertNodeType(inputNodeType)
    defaultLibPath = HFS + r'/houdini/otls/'
    defi = nodeType.definition()
    if ignoreDSO and defi is None:
        return False
    return defi.libraryFilePath().startswith(defaultLibPath)


hou.Node.isSideFXNode = isSideFXNode
hou.NodeType.isSideFXNode = isSideFXNode
hou.HDADefinition.isSideFXNode = isSideFXNode


def isSideFXDefinition(defi) -> bool:
    HFS = hou.getenv('HFS')
    if HFS is None:
        raise AttributeError('dont have HFS env')
    defaultLibPath = HFS + r'/houdini/otls/'
    return defi.libraryFilePath().startswith(defaultLibPath)


hou.HDADefinition.isSideFXDefinition = isSideFXDefinition


def checkHideFeENode(keepHide=True, detectName=True, detectPath=False):
    # keepHide is False use for IsNodeHiding mode
    TEMP_path = hou.getenv('TEMP')
    isHidingTXT_path = TEMP_path + '/isHidingFeENode.txt'

    if os.path.exists(isHidingTXT_path):
        with open(isHidingTXT_path, "r") as isHidingTXT:
            isHiding = int(isHidingTXT.read(1)) == 1
            # print(isHiding)

        if not keepHide:
            with open(isHidingTXT_path, "w") as isHidingTXT:
                isHidingTXT.write('0' if isHiding else '1')
    else:
        isHiding = False
        if not keepHide:
            # os.makedirs(isHidingTXT_path)
            with open(isHidingTXT_path, "w") as isHidingTXT:
                isHidingTXT.write('1')

    deprecatedNodeList = list(readDeprecatedNode())
    deprecatedNodeList.extend(readNeedHideNode())
    # print(deprecatedNodeList)

    nodeTypeCategories = hou.nodeTypeCategories()

    for nodeTypeCategoriesKey in nodeTypeCategories:
        # run over all nodeType
        dictNodeTypes = nodeTypeCategories[nodeTypeCategoriesKey].nodeTypes()
        for nodeTypeDictsKey in dictNodeTypes:
            nodeType = dictNodeTypes[nodeTypeDictsKey]

            nodeTypeName = nodeType.name()
            if nodeTypeName in deprecatedNodeList:
                # print(nodeTypeName)
                nodeType.setHidden(True)
                continue

    if keepHide:
        if isHiding:
            hideToggle = True
        else:
            return
    else:
        hideToggle = not isHiding

    for nodeTypeCategoriesKey in nodeTypeCategories:
        # run over all nodeType
        dictNodeTypes = nodeTypeCategories[nodeTypeCategoriesKey].nodeTypes()
        for nodeTypeDictsKey in dictNodeTypes:
            nodeType = dictNodeTypes[nodeTypeDictsKey]
            nodeTypeName = nodeType.name()
            if nodeTypeName in deprecatedNodeList:
                nodeType.setHidden(True)
                continue

            '''
            defi = nodeType.definition()
            if not defi:
                continue
            '''

            if not isFeENode(nodeType, detectName=detectName, detectPath=detectPath):
                continue

            nodeType.setHidden(hideToggle)
            """
            if 1:
                #if nodeType.hidden():
            else:
                pass
                # operation = 'opunhide' if isHiding else 'ophide'
                # hou.hscript(operation + ' Sop ' + nodeType.name())
            """


def findAllSubParmRawValue(subnet, strValue):
    for child in subnet.allSubChildren(recurse_in_locked_nodes=False):
        for parm in child.parms():
            if strValue in parm.rawValue():
                print(child)
                print(parm)


hou.Node.findAllSubParmRawValue = findAllSubParmRawValue


def removeCategoryEmbedded(nodeTypeCategory=hou.nodeTypeCategories()['Sop'], displayConfirmation=False):
    if displayConfirmation:
        fee_Utils.displayConfirmation()

    dictNodeTypes = nodeTypeCategory.nodeTypes()

    for nodeTypeDictsKey in dictNodeTypes:
        nodeType = dictNodeTypes[nodeTypeDictsKey]
        defi = nodeType.definition()
        if not defi:
            continue
        if defi.libraryFilePath() == 'Embedded':
            defi.destroy()


def removeEmbedded(displayConfirmation=False):
    if displayConfirmation:
        fee_Utils.displayConfirmation()

    nodeTypeCategories = hou.nodeTypeCategories()

    for nodeTypeCategoryKey in nodeTypeCategories:
        nodeTypeCategory = nodeTypeCategories[nodeTypeCategoryKey]
        removeCategoryEmbedded(nodeTypeCategory=nodeTypeCategory)


def hasEmbedded(netCategories) -> bool:
    dictNodeTypes = netCategories.nodeTypes()
    for nodeTypeDictsKey in dictNodeTypes:
        secNodeType = dictNodeTypes[nodeTypeDictsKey]
        defi = secNodeType.definition()
        if not defi:
            continue
        if defi.libraryFilePath() == 'Embedded':
            return True
    return False


"""
def TABSubmenuPathFromDefi(defi):
    sections = defi.sections()
    try:
        sectionToolShelf = sections[r'Tools.shelf']
    except:
        print('not found section: Tools Shelf:', end = ' ')
        print(nodeType)
    contents = sectionToolShelf.contents()#hou.compressionType.NoCompression
    #print(contents)

    '''
    try:
        import xml.etree.cElementTree as ET
    except ImportError:
        import xml.etree.ElementTree as ET
    '''
    import xml.etree.ElementTree as ET

    tree = ET.fromstring(str(contents))
    toolSubmenu = tree.find('tool').find('toolSubmenu').text
"""


def TABSubmenuPathFromContents(contents):
    # try:
    #     import xml.etree.cElementTree as ET
    # except ImportError:
    #     import xml.etree.ElementTree as ET
    import xml.etree.ElementTree as ET

    tree = ET.fromstring(str(contents))
    toolSubmenu = tree.find('tool').find('toolSubmenu').text


def TABSubmenuPathFromSectionToolShelf(sectionToolShelf):
    contents = sectionToolShelf.contents()  # hou.compressionType.NoCompression
    return TABSubmenuPathFromContents(contents)


def TABSubmenuPathFromSections(sections):
    try:
        sectionToolShelf = sections[r'Tools.shelf']
        return TABSubmenuPathFromSectionToolShelf(sectionToolShelf)
    except:
        print('not found section: Tools Shelf:', end=' ')
        return None


def TABSubmenuPathFromDefi(defi):
    sections = defi.sections()
    return TABSubmenuPathFromSections(sections)


hou.HDADefinition.TABSubmenuPathFromDefi = TABSubmenuPathFromDefi


def extractAllUsedDefiToEmbedded(ignoreNodeTypeNames=(), ignoreFeEHDA=False):
    fee_Utils.displayConfirmation(
        prevText="This tool can copy all used HDA under all selectedNodes's allSubChildren to otls/ folder by the HIP path and also can convert those HDA to Black Box \n这个工具可以把所有 selectedNodes(选择的节点) 的 allSubChildren(子节点) 内用到的HDA复制到HIP路径下的otls/文件夹内 并可选转为Black Box")

    toBlackBox = hou.ui.displayMessage(
        "Convert to Black Box?\n转为黑盒？",
        buttons=("yes", "no"),
        default_choice=0,
        close_choice=0)

    toBlackBox = toBlackBox == 0

    hdaFileType = hou.ui.displayMessage(
        "HDA File Type\nHDA类型？",
        buttons=('Seperated', 'All in One'),
        default_choice=1,
        close_choice=1)

    hdaFileType = hdaFileType == 1

    HIPPath = hou.hipFile.path()
    save_OTLFolderPath = os.path.split(HIPPath)[0] + '/otls/'
    if os.path.exists(save_OTLFolderPath):
        # print(save_OTLFolderPath)
        # print(os.listdir(save_OTLFolderPath))
        for subDir in os.listdir(save_OTLFolderPath):
            subFullDir = save_OTLFolderPath + subDir
            if not os.path.isfile(subFullDir):
                continue
            # print(os.path.splitext(subDir))
            if os.path.splitext(subDir)[1].lower() != '.hda':
                continue
            try:
                hou.hda.uninstallFile(subFullDir)
            except:
                pass
            os.remove(subFullDir)
    else:
        os.mkdir(save_OTLFolderPath)

    save_OTLPath = save_OTLFolderPath
    if toBlackBox:
        save_OTLPath += '/allUsedHDAs_blackBox.hda'
    else:
        save_OTLPath += '/allUsedHDAs.hda'

    if not os.path.exists(save_OTLFolderPath):
        os.makedirs(save_OTLFolderPath)

    convertedDefi = []
    if hdaFileType:
        newHDAFilePaths = [save_OTLPath]
    else:
        newHDAFilePaths = []

    allowEditingOfContentsNodes = []

    selectedNodes = hou.selectedNodes()
    for selectedNode in selectedNodes:
        for allSubChild in selectedNode.allSubChildren():
            if not allSubChild.matchesCurrentDefinition():
                continue
            nodeType = allSubChild.type()
            defi = nodeType.definition()
            if defi is None:
                continue
            if defi in convertedDefi:
                continue
            # if fee_HDA.isSideFXDefinition(defi):
            if isSideFXDefinition(defi):
                continue
            if ignoreFeEHDA:
                # if fee_HDA.isFeENode(nodeType):
                if isFeENode(nodeType):
                    continue
            nodeTypeName = allSubChild.type().name()
            if nodeTypeName in ignoreNodeTypeNames:
                continue

            if not hdaFileType:
                defiPath = defi.libraryFilePath()
                save_OTLPath = save_OTLFolderPath + '/' + os.path.split(defiPath)[1]

            allSubChild.allowEditingOfContents(propagate=True)
            allowEditingOfContentsNodes.append(allSubChild)
            if toBlackBox:
                defi.save(save_OTLPath, template_node=allSubChild, compile_contents=True, black_box=True,
                          create_backup=False)
                # defi.save(save_OTLPath, compile_contents=False, black_box=True, create_backup=False)
            else:
                defi.save(save_OTLPath, template_node=allSubChild, create_backup=False)

            convertedDefi.append(defi)

            if not hdaFileType and save_OTLPath not in newHDAFilePaths:
                newHDAFilePaths.append(save_OTLPath)

    for node in allowEditingOfContentsNodes:
        node.matchCurrentDefinition()

    for newHDAFilePath in newHDAFilePaths:
        hou.hda.installFile(newHDAFilePath)

    # hou.hda.uninstallFile(defiPath)
    # convertedDefi = list(set(convertedDefi)) #去重


def expandHDA(origHDAPath: str, targetHDAPath: str) -> str:
    if not os.path.exists(origHDAPath):
        raise ValueError("HDA '{}' not exist".format(origHDAPath))
    if os.path.isdir(origHDAPath):
        raise ValueError("HDA '{}' is expanded".format(origHDAPath))

    hou.hda.expandToDirectory(origHDAPath, targetHDAPath)
    return targetHDAPath
    # hou.hda.expandToDirectory(newLibraryFilePath, expandHDAPath)
    # hscriptCommand = ' '.join((r"hotl -t", expandHDAPath, newLibraryFilePath))
    # hscriptCommand = ' '.join((r"otexpand", newLibraryFilePath, expandHDAPath))
    # print(hscriptCommand)
    # hou.hscript(hscriptCommand)
