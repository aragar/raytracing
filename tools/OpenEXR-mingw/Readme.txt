This build of OpenEXR 2.0.1 has been prepared by Veselin Georgiev for the trinity project.

It contains several fixes in order to compile the ILM codebase under MinGW32 with GCC 4.7.
As OpenEXR requires zlib, its .a file is included as well (in lib).
Also, the IlmBase and OpenEXR directories have been merged.

The bin/ directory contains a small utility, hdr2exr, compiled from the tools/hdr2exr folder
of the trinity repository. See the ReadMe in bin/ for more details.
