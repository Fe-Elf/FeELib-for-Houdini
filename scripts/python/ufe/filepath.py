# from io import TextIOWrapper
# from io import TextIO
import os
import ufe.string


def splitext(__filepath: str) -> tuple[str, str]:
    # return os.path.splitext(__filepath)
    __filepath = __filepath.replace("\\", "/")
    last_forward_slash_index = __filepath.rfind("/")
    first_dot_index = __filepath.find(".", max(last_forward_slash_index, 0))

    return __filepath[:first_dot_index], __filepath[first_dot_index:]


def basename_append_suffix(__filepath: str, __suffix: str, __prefix: str = None) -> str:
    if not __suffix:
        __suffix = ""
    if not __prefix:
        __prefix = ""

    file_dirname = os.path.dirname(__filepath)
    file_basename = os.path.basename(__filepath)
    filename, fileext = splitext(file_basename)
    if file_dirname[-1] != "/":
        file_dirname += "/"
    # print(file_dirname)
    return file_dirname + __prefix + filename + __suffix + fileext


# print(splitext("/path/t.o/yo.ur/file.txt.bak"))
# _a_tmp = 0
# _a_tmp = "prefix__"
# print(basename_append_suffix("/path/t.o/yo.ur/file.txt.bak", "_suffix", _a_tmp))
# print(basename_append_suffix("D:/aaa.txt", "_suffix", _a_tmp))

# print(_a_tmp)
