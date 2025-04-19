all: compile link

compile:
	g++ -std=c++17 -c main.cpp -I"C:\Users\Fahad Azfar\Documents\libraries\SFML-2.5.1\include" -DSFML_STATIC  

link:
	g++ main.o -o main.exe -L"C:\Users\Fahad Azfar\Documents\libraries\SFML-2.5.1\lib" -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lsfml-main -lopengl32 -lfreetype -lwinmm -lgdi32 -mwindows

clean:
	del /F /Q main.exe main.o
