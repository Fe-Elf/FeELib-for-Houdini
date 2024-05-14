

def is_empty_string(__str: str) -> bool:
    return __str == ""


def get_num_space_start(in_str: str) -> int:
    return len(in_str) - len(in_str.lstrip(" "))


def lstrip(__str: str, __str_ref: str) -> str:
    flag = True
    for i in range(0, len(__str_ref)):
        if __str[i] != __str_ref[i]:
            flag = False
    return __str[len(__str_ref):] if flag else __str


def rstrip(__str: str, __str_ref: str) -> str:
    flag = True
    idx_start = len(__str) - len(__str_ref)
    for i in range(0, len(__str_ref)):
        if __str[idx_start+i] != __str_ref[i]:
            flag = False
    return __str[:len(__str) - len(__str_ref)] if flag else __str
