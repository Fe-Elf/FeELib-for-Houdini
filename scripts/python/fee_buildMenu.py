import hou


def strisDetail(argString):
    if argString.find('detail') == 0 or argString.find('global') == 0:
        return True
    else:
        return False


def strisPrim(argString):
    if argString.find('prim') == 0:
        return True
    else:
        return False


def strisPoint(argString):
    if argString.find('point') == 0:
        return True
    else:
        return False


def strisEdge(argString):
    if argString.find('edge') == 0:
        return True
    else:
        return False


def strisVertex(argString):
    if argString.find('vertex') == 0 or argString == 'vertices':
        return True
    else:
        return False


def buildAttribsMenu_sub(node, input_num=0, attribClass='point', attribType='all'):
    try:
        input_node = node.inputs()[input_num]
        geo = input_node.geometry()
        if strisPrim(attribClass):
            allAttribs = geo.primAttribs()
        elif strisPoint(attribClass):
            allAttribs = geo.pointAttribs()
        elif strisVertex(attribClass):
            allAttribs = geo.vertexAttribs()
        else:
            allAttribs = geo.globalAttribs()
        menu = []
        for attrib in allAttribs:
            if attribType == 'int':
                if attrib.dataType() != hou.attribData.Int:
                    continue
            elif attribType == 'float':
                if attrib.dataType() != hou.attribData.Float:
                    continue
            elif attribType == 'string':
                if attrib.dataType() != hou.attribData.String:
                    continue
            menu += [attrib.name()]
            menu += [attrib.name()]
    except:
        return []

    return menu


def buildMultiClassAttribsMenu(node, input_num=0, attribClasses: tuple[str] = ('point'), attribType='all'):
    menu = []
    for attribClass in attribClasses:
        try:
            menu += buildAttribsMenu_sub(node, input_num, attribClass, attribType)
        except:
            continue

    return menu


def buildAttribsMenu(node, input_num: int = 0, attribClass='point', attribType: str ='all'):
    if isinstance(attribClass, list) or isinstance(attribClass, tuple):
        return buildMultiClassAttribsMenu(node, input_num, attribClass, attribType)
    else:
        return buildAttribsMenu_sub(node, input_num, attribClass, attribType)


def buildAllClassAttribsMenu(node, input_num=0, attribType='all'):
    return buildMultiClassAttribsMenu(node, input_num, ('detail', 'prim', 'point', 'vertex'), attribType)


def buildGroupsMenu_sub(node: hou.Node, input_num: int = 0, groupClass: str = 'point'):
    try:
        input_node = node.inputs()[input_num]
        geo = input_node.geometry()
        if strisPrim(groupClass):
            allGroups = geo.primGroups()
        elif strisPoint(groupClass):
            allGroups = geo.pointGroups()
        elif strisEdge(groupClass):
            allGroups = geo.edgeGroups()
        elif strisVertex(groupClass):
            allGroups = geo.vertexGroups()
        else:
            allGroups = geo.globalGroups()
        menu = []
        for group in allGroups:
            menu += [group.name()]
            menu += [group.name()]
    except:
        return []

    return menu


def buildMultiClassGroupsMenu(node: hou.Node, input_num: int = 0, groupClasses=('point')):
    menu = []
    for groupClass in groupClasses:
        try:
            menu += buildGroupsMenu_sub(node, input_num, groupClass)
        except:
            continue

    return menu


def buildGroupsMenu(node: hou.Node, input_num: int = 0, groupClass='point'):
    if isinstance(groupClass, list) or isinstance(groupClass, tuple):
        return buildMultiClassGroupsMenu(node, input_num, groupClass)
    else:
        return buildGroupsMenu_sub(node, input_num, groupClass)


def buildAllClassGroupsMenu(node: hou.Node, input_num: int = 0):
    return buildMultiClassGroupsMenu(node, input_num, ('detail', 'prim', 'point', 'edge', 'vertex'))
