import pandas
import pywavefront
import pywavefront.visualization


def csv_to_obj(csv_filepath: str, obj_filepath: str):
    csv_file = pandas.read_csv(csv_filepath)

    obj_file = pywavefront.Wavefront(obj_filepath)
    pywavefront.visualization.draw(obj_file)


csv_to_obj(
    "D:/Houdini/GFEHoudini/hdk/New folder/mesh.csv",
    "D:/Houdini/GFEHoudini/hdk/New folder/mesh.obj")
