# SWX-ANALYSIS-GENALYZER

This project uses the CMake build system.

### Building in Linux with GCC

1. If the build directory does not already exist:  
   `> mkdir ./build`
2. Build:  
   `> cd ./build/`  
   `> cmake ..`  
   `> make`

### Building in Windows with MSVC

1. If the Visual Studio solution does not already exist, use CMake (cmake-gui) to create it.
2. Build using...
    * Visual Studio: start Visual Studio, open the solution, and build
    * The command line:
        1. Start Menu -> Visual Studio 2015 -> MSBuild Command Prompt for VS2015
        2. cd to swx_analysis_genalyzer\build
        3. Debug build:  
           `msbuild swx.sln /T:Clean;Build /p:Configuration=Debug`
        4. Release build:  
           `msbuild swx.sln /T:Clean;Build /p:Configuration=Release`
