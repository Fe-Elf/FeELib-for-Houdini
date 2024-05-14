import os
from io import TextIOWrapper
# from io import TextIO
from typing import List
import ufe.list
import ufe.file
import ufe.string


def find_line_pragma(__content: list[str], __start_line: int = 0, __end_line: int = -1) -> tuple[int, int]:
    return ufe.file.find_double_line_start_end(__content, __start_line, __end_line, "#pragma ")


def find_line_has_been_included(
        __content: list[str],
        __start_line: int,
        __end_line: int = -1
) -> tuple[int, int]:
    return ufe.file.find_double_line_start_end_base(__content,
                                                    __start_line, __end_line,
                                                    "#ifndef ", "_HAS_BEEN_INCLUDED",
                                                    "#define ", "_HAS_BEEN_INCLUDED")


def find_line_file_name(__content: list[str], __start_line: int, __end_line: int = -1) -> tuple[int, int]:
    return ufe.file.find_double_line_start_end_base(__content,
                                                    __start_line, __end_line,
                                                    "#ifndef ", "__FILE_NAME__",
                                                    "#endif", "")


def gen_line_file_name(__content: list[str], __start_line: int, filename: str) -> tuple[int, int]:
    line_file_name = find_line_file_name(__content, __start_line)
    if ufe.file.is_valid_line_tuple(line_file_name):
        return line_file_name
    else:
        __content.insert(__start_line, """#ifndef __FILE_NAME__
#define __FILE_NAME__ {0}
#define __GFE_FILE_NAME__ __FILE_NAME__
#endif
""".format(filename))
        return (__start_line, __start_line + 1)


def find_line_include(__content: list[str], __start_line: int, __end_line: int = -1) -> tuple[int, int]:
    return ufe.file.find_double_line_start_end(__content, __start_line, __end_line, "#include ")


def find_line_func(
        __content: list[str],
        __start_str: str,
        __end_str: str = "",
        __start_line: int = 0,
        __end_line: int = -1
) -> tuple[int, int]:
    __endLine_final = __end_line if ufe.file.is_valid_line(__end_line) else len(__content)
    func_line_start = -1
    for i in range(__start_line, __endLine_final):
        if __content[i].find(__start_str) == -1 or __content[i].find(__end_str) == -1:
            continue
        func_line_start = i
        break
    if func_line_start == -1:
        print("can not find funtion start " + __start_str)
        return (-1, -1)

    num_space_start = ufe.string.get_num_space_start(__content[func_line_start])
    func_end_str = ""
    for idx in range(0, num_space_start):
        func_end_str += " "
    func_end_str += "}\n"

    func_line_end = ufe.list.find(__content, func_end_str, func_line_start)
    if func_line_end == -1:
        print("can not find funtion end " + __start_str)

    return (func_line_start, func_line_end)


def find_line_gfes_begin(__content: list[str], __start_line: int = 0, __end_line: int = -1) -> int:
    return ufe.list.find(__content, "GFES_BEGIN\n", __start_line, __end_line)


def find_line_gfes_end(__content: list[str], __start_line: int = 0, __end_line: int = -1) -> int:
    return ufe.list.find(__content, "GFES_END\n", __start_line, __end_line)


def get_node_name(content: list[str]) -> str:
    node_name_line_start = "::theSOPTypeName{ufe::concat_string(gfel::SOP_Name_Prefix_FeE, \""
    node_name_line_end = "\", gfel::SOP_Name_Suffix_FeE).data()};\n"
    return ufe.file.get_str_between_two_str(content, node_name_line_start, node_name_line_end)


def get_node_label(content: list[str], __start_line: int = 0, __end_line: int = -1) -> str:
    node_name_line_start = "ufe::concat_string(gfel::SOP_Label_Prefix_FeE, \""
    node_name_line_end = "\", gfel::SOP_Label_Suffix_FeE).data(),\n"
    return ufe.file.get_str_between_two_str(content, node_name_line_start, node_name_line_end)


# def addUT_ASSERT_LEVEL(content: list[str]) -> None:
#     # if not isinstance(content, list):
#     #     raise TypeError("type")

#     for line_idx, line in enumerate(content):
#         if line == r"//#define UT_ASSERT_LEVEL 3" + "\n":
#             content.pop(line_idx)
#             break

#     if content[1] == "\n":
#         content.pop(1)

#     if content[0] == "\n":
#         content.pop(0)

# noinspection PyUnreachableCode


def code_gen_header(__content: list[str], __file: TextIOWrapper, __file_path: str = "") -> bool:
    basename = os.path.basename(__file.name)
    filename, file_ext = os.path.splitext(basename)

    line_pragma = find_line_pragma(__content, 0)
    if ufe.file.is_invalid_line_tuple(line_pragma):
        print(filename + " not found pragma")
        return False

    line_has_been_included = find_line_has_been_included(__content, line_pragma[1])
    if ufe.file.is_invalid_line_tuple(line_has_been_included):
        print(filename + " not found has_been_included")
        return False

    line_file_name = gen_line_file_name(__content, line_has_been_included[1], filename)
    if ufe.file.is_invalid_line_tuple(line_file_name):
        print(filename + " not found file_name")
        return False

    line_include = find_line_include(__content, line_file_name[1])
    if ufe.file.is_invalid_line_tuple(line_include):
        print(filename + " not found include")
        return False
    ufe.file.del_line_tuple(__content, line_include)

    __content.insert(line_include[0], """#include <GeoSop/BaseInclude.h>
#include <GeoSop/proto/{0}.proto.h>
""".format(filename))

    line_gfes_begin = find_line_gfes_begin(__content, line_include[0] + 1)
    if ufe.file.is_invalid_line(line_gfes_begin):
        print(filename + " not found gfes_begin")
        return False

    line_namespace = line_gfes_begin + 1
    # print(filename + " line_namespace is " + content[line_namespace-2])
    # print(filename + " line_namespace is " + content[line_namespace-1])
    # print(filename + " line_namespace is " + content[line_namespace])
    # print(filename + " line_namespace is " + content[line_namespace+1])
    if not __content[line_namespace].startswith("namespace"):
        print(filename + " not found namespace")
        return False
    node_shape = ufe.file.get_str_between_two_str(__content, "newOp->setNodeShape(\"", "\");\n")
    node_shape = "tilted" if ufe.string.is_empty_string(node_shape) else node_shape
    # node_name = ""
    # node_label = ""
    file_abspath = __file_path + "/../../../src/GeoSop/operator/{0}.cpp".format(filename)
    with open(file_abspath, "r") as file_cpp:
        file_content = file_cpp.readlines()
        node_name = get_node_name(file_content)
        if ufe.string.is_empty_string(node_name):
            print(filename + " no find node_name")
            return False

        new_sop_operator_func_line = find_line_func(file_content,
                                                    "void newSopOperator(OP_OperatorTable* table)", "",
                                                    10, -1)
        if ufe.file.is_invalid_line_tuple(new_sop_operator_func_line):
            print(filename + " no find new_sop_operator_func_line")
            return False

        node_label = get_node_label(file_content, new_sop_operator_func_line[0], new_sop_operator_func_line[1])
        # if is_empty_string(node_label):
        #     print(filename + " no find node_label")
        #     return False
        submenu_name = ufe.file.get_str_between_two_str(file_content, "\"Five elements Elf/", "\"};\n")
        # if is_empty_string(submenu_name):
        #     print(filename + " no find submenu_name")
        #     return False
        cook_mode = ufe.file.get_str_between_two_str(
            file_content,
            "CookMode cookMode(const SOP_NodeParms* parms) const override { return ",
            "; }\n")

        # if is_empty_string(cook_mode):
        #     print(filename + " no find cook_mode")
        #     return False
        icon_name = ufe.file.get_str_between_two_str(file_content,
                                                     "newOp->setIconName(\"",
                                                     "\");\n")
        if ufe.string.is_empty_string(icon_name):
            print(filename + " no find icon_name")
            return False
        max_num_input = -1
        try:
            for i in range(new_sop_operator_func_line[0], new_sop_operator_func_line[1]):
                if "::buildTemplates()," in file_content[i]:
                    min_num_input = int(file_content[i + 1].lstrip(" ")[0])
                    max_num_input = int(file_content[i + 2].lstrip(" ")[0])
        except:
            print(filename + " no find max_num_input")
            return False

        if ufe.file.is_invalid_line(max_num_input):
            print(filename + " no find max_num_input")
            print(new_sop_operator_func_line[0], new_sop_operator_func_line[1])

            return False

        num_output = 1
        is_ref_input = "{0"
        for i in range(1, max_num_input):
            is_ref_input += ", 1"
        is_ref_input += "}"

        input_label = "{\"InGeo\""
        for i in range(0, max_num_input - 1):
            input_label += ", \"GeoRef" + str(i) + "\""
        input_label += "}"
        # print(node_name)

    header_core_str = """    inline constexpr char NodeNameBase[] = "{node_name}";
    inline constexpr char NodeLabelBase[] = "{node_label}";
    inline constexpr char IconName[] = "{icon_name}";
    inline constexpr char SubmenuName[] = "{submenu_name}";
    inline constexpr ::SOP_NodeVerb::CookMode ECookMode = ::SOP_NodeVerb::CookMode::{cook_mode};
    inline constexpr int minNumInput = {min_num_input};
    inline constexpr int maxNumInput = {max_num_input};
    inline constexpr std::array<int, maxNumInput> IsRefInput{is_ref_input};
    inline constexpr std::array<const char*, maxNumInput> InputLabelBase{input_label};
    inline constexpr std::array<const char*, {num_output}> OutputLabelBase{{"OutGeo"}};
    inline constexpr char nodeShape[] = "{node_shape}";
    inline constexpr bool setNodeColor = false;
    inline const UT_Color nodeColor{{UT_ColorType::UT_RGB, 0.8, 0.5, 0.5}};
}} // End of Namespace {filename}_Namespace
__GFE_SOP_NODE_DECLARATION_SPECIALIZATION({filename})
GFES_END
#ifdef __GFE_FILE_NAME__
#undef __GFE_FILE_NAME__
#undef __FILE_NAME__
#endif
#endif
""".format(
        node_name=node_name,
        node_label=node_label,
        icon_name=icon_name,
        min_num_input=min_num_input,
        max_num_input=max_num_input,
        num_output=num_output,
        input_label=input_label,
        is_ref_input=is_ref_input,
        submenu_name=submenu_name,
        cook_mode=cook_mode,
        node_shape=node_shape,
        filename=filename,
    )

    __content.insert(line_namespace + 1, header_core_str)

    ufe.file.del_line(__content, line_namespace + 2)
    return True


# def code_gen_for_file(__file: TextIO) -> bool:
def code_gen_for_file(__file: TextIOWrapper, __file_path: str = "") -> bool:
    content = __file.readlines()

    basename = os.path.basename(__file.name)
    filename, file_ext = os.path.splitext(basename)

    # if not code_gen_header(content, __file, __file_path):
    #     return False
    '''
    add_str = """::SOP_NodeVerb::CookMode {0}_Verb::cookMode(const SOP_NodeParms* parms) const
{{ return {0}_Namespace::ECookMode; }}
""".format(filename)
    flag = True
    for i in range(0, len(content)):
        if content[i].startswith("const SOP_NodeVerb::Register<"):
            if not content[i+1].startswith("::SOP_NodeVerb::CookMode "):
                content.insert(i + 1, add_str)
            flag = False
            break
    if flag:
        print(filename + " not find const SOP_NodeVerb::Register")
        return False
'''

    new_sop_operator_func_line = find_line_func(content,
                                                "void SOP_FeE_",
                                                "Verb::cook(const SOP_NodeVerb::CookParms& cookparms) const",
                                                0, -1)
    if ufe.file.is_invalid_line_tuple(new_sop_operator_func_line):
        print(filename + " no find new_sop_operator_func_line")
        return False

    try_find_str = " = sop"
    for i in range(new_sop_operator_func_line[0], new_sop_operator_func_line[1]):
        find_idx = content[i].find(try_find_str)
        if find_idx == -1:
            continue
        if content[i][find_idx:].startswith(" = sopparms"):
            continue
        content[i] = content[i][:find_idx] + " = gfesop::sopParam" + content[i][find_idx + len(try_find_str):]

    # code_gen_cpp(content, __file)

    # removeSpace(content)

    # if not check_Pram_Template_Builder_Name(content, filename):
    #     return False

    # if not check_newSopOperator_h(content, filename):
    #     return False

    # sopTypeName = get_node_name(content, filename)
    # if sopTypeName == "":
    #     return False
    # print(sopTypeName)
    # if not check_add_newSopOperator_1(content, filename):
    #     return False

    # if not check_mod_newSopOperator_1(content, filename, sopTypeName):
    #     return False

    # addUT_ASSERT_LEVEL(content)

    # if not checkHasPragma(content, filename):
    #     return False
    #
    # if not checkHasInclude(content, filename):
    #     return False

    # if not check_gfelBegin(content, filename):
    #     return False

    # if not check_gfel(content, filename):
    #     return False

    # if not checkNamespaceStart(content, filename):
    #     return False

    # if not checkNamespaceEnd(content, filename):
    #     return False

    __file.truncate(0)
    __file.seek(0)
    __file.writelines(content)

    return True


def code_gen_for_folder(folder_path: str, file_format: str, ignore_files: tuple[str], active_files: tuple[str]) -> None:
    for file_root, file_dirs, file_base_names in os.walk(folder_path):
        for file_basename in file_base_names:
            if not file_basename.endswith(file_format):
                continue
            file_abspath = file_root + "/" + file_basename

            file_name, file_ext = os.path.splitext(file_basename)

            if len(active_files) > 0 and file_name not in active_files:
                continue

            if file_name in ignore_files:
                continue

            write_back = True
            with open(file_abspath, "r+" if write_back else "r") as file:
                file: TextIOWrapper
                code_gen_for_file(file, file_root)
