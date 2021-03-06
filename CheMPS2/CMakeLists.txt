
cmake_policy (SET CMP0022 NEW)
cmake_policy (SET CMP0028 NEW)

set (CHEMPS2LIB_SOURCE_FILES "CASPT2.cpp"
                             "CASSCF.cpp"
                             "CASSCFdebug.cpp"
                             "CASSCFnewtonraphson.cpp"
                             "CASSCFpt2.cpp"
                             "CDensityMatrix.cpp"
                             "CTensorT.cpp"
                             "CTensorL.cpp"                           
                             "CTensorLT.cpp"
                             "CTensorO.cpp"                            
                             "CTensorF0.cpp"
                             "CTensorF0T.cpp"
                             "CTensorF1.cpp"
                             "CTensorF1T.cpp"
                             "CTensorS0.cpp"
                             "CTensorS0T.cpp"
                             "CTensorS1.cpp"
                             "CTensorS1T.cpp"
                             "CTensorQ.cpp"
                             "CTensorQT.cpp"
                             "CTensorX.cpp"
                             "CSobject.cpp"
                             "CHeffNS.cpp"
                             "CHeffNS_1S.cpp"
                             "CHeffNS_1S_D1.cpp"
                             "CHeffNS_1S_D2.cpp"
                             "CHeffNS_1S_D3.cpp"
                             "CHeffNS_1S_D4.cpp"
                             "CHeffNSDiagrams0_B.cpp"
                             "CHeffNSDiagrams1_B.cpp"
                             "CHeffNSDiagrams2_B.cpp"
                             "CHeffNSDiagrams3_B.cpp"
                             "CHeffNSDiagrams4_B.cpp"
                             "CHeffNSDiagrams5_B.cpp"
                             "CTensorOperator.cpp"
                             "CSubSpaceExpander.cpp"
                             "CSubSpaceExpanderDiagramms1Left.cpp"
                             "CSubSpaceExpanderDiagramms1Right.cpp"
                             "CSubSpaceExpanderDiagramms2Left.cpp"
                             "CSubSpaceExpanderDiagramms2Right.cpp"
                             "CSubSpaceExpanderDiagramms3Left.cpp"
                             "CSubSpaceExpanderDiagramms3Right.cpp"
                             "CSubSpaceExpanderDiagramms4Left.cpp"
                             "CSubSpaceExpanderDiagramms4Right.cpp"
                             "ConjugateGradient.cpp"
                             "ConvergenceScheme.cpp"
                             "Correlations.cpp"
                             "COneDM.cpp"
                             "Cumulant.cpp"
                             "Davidson.cpp"
                             "DIIS.cpp"
                             "DMRG.cpp"
                             "DMRGfock.cpp"
                             "DMRGmpsio.cpp"
                             "DMRGoperators.cpp"
                             "DMRGoperators3RDM.cpp"
                             "DMRGSCFindices.cpp"
                             "DMRGSCFintegrals.cpp"
                             "DMRGSCFmatrix.cpp"
                             "DMRGSCFoptions.cpp"
                             "DMRGSCFrotations.cpp"
                             "DMRGSCFunitary.cpp"
                             "DMRGSCFwtilde.cpp"
                             "DMRGtechnics.cpp"
                             "EdmistonRuedenberg.cpp"
                             "Excitation.cpp"
                             "CFCI.cpp"
                             "FCI.cpp"
                             "FourIndex.cpp"
                             "Hamiltonian.cpp"
                             "HamiltonianOperator.cpp"
                             "Heff.cpp"
                             "HeffDiagonal.cpp"
                             "HeffDiagrams1.cpp"
                             "HeffDiagrams2.cpp"
                             "HeffDiagrams3.cpp"
                             "HeffDiagrams4.cpp"
                             "HeffDiagrams5.cpp"
                             "Initialize.cpp"
                             "Irreps.cpp"
                             "Molden.cpp"
                             "PrintLicense.cpp"
                             "Problem.cpp"
                             "Sobject.cpp"
                             "SyBookkeeper.cpp"
                             "Tensor3RDM.cpp"
                             "TensorF0.cpp"
                             "TensorF1.cpp"
                             "TensorGYZ.cpp"
                             "TensorKM.cpp"
                             "TensorL.cpp"
                             "TensorO.cpp"
                             "TensorOperator.cpp"
                             "TensorQ.cpp"
                             "TensorS0.cpp"
                             "TensorS1.cpp"
                             "TensorT.cpp"
                             "TensorX.cpp"
			                    "TimeEvolution.cpp"
                             "ThreeDM.cpp"
                             "TwoDM.cpp"
                             "CTwoDM.cpp"
                             "CTwoDMBuilder.cpp"
                             "TwoIndex.cpp"
                             "Wigner.cpp"
                             "expokit.f")


add_library (chemps2-base OBJECT ${CHEMPS2LIB_SOURCE_FILES} )
target_include_directories (chemps2-base PRIVATE ${CheMPS2_SOURCE_DIR}/CheMPS2/include/chemps2
                                                 ${HDF5_INCLUDE_DIRS})

if (BUILD_FPIC OR NOT STATIC_ONLY)
    set_target_properties (chemps2-base PROPERTIES POSITION_INDEPENDENT_CODE 1)
endif()

if (NOT STATIC_ONLY)
    add_library           (chemps2-shared SHARED  $<TARGET_OBJECTS:chemps2-base>)
    target_link_libraries (chemps2-shared PRIVATE ${LIBC_INTERJECT})
    target_link_libraries (chemps2-shared PUBLIC  tgt::lapack
                                                  ${HDF5_CXX_LIBRARIES})
                                                  
    set_target_properties (chemps2-shared PROPERTIES SOVERSION ${CheMPS2_LIB_SOVERSION}
                                                     MACOSX_RPATH ON
                                                     OUTPUT_NAME "chemps2"
                                                     EXPORT_NAME "chemps2")
endif()

if (NOT SHARED_ONLY)
    add_library           (chemps2-static STATIC  $<TARGET_OBJECTS:chemps2-base>)
    target_link_libraries (chemps2-static PRIVATE ${LIBC_INTERJECT})
    target_link_libraries (chemps2-static PUBLIC  tgt::lapack
                                                  ${HDF5_CXX_LIBRARIES})
                                                  
    set_target_properties (chemps2-static PROPERTIES SOVERSION ${CheMPS2_LIB_SOVERSION}
                                                     MACOSX_RPATH ON
                                                     OUTPUT_NAME "chemps2"
                                                     EXPORT_NAME "chemps2")
endif()

if (STATIC_ONLY)
    add_library (chemps2-lib ALIAS chemps2-static)
else ()
    add_library (chemps2-lib ALIAS chemps2-shared)
endif()

add_executable             (chemps2-bin executable.cpp)
target_link_libraries      (chemps2-bin ${HDF5_CXX_LIBRARIES} ${HDF5_CXX_HL_LIBRARIES} chemps2-lib ${LIBC_INTERJECT} )
set_target_properties      (chemps2-bin PROPERTIES OUTPUT_NAME "chemps2")

add_executable             (chemps2-dyn dynamics.cpp)
target_link_libraries      (chemps2-dyn ${HDF5_CXX_LIBRARIES} ${HDF5_CXX_HL_LIBRARIES} chemps2-lib ${LIBC_INTERJECT} )
set_target_properties      (chemps2-dyn PROPERTIES OUTPUT_NAME "chemps2dyn")

add_executable             (chemps2-fci fcidynamics.cpp)
target_link_libraries      (chemps2-fci ${HDF5_CXX_LIBRARIES} ${HDF5_CXX_HL_LIBRARIES} chemps2-lib ${LIBC_INTERJECT} tgt::lapack )
set_target_properties      (chemps2-fci PROPERTIES OUTPUT_NAME "chemps2fci")

add_executable             (chemps2-ion ionize.cpp)
target_link_libraries      (chemps2-ion ${HDF5_CXX_LIBRARIES} ${HDF5_CXX_HL_LIBRARIES} chemps2-lib ${LIBC_INTERJECT} tgt::lapack )
set_target_properties      (chemps2-ion PROPERTIES OUTPUT_NAME "chemps2ion")

add_executable             (chemps2-conv convert.cpp)
target_link_libraries      (chemps2-conv ${HDF5_CXX_LIBRARIES} ${HDF5_CXX_HL_LIBRARIES} chemps2-lib ${LIBC_INTERJECT} tgt::lapack )
set_target_properties      (chemps2-conv PROPERTIES OUTPUT_NAME "chemps2conv")

add_executable             (chemps2-play playground.cpp)
target_link_libraries      (chemps2-play ${HDF5_CXX_LIBRARIES} ${HDF5_CXX_HL_LIBRARIES} chemps2-lib ${LIBC_INTERJECT} tgt::lapack )
set_target_properties      (chemps2-play PROPERTIES OUTPUT_NAME "chemps2play")

# <<<  Install  >>>

if (NOT STATIC_ONLY)
    install (TARGETS chemps2-shared chemps2-bin chemps2-dyn chemps2-fci
                     EXPORT "${PROJECT_NAME}Targets-shared"
                     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
                     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if (NOT SHARED_ONLY)
    install (TARGETS chemps2-static chemps2-bin chemps2-dyn chemps2-fci
                     EXPORT "${PROJECT_NAME}Targets-static"
                     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
                     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install (DIRECTORY include/chemps2/
                   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/chemps2
                   FILES_MATCHING PATTERN "*.h")

# <<<  Export interface  >>>

if (NOT STATIC_ONLY)
    if (APPLE)
        set_target_properties(chemps2-shared PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
    endif()
    target_compile_definitions (chemps2-shared INTERFACE USING_${PROJECT_NAME})
    target_include_directories (chemps2-shared INTERFACE $<BUILD_INTERFACE:${CheMPS2_SOURCE_DIR}/CheMPS2/include/chemps2>
                                                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                                                         ${HDF5_INCLUDE_DIRS})
endif()

if (NOT SHARED_ONLY)
    target_compile_definitions (chemps2-static INTERFACE USING_${PROJECT_NAME})
    target_include_directories (chemps2-static INTERFACE $<BUILD_INTERFACE:${CheMPS2_SOURCE_DIR}/CheMPS2/include/chemps2>
                                                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                                                         ${HDF5_INCLUDE_DIRS})
endif()

# <<<  Export Config  >>>

    # explicit "share" not "DATADIR" for CMake search path
set (CMAKECONFIG_INSTALL_DIR "share/cmake/${PROJECT_NAME}")
if (NOT STATIC_ONLY)
    install (EXPORT "${PROJECT_NAME}Targets-shared"
                     NAMESPACE "${PROJECT_NAME}::"
                     DESTINATION ${CMAKECONFIG_INSTALL_DIR})
endif()

if (NOT SHARED_ONLY)
    install (EXPORT "${PROJECT_NAME}Targets-static"
                     NAMESPACE "${PROJECT_NAME}::"
                     DESTINATION ${CMAKECONFIG_INSTALL_DIR})
endif()

