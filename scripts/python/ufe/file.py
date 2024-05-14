# from io import TextIOWrapper
# from io import TextIO
# from typing import List


def is_invalid_line(__line: int) -> bool:
    return __line == -1


def is_valid_line(__line: int) -> bool:
    return not is_invalid_line(__line)


def is_invalid_line_tuple(__line: tuple[int, int]) -> bool:
    return is_invalid_line(__line[1])


def is_valid_line_tuple(__line: tuple[int, int]) -> bool:
    return not is_invalid_line_tuple(__line)


def del_line(__content: list[str], __start_line: int, __end_line: int = -1) -> None:
    __end_line_final = __end_line if is_valid_line(__end_line) else len(__content)
    for i in range(__end_line_final - 1, __start_line - 1, -1):
        del __content[i]


def del_line_tuple(__content: list[str], __line_tuple: tuple[int, int]) -> None:
    del_line(__content, __line_tuple[0], __line_tuple[1])


def find_first_line_start_end(
        __content: list[str],
        __start_line: int,
        __end_line: int = -1,
        __start_str: str = "",
        __end_str: str = ""
) -> int:
    __end_line_final = __end_line if is_valid_line(__end_line) else len(__content)
    __end_str_final = __end_str + "\n"

    __find_line = -1
    for i in range(__start_line, __end_line_final):
        if (__content[i].startswith(__start_str) and __content[i].endswith(__end_str_final)):
            __find_line = i
            break

    return __find_line


def find_last_line_start_end(
        __content: list[str],
        __start_line: int,
        __end_line: int = -1,
        __start_str: str = "",
        __end_str: str = ""
) -> int:
    __endLine_final = __end_line if is_valid_line(__end_line) else len(__content)
    __endStr_final = __end_str + "\n"

    __find_line = -1
    for i in range(__start_line, __endLine_final):
        if (not (__content[i].startswith(__start_str) and __content[i].endswith(__endStr_final))):
            __find_line = i
            break

    return __find_line


def find_double_line_start_end_base(
        __content: list[str],
        __start_line: int,
        __end_line: int,
        __start_str0: str,
        __end_str0: str,
        __start_str1: str,
        __end_str1: str
) -> tuple[int, int]:
    __line_start = find_first_line_start_end(__content,
                                             __start_line, __end_line, __start_str0, __end_str0)
    __line_end = -1
    if (is_invalid_line(__line_start)):
        return (__line_start, __line_end)
    __line_end = find_last_line_start_end(__content,
                                          __line_start + 1, __end_line, __start_str1, __end_str1)
    return (__line_start, __line_end)


def find_double_line_start_end(
        __content: list[str],
        __start_line: int,
        __end_line: int = -1,
        __start_str: str = "",
        __end_str: str = ""
) -> tuple[int, int]:
    return find_double_line_start_end_base(__content,
                                           __start_line, __end_line,
                                           __start_str, __end_str,
                                           __start_str, __end_str)


def move_line(content: list[str], begin_line, end_line, target_line):
    original_content = []
    for idx in range(begin_line, end_line + 1):
        original_content.append(content[begin_line])
        del content[begin_line]
    for idx in range(len(original_content) - 1, -1, -1):
        content.insert(target_line, original_content[idx])


def remove_space(content: list[str], __start_line: int, __end_line: int) -> None:
    if not isinstance(content, list):
        raise TypeError(content)

    for line_idx in range(__end_line-1, __start_line-1, -1):
        if content[line_idx] == "\n":
            content.pop(line_idx)


def get_str_between_two_str(
        __content: list[str],
        __start_str: str,
        __end_str: str = "",
        __start_line: int = 0,
        __end_line: int = -1
) -> str:
    __end_line_final = __end_line if is_valid_line(__end_line) else len(__content)
    node_name_line = -1
    for i in range(__start_line, __end_line_final):
        if __start_str in __content[i] and __end_str in __content[i]:
            node_name_line = i
            break

    if is_invalid_line(node_name_line):
        # print("not find str_between_two_str")
        return ""

    node_name = __content[node_name_line]
    find_idx = node_name.find(__start_str)
    node_name = node_name[find_idx + len(__start_str):]
    node_name = node_name[:len(node_name) - len(__end_str) - 1]
    return node_name
