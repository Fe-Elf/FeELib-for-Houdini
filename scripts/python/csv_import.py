import hou
import pandas


def create_df_vector2_op(row, name_x: str, name_y: str):
    return (row[name_x], row[name_y])


def create_df_vector3_op(row, name_x: str, name_y: str, name_z: str):
    return (row[name_x], row[name_y], row[name_z])


def create_df_pos_op(row):
    return create_df_vector3_op(row, ' POSITION.x', ' POSITION.y', ' POSITION.z')


def create_df_normal_op(row):
    return create_df_vector3_op(row, ' NORMAL.x', ' NORMAL.y', ' NORMAL.z')


def create_df_tangent_op(row):
    return create_df_vector3_op(row, ' TANGENT_BFLIP.x', ' TANGENT_BFLIP.y', ' TANGENT_BFLIP.z')


def create_df_uv_op(row):
    return create_df_vector2_op(row, ' TEXCOORD.x', ' TEXCOORD.y')


def csv_import(geo: hou.Geometry, csv_filepath: str) -> None:
    csv_file = pandas.read_csv(csv_filepath)
    # last_index = csv_file.iloc()[-1].name

    # series_idx = csv_file[" IDX"]
    series_normal : pandas.Series = csv_file.apply(create_df_normal_op, axis=1)
    series_tangent: pandas.Series = csv_file.apply(create_df_tangent_op, axis=1)
    series_uv     : pandas.Series = csv_file.apply(create_df_uv_op, axis=1)
    series_pos    : pandas.Series = csv_file.apply(create_df_pos_op, axis=1)

    attrib_default_value = (0.0, 0.0, 0.0)
    attrib_normal  = geo.addAttrib(hou.attribType.Point, "N", attrib_default_value)
    attrib_tangent = geo.addAttrib(hou.attribType.Point, "tangent", attrib_default_value)
    attrib_uv      = geo.addAttrib(hou.attribType.Point, "uv", attrib_default_value)
    numpt = len(series_pos.tolist())
    points = geo.createPoints(series_pos.tolist())

    # print(df_normal[0])
    for idx, point in enumerate(points):
        point.setAttribValue('N',       series_normal[idx])
        point.setAttribValue('tangent', series_tangent[idx])
        point.setAttribValue('uv', (series_uv[idx][0], series_uv[idx][1], 0.0))

    ptnums = []
    for i in range(int(numpt / 3)):
        i3 = i * 3
        # ptnums.append((i3, i3+1, i3+2))
        ptnums.append((i3+2, i3+1, i3))
    geo.createPolygons(ptnums)
