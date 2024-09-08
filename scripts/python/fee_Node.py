# import fee_Node
# from importlib import reload
# reload(fee_Node)

import hou
import fee_Utils
import fee_HDA


# from importlib import reload
# reload(fee_HDA)


def convertNodeTuple(node):
    if isinstance(node, hou.Node):
        return (node,)
    elif isinstance(node, list):
        return tuple(node)
    elif isinstance(node, tuple):
        for idx in range(len(node)):
            if not isinstance(node[idx], hou.Node):
                raise TypeError('Only Support hou.Node or hou.Node Tuple/List Type')
        return node

    raise TypeError('Only Support hou.Node or hou.Node Tuple/List Type')


hou.Node.convertNodeTuple = convertNodeTuple


def getUserWantedSelectedNodes(kwargsNode):
    selectedNodes = hou.selectedNodes()
    if kwargsNode in selectedNodes:
        return selectedNodes
    else:
        return (kwargsNode,)


hou.Node.getUserWantedSelectedNodes = getUserWantedSelectedNodes


def addSpareInput(kwargsNode):
    selectedNodesTuple = convertNodeTuple(kwargsNode)
    for selectedNode in selectedNodesTuple:
        pass
        # selectedNode.addSpareParmTuple(parm_template, in_folder=(), create_missing_folders=False)


hou.Node.addSpareInput = addSpareInput


def isEqual_networkChildren(node0, node1, recurseInNetwork=True, checkNodeType=True):
    if not isinstance(node0, hou.Node):
        raise TypeError('input node0 must be hou.Node')
    if not isinstance(node1, hou.Node):
        raise TypeError('input node1 must be hou.Node')

    for child in node0.children():
        childName = child.name()
        child_ref = node1.node(childName)
        if child_ref is None:
            # print(childName)
            return False

        childType = child.type()
        child_refType = child_ref.type()
        if checkNodeType and childType != child_refType:
            return False

        childTypeName = childType.name()
        if childTypeName != 'subnet':
            continue

        child_refTypeName = child_refType.name()
        if childTypeName != 'subnet' and child_refTypeName != 'subnet' and child.matchesCurrentDefinition() and child_ref.matchesCurrentDefinition():
            continue

        if recurseInNetwork and not isEqual_networkChildren(child, child_ref, recurseInNetwork, checkNodeType):
            return False

    return True


hou.Node.isEqual_networkChildren = isEqual_networkChildren


def changeNodeType_keepIO(node, targetNodeType, keep_parms=True):
    # print(targetNodeType)
    if not isinstance(targetNodeType, str):
        raise TypeError('targetNodeType must be string', targetNodeType)

    # 记录flag情况
    parent = node.parent()
    # print(parent.childTypeCategory().name())
    if parent.childTypeCategory().name() != 'Sop':
        raise TypeError('error')
    if node.isHardLocked():
        raise TypeError('isHardLocked')
    if node.isSoftLocked():
        raise TypeError('isSoftLocked')

    displayFlag = node == parent.displayNode()
    renderFlag = node == parent.renderNode()
    bypass = node.isBypassed()
    isTemplateFlagSet = node.isTemplateFlagSet()
    isHighlightFlagSet = node.isHighlightFlagSet()
    isSelectableTemplateFlagSet = node.isSelectableTemplateFlagSet()
    isUnloadFlagSet = node.isUnloadFlagSet()

    ########### rest outputs ############ 没有这部分也能跑，只是连接比较丑
    outputNodes = node.outputs()
    outputNodes = list(set(outputNodes))

    outputNodes_inputItems = []
    outputNodes_inputItem_output_index = []
    for idx in range(len(outputNodes)):

        outputNodes_inputConnectors = outputNodes[idx].inputConnectors()
        outputNodes_inputItems.append([])
        outputNodes_inputItem_output_index.append([])

        # for idy in range(len(outputNodes_inputConnectors)):
        for outputNodes_inputConnector in outputNodes_inputConnectors:
            if len(outputNodes_inputConnector) == 0:
                outputNodes_inputItems[idx].append(None)
                outputNodes_inputItem_output_index[idx].append(None)
                continue
            outputNodes_inputConnection = outputNodes_inputConnector[0]
            outputNodes_inputItem = outputNodes_inputConnection.inputItem()
            # outputNodes_inputItems[idx].append(outputNodes_inputItem)
            outputNodes_inputItems[idx].append(outputNodes_inputItem.name())
            if isinstance(outputNodes_inputItem, hou.NetworkDot) or isinstance(outputNodes_inputItem,
                                                                               hou.SubnetIndirectInput):
                outputNodes_inputItem_output_index[idx].append(0)
            else:
                outputNodes_inputItem_output_index[idx].append(outputNodes_inputConnection.outputIndex())

    ########### rest hou.NetworkDot outputs ############ 没有这部分也能跑，只是连接比较丑

    nodeName = node.name()
    outputConnectors = node.outputConnectors()
    for idx in range(len(outputConnectors)):
        # for outputConnector in outputConnectors:
        for outputConnection in outputConnectors[idx]:
            outputItem = outputConnection.outputItem()
            if not isinstance(outputItem, hou.NetworkDot):
                continue
            outputNodes.append(outputItem)
            outputNodes_inputItems.append([nodeName, ])
            outputNodes_inputItem_output_index.append([idx, ])

    inputConnectors = node.inputConnectors()
    outputNodes.append(nodeName)
    outputNodes_inputItems.append([])
    outputNodes_inputItem_output_index.append([])
    for idx in range(len(inputConnectors)):
        # for outputConnector in inputConnectors:
        if len(inputConnectors[idx]) == 0:
            continue
        inputConnection = inputConnectors[idx][0]
        inputItem = inputConnection.inputItem()

        outputNodes_inputItems[-1].append(inputItem)
        if isinstance(inputItem, hou.NetworkDot) or isinstance(inputItem, hou.SubnetIndirectInput):
            outputNodes_inputItem_output_index[-1].append(0)
        else:
            outputNodes_inputItem_output_index[-1].append(inputConnection.outputIndex())

    ############ 修改节点类型 ############
    origNodeshape = node.userData('nodeshape')
    copyOrigNode = hou.copyNodesTo([node], parent)[0]
    newNode = node.changeNodeType(targetNodeType, keep_parms=keep_parms)
    newNode.matchCurrentDefinition()
    if origNodeshape is not None:
        newNode.setUserData('nodeshape', origNodeshape)
    # newNode.removeSpareParms()

    '''

    ############ 处理0-3号输入口 ########### 没有这部分也能跑，只是连接比较丑

    ############ set Input ###############
    for idx in range(min(nInputs, 4)):
        if inputItems[idx] is None:
            continue
        newNode.setInput(idx, inputItems[idx], inputItem_output_index[idx])
        """
        try:
            newNode.setInput(idx, inputItems[idx], inputItem_output_index[idx])
        except:
            print(newNode)
            print(inputItems[idx])
            print(inputItem_output_index[idx])
            raise ValueError('setInput Error')
        """
    ############ set position ###############
    oldIndirectInputs = copyOrigNode.indirectInputs()
    newIndirectInputs = newNode.indirectInputs()
    for idx in range(0, min(len(oldIndirectInputs), 4)):
        pos = oldIndirectInputs[idx].position()
        newIndirectInputs[idx].setPosition(pos)
        """
        try:
            pos = oldIndirectInputs[idx].position()
            newIndirectInputs[idx].setPosition(pos)
        except:
            print(copyOrigNode)
            print(newNode)
            print(oldIndirectInputs[idx])
            print(newIndirectInputs[idx])
            raise ValueError('fee Error')
        """

    '''

    ########### 处理输出口 ############ 没有这部分也能跑，只是连接比较丑
    if 1:
        parent = newNode.parent()
        # print(nodeName)
        # print(outputNodes)
        # print(outputNodes_inputItems)
        # print(outputNodes_inputItem_output_index)
        for idx in range(len(outputNodes)):
            if isinstance(outputNodes[idx], str):
                outputNodes[idx] = parent.item(outputNodes[idx])

            for idy in range(len(outputNodes_inputItems[idx])):
                if outputNodes_inputItems[idx][idy] is None:
                    continue
                if isinstance(outputNodes_inputItems[idx][idy], str):
                    outputNodes_inputItems[idx][idy] = parent.item(outputNodes_inputItems[idx][idy])

                outputNodes[idx].setInput(idy, outputNodes_inputItems[idx][idy],
                                          outputNodes_inputItem_output_index[idx][idy])
                # try:
                #     outputNodes[idx].setInput(idy, outputNodes_inputItems[idx][idy], outputNodes_inputItem_output_index[idx][idy])
                # except:
                #     print(idy)
                #     print(outputNodes_inputItems[idx][idy])
                #     print(outputNodes_inputItem_output_index[idx][idy])
                #     raise TypeError('1')

    newNode.setDisplayFlag(displayFlag)
    newNode.setRenderFlag(renderFlag)
    newNode.bypass(bypass)
    newNode.setTemplateFlag(isTemplateFlagSet)
    newNode.setHighlightFlag(isHighlightFlagSet)
    newNode.setSelectableTemplateFlag(isSelectableTemplateFlagSet)
    newNode.setUnloadFlag(isUnloadFlagSet)

    copyOrigNode.destroy()

    return newNode


hou.Node.changeNodeType_keepIO = changeNodeType_keepIO


def changeNodeTypeToSubnet(node, keep_parms=True):
    newNode = node.changeNodeType('subnet', keep_parms=keep_parms)
    return newNode


hou.Node.changeNodeTypeToSubnet = changeNodeTypeToSubnet


# def parmTemplateIsHidden(parmTemplate):
#     if not isinstance(parmTemplate, hou.ParmTemplate):
#         raise TypeError('input not hou.Parm')

#     return isinstance(parmTemplate, hou.MenuParmTemplate) and parmTemplate.isHidden()


def copyParms_NodetoNode(sourceNode, targetNode, copyNoneExistParms=False, ignoreInvisibleParms=False, debugMode=0):
    if copyNoneExistParms:
        sourceParmTemplateGroup = sourceNode.parmTemplateGroup()
        targetParmTemplateGroup = targetNode.parmTemplateGroup()
        # copyParmTemplates = []
        for sourceParmTemplate in sourceParmTemplateGroup.parmTemplates():
            if ignoreInvisibleParms and sourceParmTemplate.isHidden():
                continue
            sourceParmTemplateName = sourceParmTemplate.name()
            targetParmTemplate = targetParmTemplateGroup.find(sourceParmTemplateName)
            if targetParmTemplate is None:
                targetParmTemplateGroup.append(sourceParmTemplate)
                # copyParmTemplates.append(sourceParmTemplate)

        # for copyParmTemplate in copyParmTemplates:
        #     targetParmTemplateGroup.append(copyParmTemplate)

        try:
            targetNode.setParmTemplateGroup(targetParmTemplateGroup, rename_conflicting_parms=False)
        except:
            print(targetNode)
            print(targetParmTemplateGroup)
            print(
                "hou.OperationFailed: Parameters don't support MinMax, MaxMin, StartEnd, BeginEnd, or XYWH parmNamingSchemes")
            if debugMode == 1:
                # 如果运行到这里，经常会崩溃
                raise SystemError(
                    "hou.OperationFailed: Parameters don't support MinMax, MaxMin, StartEnd, BeginEnd, or XYWH parmNamingSchemes")

        # print(sourceParmTemplateGroup.asDialogScript())
        '''
        if 1:
            targetNode.setParmTemplateGroup(sourceParmTemplateGroup, rename_conflicting_parms=False)
        else:
            try:
                targetNode.setParmTemplateGroup(sourceParmTemplateGroup, rename_conflicting_parms=False)
            except:
                # Parameters don't support MinMax, MaxMin, StartEnd, BeginEnd, or XYWH parmNamingSchemes
                print(sourceNode, targetNode)
                print(sourceParmTemplateGroup.asDialogScript())
        '''

    for parm in sourceNode.parms():
        if ignoreInvisibleParms and not parm.isVisible():
            # print(parm)
            continue

        if parm.parmTemplate().type() == hou.parmTemplateType.Folder:
            parmVal = parm.evalAsInt()
            # print(parmVal)
            # print(parm.multiParmStartOffset())
            parentFolder = targetNode.parm(parm.name())
            if parentFolder is None:
                continue
            # parentFolder.setFromParm(parm)
            for idx in range(0, parentFolder.evalAsInt()):
                parentFolder.removeMultiParmInstance(0)
            for idx in range(0, parmVal):
                parentFolder.insertMultiParmInstance(idx)
            # if sourceParmTemplate.type() == hou.parmTemplateType.Folder and sourceParmTemplate.folderType() == hou.folderType.MultiparmBlock:
            #     sourceParmTemplate.set
            '''
            try:
                parmVal = parm.evalAsInt()
                #print(parmVal)
                #print(parm.multiParmStartOffset())
                parentFolder = targetNode.parm(parm.name())
                if parentFolder is None:
                    continue
                #parentFolder.setFromParm(parm)
                for idx in range(0, parentFolder.evalAsInt()):
                    parentFolder.removeMultiParmInstance(0)
                for idx in range(0, parmVal):
                    parentFolder.insertMultiParmInstance(idx)
            except:
                print('')
                #print(targetNode)
                print(parm)
            '''

    if 0:
        for parm in targetNode.parms():
            if parm.isMultiParmInstance():
                print(parm)

    for targetparm in targetNode.parms():
        if ignoreInvisibleParms and not targetparm.isVisible():
            # print(targetparm)
            continue
        # break
        sourceparm = sourceNode.parm(targetparm.name())
        if sourceparm is None:
            continue
        sourceParmTemplate = sourceparm.parmTemplate()
        if sourceParmTemplate.type() == hou.parmTemplateType.Folder and sourceParmTemplate.folderType() == hou.folderType.MultiparmBlock:
            try:
                targetparm.setExpression(sourceparm.expression(), sourceparm.expressionLanguage())
            except:
                pass
        else:
            targetparm.deleteAllKeyframes()
            targetparm.setFromParm(sourceparm)
        '''
        try:
            sourceparm = sourceNode.parm(targetparm.name())
            if sourceparm is None:
                continue
            targetparm.deleteAllKeyframes()
            targetparm.setFromParm(sourceparm)
        except:
            #print(targetNode)
            #print(targetNode.parm(targetparm.name()))
            print(targetparm)
        '''
    '''
    for sourceparm in sourceNode.parms():
        #break
        try:
            targetparm = targetNode.parm(sourceparm.name())
            if targetparm is None:
                continue
            targetparm.deleteAllKeyframes()
            targetparm.setFromParm(sourceparm)
        except:
            #print(targetNode)
            #print(targetNode.parm(sourceparm.name()))
            print(sourceparm)
    '''


def copyRelRef_NodetoNode(sourceNode, targetNode, prefix='', sufix='', ignoreInvisibleParms=False):
    for sourceParm in sourceNode.parms():
        if ignoreInvisibleParms and not sourceParm.isVisible():
            # print(sourceParm)
            continue
        parmName = sourceParm.name()
        targetParm = targetNode.parm(prefix + parmName + sufix)
        if targetParm is not None:
            targetParm.deleteAllKeyframes()
            targetParm.set(sourceParm)


def bake_optypeExp_to_str(targetNode, sourceNode, optype_exp, optype_exp1):
    relativePathTo = targetNode.relativePathTo(sourceNode)
    str_optype0 = r"optype('" + relativePathTo + r"/.')"  # r"optype('../.')"
    str_optype1 = r"optype('" + relativePathTo + r"')"  # r"optype('..')"
    str_py_optype0 = r"hou.node('" + relativePathTo + r"/.').type().nameComponents()[2]"  # r"hou.node('../.').type().nameComponents()[2]"
    str_py_optype1 = r"hou.node('" + relativePathTo + r"').type().nameComponents()[2]"  # r"hou.node('..').type().nameComponents()[2]"
    # print(str_py_optype0)

    parms = targetNode.parms()
    for parm in parms:
        rawValue = parm.rawValue()
        try:
            expLang = parm.expressionLanguage()
        except:
            # 没有exp
            if parm.parmTemplate().dataType() == hou.parmData.String:
                # 是string类型
                if r'`' + str_optype0 + r'`' in rawValue:
                    rawValue = rawValue.replace(r'`' + str_optype0 + r'`', optype_exp)
                    parm.set(rawValue)
                if r'`' + str_optype1 + r'`' in rawValue:
                    rawValue = rawValue.replace(r'`' + str_optype1 + r'`', optype_exp)
                    parm.set(rawValue)
                if str_optype0 in rawValue:
                    rawValue = rawValue.replace(str_optype0, optype_exp1)
                    parm.set(rawValue)
                if str_optype1 in rawValue:
                    rawValue = rawValue.replace(str_optype1, optype_exp1)
                    parm.set(rawValue)
        else:
            # 有exp
            if expLang == hou.exprLanguage.Python:
                if str_py_optype0 in rawValue:
                    rawValue = rawValue.replace(str_py_optype0, optype_exp1)
                    parm.deleteAllKeyframes()
                    parm.setExpression(rawValue, hou.exprLanguage.Python, True)
                if str_py_optype1 in rawValue:
                    rawValue = rawValue.replace(str_py_optype1, optype_exp1)
                    parm.deleteAllKeyframes()
                    parm.setExpression(rawValue, hou.exprLanguage.Python, True)

            elif expLang == hou.exprLanguage.Hscript:
                if str_optype0 in rawValue:
                    rawValue = rawValue.replace(str_optype0, optype_exp1)
                    parm.deleteAllKeyframes()
                    parm.setExpression(rawValue, hou.exprLanguage.Hscript, True)
                if str_optype1 in rawValue:
                    rawValue = rawValue.replace(str_optype1, optype_exp1)
                    parm.deleteAllKeyframes()
                    parm.setExpression(rawValue, hou.exprLanguage.Hscript, True)


def deleteAllSubNodeMatchesType(node, inputDeleteNodeType):
    if isinstance(inputDeleteNodeType, list) or isinstance(inputDeleteNodeType, str):
        deleteNodeType = tuple(inputDeleteNodeType)
    else:
        deleteNodeType = inputDeleteNodeType

    if not isinstance(deleteNodeType, tuple):
        raise TypeError('deleteNodeType must be sequence')

    if len(deleteNodeType) == 0:
        return False

    if not isinstance(deleteNodeType[0], str):
        raise TypeError('deleteNodeType must be string')

    for childNode in node.children():

        if childNode.matchesCurrentDefinition():
            continue
        childType = childNode.type()
        childTypeName = childType.name()
        if childTypeName in deleteNodeType:
            # parentNode = childNode.parent()
            # parentNode.allowEditingOfContents(True)
            try:
                childNode.destroy()
            except:
                # print(childNode)
                raise RuntimeError('cant delete node' + childNode.path())

        # elif not childNode.matchesCurrentDefinition():
        else:
            deleteAllSubNodeMatchesType(childNode, inputDeleteNodeType)

    return True


hou.Node.deleteAllSubNodeMatchesType = deleteAllSubNodeMatchesType


def changeAllSubNodeTypeMatchesType(node, changeNodeTypeDict):
    if not isinstance(changeNodeTypeDict, dict):
        raise TypeError('changeNodeTypeDict must be dict')

    for childNode in node.children():
        childType = childNode.type()
        childTypeName = childType.name()
        if childTypeName in changeNodeTypeDict:
            childNode.parent().allowEditingOfContents(propagate=True)
            newNode = childNode.changeNodeType(changeNodeTypeDict[childTypeName], keep_parms=True)
        # elif not childNode.matchesCurrentDefinition():
        else:
            newNode = childNode
        changeAllSubNodeTypeMatchesType(newNode, changeNodeTypeDict)


hou.Node.changeAllSubNodeTypeMatchesType = changeAllSubNodeTypeMatchesType


def change_nodeTypeName_namespace(nodeTypeName: str, target_namespace: str) -> str:
    nodeTypeNameComponents = fee_HDA.splitTypeNametoNameComponents(nodeTypeName)
    nodeTypeNameComponents[1] = target_namespace
    return fee_HDA.combineNameComponents(nodeTypeNameComponents)


def convertSubnet(node: hou.Node, ignoreUnlock=False, ignore_SideFX_HDA=True, nodeFilterFunc=None, convertFeENode=False,
                  detectName=True, detectPath=False, debugMode=0):
    if not isinstance(node, hou.Node):
        raise TypeError('invalid node', node)

    nodeType = node.type()
    nodeTypeName = nodeType.name()
    ##### 检测各种情况
    if nodeTypeName == 'subnet':
        convertSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode,
                                      detectName, detectPath, debugMode=debugMode)
        return False

    defi = nodeType.definition()
    if defi is None:
        if fee_HDA.isFeENode(nodeType, detectName, detectPath):
            targetNodeTypeName = change_nodeTypeName_namespace(nodeTypeName, "Custom")
            changeNodeType_keepIO(node, targetNodeTypeName)
        return False

    if ignoreUnlock and not node.matchesCurrentDefinition():
        # print(node)
        convertSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode,
                                      detectName, detectPath, debugMode=debugMode)
        return False

    if ignore_SideFX_HDA and fee_HDA.isSideFXDefinition(defi):
        convertSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode,
                                      detectName, detectPath, debugMode=debugMode)
        return False
    else:
        flag = not fee_HDA.isSideFXDefinition(defi)

    if flag and convertFeENode:
        if fee_HDA.isFeENode(nodeType, detectName, detectPath):
            flag = False

    if flag and nodeFilterFunc is not None:
        if nodeFilterFunc(node):
            flag = False

    if flag:
        convertSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode,
                                      detectName, detectPath, debugMode=debugMode)
        return False

    node.allowEditingOfContents()

    ##### 记录flag情况
    parent = node.parent()
    # print(parent.childTypeCategory().name())
    if parent.childTypeCategory().name() != 'Sop':
        raise TypeError('error')
    if node.isHardLocked():
        raise TypeError('isHardLocked')
    if node.isSoftLocked():
        raise TypeError('isSoftLocked')

    displayFlag = node == parent.displayNode()
    renderFlag = node == parent.renderNode()
    bypass = node.isBypassed()
    isTemplateFlagSet = node.isTemplateFlagSet()
    isHighlightFlagSet = node.isHighlightFlagSet()
    isSelectableTemplateFlagSet = node.isSelectableTemplateFlagSet()
    isUnloadFlagSet = node.isUnloadFlagSet()

    inputConnectors = node.inputConnectors()
    nInputs = len(inputConnectors)

    ############ 处理4号以后的输入口
    if nInputs > 4:
        shiftVector2 = hou.Vector2(0.0, -1.0)
        nulls = []

        ###算出nulls列表的所有元素，后面有用
        for idx in range(4, nInputs):
            if len(inputConnectors[idx]) == 0:
                # 说明没有连
                nulls.append(None)
                continue

            inputConnection = inputConnectors[idx][0]
            subnetIndirectInput = inputConnection.subnetIndirectInput()

            #### 查找输入的类型
            flag = False
            if subnetIndirectInput is None:
                inputNode = inputConnection.inputNode()
                if inputNode is None:
                    ####说明输入是一个个dot，最终连接到subnet input
                    inputItem = inputConnection.inputItem()
                    while type(inputItem).__name__ == 'NetworkDot':
                        ####寻找dot的最终链接
                        inputItem = inputItem.inputItem()

                    if type(inputItem).__name__ == 'SubnetIndirectInput':
                        subnetIndirectInput = inputItem
                        flag = True
                    else:
                        raise ValueError('fee : 不可能的情况')
                        # nulls.append(inputItem)
                        # flag = False
                else:
                    ####说明输入是一个node
                    nulls.append(inputNode)
                    flag = False
            else:
                ####说明输入是一个subnet input
                flag = True

            #### 建立null节点
            if flag:
                null = None
                for outputConnection in subnetIndirectInput.outputConnections():
                    outputNode = outputConnection.outputNode()
                    if outputNode.type().name() == 'null':
                        if outputNode.parm('copyinput').evalAsInt() == 1:
                            null = outputNode
                            nulls.append(null)
                            break

                if null is None:
                    if 1:
                        null = parent.createNode('null', exact_type_name=True)
                        null.setInput(0, subnetIndirectInput, output_index=0)
                    else:
                        try:
                            null = subnetIndirectInput.createOutputNode('null', exact_type_name=True)
                        except:
                            print(subnetIndirectInput)
                            raise ValueError('')
                    # null.setPosition(subnetIndirectInput.position().__add__(shiftVector2))
                    null.setPosition(subnetIndirectInput.position() + shiftVector2)
                    nulls.append(null)

        indirectInputs = node.indirectInputs()
        for idx in range(4, nInputs):
            if nulls[idx - 4] is None:  # 没有连
                continue
            objectMerge = node.createNode('object_merge', exact_type_name=True)
            objectMerge.parm('objpath1').set('../../' + nulls[idx - 4].name())
            objectMerge.setPosition(indirectInputs[idx].position())
            for outputConnection in indirectInputs[idx].outputConnections():
                outputNode = outputConnection.outputNode()
                outputNode.setInput(outputConnection.inputIndex(), objectMerge)

    ############ 处理0-3号输入口 ########### 没有这部分也能跑，只是连接比较丑
    inputItems = []
    inputItem_output_index = []
    for idx in range(min(nInputs, 4)):
        if len(inputConnectors[idx]) == 0:
            # 说明没有连
            inputItems.append(None)
            inputItem_output_index.append(None)
            continue
        inputConnection = inputConnectors[idx][0]
        inputItem = inputConnection.inputItem()
        inputItems.append(inputItem)
        if isinstance(inputItem, hou.NetworkDot) or isinstance(inputItem, hou.SubnetIndirectInput):
            inputItem_output_index.append(0)
        else:
            inputItem_output_index.append(inputConnection.outputIndex())

    ########### rest outputs ############ 没有这部分也能跑，只是连接比较丑
    outputNodes = node.outputs()
    outputNodes = list(set(outputNodes))

    outputNodes_inputItems = []
    outputNodes_inputItem_output_index = []
    for idx in range(len(outputNodes)):

        outputNodes_inputConnectors = outputNodes[idx].inputConnectors()
        outputNodes_inputItems.append([])
        outputNodes_inputItem_output_index.append([])

        # for idy in range(len(outputNodes_inputConnectors)):
        for outputNodes_inputConnector in outputNodes_inputConnectors:
            if len(outputNodes_inputConnector) == 0:
                outputNodes_inputItems[idx].append(None)
                outputNodes_inputItem_output_index[idx].append(None)
                continue
            outputNodes_inputConnection = outputNodes_inputConnector[0]
            outputNodes_inputItem = outputNodes_inputConnection.inputItem()
            # outputNodes_inputItems[idx].append(outputNodes_inputItem)
            outputNodes_inputItems[idx].append(outputNodes_inputItem.name())
            if isinstance(outputNodes_inputItem, hou.NetworkDot) or isinstance(outputNodes_inputItem,
                                                                               hou.SubnetIndirectInput):
                outputNodes_inputItem_output_index[idx].append(0)
            else:
                outputNodes_inputItem_output_index[idx].append(outputNodes_inputConnection.outputIndex())

    ########### rest hou.NetworkDot outputs ############ 没有这部分也能跑，只是连接比较丑

    nodeName = node.name()
    outputConnectors = node.outputConnectors()
    for idx in range(len(outputConnectors)):
        # for outputConnector in outputConnectors:
        for outputConnection in outputConnectors[idx]:
            outputItem = outputConnection.outputItem()
            if not isinstance(outputItem, hou.NetworkDot):
                continue
            outputNodes.append(outputItem)
            outputNodes_inputItems.append([nodeName, ])
            outputNodes_inputItem_output_index.append([idx, ])

    ############ 修改节点类型 ############
    # origNodeshape = node.userData('nodeshape')
    copyOrigNode = hou.copyNodesTo([node], parent)[0]  ## , run_init_scripts=False, load_contents=False
    newNode = node.changeNodeType('subnet', keep_parms=False)
    newNode.removeSpareParms()

    ############ 处理0-3号输入口 ########### 没有这部分也能跑，只是连接比较丑

    ############ set Input ###############
    for idx in range(min(nInputs, 4)):
        if inputItems[idx] is None:
            continue
        newNode.setInput(idx, inputItems[idx], inputItem_output_index[idx])
        """
        try:
            newNode.setInput(idx, inputItems[idx], inputItem_output_index[idx])
        except:
            print(newNode)
            print(inputItems[idx])
            print(inputItem_output_index[idx])
            raise ValueError('setInput Error')
        """
    ############ set position ###############
    oldIndirectInputs = copyOrigNode.indirectInputs()
    newIndirectInputs = newNode.indirectInputs()
    for idx in range(0, min(len(oldIndirectInputs), 4)):
        pos = oldIndirectInputs[idx].position()
        newIndirectInputs[idx].setPosition(pos)
        """
        try:
            pos = oldIndirectInputs[idx].position()
            newIndirectInputs[idx].setPosition(pos)
        except:
            print(copyOrigNode)
            print(newNode)
            print(oldIndirectInputs[idx])
            print(newIndirectInputs[idx])
            raise ValueError('fee Error')
        """

    ########### 处理输出口 ############ 没有这部分也能跑，只是连接比较丑
    if 1:
        parent = newNode.parent()
        # print(outputNodes)
        # print(outputNodes_inputItems)
        # print(outputNodes_inputItem_output_index)
        for idx in range(len(outputNodes)):
            for idy in range(len(outputNodes_inputItems[idx])):
                if outputNodes_inputItems[idx][idy] is None:
                    continue
                outputNodes_inputItems[idx][idy] = parent.item(outputNodes_inputItems[idx][idy])
                outputNodes[idx].setInput(idy, outputNodes_inputItems[idx][idy],
                                          outputNodes_inputItem_output_index[idx][idy])

    # if origNodeshape is not None:
    # 这个是自动的啦
    # pass
    # newNode.setUserData('nodeshape', origNodeshape)

    ############### hide subnet input label parm #################
    inputLabels = copyOrigNode.inputLabels()
    # print(newNode.parm('label1'))
    # newNode.parm('label1').hide(True)
    # newNode.parm('label2').hide(True)
    # newNode.parm('label3').hide(True)
    # newNode.parm('label4').hide(True)
    for idx in range(4):
        inputLabel = newNode.parm('label' + str(idx + 1))
        inputLabel.hide(True)
        if idx < len(inputLabels):
            inputLabel.set(inputLabels[idx])
        else:
            inputLabel.set('')

    parmTemplateGroup = newNode.parmTemplateGroup()
    for idx in range(4):
        parmName = 'label' + str(idx + 1)
        inputLabel = parmTemplateGroup.find(parmName)
        inputLabel.hide(True)
        parmTemplateGroup.replace(parmName, inputLabel)
    newNode.setParmTemplateGroup(parmTemplateGroup)

    ############### parm type #################
    # newNode.parm('standardfolder').hide(True)
    if 1:
        tmpParm = hou.StringParmTemplate('tmpParm_origNodeType_createdByPy', 'Temp Parm Original Node Type', 1,
                                         default_value=(nodeTypeName,))
        tmpParm.hide(True)
        newNode.addSpareParmTuple(tmpParm)
        copyParms_NodetoNode(copyOrigNode, newNode, copyNoneExistParms=True, debugMode=debugMode)
    else:
        ### 这样顺序反过来，就会丢失linked channal链接关系哦，所以不能这样写
        copyParms_NodetoNode(copyOrigNode, newNode, copyNoneExistParms=True, debugMode=debugMode)
        tmpParm = hou.StringParmTemplate('tmpParm_origNodeType_createdByPy', 'Temp Parm Original Node Type', 1,
                                         default_value=(nodeTypeName,))
        tmpParm.hide(True)
        newNode.addSpareParmTuple(tmpParm)

    # nameComponents = nodeType.nameComponents()
    # nameComponents[2]
    '''
    targetParmTemplateGroup = newNode.parmTemplateGroup()
    newNodeParmTemplateGroup = newNode.parmTemplateGroup()
    newNodeParmTemplateGroup.append(tmpParm)
    newNode.setParmTemplateGroup(newNodeParmTemplateGroup)
    '''
    # newNode.removeSpareParmFolder(('Standard', ))
    # newNode.removeSpareParmTuple(newNode.parmTuple('Standard'))

    newNode.setDisplayFlag(displayFlag)
    newNode.setRenderFlag(renderFlag)
    newNode.bypass(bypass)
    newNode.setTemplateFlag(isTemplateFlagSet)
    newNode.setHighlightFlag(isHighlightFlagSet)
    newNode.setSelectableTemplateFlag(isSelectableTemplateFlagSet)
    newNode.setUnloadFlag(isUnloadFlagSet)

    optype_exp = copyOrigNode.type().nameComponents()[2]
    optype_exp1 = '\'' + optype_exp + '\''
    bake_optypeExp_to_str(newNode, newNode, optype_exp, optype_exp1)
    convertSubnet_recurseSubChild(newNode, newNode, optype_exp, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc,
                                  convertFeENode, detectName, detectPath, debugMode=debugMode)
    # print(optype_exp)

    copyOrigNode.destroy()

    return True


hou.Node.convertSubnet = convertSubnet


def convertSubnet_recurseSubChild(sourceNode, recurseNode, optype_exp='', ignoreUnlock=False, ignore_SideFX_HDA=True,
                                  nodeFilterFunc=None, convertFeENode=True, detectName=True, detectPath=False,
                                  debugMode=0):
    nodeType = recurseNode.type()
    nodeTypeName = nodeType.name()
    defi = nodeType.definition()

    if nodeTypeName != 'subnet' and (recurseNode.matchesCurrentDefinition() or defi is None):
        return False

    if optype_exp == '':
        optype_exp = sourceNode.type().nameComponents()[2]

    optype_exp1 = '\'' + optype_exp + '\''

    for child in recurseNode.children():
        bake_optypeExp_to_str(child, sourceNode, optype_exp, optype_exp1)

        # relativePathTo = child.relativePathTo(sourceNode)
        # str_optype = r"optype('" + relativePathTo + r"/.')" # r"optype('../.')"
        # str_py_optype = r"hou.node('" + relativePathTo + r"/.').type().nameComponents()[2]" # r"hou.node('../.').type().nameComponents()[2]"
        # # print(str_py_optype)

        # parms = child.parms()
        # for parm in parms:
        #     rawValue = parm.rawValue()
        #     try:
        #         expLang = parm.expressionLanguage()
        #     except:
        #         #没有exp
        #         if parm.parmTemplate().dataType() == hou.parmData.String:
        #             #是string类型
        #             if r'`' + str_optype + r'`' in rawValue:
        #                 rawValue = rawValue.replace(r'`' + str_optype + r'`', optype_exp)
        #                 parm.set(rawValue)
        #             if str_optype in rawValue:
        #                 rawValue = rawValue.replace(str_optype, optype_exp1)
        #                 parm.set(rawValue)
        #     else:
        #         #有exp
        #         if expLang == hou.exprLanguage.Python:
        #             if str_py_optype in rawValue:
        #                 rawValue = rawValue.replace(str_py_optype, optype_exp1)
        #                 parm.deleteAllKeyframes()
        #                 parm.setExpression(rawValue, hou.exprLanguage.Python, True)
        #         elif expLang == hou.exprLanguage.Hscript:
        #             if str_optype in rawValue:
        #                 rawValue = rawValue.replace(str_optype, optype_exp1)
        #                 parm.deleteAllKeyframes()
        #                 parm.setExpression(rawValue, hou.exprLanguage.Hscript, True)

        #         #print(rawValue)

        # childNodeType = child.type()
        # childNodeTypeName = childNodeType.name()
        # #if "fee" in childNodeTypeName.lower() and child.matchesCurrentDefinition():

        convertSubnet(child, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath,
                      debugMode=debugMode)
        # if childNodeTypeName == 'subnet':
        #     convertSubnet_recurseSubChild(sourceNode, child, optype_exp, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath)
        #     continue

        # # if childNodeTypeName == 'deltamush::1.0':
        # #     print(fee_HDA.isFeENode(childNodeType, detectName, detectPath))
        # #     print(childNodeType)
        # #     print(detectName)
        # #     print(detectPath)
        # if convertFeENode:
        #     if fee_HDA.isFeENode(childNodeType, detectName, detectPath):
        #         convertSubnet(child, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath)
        #         continue

        # if nodeFilterFunc is not None:
        #     if nodeFilterFunc(child):
        #         convertSubnet(child, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath)
        #         continue

        # convertSubnet_recurseSubChild(sourceNode, child, optype_exp, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath)

    return True


def convertSubnet_custom(inputNodes, ignoreUnlock=True, ignore_SideFX_HDA=True, nodeFilterFunc=None,
                         convertFeENode=True, displayConfirmation=False, debugMode=0):
    if displayConfirmation:
        fee_Utils.displayConfirmation(prevText='plz backup HIP before do this\n建议先备份HIP')

    nodes = convertNodeTuple(inputNodes)
    for node in nodes:
        node.allowEditingOfContents()
        for child in node.children():
            convertSubnet(child, ignoreUnlock=ignoreUnlock, ignore_SideFX_HDA=ignore_SideFX_HDA,
                          nodeFilterFunc=nodeFilterFunc, convertFeENode=convertFeENode, detectName=True,
                          detectPath=False, debugMode=debugMode)


def convert_All_FeENode_to_Subnet(inputNodes, ignoreUnlock=True, ignore_SideFX_HDA=True, nodeFilterFunc=None,
                                  detectName=True, detectPath=False, displayConfirmation=False, debugMode=0):
    if displayConfirmation:
        fee_Utils.displayConfirmation(prevText='plz backup HIP before do this\n建议先备份HIP')

    nodes = convertNodeTuple(inputNodes)
    for node in nodes:
        node.allowEditingOfContents()
        for child in node.children():
            convertSubnet(child, ignoreUnlock=ignoreUnlock, ignore_SideFX_HDA=ignore_SideFX_HDA,
                          nodeFilterFunc=nodeFilterFunc, convertFeENode=True, detectName=detectName,
                          detectPath=detectPath, debugMode=debugMode)


def convert_All_HDA_to_Subnet(inputNodes: hou.Node, ignoreUnlock=True, ignore_SideFX_HDA=True,
                              displayConfirmation=False, debugMode=0):
    if displayConfirmation:
        fee_Utils.displayConfirmation(prevText='plz backup HIP before do this\n建议先备份HIP')

    def nodeFilter_allNode(node: hou.Node):
        if isinstance(node, hou.Node):
            if node.type().definition():
                return True
        elif isinstance(node, str):
            if hou.nodeType(node).type().definition():
                return True
        else:
            raise TypeError('invalid input', node)

        return False

    nodes = convertNodeTuple(inputNodes)
    for node in nodes:
        node.allowEditingOfContents()
        for child in node.children():
            convertSubnet(child, ignoreUnlock=ignoreUnlock, ignore_SideFX_HDA=ignore_SideFX_HDA,
                          nodeFilterFunc=nodeFilter_allNode, convertFeENode=True, detectName=True, detectPath=False,
                          debugMode=debugMode)


def recoverSubnet(node, ignoreUnlock=False, ignore_SideFX_HDA=True, nodeFilterFunc=None, convertFeENode=True,
                  detectName=True, detectPath=False, checkMatchSubNode=True):
    nodeType = node.type()
    nodeTypeName = nodeType.name()
    ##### 检测各种情况
    if nodeTypeName != 'subnet':
        recoverSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode,
                                      detectName, detectPath)
        return False

    ############ 能到这里的节点一定是subnet ############

    # defi = nodeType.definition()

    # if not ignoreUnlock and not node.matchesCurrentDefinition():
    #     #print(node)
    #     recoverSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath)
    #     return False

    # if convertFeENode:
    #     if not fee_HDA.isFeENode(nodeType, detectName, detectPath):
    #         return False

    # if ignore_SideFX_HDA:
    #     # defaultLibPath = hou.getenv('HFS') + r'/houdini/otls/'
    #     # if defi.libraryFilePath().startswith(defaultLibPath):
    #     if fee_HDA.isSideFXDefinition(defi):
    #         recoverSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath)
    #         return False

    ############# try found tmp Parm ###########
    flag = True
    for tmpStr in ('tmpParm_origNodeType_createdByPy',):
        origNodeTypeNameParm = node.parm(tmpStr)
        if origNodeTypeNameParm is None:
            continue
        flag = False
        break

    if flag:
        recoverSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode,
                                      detectName, detectPath)
        return False

    origNodeTypeName = origNodeTypeNameParm.evalAsString()
    # print(node)

    parent = node.parent()
    newRefNode = parent.createNode(origNodeTypeName, run_init_scripts=False, load_contents=True, exact_type_name=True)

    if checkMatchSubNode and not isEqual_networkChildren(node, newRefNode, checkNodeType=False):
        newRefNode.destroy()
        recoverSubnet_recurseSubChild(node, node, '', ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode,
                                      detectName, detectPath)
        return False

    newRefNode.destroy()

    node.removeSpareParmTuple(origNodeTypeNameParm)

    ##### 记录flag情况
    # print(parent.childTypeCategory().name())
    # if parent.childTypeCategory().name() != 'Sop':
    #     raise TypeError('error')
    # if node.isHardLocked():
    #     raise TypeError('isHardLocked')
    # if node.isSoftLocked():
    #     raise TypeError('isSoftLocked')

    # displayFlag = node == parent.displayNode()
    # renderFlag = node == parent.renderNode()
    # bypass = node.isBypassed()
    # isTemplateFlagSet = node.isTemplateFlagSet()
    # isHighlightFlagSet = node.isHighlightFlagSet()
    # isSelectableTemplateFlagSet = node.isSelectableTemplateFlagSet()
    # isUnloadFlagSet = node.isUnloadFlagSet()

    # inputConnectors = node.inputConnectors()
    # nInputs = len(inputConnectors)

    """
    ############ 处理4号以后的输入口
    if nInputs > 4:
        shiftVector2 = hou.Vector2(0.0, -1.0)
        nulls = []

        ###算出nulls列表的所有元素，后面有用
        for idx in range(4, nInputs):
            if len(inputConnectors[idx]) == 0:
                # 说明没有连
                nulls.append(None)
                continue
            
            inputConnection = inputConnectors[idx][0]
            subnetIndirectInput = inputConnection.subnetIndirectInput()

            #### 查找输入的类型
            flag = False
            if subnetIndirectInput is None:
                inputNode = inputConnection.inputNode()
                if inputNode is None:
                    ####说明输入是一个个dot，最终连接到subnet input
                    inputItem = inputConnection.inputItem()
                    while type(inputItem).__name__ == 'NetworkDot':
                        ####寻找dot的最终链接
                        inputItem = inputItem.inputItem()
                    
                    if type(inputItem).__name__ == 'SubnetIndirectInput':
                        subnetIndirectInput = inputItem
                        flag = True
                    else:
                        raise ValueError('fee : 不可能的情况')
                        #nulls.append(inputItem)
                        #flag = False
                else:
                    ####说明输入是一个node
                    nulls.append(inputNode)
                    flag = False
            else:
                ####说明输入是一个subnet input
                flag = True
            
            #### 建立null节点
            if flag:
                null = None
                for outputConnection in subnetIndirectInput.outputConnections():
                    outputNode = outputConnection.outputNode()
                    if outputNode.type().name() == 'null':
                        if outputNode.parm('copyinput').evalAsInt() == 1:
                            null = outputNode
                            nulls.append(null)
                            break

                if null is None:
                    if 1:
                        null = parent.createNode('null', exact_type_name=True)
                        null.setInput(0, subnetIndirectInput, output_index=0)
                    else:
                        try:
                            null = subnetIndirectInput.createOutputNode('null', exact_type_name=True)
                        except:
                            print(subnetIndirectInput)
                            raise ValueError('')
                    # null.setPosition(subnetIndirectInput.position().__add__(shiftVector2))
                    null.setPosition(subnetIndirectInput.position() + shiftVector2)
                    nulls.append(null)
            
        indirectInputs = node.indirectInputs()
        for idx in range(4, nInputs):
            if nulls[idx-4] is None: #没有连
                continue
            objectMerge = node.createNode('object_merge', exact_type_name=True)
            objectMerge.parm('objpath1').set('../../' + nulls[idx-4].name() )
            objectMerge.setPosition(indirectInputs[idx].position())
            for outputConnection in indirectInputs[idx].outputConnections():
                outputNode = outputConnection.outputNode()
                outputNode.setInput(outputConnection.inputIndex(), objectMerge)


    ############ 处理0-3号输入口 ########### 没有这部分也能跑，只是连接比较丑
    inputItems = []
    inputItem_output_index = []
    for idx in range(min(nInputs, 4)):
        if len(inputConnectors[idx]) == 0:
            # 说明没有连
            inputItems.append(None)
            inputItem_output_index.append(None)
            continue
        inputConnection = inputConnectors[idx][0]
        inputItem = inputConnection.inputItem()
        inputItems.append(inputItem)
        if isinstance(inputItem, hou.NetworkDot) or isinstance(inputItem, hou.SubnetIndirectInput):
            inputItem_output_index.append(0)
        else:
            inputItem_output_index.append(inputConnection.outputIndex())

    """

    ########### rest outputs ############ 没有这部分也能跑，只是连接比较丑
    outputNodes = node.outputs()
    outputNodes = list(set(outputNodes))

    outputNodes_inputItems = []
    outputNodes_inputItem_output_index = []
    for idx in range(len(outputNodes)):

        outputNodes_inputConnectors = outputNodes[idx].inputConnectors()
        outputNodes_inputItems.append([])
        outputNodes_inputItem_output_index.append([])

        # for idy in range(len(outputNodes_inputConnectors)):
        for outputNodes_inputConnector in outputNodes_inputConnectors:
            if len(outputNodes_inputConnector) == 0:
                outputNodes_inputItems[idx].append(None)
                outputNodes_inputItem_output_index[idx].append(None)
                continue
            outputNodes_inputConnection = outputNodes_inputConnector[0]
            outputNodes_inputItem = outputNodes_inputConnection.inputItem()
            # outputNodes_inputItems[idx].append(outputNodes_inputItem)
            outputNodes_inputItems[idx].append(outputNodes_inputItem.name())
            if isinstance(outputNodes_inputItem, hou.NetworkDot) or isinstance(outputNodes_inputItem,
                                                                               hou.SubnetIndirectInput):
                outputNodes_inputItem_output_index[idx].append(0)
            else:
                outputNodes_inputItem_output_index[idx].append(outputNodes_inputConnection.outputIndex())

    ########### rest hou.NetworkDot outputs ############ 没有这部分也能跑，只是连接比较丑

    nodeName = node.name()
    outputConnectors = node.outputConnectors()
    for idx in range(len(outputConnectors)):
        # for outputConnector in outputConnectors:
        for outputConnection in outputConnectors[idx]:
            outputItem = outputConnection.outputItem()
            if not isinstance(outputItem, hou.NetworkDot):
                continue
            outputNodes.append(outputItem)
            outputNodes_inputItems.append([nodeName, ])
            outputNodes_inputItem_output_index.append([idx, ])

    inputConnectors = node.inputConnectors()
    outputNodes.append(nodeName)
    outputNodes_inputItems.append([])
    outputNodes_inputItem_output_index.append([])
    for idx in range(len(inputConnectors)):
        # for outputConnector in inputConnectors:
        if len(inputConnectors[idx]) == 0:
            continue
        inputConnection = inputConnectors[idx][0]
        inputItem = inputConnection.inputItem()
        outputNodes_inputItems[-1].append(inputItem)
        if isinstance(inputItem, hou.NetworkDot):
            outputNodes_inputItem_output_index[-1].append(0)
        else:
            outputNodes_inputItem_output_index[-1].append(inputConnection.outputIndex())

    ############ 修改节点类型 ############
    origNodeshape = node.userData('nodeshape')
    copyOrigNode = hou.copyNodesTo([node], parent)[0]
    newNode = node.changeNodeType(origNodeTypeName, keep_parms=True)
    newNode.matchCurrentDefinition()
    if origNodeshape is not None:
        newNode.setUserData('nodeshape', origNodeshape)
    # newNode.removeSpareParms()

    '''

    ############ 处理0-3号输入口 ########### 没有这部分也能跑，只是连接比较丑

    ############ set Input ###############
    for idx in range(min(nInputs, 4)):
        if inputItems[idx] is None:
            continue
        newNode.setInput(idx, inputItems[idx], inputItem_output_index[idx])
        """
        try:
            newNode.setInput(idx, inputItems[idx], inputItem_output_index[idx])
        except:
            print(newNode)
            print(inputItems[idx])
            print(inputItem_output_index[idx])
            raise ValueError('setInput Error')
        """
    ############ set position ###############
    oldIndirectInputs = copyOrigNode.indirectInputs()
    newIndirectInputs = newNode.indirectInputs()
    for idx in range(0, min(len(oldIndirectInputs), 4)):
        pos = oldIndirectInputs[idx].position()
        newIndirectInputs[idx].setPosition(pos)
        """
        try:
            pos = oldIndirectInputs[idx].position()
            newIndirectInputs[idx].setPosition(pos)
        except:
            print(copyOrigNode)
            print(newNode)
            print(oldIndirectInputs[idx])
            print(newIndirectInputs[idx])
            raise ValueError('fee Error')
        """

    '''

    ########### 处理输出口 ############ 没有这部分也能跑，只是连接比较丑
    if 1:
        parent = newNode.parent()
        # print(nodeName)
        # print(outputNodes)
        # print(outputNodes_inputItems)
        # print(outputNodes_inputItem_output_index)
        for idx in range(len(outputNodes)):
            if isinstance(outputNodes[idx], str):
                outputNodes[idx] = parent.item(outputNodes[idx])

            for idy in range(len(outputNodes_inputItems[idx])):
                if outputNodes_inputItems[idx][idy] is None:
                    continue
                if isinstance(outputNodes_inputItems[idx][idy], str):
                    outputNodes_inputItems[idx][idy] = parent.item(outputNodes_inputItems[idx][idy])

                if 1:
                    if idy < len(outputNodes[idx].inputConnectors()):
                        outputNodes[idx].setInput(idy, outputNodes_inputItems[idx][idy],
                                                  outputNodes_inputItem_output_index[idx][idy])
                else:
                    try:
                        outputNodes[idx].setInput(idy, outputNodes_inputItems[idx][idy],
                                                  outputNodes_inputItem_output_index[idx][idy])
                    except:
                        print(outputNodes[idx])
                        print(idy)
                        print(outputNodes_inputItems[idx][idy])
                        print(outputNodes_inputItem_output_index[idx][idy])

    # if origNodeshape is not None:
    # 这个是自动的啦
    # pass
    # newNode.setUserData('nodeshape', origNodeshape)

    '''

    ############### parm type #################
    #newNode.parm('standardfolder').hide(True)
    # newNode.parm('label1').hide(True)
    # newNode.parm('label2').hide(True)
    # newNode.parm('label3').hide(True)
    # newNode.parm('label4').hide(True)
    if 1:
        tmpParm = hou.StringParmTemplate('tmpParm_origNodeType_createdByPy', 'Temp Parm Original Node Type', 1, default_value=(nodeTypeName, ) )
        tmpParm.hide(True)
        newNode.addSpareParmTuple(tmpParm)
        copyParms_NodetoNode(copyOrigNode, newNode, copyNoneExistParms = True, debugMode = debugMode)
    else:
        ### 这样顺序反过来，就会丢失linked channal链接关系哦，所以不能这样写
        copyParms_NodetoNode(copyOrigNode, newNode, copyNoneExistParms = True, debugMode = debugMode)
        tmpParm = hou.StringParmTemplate('tmpParm_origNodeType_createdByPy', 'Temp Parm Original Node Type', 1, default_value=(nodeTypeName, ) )
        tmpParm.hide(True)
        newNode.addSpareParmTuple(tmpParm)
    '''

    # copyParms_NodetoNode(copyOrigNode, newNode, copyNoneExistParms = True, debugMode = debugMode)

    # nameComponents = nodeType.nameComponents()
    # nameComponents[2]
    """
    targetParmTemplateGroup = newNode.parmTemplateGroup()
    newNodeParmTemplateGroup = newNode.parmTemplateGroup()
    newNodeParmTemplateGroup.append(tmpParm)
    newNode.setParmTemplateGroup(newNodeParmTemplateGroup)
    """
    # newNode.removeSpareParmFolder(('Standard', ))
    # newNode.removeSpareParmTuple(newNode.parmTuple('Standard'))

    # newNode.setDisplayFlag(displayFlag)
    # newNode.setRenderFlag(renderFlag)
    # newNode.bypass(bypass)
    # newNode.setTemplateFlag(isTemplateFlagSet)
    # newNode.setHighlightFlag(isHighlightFlagSet)
    # newNode.setSelectableTemplateFlag(isSelectableTemplateFlagSet)
    # newNode.setUnloadFlag(isUnloadFlagSet)

    # optype_exp = copyOrigNode.type().nameComponents()[2]
    # optype_exp1 = '\'' + optype_exp + '\''
    # bake_optypeExp_to_str(newNode, newNode, optype_exp, optype_exp1)

    # print(optype_exp)

    copyOrigNode.destroy()

    return True


def recoverSubnet_recurseSubChild(sourceNode, recurseNode, optype_exp='', ignoreUnlock=False, ignore_SideFX_HDA=True,
                                  nodeFilterFunc=None, convertFeENode=True, detectName=True, detectPath=False):
    nodeType = sourceNode.type()
    nodeTypeName = nodeType.name()

    if nodeTypeName != 'subnet' and sourceNode.matchesCurrentDefinition():
        return False

    for child in recurseNode.children():
        recoverSubnet(child, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath)

    return True


def recoverNodesFromSubnet(inputNodes, ignoreUnlock=False, ignore_SideFX_HDA=True, nodeFilterFunc=None,
                           convertFeENode=True, detectName=True, detectPath=False, displayConfirmation=False,
                           checkMatchSubNode=True):
    if displayConfirmation:
        fee_Utils.displayConfirmation(prevText='plz backup HIP before do this\n建议先备份HIP')

    nodes = convertNodeTuple(inputNodes)
    for node in nodes:
        recoverSubnet(node, ignoreUnlock, ignore_SideFX_HDA, nodeFilterFunc, convertFeENode, detectName, detectPath,
                      checkMatchSubNode)


def unlock_All_HDA(inputNodes, detectName=True, detectPath=False, displayConfirmation=False):
    if displayConfirmation:
        fee_Utils.displayConfirmation(prevText='plz backup HIP before do this\n建议先备份HIP')

    nodes = convertNodeTuple(inputNodes)
    for node in nodes:
        node.allowEditingOfContents()
        for child in node.children():
            childNodeType = child.type()
            if fee_HDA.isFeENode(childNodeType, detectName, detectPath):
                # child.allowEditingOfContents()
                unlock_All_FeENode(child, detectName, detectPath)
            elif fee_HDA.isUnlockedHDA(child):
                unlock_All_FeENode(child, detectName, detectPath)


hou.Node.unlock_All_HDA = unlock_All_HDA


def unlock_All_FeENode(inputNodes, detectName=True, detectPath=False, displayConfirmation=False):
    if displayConfirmation:
        fee_Utils.displayConfirmation(prevText='plz backup HIP before do this\n建议先备份HIP')

    nodes = convertNodeTuple(inputNodes)
    for node in nodes:
        node.allowEditingOfContents()
        for child in node.children():
            childNodeType = child.type()
            if fee_HDA.isFeENode(childNodeType, detectName, detectPath):
                # child.allowEditingOfContents()
                unlock_All_FeENode(child, detectName, detectPath)
            elif fee_HDA.isUnlockedHDA(child):
                unlock_All_FeENode(child, detectName, detectPath)


hou.Node.unlock_All_FeENode = unlock_All_FeENode


# def unlock_All_FeENode_recurseSubChild(node, detectName = True, detectPath = False):


def nodesBottomRightPosition(selectedNodes):
    selectedNodesTuple = convertNodeTuple(selectedNodes)
    if len(selectedNodesTuple) == 0:
        raise ValueError('No Input Nodes')

    BottomRightPosition = hou.Vector2(1e10, -1e10)
    for selectedNode in selectedNodesTuple:
        nodePosition = selectedNode.position()
        BottomRightPosition[0] = min(BottomRightPosition[0], nodePosition[0])
        BottomRightPosition[1] = max(BottomRightPosition[1], nodePosition[1])

    return BottomRightPosition


hou.Node.nodesBottomRightPosition = nodesBottomRightPosition


def createCopyNodeParmsNode(selectedNodes):
    selectedNodesTuple = convertNodeTuple(selectedNodes)
    if len(selectedNodesTuple) == 0:
        raise ValueError('No Input Nodes')

    parentNode = selectedNodes[0].parent()

    copyParmsNode = parentNode.node('./copyNodeParms_tempNodeforPython1')
    if copyParmsNode is None:
        copyParmsNode = parentNode.createNode('FeE::copyNodeParms', node_name='copyNodeParms_tempNodeforPython1')

    sourceNodesStrs = []
    for selectedNode in selectedNodes:
        sourceNodesStrs.append(selectedNode.name())

    copyParmsNode.parm('sourceNodes').set('../' + ' ../'.join(sourceNodesStrs))

    nwePos = nodesBottomRightPosition(selectedNodesTuple) + hou.Vector2(0, -1)
    copyParmsNode.setPosition(nwePos)


def setCopyNodeParmsNode(selectedNodes):
    selectedNodesTuple = convertNodeTuple(selectedNodes)
    if len(selectedNodesTuple) == 0:
        raise ValueError('No Input Nodes')

    parentNode = selectedNodes[0].parent()

    copyParmsNode = parentNode.node('./copyNodeParms_tempNodeforPython1')
    sourceNodesStrs = []
    for selectedNode in selectedNodes:
        sourceNodesStrs.append(selectedNode.name())

    copyParmsNode.parm('targetNodes').set('../' + ' ../'.join(sourceNodesStrs))


def extract_LockedNull(selectedNodes, subPath='extractLockedGeo', savePath_rel_to_HIP=True, keep_name=True):
    fee_Utils.displayConfirmation()
    selectedNodesTuple = convertNodeTuple(selectedNodes)
    for selectedNode in selectedNodesTuple:
        nodetype = selectedNode.type()
        if nodetype.name() == 'null' and selectedNode.isGenericFlagSet(hou.nodeFlag.Lock):

            filepath = ''.join(['/', subPath, '/', selectedNode.path().lstrip('/').replace('/', '.'), '.bgeo'])

            envName = "HIP" if savePath_rel_to_HIP else "TEMP"
            filepath_abs = hou.getenv(envName) + filepath
            filepath_rel = '$' + envName + filepath
            # print(filepath_abs)
            # print(filepath_rel)
            import os
            pathDir = os.path.dirname(filepath_abs)
            if not os.path.exists(pathDir):
                os.mkdir(pathDir)
            # print(pathDir)
            selectedNode.geometry().saveToFile(filepath_abs)

            selectedNode.setGenericFlag(hou.nodeFlag.Lock, 0)
            newnode = selectedNode.changeNodeType(new_node_type='file', keep_name=keep_name, keep_parms=False,
                                                  keep_network_contents=True, force_change_on_node_type_match=True)
            newnode.setParms({"file": filepath_rel})

        elif selectedNode.isNetwork() and nodetype.category().name() == 'Sop' and not selectedNode.isLockedHDA():
            for child in selectedNode.children():
                foreverychildren(child)


def getAllSubOutputNodes(selectedNode, orderdByOutputIndex=True):
    if not isinstance(selectedNode, hou.Node):
        raise TypeError('not hou.Node')

    outputRenderNodes = []
    selectedParent = selectedNode.parent()
    for child in selectedParent.children():
        if child.type().name() != 'output':
            continue
        outputRenderNodes.append(child)
        # outputRenderNodesIdx.append(child.parm('outputidx').evalAsInt())
        # for node in outputRenderNodes:
        #     print(node.parm('outputidx').evalAsInt())

    if orderdByOutputIndex and outputRenderNodes:
        outputRenderNodes.sort(key=lambda node: node.parm('outputidx').evalAsInt())
        # outputRenderNodes = [i[0] for i in sorted(zip(outputRenderNodes, outputRenderNodesIdx), key=lambda n: n[1])]

    return tuple(outputRenderNodes)


def setNodeAsOutput(selectedNode):
    # selectedNode = hou.selectedNodes()[0]
    if not isinstance(selectedNode, hou.Node):
        raise TypeError('not hou.Node')

    outputRenderNodes = getAllSubOutputNodes(selectedNode)
    if outputRenderNodes:
        flag = True

        for outputNode in selectedNode.outputs():
            if outputNode not in outputRenderNodes:
                continue
            outputNode.setInput(0, None)
            outputRenderNodes[(outputRenderNodes.index(outputNode) + 1) % len(outputRenderNodes)].setInput(0,
                                                                                                           selectedNode)
            flag = False
            break

        if flag:
            outputRenderNodes[0].setInput(0, selectedNode)

    else:
        newOutputNode = selectedNode.createOutputNode('output')
        newOutputNode.setPosition(selectedNode.position() + hou.Vector2(0.0, -1.0))


hou.Node.setNodeAsOutput = setNodeAsOutput


def displayOutput(selectedNode):
    parent = selectedNode.parent()
    outputRenderNodes = getAllSubOutputNodes(selectedNode)
    if outputRenderNodes:
        parentDisplayNode = parent.displayNode()
        if parentDisplayNode in outputRenderNodes:
            outputRenderNodes[(outputRenderNodes.index(parentDisplayNode) + 1) % len(outputRenderNodes)].setDisplayFlag(
                True)
        else:
            outputRenderNodes[0].setDisplayFlag(True)


hou.Node.displayOutput = displayOutput


def subNodeCount(selectedNodes):
    selectedNodesTuple = convertNodeTuple(selectedNodes)

    subNodeCountSum = len(selectedNodesTuple)
    for selectedNode in selectedNodesTuple:
        allSubChildren = selectedNode.allSubChildren()
        subNodeCountSum += len(allSubChildren)

    return subNodeCountSum


hou.Node.subNodeCount = subNodeCount


def bakeAllParms(selectedNodes):
    selectedNodesTuple = convertNodeTuple(selectedNodes)

    for selectedNode in selectedNodesTuple:
        if 0:
            import fee_Parm
            fee_Parm.bakeParm(selectedNode.parms())
        else:
            for parm in selectedNode.parms():
                value = parm.eval()
                parm.deleteAllKeyframes()
                parm.set(value)


hou.Node.bakeAllParms = bakeAllParms


def setToDefaultPosition(selectedItems):
    for selectedItem in selectedItems:
        if selectedItem.networkItemType() == hou.networkItemType.SubnetIndirectInput:
            idx = int(selectedItem.name()) - 1
            selectedItem.setPosition(hou.Vector2(0, 6) + hou.Vector2(3, 0.5) * idx)


def setToDefaultPositionWithNode(selectedItems):
    shiftedPosition = hou.Vector2(0, 0)

    for selectedItem in selectedItems:
        if selectedItem.networkItemType() == hou.networkItemType.SubnetIndirectInput:
            if selectedItem.name() == '1':
                shiftedPosition = hou.Vector2(0, 6) - selectedItem.position()
                break

    for selectedItem in selectedItems:
        selectedItem.move(shiftedPosition)


def selectSameNodes(selectedNodes):
    def select(node):
        if node.type() == selectedNodeType:
            node.setSelected(1)

    for selectedNode in selectedNodes:
        siblingNodes = selectedNode.parent().children()
        selectedNodeType = selectedNode.type()
        if 1:
            for siblingNode in siblingNodes:
                if siblingNode.type() == selectedNodeType:
                    siblingNode.setSelected(1)
        else:
            map(select, siblingNodes)


hou.Node.selectSameNodes = selectSameNodes


def createOutputNode_allItemVer(inputItem, node_type_name, node_name=None, run_init_scripts=True, load_contents=True,
                                exact_type_name=False):
    # if isinstance(inputItem, hou.SubnetIndirectInput):
    if isinstance(inputItem, hou.SubnetIndirectInput):
        # shiftPos = hou.Vector2(0, -1)
        if node_type_name == 'null':
            shiftPos = hou.Vector2(0, -1.1692999999999998)
            # shiftPos[1] = -1.1692999999999998
        else:
            shiftPos = hou.Vector2(0, -1)

        parent = inputItem.parent()
        upstreamPos = inputItem.position()
        newNode = parent.createNode(node_type_name, node_name, run_init_scripts, load_contents, exact_type_name)

        # print(upstreamPos + shiftPos)
        newNode.setPosition(upstreamPos + shiftPos)

        outputConnections = inputItem.outputConnections()
        for outputConnection in outputConnections:
            outputConnectionNode = outputConnection.outputNode()
            outputConnectionNode.setInput(outputConnection.inputIndex(), newNode, output_index=0)

        newNode.setInput(0, inputItem, output_index=0)


    elif isinstance(inputItem, hou.Node):
        newNode = inputItem.createOutputNode(node_type_name, node_name, run_init_scripts, load_contents,
                                             exact_type_name)
    else:
        raise TypeError('invalid input type')
    return newNode


def setToDefaultColorShape(selectedItems):
    defaultColorItemValue = 0.7
    defaultColorItem = hou.Color(defaultColorItemValue, defaultColorItemValue, defaultColorItemValue)

    defaultColorFile = hou.Color(0.9, 0.8, 0.55)
    defaultColorRop = hou.Color(0.65, 0.4, 0.5)
    defaultColorLoopBlock = hou.Color(0.75, 0.4, 0)

    # defaultColorNodeValue = 0.8
    # defaultColorNode = hou.Color(defaultColorNodeValue, defaultColorNodeValue, defaultColorNodeValue)

    for selectedItem in selectedItems:
        networkItemType = selectedItem.networkItemType()
        if networkItemType == hou.networkItemType.Node:
            nodeTypeName = selectedItem.type().name()

            if nodeTypeName == 'file_fbx_fee':
                selectedItem.setColor(defaultColorFile)
            elif nodeTypeName == 'exportUE4_fbx_fee':
                selectedItem.setColor(defaultColorRop)
            elif nodeTypeName == 'block_begin':
                selectedItem.setColor(defaultColorLoopBlock)
            elif nodeTypeName == 'block_end':
                selectedItem.setColor(defaultColorLoopBlock)

            else:
                selectedItem.setColor(selectedItem.type().defaultColor())

            print(selectedItem.userData('nodeshape'))
            if nodeTypeName == 'convertline_fee::2.0':
                selectedItem.setUserData('nodeshape', 'trapezoid_up')
            elif nodeTypeName == 'exportUE4_fbx_fee':
                pass

            '''
            defaultShape = selectedItem.type().defaultShape()
            print(selectedItem.type())
            print(selectedItem.userDataDict())
            print(defaultShape)
            selectedItem.setUserData('nodeshape', defaultShape)
            '''

        elif networkItemType == hou.networkItemType.SubnetIndirectInput:
            selectedItem.setColor(defaultColorItem)


def correctWrangleParmeter(selectedNodes):
    import re
    for node in selectedNodes:
        nodeType = node.type()
        nodeTypeName = nodeType.name()
        if nodeTypeName != 'attribwrangle':
            continue

        snippet = node.evalParm('snippet')

        ######################### vex_matchattrib ##########################
        parm_vex_matchattrib = node.parm('vex_matchattrib')
        vex_matchattrib = parm_vex_matchattrib.eval()
        if vex_matchattrib == 'id':
            parm_vex_matchattrib.set('')

        ######################### vex_inplace ##########################
        vex_inplace = node.parm('vex_inplace')
        vex_inplaceVal = True
        for substr in ('attrib(0,', 'prim(0,', 'point(0,', 'vertex(0,', 'detail(0,'):
            if substr in snippet:
                vex_inplaceVal = False
                break
        if vex_inplaceVal:
            classParmVal = node.parm('class').evalAsString()
            if classParmVal != 'detail' and classParmVal != 'number':
                # print("findattribval(0, '" + classParmVal + "'")
                if "findattribval(0, '" + classParmVal + "'" in snippet:
                    vex_inplaceVal = False

        vex_inplace.set(vex_inplaceVal)

        # print(usedAttribNames)
        ############################ autobind ######################
        if 1 or node.evalParm('autobind') == 1 or node.evalParm('groupautobind') == 1:
            node.parm('autobind').set(False)
            node.parm('groupautobind').set(False)
            usedAttribNames = []
            pattern = re.compile(r'@[a-zA-Z0-9_]*')
            usedAttribNames = pattern.findall(snippet)
            usedAttribNames = list(set(usedAttribNames))
            usedGroupNames = []
            # print(snippet)
            # print(usedAttribNames)
            for idx in range(len(usedAttribNames) - 1, -1, -1):
                if len(usedAttribNames[idx]) == 0:
                    raise ValueError('usedAttribNames len is 0', usedAttribNames[idx])
                    continue
                usedAttribNames[idx] = usedAttribNames[idx][1:]
                if usedAttribNames[idx].startswith('group_'):
                    usedGroupNames.append(usedAttribNames[idx][len('group_'):])
                    del usedAttribNames[idx]

            for topStr in (
                    'elemnum', 'ptnum', 'primnum', 'vtxnum', 'numelem', 'numprim', 'numpt', 'numvtx', 'OpInput1',
                    'OpInput2',
                    'OpInput3', 'OpInput4'):
                if topStr in usedAttribNames:
                    usedAttribNames.remove(topStr)

            for topStr in (
                    'color', 'Alpha', 'Cd', 'boneCapture_data', 'boneCapture_index', 'compxform',
                    'boneCapture_pCaptData',
                    'boneCapture_pCaptPath', 'pCaptSkelRoot', 'length', 'width', 'xform', 'transform', 'rotate',
                    'scale',
                    'pscale', 'binormal', 'bitangent', 'tangent', 'up', 'N', 'P'):
                if topStr not in usedAttribNames:
                    continue
                usedAttribNames.remove(topStr)
                usedAttribNames.insert(0, topStr)

            node.parm('bindings').set(len(usedAttribNames))
            for idx in range(len(usedAttribNames)):
                stridx = str(idx + 1)
                node.parm('bindname' + stridx).set(usedAttribNames[idx])
                node.parm('bindparm' + stridx).set(usedAttribNames[idx])


def convertDefiByFilter(selectedNode, targetHDAPath='',
                        ignoreUnlock=True, nodeFilterFunc=None, convertFeENode=True,
                        namespaceOriginal='', namespaceTarget='',
                        subMenuOriginalPrefix='', subMenuTargetPrefix='',
                        nodeTypeNameFromLabel=False, hdaNameFromNodeTypeName=False,
                        name_lower=False, name_convertSpacetoUnderscore=False,
                        deleteNodeType=(), bypassNodeType=(), bypassNode={},
                        strictNodeNameDict={}, strictNodeLabelDict={}, subMenuDict={}, changeNodeTypeDict={},
                        strictNodeParmDefaultValueDict={}, hideParmNodeDict={},
                        debugMode=0):
    # return Value is definition

    if not isinstance(targetHDAPath, str):
        raise TypeError('must be str', targetHDAPath)
    if not isinstance(namespaceOriginal, str):
        raise TypeError('must be str', namespaceOriginal)

    if not isinstance(subMenuOriginalPrefix, str):
        raise TypeError('must be str', subMenuOriginalPrefix)
    if not isinstance(subMenuTargetPrefix, str):
        raise TypeError('must be str', subMenuTargetPrefix)

    if not isinstance(deleteNodeType, tuple):
        raise TypeError('must be tuple', deleteNodeType)
    if not isinstance(bypassNodeType, tuple):
        raise TypeError('must be tuple', bypassNodeType)
    if not isinstance(bypassNode, dict):
        raise TypeError('must be dict', bypassNode)

    if not isinstance(strictNodeNameDict, dict):
        raise TypeError('must be dict', strictNodeNameDict)
    if not isinstance(subMenuDict, dict):
        raise TypeError('must be dict', subMenuDict)
    if not isinstance(changeNodeTypeDict, dict):
        raise TypeError('must be dict', changeNodeTypeDict)

    if not isinstance(strictNodeParmDefaultValueDict, dict):
        raise TypeError('must be dict', strictNodeParmDefaultValueDict)
    if not isinstance(hideParmNodeDict, dict):
        raise TypeError('must be dict', hideParmNodeDict)

    import os
    import re

    nodeType = selectedNode.type()
    defi = nodeType.definition()
    nodeTypeName = nodeType.name()
    nodeNameComponents = list(nodeType.nameComponents())

    if defi is None:
        raise TypeError('must have defnition', selectedNode)

    nodeTypeCategory = defi.nodeTypeCategory()
    if nodeTypeCategory.name() != 'Sop':
        raise TypeError('support SOP node ONLY', selectedNode)
    # for childNode in selectedNode.allNodes():

    # fee_Node.changeAllSubNodeTypeMatchesType(selectedNode)
    # fee_Node.convert_All_FeENode_to_Subnet(selectedNode, ignoreUnlock = False, ignore_SideFX_HDA = True, detectName = False, detectPath = True)
    # fee_Node.deleteAllSubNodeMatchesType(selectedNode)

    changeAllSubNodeTypeMatchesType(selectedNode, changeNodeTypeDict)
    # convert_All_FeENode_to_Subnet(selectedNode, ignoreUnlock = ignoreUnlock, ignore_SideFX_HDA = True, detectName = False, detectPath = True)
    convertSubnet_custom(selectedNode, ignoreUnlock=ignoreUnlock, ignore_SideFX_HDA=True, nodeFilterFunc=nodeFilterFunc,
                         convertFeENode=convertFeENode, displayConfirmation=False, debugMode=debugMode)
    deleteAllSubNodeMatchesType(selectedNode, deleteNodeType)

    # for allSubChild in selectedNode.allSubChiledren():
    #     newNode = allSubChild.changeNodeType(targetNodeTypeName, keep_parms=True)

    ########################## save new defi

    ##################### set new Name and Label

    if 1:
        # initial node label
        newNodeLabel = nodeType.description()
        if nodeTypeName in strictNodeLabelDict:
            newNodeLabel = strictNodeLabelDict[nodeTypeName]
        else:
            if namespaceOriginal != '':
                newNodeLabel = newNodeLabel.lstrip(namespaceOriginal)
            newNodeLabel = newNodeLabel.lstrip(' ')

    if 1:
        # initial node name
        if nodeTypeName in strictNodeNameDict:
            if isinstance(strictNodeNameDict[nodeTypeName], list):
                choiceNodeType = []
                for strictNodeNameDictSubElem in strictNodeNameDict[nodeTypeName]:
                    choiceNodeType.append(strictNodeNameDictSubElem[0])
                choiceNodeType = tuple(choiceNodeType)
                # print(choiceNodeType)
                choiceNodeTypeidx = hou.ui.displayMessage('Choice Node Type', buttons=choiceNodeType, default_choice=0,
                                                          close_choice=0)
                newNodeTypeName = strictNodeNameDict[nodeTypeName][choiceNodeTypeidx][0]
                # print(newNodeTypeName)
                # return 0
            else:
                newNodeTypeName = strictNodeNameDict[nodeTypeName][0]
            bypassNodeIdentifyString = newNodeTypeName
            nodeNameComponents[2] = fee_HDA.splitTypeNametoNameComponents(newNodeTypeName)[2]
        else:
            bypassNodeIdentifyString = nodeTypeName
            if namespaceTarget != '':
                nodeNameComponents[1] = namespaceTarget
            elif nodeNameComponents[1] == namespaceOriginal:
                nodeNameComponents[1] = ''

            newNodeTypeName = newNodeLabel if nodeTypeNameFromLabel else nodeNameComponents[2]

            if namespaceOriginal != '':
                # newNodeTypeName = newNodeTypeName.lstrip(namespaceOriginal + '_')
                # newNodeTypeName = newNodeTypeName.lstrip(namespaceOriginal.lower() + '_')
                # newNodeTypeName = newNodeTypeName.lstrip(namespaceOriginal)
                # newNodeTypeName = newNodeTypeName.lstrip(namespaceOriginal.lower())
                # newNodeTypeName = newNodeTypeName.rstrip('_' + namespaceOriginal)
                # newNodeTypeName = newNodeTypeName.rstrip('_' + namespaceOriginal.lower())

                if newNodeTypeName.startswith(namespaceOriginal + '_'):
                    newNodeTypeName = newNodeTypeName[len(namespaceOriginal) + 1:]
                if newNodeTypeName.startswith(namespaceOriginal.lower() + '_'):
                    newNodeTypeName = newNodeTypeName[len(namespaceOriginal) + 1:]
                if newNodeTypeName.endswith('_' + namespaceOriginal):
                    newNodeTypeName = newNodeTypeName[:len(newNodeTypeName) - len(namespaceOriginal) - 1]
                if newNodeTypeName.endswith('_' + namespaceOriginal.lower()):
                    newNodeTypeName = newNodeTypeName[:len(newNodeTypeName) - len(namespaceOriginal) - 1]
            if name_lower:
                newNodeTypeName = newNodeTypeName.lower()
            newNodeTypeName = re.sub('generator', 'gen', newNodeTypeName)
            newNodeTypeName = re.sub(' ', '_', newNodeTypeName)

            nodeNameComponents[2] = newNodeTypeName
            newNodeTypeName = fee_HDA.combineNameComponents(nodeNameComponents)

    if bypassNodeIdentifyString in bypassNode:
        nodePath = selectedNode.path() + '/'
        for bypassNodeSubElem in bypassNode[bypassNodeIdentifyString]:
            print(nodePath + bypassNodeSubElem)
            hou.node(nodePath + bypassNodeSubElem).bypass(True)

    if targetHDAPath == '':
        # initial hda name

        if hdaNameFromNodeTypeName:
            hdaName = nodeNameComponents[2] + '.hda'
        else:
            libraryFilePath = defi.libraryFilePath()
            # splitLibraryFilePath = os.path.split(path)
            hdaName = os.path.basename(libraryFilePath)
            hdaName = hdaName.lstrip(namespaceOriginal)
            hdaName = hdaName.lstrip(' ')
            # libraryFilePath = splitLibraryFilePath[0] + '/' + hdaName

        if name_convertSpacetoUnderscore:
            hdaName = hdaName.replace(' ', '_')
        if name_lower:
            hdaName = hdaName.lower()

        # print(hdaName)
        targetHDAPath = hou.getenv('TEMP') + '/' + hdaName

        # defi.save(targetHDAPath, selectedNode)
    else:
        targetHDAPathBasename = os.path.basename(targetHDAPath)
        if not targetHDAPathBasename.endswith('.hda'):
            raise ValueError('is not path of a hda file', targetHDAPath)
        targetHDAPath = targetHDAPath.replace('\\', '/')

    # print(newNodeTypeName)
    # print(newNodeLabel)

    '''
    otls_path = hou.getenv("TOOLS").replace("//","/") + "/otls/"
    _ , hda_name = os.path.split(targetHDAPath)
    target_path = otls_path + hda_name

    newDefi = hou.hdaDefinition(hou.sopNodeTypeCategory(), newNodeTypeName, target_path)
    if newDefi is not None:
        newDefi.destroy()
    '''

    '''
    newDefi = hou.hdaDefinition(hou.sopNodeTypeCategory(), newNodeTypeName, targetHDAPath)
    if newDefi is not None:
        newDefi.destroy()
    '''

    # return False
    try:
        os.makedirs(os.path.dirname(targetHDAPath))
    except:
        pass

    defi.copyToHDAFile(targetHDAPath, new_name=newNodeTypeName, new_menu_name=newNodeLabel)
    hou.hda.installFile(targetHDAPath)
    newDefi = hou.hdaDefinition(hou.sopNodeTypeCategory(), newNodeTypeName, targetHDAPath)

    newDefi.updateFromNode(selectedNode)

    ############################### set section #################################
    if subMenuTargetPrefix != '':
        sections = newDefi.sections()
        sectionToolShelf = sections[r'Tools.shelf']
        # try:
        #     sectionToolShelf = sections[r'Tools.shelf']
        # except:
        #     print('not found section: Tools Shelf:', end = ' ')
        #     print(nodeType)

        contents = sectionToolShelf.contents()  # hou.compressionType.NoCompression
        if nodeTypeName in subMenuDict:
            # print(contents)
            newcontent = '<toolSubmenu>' + subMenuTargetPrefix + '/' + subMenuDict[nodeTypeName] + '</toolSubmenu>'
            # print(newcontent)
            newcontent = re.sub('<toolSubmenu>.*</toolSubmenu>', newcontent, contents, re.M)
            # print(newcontent)
        elif subMenuOriginalPrefix != '':
            newcontent = re.sub(subMenuOriginalPrefix, subMenuTargetPrefix, contents, re.M)
        # print(newcontent)
        sectionToolShelf.setContents(newcontent)

    ############## set parms ##############
    parmTemplateGroup = defi.parmTemplateGroup()
    # parmTemplateGroup = newDefi.parmTemplateGroup() #### attention! : not this

    # print(parmTemplateGroup)

    ############### empty itemGeneratorScript ######################
    for parmTemplate in parmTemplateGroup.parmTemplates():
        # print(parmTemplate)
        if isinstance(parmTemplate, hou.StringParmTemplate) or isinstance(parmTemplate, hou.MenuParmTemplate):
            if 'fee_buildMenu' in parmTemplate.itemGeneratorScript():
                # print('has menu:')
                # print(parmTemplate)
                parmTemplate.setItemGeneratorScript('')

                parmName = parmTemplate.name()
                parmTemplateGroup.replace(parmName, parmTemplate)

        if isinstance(parmTemplate, hou.StringParmTemplate):
            defaultValueTuple = parmTemplate.defaultValue()
            replaceStrs = (
                ("if 1:\n    import fee_HDA", 'if 0:\n    import fee_HDA'),
                ("fee_HDA.", ''),
                ("if 1:\n    import fee_Node", 'if 0:\n    import fee_Node'),
                ("fee_Node.", ''),
                ("if 1:\n    import fee_Utils", 'if 0:\n    import fee_Utils'),
                ("fee_Utils.", ''),
                ("if 1:\n    import fee_Parm", 'if 0:\n    import fee_Parm'),
                ("fee_Parm.", ''),
                ("if 1:\n    import fee_Math", 'if 0:\n    import fee_Math'),
                ("fee_Math.", ''),
            )

            defaultValueList = list(defaultValueTuple)
            # if parmTemplate.name() == 'pymodule':
            # print(defaultValueList)

            flag = False
            for idx, defaultValue in enumerate(defaultValueList):
                '''
                if parmTemplate.name() == 'pymodule':
                    print(defaultValue)
                    print(replaceStr in defaultValue)
                    print(replaceStr in str(defaultValue))
                    #print(str(defaultValue).find(replaceStr))
                '''

                for replaceStr in replaceStrs:
                    if replaceStr[0] in defaultValue:
                        defaultValue = defaultValue.replace(replaceStr[0], replaceStr[1])
                        flag = True
                # if flag:
                #     print(defaultValue)
                defaultValueList[idx] = defaultValue

            if flag:
                defaultValueTuple = tuple(defaultValueList)
                parmTemplate.setDefaultValue(defaultValueTuple)
                parmName = parmTemplate.name()
                parmTemplateGroup.replace(parmName, parmTemplate)

    ######################### default parms ########################
    if nodeTypeName in strictNodeParmDefaultValueDict:
        for parmDefaultValue in strictNodeParmDefaultValueDict[nodeTypeName]:
            parmName = parmDefaultValue[0]
            # print(parmName)
            parmTemplate = parmTemplateGroup.find(parmName)
            if parmTemplate is None:
                print('not find parm: ' + parmName)
            if isinstance(parmTemplate, hou.ButtonParmTemplate):
                continue
            # print(parmDefaultValue[1])
            # print(isinstance(parmTemplate, hou.ToggleParmTemplate))
            # setDefaultExpressionLanguage(hou.scriptLanguage.Hscript)

            if parmDefaultValue[1] == 0:
                if isinstance(parmTemplate, hou.ToggleParmTemplate):
                    parmTemplate.setDefaultExpression('on' if parmDefaultValue[2] else 'off')


                else:
                    parmTemplate.setDefaultExpression('')
                    if isinstance(parmTemplate, hou.IntParmTemplate):
                        if isinstance(parmDefaultValue[2], int):
                            parmTemplate.setDefaultValue((parmDefaultValue[2],))
                        else:
                            parmTemplate.setDefaultValue(parmDefaultValue[2])


                    elif isinstance(parmTemplate, hou.FloatParmTemplate):
                        if isinstance(parmDefaultValue[2], float):
                            parmTemplate.setDefaultValue((parmDefaultValue[2],))
                        else:
                            parmTemplate.setDefaultValue(parmDefaultValue[2])


                    elif isinstance(parmTemplate, hou.StringParmTemplate):
                        print(parmDefaultValue[2])
                        if isinstance(parmDefaultValue[2], str):
                            parmTemplate.setDefaultValue((parmDefaultValue[2],))
                        else:
                            parmTemplate.setDefaultValue(parmDefaultValue[2])


                    else:
                        parmTemplate.setDefaultValue(parmDefaultValue[2])


            elif parmDefaultValue[1] == 1:
                parmTemplate.setDefaultExpression(parmDefaultValue[2])
                parmTemplate.setDefaultExpressionLanguage((hou.scriptLanguage.Hscript,))
            elif parmDefaultValue[1] == 2:
                parmTemplate.setDefaultExpression(parmDefaultValue[2])
                parmTemplate.setDefaultExpressionLanguage((hou.scriptLanguage.Python,))
            else:
                print('error')
            # print(parmTemplate.defaultValue())
            # print(parmTemplate)
            # print(parmTemplate.defaultExpression())
            parmTemplateGroup.replace(parmName, parmTemplate)

    ######################### hide parms ########################
    # print(nodeTypeName)
    if nodeTypeName in hideParmNodeDict:
        for parmName in hideParmNodeDict[nodeTypeName]:
            # print(parmName)
            parmTemplate = parmTemplateGroup.find(parmName)
            parmTemplate.hide(True)
            parmTemplateGroup.replace(parmName, parmTemplate)

    newDefi.setParmTemplateGroup(parmTemplateGroup)

    return newDefi


hou.Node.convertDefiByFilter = convertDefiByFilter


def getOutputNode(inputNode, idx):
    for node in inputNode.children():
        if node.type().name() != 'output':
            continue
        if node.evalParm('outputidx') == idx:
            return node

    if idx == 0:
        return inputNode.displayNode()

    raise ValueError('not found inputNode idx', inputNode)


hou.Node.getOutputNode = getOutputNode


def isFileCacheNode(node):
    nodeTypeName = node.type().name()
    return nodeTypeName == 'filecache' or nodeTypeName == r'FeE::fileCache' or nodeTypeName == 'filecache_fee'


hou.Node.isFileCacheNode = isFileCacheNode


class NodeChain(object):
    def __init__(self, node):
        self.node = node
        self.inputNodeChains = []

    # def buildNodeChain(self, node):
    #     self.inputNodeChains = [self.buildNodeChain(upstreamNode) for upstreamNode in searchUpstreamFileCacheNodes(node)]
    #     return self

    def ropName(self):
        return self.node.path().replace('/', '_')

    def isFileCacheNode(self):
        return self.node.isFileCacheNode()

    def createRopNodes(self, ropNodeParent):
        childTypeCategory = ropNodeParent.childTypeCategory()
        if not childTypeCategory:
            raise TypeError('not a subnet', ropNodeParent)

        if childTypeCategory.name() != 'Driver':
            raise TypeError('not a rop net', ropNodeParent)

        ropNode = ropNodeParent.node(self.ropName())

        if ropNode:
            for idx in range(len(ropNode.inputs())):
                ropNode.setInput(idx, None)
        else:
            # take the case that first selected node is not filecache into consideration
            ropNode = ropNodeParent.createNode('fetch' if self.isFileCacheNode() else 'merge')
            ropNode.setName(self.ropName())

        ropNode.setColor(self.node.color())
        if self.node.type().name() == 'filecache':
            ropNode.parm('source').set(self.node.path() + '/render')
        elif self.isFileCacheNode():
            ropNode.parm('source').set(self.node.path() + '/execute')

        for idx, input_rop in enumerate(
                [inputNodeChain.createRopNodes(ropNodeParent) for inputNodeChain in self.inputNodeChains]):
            ropNode.setInput(idx, input_rop)

        ropNode.moveToGoodPosition()

        return ropNode


def nodeSubInputs(node, stopFunc):
    # def nodeSubInputs(node):
    inputNodes = []
    for inputConnection in node.inputConnections():
        inputNode = inputConnection.inputNode()
        if inputNode is None:
            continue

        if not inputNode.type().definition():
            inputNodes.append(inputNode)
            continue

        if stopFunc(inputNode):
            # if isFileCacheNode(inputNode):
            inputNodes.append(inputNode)
            continue

        outputIndex = inputConnection.outputIndex()
        realOutputNode = inputNode.getOutputNode(outputIndex)
        if stopFunc(realOutputNode):
            # if isFileCacheNode(realOutputNode):
            inputNodes.append(realOutputNode)
            continue

        # if not realOutputNode.type().definition():
        #     inputNodes.append(realOutputNode)
        #     continue

        while not stopFunc(realOutputNode) and realOutputNode.type().definition():
            # while not isFileCacheNode(realOutputNode) and realOutputNode.type().definition():
            realOutputNode = realOutputNode.getOutputNode(0)

        inputNodes.append(realOutputNode)
        continue

    return inputNodes


hou.Node.nodeSubInputs = nodeSubInputs


def searchUpstreamFileCacheNodes(node):
    fileCacheNodes = []
    # if inputNodes is None:
    #     return None
    # for inputNode in node.inputs():
    for inputNode in node.nodeSubInputs(isFileCacheNode):
        # for inputNode in node.nodeSubInputs():
        if inputNode is None:
            continue
        if inputNode.isFileCacheNode():
            fileCacheNodes.append(inputNode)
            continue

        upstreamFileCacheNodes = searchUpstreamFileCacheNodes(inputNode)
        if upstreamFileCacheNodes is not None:
            fileCacheNodes.extend(upstreamFileCacheNodes)
    return fileCacheNodes


hou.Node.searchUpstreamFileCacheNodes = searchUpstreamFileCacheNodes


def buildNodeChain(node):
    nodeChain = NodeChain(node)
    nodeChain.inputNodeChains = [buildNodeChain(upstreamNode) for upstreamNode in searchUpstreamFileCacheNodes(node)]
    return nodeChain


def buildFileCacheROPNodeChain(inputNodes):
    fee_Utils.displayConfirmation()
    nodes = convertNodeTuple(inputNodes)
    for node in nodes:
        ropnetNode = node.parent().createNode('ropnet')
        ropnetNode.setName('ropnet_fileCache1', unique_name=True)
        ropnetNode.setPosition(node.nodesBottomRightPosition() + hou.Vector2(1, -1))

        nodeChain = buildNodeChain(node)
        nodeChain.createRopNodes(ropnetNode)


def findChildNodeMatchesType(inputNodes, matchNodeTypeName):
    for childNode in inputNodes.allSubChildren():
        childNodeType = childNode.type()
        childNodeTypeName = childNodeType.name()
        if childNodeTypeName != matchNodeTypeName:
            continue
        print(childNode)


def searchParmsValue(inputNodes, searchValue):
    returnParms = []
    nodes = convertNodeTuple(inputNodes)
    for inputNode in nodes:
        for child in inputNode.allSubChildren(recurse_in_locked_nodes=False):
            for parm in child.parms():
                if searchValue in parm.rawValue():
                    returnParms.append(parm)
    return returnParms


def searchParmsValue_print(inputNodes, searchValue):
    if inputNodes:
        print('\n')
    nodes = convertNodeTuple(inputNodes)
    for inputNode in nodes:
        for child in inputNode.allSubChildren(recurse_in_locked_nodes=False):
            for parm in child.parms():
                if searchValue in parm.rawValue():
                    # raise SystemError('a', parm)
                    print(parm)


def getAllUsedHDKNode(node):
    ignoreNames = (
        'output',
        'subnet',
        'block_end',
        'block_begin_if',
        'compile_begin',
        'compile_end',
        'invoke',
        'matnet',
        'material',
        'materialbuilder',
        'topobuild',
        'parameter',
        'subinput',
        'suboutput',
        'snippet',
    )
    usedNodeType = []
    for subNode in node.allSubChildren():
        nodeType = subNode.type()
        defi = nodeType.definition()
        if defi is not None:
            continue
        if nodeType in usedNodeType:
            continue
        usedNodeType.append(nodeType)

    for nodeType in usedNodeType:
        name = nodeType.name()
        if name in ignoreNames:
            continue
        print(name)

    for name in ignoreNames:
        print(name)
