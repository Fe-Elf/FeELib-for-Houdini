
//#define UT_ASSERT_LEVEL 3
#include "SOP_FeE_MatchBBox_2_0.h"

#include "SOP_FeE_MatchBBox_2_0.proto.h"

#include "GA/GA_Detail.h"
#include "PRM/PRM_TemplateBuilder.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_DSOVersion.h"


#include "GFE/GFE_MatchBBox.h"

using namespace SOP_FeE_MatchBBox_2_0_Namespace;


static const char *theDsFile = R"THEDSFILE(
{
    name        parameters
    parm {
        name    "group"
        cppname "Group"
        label   "Group"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupType')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "groupType"
        cppname "GroupType"
        label   "Group Type"
        type    ordinal
        default { "guess" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }

    parm {
        name    "groupRef"
        cppname "GroupRef"
        label   "GroupRef"
        type    string
        default { "" }
        parmtag { "script_action" "import soputils\nkwargs['geometrytype'] = kwargs['node'].parmTuple('groupTypeRef')\nkwargs['inputindex'] = 0\nsoputils.selectGroupParm(kwargs)" }
        parmtag { "script_action_help" "Select geometry from an available viewport.\nShift-click to turn on Select Groups." }
        parmtag { "script_action_icon" "BUTTONS_reselect" }
    }
    parm {
        name    "groupTypeRef"
        cppname "GroupTypeRef"
        label   "Group Type Ref"
        type    ordinal
        default { "guess" }
        menu {
            "guess"     "Guess from Group"
            "prim"      "Primitive"
            "point"     "Point"
            "vertex"    "Vertex"
            "edge"      "Edge"
        }
    }
    parm {
        name    "posAttrib"
        cppname "PosAttrib"
        label   "Pos Attrib"
        type    string
        default { "P" }
    }
    parm {
        name    "posRefAttrib"
        cppname "PosRefAttrib"
        label   "Pos Ref Attrib"
        type    string
        default { "P" }
    }
    parm {
        name    "posRestAttrib"
        cppname "PosRestAttrib"
        label   "Pos Rest Attrib"
        type    string
        default { "P" }
    }

    parm {
        name    "primAttribToXform"
        cppname "PrimAttribToXform"
        label   "Prim Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "pointAttribToXform"
        cppname "PointAttribToXform"
        label   "Point Attrib to Transform"
        type    string
        default { "P" }
    }
    parm {
        name    "vertexAttribToXform"
        cppname "VertexAttribToXform"
        label   "Vertex Attrib to Transform"
        type    string
        default { "" }
    }
    parm {
        name    "detailAttribToXform"
        cppname "DetailAttribToXform"
        label   "Detail Attrib to Transform"
        type    string
        default { "" }
    }


    //parm {
    //    name    "ezMode"
    //    cppname "EZMode"
    //    label   "Eazy Mode"
    //    type    toggle
    //    default { "0" }
    //}


    parm {
        name    "useSelfAsRef"
        cppname "UseSelfAsRef"
        label   "Use Self as Ref"
        type    toggle
        default { "0" }
    }
    //parm {
    //    name    "repairPrecision"
    //    cppname "RepairPrecision"
    //    label   "Repair Precision"
    //    type    toggle
    //    default { "1" }
    //    joinnext
    //}
    //parm {
    //    name    "repairPrecisionThreshold"
    //    cppname "RepairPrecisionThreshold"
    //    label   "Repair Precision Threshold"
    //    type    log
    //    default { "1e-07" }
    //    disablewhen "{ repairPrecision == 0 }"
    //    range   { 1e-05 0.001 }
    //    nolabel
    //}
    parm {
        name    "usePrimBounding"
        cppname "UsePrimBounding"
        label   "Use Primitive Bounding"
        type    toggle
        default { "0" }
    }



    // parm {
    //     name    "xOrd"
    //     cppname "XOrd"
    //     label   "Transform Order"
    //     type    ordinal
    //     joinnext
    //     default { "tsr" }
    //     menu {
    //         "srt"   "Scale Rot Trans"
    //         "str"   "Scale Trans Rot"
    //         "rst"   "Rot Scale Trans"
    //         "rts"   "Rot Trans Scale"
    //         "tsr"   "Trans Scale Rot"
    //         "trs"   "Trans Rot Scale"
    //     }
    // }
    // parm {
    //     name    "rOrd"
    //     cppname "ROrd"
    //     label   "Rotate Order"
    //     type    ordinal
    //     nolabel
    //     default { "xyz" }
    //     menu {
    //         "xyz"   "Rx Ry Rz"
    //         "xzy"   "Rx Rz Ry"
    //         "yxz"   "Ry Rx Rz"
    //         "yzx"   "Ry Rz Rx"
    //         "zxy"   "Rz Rx Ry"
    //         "zyx"   "Rz Ry Rx"
    //     }
    // }
    parm {
        name    "doTranslate"
        cppname "DoTranslate"
        label   "Translate"
        type    toggle
        default { "on" }
        range   { 0 1 }
    }
    parm {
        name    "doScale"
        cppname "DoScale"
        label   "Scale"
        type    toggle
        default { "off" }
        range   { 0 1 }
    }

)THEDSFILE"
R"THEDSFILE(


    // group {
    //     name    "matchingtranslate_2"
    //     label   "Ref BBox"
    //     hidewhentab "{ ezMode == 1 }"
    // 
    //     parm {
    //         name    "minpad_absx"
    //         cppname "minpad_absx"
    //         label   "Min Padding Abs X"
    //         type    float
    //         default { "0" }
    //         range   { -1 1 }
    //     }
    //     parm {
    //         name    "minpad_absy"
    //         cppname "minpad_absy"
    //         label   "Min Padding Abs Y"
    //         type    float
    //         default { "0" }
    //         range   { -1 1 }
    //     }
    //     parm {
    //         name    "minpad_absz"
    //         cppname "minpad_absz"
    //         label   "Min Padding Abs Z"
    //         type    float
    //         default { "0" }
    //         range   { -1 1 }
    //     }
    //     parm {
    //         name    "maxpad_absx"
    //         cppname "maxpad_absx"
    //         label   "Max Padding Abs X"
    //         type    float
    //         default { [ "ch(\"minpad_absx\")" hscript-expr ] }
    //         range   { -1 1 }
    //     }
    //     parm {
    //         name    "maxpad_absy"
    //         cppname "maxpad_absy"
    //         label   "Max Padding Abs Y"
    //         type    float
    //         default { [ "ch(\"minpad_absy\")" hscript-expr ] }
    //         range   { -1 1 }
    //     }
    //     parm {
    //         name    "maxpad_absz"
    //         cppname "maxpad_absz"
    //         label   "Max Padding Abs Z"
    //         type    float
    //         default { [ "ch(\"minpad_absz\")" hscript-expr ] }
    //         range   { -1 1 }
    //     }
    // }

    group {
        name    "matchingtranslate_2_1"
        label   "Matching Translate"
        hidewhentab "{ doTranslate == 0 }"

        // parm {
        //     name    "bboxtx"
        //     cppname "bboxtx"
        //     label   "BBox Translate X"
        //     type    float
        //     default { "0" }
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "bboxty"
        //     cppname "bboxty"
        //     label   "BBox Translate Y"
        //     type    float
        //     default { "0" }
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "bboxtz"
        //     cppname "bboxtz"
        //     label   "BBox Translate Z"
        //     type    float
        //     default { "0" }
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "invertTRef"
        //     cppname "invertTRef"
        //     label   "Invert T Ref"
        //     type    toggle
        //     default { "0" }
        // }
        // parm {
        //     name    "bbox_ref_tx"
        //     cppname "bbox_ref_tx"
        //     label   "BBox Ref Translate X"
        //     type    float
        //     default { [ "if(ch(\"invertTRef\"), -1, 1) * ch(\"bboxtx\")1" hscript-expr ] }
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "bbox_ref_ty"
        //     cppname "bbox_ref_ty"
        //     label   "BBox Ref Translate Y"
        //     type    float
        //     default { [ "if(ch(\"invertTRef\"), -1, 1) * ch(\"bboxty\")1" hscript-expr ] }
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "bbox_ref_tz"
        //     cppname "bbox_ref_tz"
        //     label   "BBox Ref Translate Z"
        //     type    float
        //     default { [ "if(ch(\"invertTRef\"), -1, 1) * ch(\"bboxtz\")1" hscript-expr ] }
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "sepparm2"
        //     label   "Separator"
        //     type    separator
        //     default { "" }
        // }
        // groupsimple {
        //     name    "bboxcentert_folder"
        //     label   "BBox Center Translate"
        //     hidewhen "{ ezMode == 1 }"
        //     grouptag { "group_type" "simple" }
        // 
        //     parm {
        //         name    "bboxcx"
        //         cppname "bboxcx"
        //         label   "BBox Center X"
        //         type    float
        //         default { "1" }
        //         range   { 0 1 }
        //     }
        //     parm {
        //         name    "bboxcy"
        //         cppname "bboxcy"
        //         label   "BBox Center Y"
        //         type    float
        //         default { "1" }
        //         range   { 0 1 }
        //     }
        //     parm {
        //         name    "bboxcz"
        //         cppname "bboxcz"
        //         label   "BBox Center Z"
        //         type    float
        //         default { "1" }
        //         range   { 0 1 }
        //     }
        //     parm {
        //         name    "bbox_ref_cx"
        //         cppname "bbox_ref_cx"
        //         label   "BBox Ref Center X"
        //         type    float
        //         default { "1" }
        //         range   { 0 1 }
        //     }
        //     parm {
        //         name    "bbox_ref_cy"
        //         cppname "bbox_ref_cy"
        //         label   "BBox Ref Center Y"
        //         type    float
        //         default { "1" }
        //         range   { 0 1 }
        //     }
        //     parm {
        //         name    "bbox_ref_cz"
        //         cppname "bbox_ref_cz"
        //         label   "BBox Ref Center Z"
        //         type    float
        //         default { "1" }
        //         range   { 0 1 }
        //     }
        // }

        parm {
            name    "tBiasRestx"
            cppname "TBiasRestx"
            label   "Bias Translate Rest X"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "tBiasResty"
            cppname "TBiasResty"
            label   "Bias Translate Rest Y"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "tBiasRestz"
            cppname "TBiasRestz"
            label   "Bias Translate Rest Z"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "tBiasRefx"
            cppname "TBiasRefx"
            label   "Bias Translate Ref X"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "tBiasRefy"
            cppname "TBiasRefy"
            label   "Bias Translate Ref Y"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "tBiasRefz"
            cppname "TBiasRefz"
            label   "Bias Translate Ref Z"
            type    float
            default { "0" }
            range   { -1 1 }
        }

        parm {
            name    "tx"
            cppname "Tx"
            label   "Translate X"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "ty"
            cppname "Ty"
            label   "Translate Y"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "tz"
            cppname "Tz"
            label   "Translate Z"
            type    float
            default { "0" }
            range   { -1 1 }
        }
    }

    //group {
    //    name    "matchingtranslate_2_2"
    //    label   "Center"
    //
    //    parm {
    //        name    "autoBindCenter"
    //        cppname "autoBindCenter"
    //        label   "Auto Bind Center"
    //        type    toggle
    //        default { "on" }
    //        disablewhen "{ doTranslate == 0 }"
    //    }
    //    parm {
    //        name    "bbox_ref_px"
    //        cppname "bbox_ref_px"
    //        label   "BBox Ref Translate X"
    //        type    float
    //        default { "0" }
    //        disablewhen "{ autoBindCenter == 1 doTranslate == 1 }"
    //        range   { -1 1 }
    //    }
    //    parm {
    //        name    "bbox_ref_py"
    //        cppname "bbox_ref_py"
    //        label   "BBox Ref Translate Y"
    //        type    float
    //        default { "0" }
    //        disablewhen "{ autoBindCenter == 1 doTranslate == 1 }"
    //        range   { -1 1 }
    //    }
    //    parm {
    //        name    "bbox_ref_pz"
    //        cppname "bbox_ref_pz"
    //        label   "BBox Ref Translate Z"
    //        type    float
    //        default { "0" }
    //        disablewhen "{ autoBindCenter == 1 doTranslate == 1 }"
    //        range   { -1 1 }
    //    }
    //    parm {
    //        name    "bbox_ref_pcx"
    //        cppname "bbox_ref_pcx"
    //        label   "BBox Ref Center X"
    //        type    float
    //        default { "1" }
    //        disablewhen "{ autoBindCenter == 1 doTranslate == 1 }"
    //        hidewhen "{ ezMode == 1 }"
    //        range   { 0 1 }
    //    }
    //    parm {
    //        name    "bbox_ref_pcy"
    //        cppname "bbox_ref_pcy"
    //        label   "BBox Ref Center Y"
    //        type    float
    //        default { "1" }
    //        disablewhen "{ autoBindCenter == 1 doTranslate == 1 }"
    //        hidewhen "{ ezMode == 1 }"
    //        range   { 0 1 }
    //    }
    //    parm {
    //        name    "bbox_ref_pcz"
    //        cppname "bbox_ref_pcz"
    //        label   "BBox Ref Center Z"
    //        type    float
    //        default { "1" }
    //        disablewhen "{ autoBindCenter == 1 doTranslate == 1 }"
    //        hidewhen "{ ezMode == 1 }"
    //        range   { 0 1 }
    //    }
    //    parm {
    //        name    "sepparm3"
    //        label   "Separator"
    //        type    separator
    //        default { "" }
    //    }
    //    parm {
    //        name    "px"
    //        cppname "px"
    //        label   "Pivot Translate X"
    //        type    float
    //        default { "0" }
    //        range   { -1 1 }
    //    }
    //    parm {
    //        name    "py"
    //        cppname "py"
    //        label   "Pivot Translate Y"
    //        type    float
    //        default { "0" }
    //        range   { -1 1 }
    //    }
    //    parm {
    //        name    "pz"
    //        cppname "pz"
    //        label   "Pivot Translate Z"
    //        type    float
    //        default { "0" }
    //        range   { -1 1 }
    //    }
    //}



)THEDSFILE"
R"THEDSFILE(


    group {
        name    "matchingtranslate_2_3"
        label   "Matching Scale"
        hidewhentab "{ doScale == 0 }"

        parm {
            name    "scaleVolume"
            cppname "ScaleVolume"
            label   "Scale Volume"
            type    toggle
            default { "0" }
            joinnext
        }
        parm {
            name    "volumeGroup"
            cppname "VolumeGroup"
            label   "Volume Group"
            type    string
            default { "@name=height" }
            disablewhen "{ scaleVolume == 0 }"
            nolabel
        }
        parm {
            name    "sBiasx"
            cppname "SBiasx"
            label   "Bias Scale X"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "sBiasy"
            cppname "SBiasy"
            label   "Bias Scale Y"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        parm {
            name    "sBiasz"
            cppname "SBiasz"
            label   "Bias Scale Z"
            type    float
            default { "0" }
            range   { -1 1 }
        }
        //parm {
        //    name    "uniScale"
        //    cppname "UniScale"
        //    label   "Uniform Scale"
        //    type    toggle
        //    default { "off" }
        //}
        parm {
            name    "scaleAxis"
            cppname "ScaleAxis"
            label   "Scale Axis"
            type    ordinal
            default { "min" }
            //hidewhen "{ uniScale == 0 }"
            menu {
                "xyz"       "XYZ"
                "x"         "X"
                "y"         "Y"
                "z"         "Z"
                "xyzmin"    "XYZ Min"
                "xyzmiddle" "XYZ Middle"
                "xyzmax"    "XYZ Max"
                "xymin"     "XY Min"
                "yzmin"     "YZ Min"
                "zxmin"     "ZX Min"
                "xymax"     "XY Max"
                "yzmax"     "YZ Max"
                "zxmax"     "ZX Max"
            }
        }
        // parm {
        //     name    "labelparm"
        //     label   " "
        //     type    label
        //     default { "BBox Size 大部分時候设成1即可" }
        // }
        // parm {
        //     name    "labelparm2"
        //     label   " "
        //     type    label
        //     default { "BBox Size set to 1 most of time    " }
        // }
        // parm {
        //     name    "bboxsx"
        //     cppname "bboxsx"
        //     label   "BBox Size X"
        //     type    float
        //     default { "1" }
        //     disablewhen "{ lerpsx == 0 }"
        //     hidewhen "{ ezMode == 1 }"
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "bboxsy"
        //     cppname "bboxsy"
        //     label   "BBox Size Y"
        //     type    float
        //     default { "1" }
        //     disablewhen "{ lerpsy == 0 }"
        //     hidewhen "{ ezMode == 1 }"
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "bboxsz"
        //     cppname "bboxsz"
        //     label   "BBox Size Z"
        //     type    float
        //     default { "1" }
        //     disablewhen "{ lerpsz == 0 }"
        //     hidewhen "{ ezMode == 1 }"
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "bbox_ref_sx"
        //     cppname "bbox_ref_sx"
        //     label   "BBox Ref Scale X"
        //     type    float
        //     default { "1" }
        //     disablewhen "{ lerpsx == 0 }"
        //     hidewhen "{ uniScale == 1 }"
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "bbox_ref_sy"
        //     cppname "bbox_ref_sy"
        //     label   "BBox Ref Scale Y"
        //     type    float
        //     default { "1" }
        //     disablewhen "{ lerpsy == 0 }"
        //     hidewhen "{ uniScale == 1 }"
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "bbox_ref_sz"
        //     cppname "bbox_ref_sz"
        //     label   "BBox Ref Scale Z"
        //     type    float
        //     default { "1" }
        //     disablewhen "{ lerpsz == 0 }"
        //     hidewhen "{ uniScale == 1 }"
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "sepparm"
        //     label   "Separator"
        //     type    separator
        //     default { "" }
        // }
        parm {
            name    "scale"
            cppname "Scale"
            label   "Uniform Scale"
            type    float
            default { "1" }
            range   { -1 1 }
        }
        parm {
            name    "sx"
            cppname "Sx"
            label   "Scale X"
            type    float
            default { "1" }
            //disablewhen "{ lerpsx == 0 }"
            range   { -1 1 }
        }
        parm {
            name    "sy"
            cppname "Sy"
            label   "Scale Y"
            type    float
            default { "1" }
            //disablewhen "{ lerpsy == 0 }"
            range   { -1 1 }
        }
        parm {
            name    "sz"
            cppname "Sz"
            label   "Scale Z"
            type    float
            default { "1" }
            //disablewhen "{ lerpsz == 0 }"
            range   { -1 1 }
        }
        // parm {
        //     name    "padsx"
        //     cppname "padsx"
        //     label   "Padding Scale X"
        //     type    float
        //     default { "0" }
        //     disablewhen "{ lerpsx == 0 }"
        //     hidewhen "{ ezMode == 1 }"
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "padsy"
        //     cppname "padsy"
        //     label   "Padding Scale Y"
        //     type    float
        //     default { "0" }
        //     disablewhen "{ lerpsy == 0 }"
        //     hidewhen "{ ezMode == 1 }"
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "padsz"
        //     cppname "padsz"
        //     label   "Padding Scale Z"
        //     type    float
        //     default { "0" }
        //     disablewhen "{ lerpsz == 0 }"
        //     hidewhen "{ ezMode == 1 }"
        //     range   { -1 1 }
        // }
        // parm {
        //     name    "lerpsx"
        //     cppname "lerpsx"
        //     label   "Lerp Scale X"
        //     type    float
        //     default { "1" }
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "lerpsy"
        //     cppname "lerpsy"
        //     label   "Lerp Scale Y"
        //     type    float
        //     default { "1" }
        //     range   { 0 1 }
        // }
        // parm {
        //     name    "lerpsz"
        //     cppname "lerpsz"
        //     label   "Lerp Scale Z"
        //     type    float
        //     default { "1" }
        //     range   { 0 1 }
        // }
    }













    parm {
        name    "outXformAttrib"
        cppname "OutXformAttrib"
        label   "Out Xform Attrib"
        type    toggle
        default { "0" }
        joinnext
        nolabel
    }
    parm {
        name    "xformAttribName"
        cppname "XformAttribName"
        label   "Xform Attrib Name"
        type    string
        default { "xform" }
        disablewhen "{ outXformAttrib == 0 }"
        joinnext
    }
    parm {
        name    "xformMergeType"
        cppname "XformMergeType"
        label   "Xform Merge Type"
        type    ordinal
        nolabel
        default { "replace" }
        menu {
            "replace"   "Replace Existing"
            "pre"       "Pre-Multiply"
            "post"      "Post-Multiply"
        }
        disablewhen "{ outXformAttrib == 0 }"
    }




    parm {
        name    "subscribeRatio"
        cppname "SubscribeRatio"
        label   "Subscribe Ratio"
        type    integer
        default { 64 }
        range   { 0! 256 }
    }
    parm {
        name    "minGrainSize"
        cppname "MinGrainSize"
        label   "Min Grain Size"
        type    intlog
        default { 1024 }
        range   { 0! 2048 }
    }
}
)THEDSFILE";

PRM_Template*
SOP_FeE_MatchBBox_2_0::buildTemplates()
{
    static PRM_TemplateBuilder templ("SOP_FeE_MatchBBox_2_0.C"_sh, theDsFile);
    if (templ.justBuilt())
    {
        templ.setChoiceListPtr("group"_sh,    &SOP_Node::allGroupMenu);
        templ.setChoiceListPtr("groupRef"_sh, &SOP_Node::allGroupMenu);
        
        templ.setChoiceListPtr("primAttribToXform"_sh,   &SOP_Node::primAttribMenu);
        templ.setChoiceListPtr("pointAttribToXform"_sh,  &SOP_Node::pointAttribMenu);
        templ.setChoiceListPtr("vertexAttribToXform"_sh, &SOP_Node::vertexAttribMenu);
        templ.setChoiceListPtr("detailAttribToXform"_sh, &SOP_Node::detailAttribMenu);
        
        templ.setChoiceListPtr("posAttrib"_sh,     &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("posRefAttrib"_sh,  &SOP_Node::pointAttribReplaceMenu);
        templ.setChoiceListPtr("posRestAttrib"_sh, &SOP_Node::pointAttribReplaceMenu);
        
        templ.setChoiceListPtr("xformAttribName"_sh, &SOP_Node::detailAttribReplaceMenu);
    }
    return templ.templates();
}

const UT_StringHolder SOP_FeE_MatchBBox_2_0::theSOPTypeName("FeE::matchBBox::2.0"_sh);

void
newSopOperator(OP_OperatorTable* table)
{
    OP_Operator* newOp = new OP_Operator(
        SOP_FeE_MatchBBox_2_0::theSOPTypeName,
        "FeE Match Bounding Box",
        SOP_FeE_MatchBBox_2_0::myConstructor,
        SOP_FeE_MatchBBox_2_0::buildTemplates(),
        1,
        3,
        nullptr,
        OP_FLAG_GENERATOR,
        nullptr,
        1,
        "Five elements Elf/Filter/Match");

    newOp->setIconName("SOP_matchsize");
    table->addOperator(newOp);

}





class SOP_FeE_MatchBBox_2_0Verb : public SOP_NodeVerb
{
public:
    SOP_FeE_MatchBBox_2_0Verb() {}
    virtual ~SOP_FeE_MatchBBox_2_0Verb() {}

    virtual SOP_NodeParms *allocParms() const { return new SOP_FeE_MatchBBox_2_0Parms(); }
    virtual UT_StringHolder name() const { return SOP_FeE_MatchBBox_2_0::theSOPTypeName; }

    virtual CookMode cookMode(const SOP_NodeParms *parms) const { return COOK_INPLACE; }

    virtual void cook(const CookParms &cookparms) const;
    
    /// This static data member automatically registers
    /// this verb class at library load time.
    static const SOP_NodeVerb::Register<SOP_FeE_MatchBBox_2_0Verb> theVerb;
};

// The static member variable definition has to be outside the class definition.
// The declaration is inside the class.
const SOP_NodeVerb::Register<SOP_FeE_MatchBBox_2_0Verb> SOP_FeE_MatchBBox_2_0Verb::theVerb;

const SOP_NodeVerb *
SOP_FeE_MatchBBox_2_0::cookVerb() const 
{ 
    return SOP_FeE_MatchBBox_2_0Verb::theVerb.get();
}




static GA_GroupType
sopGroupType(const SOP_FeE_MatchBBox_2_0Parms::GroupType parmGroupType)
{
    using namespace SOP_FeE_MatchBBox_2_0Enums;
    switch (parmGroupType)
    {
    case GroupType::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupType::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupType::POINT:     return GA_GROUP_POINT;      break;
    case GroupType::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupType::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}


static GA_GroupType
sopGroupType(const SOP_FeE_MatchBBox_2_0Parms::GroupTypeRef parmGroupType)
{
    using namespace SOP_FeE_MatchBBox_2_0Enums;
    switch (parmGroupType)
    {
    case GroupTypeRef::GUESS:     return GA_GROUP_INVALID;    break;
    case GroupTypeRef::PRIM:      return GA_GROUP_PRIMITIVE;  break;
    case GroupTypeRef::POINT:     return GA_GROUP_POINT;      break;
    case GroupTypeRef::VERTEX:    return GA_GROUP_VERTEX;     break;
    case GroupTypeRef::EDGE:      return GA_GROUP_EDGE;       break;
    }
    UT_ASSERT_MSG(0, "Unhandled geo0Group type!");
    return GA_GROUP_INVALID;
}


static GFE_ScaleAxis
sopScaleAxis(const SOP_FeE_MatchBBox_2_0Parms::ScaleAxis parmScaleAxis)
{
    using namespace SOP_FeE_MatchBBox_2_0Enums;
    switch (parmScaleAxis)
    {
    case ScaleAxis::XYZ:       return GFE_ScaleAxis::Invalid  ; break;
    case ScaleAxis::X:         return GFE_ScaleAxis::X        ; break;
    case ScaleAxis::Y:         return GFE_ScaleAxis::Y        ; break;
    case ScaleAxis::Z:         return GFE_ScaleAxis::Z        ; break;
    case ScaleAxis::XYZMIN:    return GFE_ScaleAxis::XYZMin   ; break;
    case ScaleAxis::XYZMIDDLE: return GFE_ScaleAxis::XYZMiddle; break;
    case ScaleAxis::XYZMAX:    return GFE_ScaleAxis::XYZMax   ; break;
    case ScaleAxis::XYMIN:     return GFE_ScaleAxis::XYMin    ; break;
    case ScaleAxis::YZMIN:     return GFE_ScaleAxis::YZMin    ; break;
    case ScaleAxis::ZXMIN:     return GFE_ScaleAxis::ZXMin    ; break;
    case ScaleAxis::XYMAX:     return GFE_ScaleAxis::XYMax    ; break;
    case ScaleAxis::YZMAX:     return GFE_ScaleAxis::YZMax    ; break;
    case ScaleAxis::ZXMAX:     return GFE_ScaleAxis::ZXMax    ; break;
    }
    UT_ASSERT_MSG(0, "Unhandled Scale Axis!");
    return GFE_ScaleAxis::Invalid;
}




void
SOP_FeE_MatchBBox_2_0Verb::cook(const SOP_NodeVerb::CookParms &cookparms) const
{
    auto&& sopparms = cookparms.parms<SOP_FeE_MatchBBox_2_0Parms>();
    GA_Detail& outGeo0 = *cookparms.gdh().gdpNC();
    //auto sopcache = (SOP_FeE_MatchBBox_2_0Cache*)cookparms.cache();

    //const GA_Detail& inGeo0 = *cookparms.inputGeo(0);
    const GA_Detail* const inGeo1 = cookparms.inputGeo(1);
    const GA_Detail* const inGeo2 = cookparms.inputGeo(2);

    //outGeo0.replaceWith(inGeo0);

    if (!(sopparms.getDoTranslate() || sopparms.getDoScale() || sopparms.getOutXformAttrib()))
        return;
    
    const GA_GroupType groupType    = sopGroupType(sopparms.getGroupType());
    const GA_GroupType groupTypeRef = sopGroupType(sopparms.getGroupTypeRef());
    const GFE_ScaleAxis scaleAxis = sopScaleAxis(sopparms.getScaleAxis());
    
    UT_AutoInterrupt boss("Processing");
    if (boss.wasInterrupted())
        return;

    const GA_Detail* const geoRef0 = inGeo1 ? inGeo1 : (sopparms.getUseSelfAsRef() ? &outGeo0 : nullptr);
    GFE_MatchBBox matchBBox(outGeo0, geoRef0, inGeo2, &cookparms);
    
    matchBBox.setComputeParm(sopparms.getSubscribeRatio(), sopparms.getMinGrainSize());
    matchBBox.scaleAxis = scaleAxis;
    matchBBox.doTranslate = sopparms.getDoTranslate();
    matchBBox.doScale     = sopparms.getDoScale();
    matchBBox.usePrimBounding = sopparms.getUsePrimBounding();

    
    matchBBox.setPositionAttrib(sopparms.getPosAttrib());
    matchBBox.setPositionRef0Attrib(sopparms.getPosRefAttrib());
    matchBBox.setPositionRef1Attrib(sopparms.getPosRestAttrib());

    
    matchBBox.getOutAttribArray().appendPrimitives(sopparms.getPrimAttribToXform());
    matchBBox.getOutAttribArray().appendPoints    (sopparms.getPointAttribToXform());
    matchBBox.getOutAttribArray().appendVertices  (sopparms.getVertexAttribToXform());
    matchBBox.getOutAttribArray().appendDetails   (sopparms.getDetailAttribToXform());
    
    if (sopparms.getOutXformAttrib())
        matchBBox.setXformAttrib(false, GA_STORE_INVALID, sopparms.getXformAttribName());
    
    
    if (matchBBox.doTranslate)
    {
        matchBBox.tBiasRest[0] = sopparms.getTBiasRestx();
        matchBBox.tBiasRest[1] = sopparms.getTBiasResty();
        matchBBox.tBiasRest[2] = sopparms.getTBiasRestz();
        matchBBox.tBiasRef[0]  = sopparms.getTBiasRefx();
        matchBBox.tBiasRef[1]  = sopparms.getTBiasRefy();
        matchBBox.tBiasRef[2]  = sopparms.getTBiasRefz();
        
        matchBBox.tPost[0] = sopparms.getTx();
        matchBBox.tPost[1] = sopparms.getTy();
        matchBBox.tPost[2] = sopparms.getTz();
    }
    if (matchBBox.doScale)
    {
        matchBBox.sBias[0] = sopparms.getSBiasx();
        matchBBox.sBias[1] = sopparms.getSBiasy();
        matchBBox.sBias[2] = sopparms.getSBiasz();
        
        matchBBox.sPost[0] = sopparms.getSx();
        matchBBox.sPost[1] = sopparms.getSy();
        matchBBox.sPost[2] = sopparms.getSz();
        matchBBox.sPost   *= sopparms.getScale();
    }
    
    //if (sopparms.getRepairPrecision())
    //    matchBBox.setRepairPrecision(sopparms.getRepairPrecisionThreshold());
    
    
    matchBBox.groupParser.setGroup(groupType, sopparms.getGroup());
    matchBBox.groupParserRef0.setGroup(groupTypeRef, sopparms.getGroupRef());
    matchBBox.computeAndBumpDataId();

}


