# import fee_Math
# from importlib import reload
# reload(fee_Math)


def isNumber(strInput):
    try:
        float(strInput)
        return True
    except ValueError:
        pass
    return False


def valueType(strInput):
    try:
        float(strInput)
        if isinstance(eval(strInput), int):
            return 0
        else:
            return 1
    except ValueError:
        pass

    return 2


def findValueByKey(arg, key):
    if not isinstance(key, str):
        raise TypeError('key is not string')

    resultValues = []

    if isinstance(arg, dict):
        for subkey in arg.keys():
            if subkey == key:
                resultValues.append(arg[key])
            else:
                resultValues.extend(findValueByKey(arg[subkey], key))

    if isinstance(arg, list) or isinstance(arg, tuple):
        for subelem in arg:
            resultValues.extend(findValueByKey(subelem, key))

    return resultValues
