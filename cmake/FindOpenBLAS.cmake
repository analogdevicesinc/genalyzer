SET(Open_BLAS_INCLUDE_SEARCH_PATHS
    /usr/include
    /usr/include/openblas
    /usr/include/openblas-base
    /usr/local/include
    /usr/local/include/openblas
    /usr/local/include/openblas-base
    /opt/OpenBLAS/include
    $ENV{OpenBLAS_HOME}
    $ENV{OpenBLAS_HOME}/include
)

SET(Open_BLAS_LIB_SEARCH_PATHS
    /lib/
    /lib/openblas-base
    /lib64/
    /usr/lib
    /usr/lib/openblas-base
    /usr/lib64
    /usr/local/lib
    /usr/local/lib64
    /opt/OpenBLAS/lib
    $ENV{OpenBLAS}cd
    $ENV{OpenBLAS}/lib
    $ENV{OpenBLAS_HOME}
    $ENV{OpenBLAS_HOME}/lib
)

FIND_PATH(OpenBLAS_INCLUDE_DIR NAMES
    cblas.h PATHS ${Open_BLAS_INCLUDE_SEARCH_PATHS}
)

FIND_LIBRARY(OpenBLAS_LIB NAMES
  openblas PATHS ${Open_BLAS_LIB_SEARCH_PATHS}
)

message("OpenBLAS_INCLUDE_DIR: ${OpenBLAS_INCLUDE_DIR}, OpenBLAS_LIB: ${OpenBLAS_LIB}")
