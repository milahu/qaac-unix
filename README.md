# qaac-unix

QuickTime AAC encoder, ported to Unix



## status

not working



## build

see [build.sh](build.sh)



## todo

use dll files on unix

possible solution: [loadlibrary](https://github.com/taviso/loadlibrary)

```
$ ./build.sh
+ make
[  0%] Building CXX object CMakeFiles/qaac.dir/main.cpp.o
[  1%] Linking CXX executable bin/qaac
ld: CMakeFiles/qaac.dir/main.cpp.o: in function `AudioCodecX::getAvailableOutputSampleRates()':
AudioCodecX.h:58: undefined reference to `AudioCodecGetPropertyInfo'

$ grep AudioCodecGetPropertyInfo *.dll
grep: CoreAudioToolbox.dll: binary file matches
```
