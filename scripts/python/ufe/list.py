from typing import List


def find(__list: list[str], __elem: str, __start: int = 0, __end: int = -1) -> int:
    try:
        __end_final = len(__list) if __end == -1 else __end
        __index = __list.index(__elem, __start, __end_final)
    except ValueError:
        __index = -1
    return __index
