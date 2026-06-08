add_subdirectory(c)

# The C# (.NET) binding is optional and only wired in when both the
# BUILD_CSHARP_BINDING option is ON and a `dotnet` SDK is discoverable, so the
# default build never requires the .NET SDK.
if(BUILD_CSHARP_BINDING)
  find_program(DOTNET_EXECUTABLE dotnet)
  if(DOTNET_EXECUTABLE)
    add_subdirectory(csharp)
  else()
    message(STATUS "dotnet SDK not found, skipping C# binding")
  endif()
endif()
