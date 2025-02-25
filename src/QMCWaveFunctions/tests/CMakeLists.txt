#//////////////////////////////////////////////////////////////////////////////////////
#// This file is distributed under the University of Illinois/NCSA Open Source License.
#// See LICENSE file in top directory for details.
#//
#// Copyright (c) 2016 Jeongnim Kim and QMCPACK developers.
#//
#// File developed by: Ye Luo, yeluo@anl.gov, Argonne National Laboratory
#//                    Mark Dewing, markdewing@gmail.com, University of Illinois at Urbana-Champaign
#//
#// File created by: Ye Luo, yeluo@anl.gov, Argonne National Laboratory
#//////////////////////////////////////////////////////////////////////////////////////
    
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QMCPACK_UNIT_TEST_DIR})

SET(SRC_DIR wavefunction)
SET(UTEST_EXE test_${SRC_DIR})
SET(UTEST_NAME deterministic-unit_test_${SRC_DIR})

SET(UTEST_DIR ${qmcpack_BINARY_DIR}/tests/wavefunction)
#SET(UTEST_HDF_INPUT ${qmcpack_SOURCE_DIR}/tests/solids/monoO_1x1x1_pp/pwscf.pwscf.h5)
SET(UTEST_HDF_INPUT ${qmcpack_SOURCE_DIR}/tests/solids/diamondC_1x1x1_pp/pwscf.pwscf.h5)
SET(UTEST_HDF_INPUT2 ${qmcpack_SOURCE_DIR}/tests/solids/bccH_1x1x1_ae/pwscf.pwscf.h5)
SET(UTEST_HDF_INPUT3 ${qmcpack_SOURCE_DIR}/tests/solids/LiH_solid_1x1x1_pp/LiH-arb.pwscf.h5)
SET(UTEST_HDF_INPUT4 ${qmcpack_SOURCE_DIR}/tests/solids/monoO_noncollinear_1x1x1_pp/o2_45deg_spins.pwscf.h5)
SET(UTEST_HDF_INPUT5 ${qmcpack_SOURCE_DIR}/tests/solids/diamondC_1x1x1-Gaussian_pp_Tw_cplx/C_diamond-twist-third.h5)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory "${UTEST_DIR}")
MAYBE_SYMLINK(${UTEST_HDF_INPUT} ${UTEST_DIR}/pwscf.pwscf.h5)
MAYBE_SYMLINK(${UTEST_HDF_INPUT2} ${UTEST_DIR}/bccH.pwscf.h5)
MAYBE_SYMLINK(${UTEST_HDF_INPUT3} ${UTEST_DIR}/LiH-arb.pwscf.h5)
MAYBE_SYMLINK(${UTEST_HDF_INPUT4} ${UTEST_DIR}/o2_45deg_spins.pwscf.h5)
MAYBE_SYMLINK(${UTEST_HDF_INPUT5} ${UTEST_DIR}/C_diamond-twist-third.h5)

SET(FILES_TO_COPY he_sto3g.wfj.xml  ne_def2_svp.wfnoj.xml hcn.structure.xml hcn.wfnoj.xml hcn_downdet.cuspInfo.xml hcn_updet.cuspInfo.xml
    ethanol.structure.xml ethanol.wfnoj.xml ethanol_updet.cuspInfo.xml ethanol_downdet.cuspInfo.xml C_diamond-twist-third.structure.xml C_diamond-twist-third.wfj.xml)

FOREACH(fname ${FILES_TO_COPY})
  EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${fname}" ${UTEST_DIR})
ENDFOREACH()

IF ((NOT QMC_MIXED_PRECISION) AND (NOT QMC_CUDA))
  IF (NOT QMC_COMPLEX)
    SET(MO_SRCS test_MO.cpp test_multiquintic_spline.cpp)
    IF (ENABLE_SOA)
      SET(MO_SRCS ${MO_SRCS} test_soa_cusp_corr.cpp)
    ELSE()
      SET(MO_SRCS ${MO_SRCS} test_cusp_corr.cpp)
    ENDIF()
  ELSE()
    IF (ENABLE_SOA)
      SET(MO_SRCS test_pyscf_complex_MO.cpp)
    ENDIF()
  ENDIF()
ENDIF()

ADD_EXECUTABLE(${UTEST_EXE} test_wf.cpp test_bspline_jastrow.cpp test_counting_jastrow.cpp test_einset.cpp test_pw.cpp
               test_polynomial_eeI_jastrow.cpp test_dirac_det.cpp test_multi_dirac_determinant.cpp test_dirac_matrix.cpp
               test_wavefunction_factory.cpp test_rpa_jastrow.cpp test_example_he.cpp test_user_jastrow.cpp
               test_lattice_gaussian.cpp test_kspace_jastrow.cpp
               test_short_range_cusp_jastrow.cpp test_TrialWaveFunction.cpp ${MO_SRCS})
TARGET_LINK_LIBRARIES(${UTEST_EXE} qmc qmcwfs qmcbase qmcutil ${QMC_UTIL_LIBS} ${MPI_LIBRARY})

ADD_UNIT_TEST(${UTEST_NAME} "${QMCPACK_UNIT_TEST_DIR}/${UTEST_EXE}")
SET_TESTS_PROPERTIES(${UTEST_NAME} PROPERTIES WORKING_DIRECTORY ${UTEST_DIR})
