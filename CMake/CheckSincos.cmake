INCLUDE(${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)

SET(SINCOS_INCLUDE cmath)

SET(ENABLE_MASS FALSE CACHE BOOL "ENABLE MASS math libraries for Power architecture")
# This needs to go before HAVE_SINCOS
IF(ENABLE_MASS)
  INCLUDE(CMake/FindIBMMASS.cmake)
  IF(HAVE_MASS)
    SET(CMAKE_REQUIRED_INCLUDES ${MASS_INCLUDE_DIRECTORIES} ${CMAKE_REQUIRED_INCLUDES})
    SET(CMAKE_REQUIRED_LINK_OPTIONS ${MASS_LINKER_FLAGS})
    SET(CMAKE_REQUIRED_LIBRARIES ${MASS_LIBRARIES})
  ENDIF(HAVE_MASS)
ENDIF(ENABLE_MASS)

MESSAGE("CMAKE_REQUIRED_INCLUDES: ${CMAKE_REQUIRED_INCLUDES}")
MESSAGE("CMAKE_REQUIRED_LINK_OPTIONS: ${CMAKE_REQUIRED_LINK_OPTIONS}")
MESSAGE("CMAKE_REQUIRED_LIBRARIES: ${CMAKE_REQUIRED_LIBRARIES}")

SET( SINCOS_TEST_SRC
  "#include \"${SINCOS_INCLUDE}\"
int main(void) {
double input = 1.1;
double outsin, outcos;;
sincos(input,&outsin, &outcos);
}
")

CHECK_CXX_SOURCE_COMPILES("${SINCOS_TEST_SRC}" HAVE_SINCOS)
