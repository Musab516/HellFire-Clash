all: compile link

compile:
	g++ -std=c++17 -c main.cpp -I"C:\SFML-2.5.1\include" -DSFML_STATIC  

link:
	g++ main.o -o main.exe -L"C:\SFML-2.5.1\lib" \
	-lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s -lsfml-main \
	-lfreetype -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg \
	-lopengl32 -lwinmm -lgdi32 -luser32 -lkernel32 -mwindows

clean:
	del /F /Q main.exe main.o

