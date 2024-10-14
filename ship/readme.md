```
    gcc -Wall ship.c -o ship $(sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf
    
    ./ship
```