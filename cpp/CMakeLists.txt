cmake_minimum_required( VERSION 3.6 )

project( HDK_FeELib_Project )

# CMAKE_PREFIX_PATH must contain the path to the toolkit/cmake subdirectory of
# the Houdini installation. See the "Compiling with CMake" section of the HDK
# documentation for more details, which describes several options for
# specifying this path.
list( APPEND CMAKE_PREFIX_PATH "$ENV{HFSLatest}/toolkit/cmake" )

# Locate Houdini's libraries and header files.
# Registers an imported library target named 'Houdini'.
find_package( Houdini REQUIRED )



if (1)
    function ( houdini_sop_add_library_std )
        cmake_parse_arguments( H_LIBRARY "" "OUTPUT_VAR;API" "FILES" ${ARGN} )
        foreach ( library_name ${H_LIBRARY_FILES} )

            # Code generation for the embedded DS file in ${library_name}.C.
            houdini_generate_proto_headers( FILES "./src/SOP/${library_name}/${library_name}.C" )

            # Add a library and its source files.
            add_library( ${library_name} SHARED
                "./src/SOP/${library_name}/${library_name}.C"
                "./src/SOP/${library_name}/${library_name}.h"
            )
            # Link against the Houdini libraries, and add required include directories and
            # compile definitions.
            target_link_libraries( ${library_name} Houdini )


            if ( 1 )
                set( lib_path "custom/houdini/dsolib/openvdb_sesi.lib" )
                set( base_target_name "openvdb_sesi" )
                set( target_name ${_houdini_install_root}/${lib_path} )
                # if ( 1 )
                #     set( target_name ${library_name}::Dep::${base_target_name} )
                # else ()
                #     set( target_name ${library_name}::${base_target_name} )
                # endif ()
        
                # add_library( ${target_name} ${H_LIB_TYPE} IMPORTED )
        
                # set( import_property IMPORTED_LOCATION )
        
                # set_target_properties(
                #     ${target_name}
                #     PROPERTIES
                #         ${import_property} ${_houdini_install_root}/${lib_path}
                # )
        
                target_link_libraries( ${library_name} ${target_name} )
            endif ()
    
            # target_link_libraries( ${library_name} "C:/Program Files/Side Effects Software/Houdini 19.5.435/custom/houdini/dsolib/openvdb_sesi.lib" )




            # Include ${CMAKE_CURRENT_BINARY_DIR} for the generated header.
            target_include_directories( ${library_name} PRIVATE
                ${CMAKE_CURRENT_BINARY_DIR}
                "./include"
            )

            # Sets several common target properties, such as the library's output directory.
            houdini_configure_target( ${library_name} INSTDIR "./../dso/19.5" LIB_PREFIX "" )
    
        endforeach ()
    endfunction ()

else()

    function ( houdini_sop_add_library_std )
        cmake_parse_arguments( H_LIBRARY "" "OUTPUT_VAR;API" "FILES" ${ARGN} )

        set( HDK_FeELib_SOP_Files )
        foreach ( library_name ${H_LIBRARY_FILES} )
        
            # Code generation for the embedded DS file in ${library_name}.C.
            houdini_generate_proto_headers( FILES "./src/SOP/${library_name}/${library_name}.C" )
            
            # Add a library and its source files.
            list(APPEND HDK_FeELib_SOP_Files "./src/SOP/${library_name}/${library_name}.C")
            list(APPEND HDK_FeELib_SOP_Files "./src/SOP/${library_name}/${library_name}.h")
        
        endforeach ()

        # message(${HDK_FeELib_SOP_Files})
        add_library(HDK_FeELib_SOP SHARED
            ${HDK_FeELib_SOP_Files}
        )

        # Link against the Houdini libraries, and add required include directories and
        # compile definitions.
        target_link_libraries( HDK_FeELib_SOP Houdini )
        
        # Include ${CMAKE_CURRENT_BINARY_DIR} for the generated header.
        target_include_directories( HDK_FeELib_SOP PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
            "./include"
        )
        
        # Sets several common target properties, such as the library's output directory.
        houdini_configure_target( HDK_FeELib_SOP INSTDIR "./../dso/19.5" LIB_PREFIX "" )

    endfunction ()

endif()


function ( houdini_dm_add_library_std )
    cmake_parse_arguments( H_LIBRARY "" "OUTPUT_VAR;API" "FILES" ${ARGN} )

    foreach ( library_name ${H_LIBRARY_FILES} )
    
        # Add a library and its source files.
        add_library( ${library_name} SHARED
            "./src/DM/${library_name}/${library_name}.C"
        )
        
        # Link against the Houdini libraries, and add required include directories and
        # compile definitions.
        target_link_libraries( ${library_name} Houdini )
        
        # Include ${CMAKE_CURRENT_BINARY_DIR} for the generated header.
        target_include_directories( ${library_name} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}
            "./include"
        )
        
        # Sets several common target properties, such as the library's output directory.
        houdini_configure_target( ${library_name} INSTDIR "./../dso/19.5" LIB_PREFIX "SOP_GFE_" )
    
    endforeach ()

endfunction ()




# file( GLOB HDK_FEELIB_FILE_NAMES LIST_DIRECT false /src/SOP/* )
# aux_source_directory( ./src/SOP/ HDK_FEELIB_FILE_NAMES )

# houdini_sop_add_library_std( FILES ${HDK_FEELIB_FILE_NAMES} )



if (0)

# houdini_sop_add_library_std( FILES
#     SOP_FeE_AttribDel_2_0
# )

houdini_sop_add_library_std( FILES
    SOP_FeE_VDBActivateByPoint_1_0
)
else ()


houdini_sop_add_library_std( FILES
    SOP_FeE_AttribBlend_1_0
    SOP_FeE_AttribCast_1_0
    SOP_FeE_AttribCopy_4_0
    #SOP_FeE_AttribCopyFast_1_0
    SOP_FeE_AttribDel_2_0
    SOP_FeE_AttribDuplicate_1_0
    SOP_FeE_AttribExtremum_2_0
    SOP_FeE_AttribInterpRay_2_0
    SOP_FeE_AttribNormalize_2_0
    SOP_FeE_AttribOut_1_0
    SOP_FeE_AttribPromote_1_0
    SOP_FeE_AttribRemap_1_0
    SOP_FeE_AttribScale_1_0
    #SOP_FeE_AttribSetToDefault_1_0
    SOP_FeE_AttribValReplace_1_0
    SOP_FeE_BezierCurveInsertPoint_1_0
    SOP_FeE_BlockStopCondition_2_0
    SOP_FeE_Carve_1_0
    SOP_FeE_Clip_1_0
    SOP_FeE_Connectivity_1_0
    SOP_FeE_ConvertLine_1_0
    SOP_FeE_ConvexPoly_1_0
    SOP_FeE_Cook_1_0
    SOP_FeE_CornerPoint_1_0
    SOP_FeE_CullingByVolume2DTrace_1_0
    SOP_FeE_CurveDirSoftBlend_1_0
    SOP_FeE_CurveExpand2d_3_0
    SOP_FeE_CurveExtendedLine_3_0
    SOP_FeE_CurveSetExpand2d_3_0
    SOP_FeE_CurveSubtractMesh_1_0
    SOP_FeE_CurveUV_1_0
    SOP_FeE_DelAllAttribGroup_1_0
    SOP_FeE_DelAndUnpack_1_0
    SOP_FeE_DelaunayTriangulate_1_0
    SOP_FeE_DelElement_1_0
    SOP_FeE_DelVertex_1_0
    SOP_FeE_DirCross_2_0
    SOP_FeE_DissolveEdge_1_0
    SOP_FeE_Dynamesh_1_0
    SOP_FeE_EdgeGroupTransfer_3_0
    SOP_FeE_EnumCombine_1_0
    SOP_FeE_Enumerate_1_0
    SOP_FeE_EnumPromote_1_0
    SOP_FeE_ExtendCurve_1_0
    SOP_FeE_ExtendCurveStraight_1_0
    SOP_FeE_ExtractPoint_1_0
    SOP_FeE_ExtremeElem_1_0
    SOP_FeE_ExtrudeBorder2D_1_0
    SOP_FeE_FitOrthoPlane_1_0
    SOP_FeE_FlatEdge_2_0
    SOP_FeE_Fuse_1_0
    SOP_FeE_GroupAttribBoundary_1_0
    SOP_FeE_GroupCombine_1_0
    SOP_FeE_GroupConvexPoint_1_0
    SOP_FeE_GroupCurveEnds_2_0
    SOP_FeE_GroupElemByDir_1_0
    SOP_FeE_GroupExpand_1_0
    SOP_FeE_GroupFillByEnum_1_0
    SOP_FeE_GroupLoopedPrim_1_0
    SOP_FeE_GroupMeshByWinding_1_0
    SOP_FeE_GroupOverlapEdge_1_0
    SOP_FeE_GroupOneNebPoint_1_0
    SOP_FeE_GroupPolyByWinding_1_0
    SOP_FeE_GroupRandomElem_1_0
    SOP_FeE_GroupReverse_1_0
    SOP_FeE_GroupUnshared_1_0
    SOP_FeE_GroupVis_2_0
    SOP_FeE_HasGroup_1_0
    SOP_FeE_HeightFieldProject_2_0
    SOP_FeE_InlinePoint_1_0
    SOP_FeE_InsertPoint_1_0
    SOP_FeE_InsertPointEdge_1_0
    SOP_FeE_Intersection_1_0
    SOP_FeE_JoinCurve_2_0
    SOP_FeE_KuhnMunkres_2_0
    SOP_FeE_MatchAxis_1_0
    SOP_FeE_MatchBBox_2_0
    SOP_FeE_MatchDir2D_1_0
    SOP_FeE_MatchDirOrient_1_0
    SOP_FeE_MatchQuadrant_1_0
    SOP_FeE_Measure_3_0
    SOP_FeE_MeshTopology_1_0
    SOP_FeE_MiniSurf_1_0
    SOP_FeE_Normal2D_1_0
    SOP_FeE_OffsetAttribToIndex_1_0
    SOP_FeE_OffsetVertexOrder_2_0
    SOP_FeE_OnPoly_1_0
    SOP_FeE_OrientAlongCurve_1_0
    SOP_FeE_PointFromVoxel_1_0
    SOP_FeE_PointGenPerElem_1_0
    SOP_FeE_PointInBBox_2_0
    SOP_FeE_PointInMesh_WN_2_0
    SOP_FeE_PolyCut_3_0
    SOP_FeE_PolyExpand2DSimple_2_0
    SOP_FeE_PolyInsertPoint_1_0
    SOP_FeE_PolyReduce2D_4_0
    SOP_FeE_Ray_1_0
    SOP_FeE_RayPrim_1_0
    SOP_FeE_RestDir2D_1_0
    SOP_FeE_RestVectorComponent_1_0
    SOP_FeE_SetVectorComponent_1_0
    SOP_FeE_Skin_1_0
    SOP_FeE_SkinIntersectCurve_2_0
    SOP_FeE_SkinPiece_4_0
    SOP_FeE_SmallPiece_1_0
    # SOP_FeE_Split_2_0
    SOP_FeE_SplitPoint_1_0
    SOP_FeE_Star_1_0
    SOP_FeE_Symmetrise_1_0
    SOP_FeE_Triangulate2D_2_0
    SOP_FeE_UnpackByGroup_1_0
    SOP_FeE_UVGridify_1_0
    SOP_FeE_UVRectify_2_0
    SOP_FeE_UVScaletoWorldSize_0_5
    SOP_FeE_UVScaletoWorldSize_3_0
    SOP_FeE_UVTileSize_1_0
    SOP_FeE_VDBActivateByPoint_1_0
    SOP_FeE_VerticalNormal_1_0
    SOP_FeE_VolumeBlur_1_0
    SOP_FeE_VolumeClear_1_0
    SOP_FeE_VolumeExtremum_1_0
    SOP_FeE_VolumeFromArray_1_0
    SOP_FeE_VolumeNormalize_1_0
    SOP_FeE_VolumeProject_1_0
    SOP_FeE_VolumeRemap_1_0
    SOP_FeE_VoxelFromPoint_1_0
    SOP_FeE_WindingNumber_1_0
    SOP_FeE_XformByAttrib_1_0
)

endif ()


# houdini_dm_add_library_std( FILES
#     DM_InfoHook
#     DM_LightBloomHook
# )










