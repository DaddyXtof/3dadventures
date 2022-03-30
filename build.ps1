gcc fastnoiselite.c main.c -o main.exe -Wall -O1 -std=c99 -Wno-missing-braces -I include/ -I D:/msys64/mingw64/include/ -L lib/ -L D:/msys64/mingw64/lib/  -lraylib -lopengl32 -lgdi32 -lwinmm
# Add following parameter for removing console:
# -mwindows
