rem resourceフォルダに.png .wav .mp3を格納しておいてください。
cd resource
call em++ ..\main.cpp -o shooter.html -I/raylib/raylib/src -L/raylib/raylib/src -lraylib.web -s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 --shell-file /raylib/raylib/src/shell.html  -DPLATFORM_WEB -O2 --embed-file .
cd ..
