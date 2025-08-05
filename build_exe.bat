g++ -static-libgcc -static-libstdc++ -c -o PopupWindow.o PopupWindow.cpp -mwindows -municode
g++ -static-libgcc -static-libstdc++ -o Calendar main.cpp PopupWindow.o resource.o -mwindows -municode
