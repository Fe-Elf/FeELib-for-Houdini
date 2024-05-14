node = hou.pwd()
out = 'f@`chs(\"attribname\")` = 0;'

if(node.parm(attribname).eval() != ''):
        return out
else:
        return ""
