
import hou

# import fee_Parm
# from importlib import reload
# reload(fee_Parm)


def convertParmTuple(parms):
    if isinstance(parms, hou.Parm):
        return (parms, )
    elif isinstance(parms, list):
        return tuple(parms)
    elif isinstance(parms, tuple):
        for idx in range(len(parms)):
            if not isinstance(parms[idx], hou.Parm):
                raise TypeError('Only Support hou.Parm or hou.Parm Tuple/List Type')
        return parms
    
    raise TypeError('Only Support hou.Parm or hou.Parm Tuple/List Type')

hou.Parm.convertParmTuple = convertParmTuple


def bakeParm(inputParm):
    parmTuple = convertParmTuple(inputParm)
    for parm in parmTuple:
        value = parm.eval()
        parm.deleteAllKeyframes()
        parm.set(value)

hou.Parm.bakeParm = bakeParm



def setOptypePython(inputParm):
    parmTuple = convertParmTuple(inputParm)
    for parm in parmTuple:
        if not isinstance(parm.parmTemplate(), hou.StringParmTemplate):
            continue

        try:
            parm.expressionLanguage()
        except:
            parmRawValue = parm.rawValue()
            parmRawValue = parmRawValue.lstrip('_')

            flag = 1
            if parmRawValue.endswith("_`optype('../.')`"):
                flag = 1
                parmRawValue = parmRawValue.rstrip("_`optype('../.')`")
            elif parmRawValue.endswith("_`optype('.')`"):
                flag = 0
                parmRawValue = parmRawValue.rstrip("_`optype('.')`")
            else:
                flag = 1
                parmRawValue = parm.evalAsString()
            
            parm.setExpression("'__" + parmRawValue + "_' + " + "hou.node('" + "../"*flag + ".').type().nameComponents()[2]", language=hou.exprLanguage.Python)


hou.Parm.setOptypePython = setOptypePython
