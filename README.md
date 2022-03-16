# libquicktime vita port

Based on latest libquicktime release with patches for newer ffmpeg.  
Original libquicktime uses loadable plugins to support different formats, vita port was rewritten to have them built-in instead.  
Currently only audiocodec, videocodec and ffmpeg plugins are built-in, that should be enough for most use-cases.

## Building / Installing

```
mkdir build
cd build
cmake ..
make
make install
```

