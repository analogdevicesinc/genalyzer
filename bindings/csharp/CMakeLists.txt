# ---------------------------------------------------------------------------
# C# (.NET) binding build integration
#
# Driven by the `dotnet` SDK (located by the parent bindings/CMakeLists.txt and
# cached as DOTNET_EXECUTABLE).  Mirrors the add_custom_target style used by
# doc/CMakeLists.txt.  The managed assembly P/Invokes into the native
# `genalyzer` shared library at runtime, so every dotnet invocation is given
# GENALYZER_LIB_PATH pointing at the freshly built native library directory,
# which the binding's DllImportResolver (see NativeMethods.cs) honors.
# ---------------------------------------------------------------------------

set(CSHARP_SOLUTION  ${CMAKE_CURRENT_SOURCE_DIR}/Genalyzer.sln)
set(CSHARP_PROJECT   ${CMAKE_CURRENT_SOURCE_DIR}/Genalyzer/Genalyzer.csproj)
set(CSHARP_TESTS     ${CMAKE_CURRENT_SOURCE_DIR}/tests/Genalyzer.Tests.csproj)

# Match the .NET build configuration to the CMake configuration.
set(CSHARP_CONFIG $<IF:$<CONFIG:Debug>,Debug,Release>)

# Directory holding the freshly built native libgenalyzer, handed to the
# managed resolver/tests via the GENALYZER_LIB_PATH environment variable.
set(GENALYZER_NATIVE_DIR $<TARGET_FILE_DIR:genalyzer>)

# Build the managed library (all target frameworks declared in the .csproj).
add_custom_target(csharp_binding ALL
  COMMAND ${CMAKE_COMMAND} -E env
          GENALYZER_LIB_PATH=${GENALYZER_NATIVE_DIR}
          ${DOTNET_EXECUTABLE} build ${CSHARP_PROJECT} -c ${CSHARP_CONFIG} --nologo
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS genalyzer
  COMMENT "Building C# binding (dotnet build)")

# Optional: produce a NuGet package (`cmake --build . --target csharp_pack`).
add_custom_target(csharp_pack
  COMMAND ${DOTNET_EXECUTABLE} pack ${CSHARP_PROJECT} -c ${CSHARP_CONFIG} --nologo
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS csharp_binding
  COMMENT "Packing C# binding (dotnet pack)")

# Register the C# test suite with CTest when tests/examples are enabled and the
# test project exists.  The test runner is pointed at the native library via
# GENALYZER_LIB_PATH so the resolver loads the just-built libgenalyzer.
if(BUILD_TESTS_EXAMPLES AND EXISTS ${CSHARP_TESTS})
  add_test(
    NAME csharp_tests
    COMMAND ${CMAKE_COMMAND} -E env
            GENALYZER_LIB_PATH=${GENALYZER_NATIVE_DIR}
            ${DOTNET_EXECUTABLE} test ${CSHARP_TESTS} -c ${CSHARP_CONFIG} --nologo
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
