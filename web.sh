/usr/lib/emscripten/emcc -o index.html src/main.cpp -Os -Wall libraylib.a -I. -I ./../raylib/src -s USE_GLFW=3 -sFETCH -DPLATFORM_WEB -sGL_ENABLE_GET_PROC_ADDRESS -sTOTAL_MEMORY=512MB
