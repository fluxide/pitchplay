#make sure you add cmake to your path for this script to work.
#also input the vcpkg toolchain file below.

if (Test-Path build) {Remove-Item build -Recurse}
cmake -S . -B .\build --fresh "-DCMAKE_TOOLCHAIN_FILE=C:/Users/Administrator/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build
pause