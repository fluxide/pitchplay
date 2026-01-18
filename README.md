# pitchplay
play sound files, rpgmaker style

this program offers an interface for browsing .ogg, .wav or .mp3 sound files in a directory and playing them at varying pitch and volume.

run the config-build.ps1 file to build everything.
make sure you add cmake to your PATH environment variable for this script to work.
also input the vcpkg toolchain file by editing the file.

### QT SDK is required for building! 
if you dont have it, you might need to get the dlls from the release package, or run windeployqt.exe for the app directory (not guaranteed to work). 