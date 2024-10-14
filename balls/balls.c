/*
	sudo apt install libsdl2-2.0-0 libsdl2-gfx-1.0-0 libsdl2-image-2.0-0 libsdl2-mixer-2.0-0 libsdl2-net-2.0-0 libsdl2-ttf-2.0-0
	sudo apt-get install libsdl2-ttf-dev
	gcc -Wall balls.c -o balls $(sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>
#include <SDL_ttf.h>

#define tailleFenetreH		400
#define tailleFenetreW	640
#define nbBalls				50

// Creation de la fenetre et du canvas de la fenetre
int init(SDL_Window ** mafenetre, SDL_Renderer * canvas, SDL_Renderer ** renderer)
{
	int res=0;
	if (SDL_VideoInit(NULL) < 0)  res = 1;  // SDL_VideoInit renvoie 0 en cas de succes
	SDL_CreateWindowAndRenderer(tailleFenetreW,tailleFenetreH, SDL_WINDOW_SHOWN,mafenetre,renderer);
	SDL_SetRenderDrawColor(canvas, 0, 0, 0, 255);
	SDL_RenderClear(canvas);
	return res;
}

SDL_Texture * getTextureFromImage(const char * nomPic, SDL_Renderer * renderer)
{
	SDL_Surface * image = IMG_Load(nomPic);
	if(!image)
	{
		printf("Erreur de chargement de l'image : %s",SDL_GetError());
		return NULL;
	}
	
	SDL_Texture * texSprite=SDL_CreateTextureFromSurface(renderer, image);
	SDL_FreeSurface(image);

	return texSprite;
}

struct balls
{
	SDL_Texture *  texture;
	SDL_Rect position;
	int friction;
	int delay;
	int vx;
	int vy;
};

int main() 
{
	SDL_Window * mafenetre; 		// Fenetre du programme
	SDL_Event event; 					// Structure pour gerer les evenements clavier, souris, joystick
	SDL_Renderer * renderer;		// Canvas

    TTF_Init();
	TTF_Font * font = TTF_OpenFont("time.ttf", 16);
	SDL_Color blanc = {255, 255, 255}; //attention ce n'est pas un Uint32
	SDL_Surface * texte;
	SDL_Rect textPosition;
	textPosition.x = 0;
	textPosition.y = 0;
	textPosition.w = 128;
	textPosition.h = 32;
	char letemps [48];
		
	init(&mafenetre,renderer,&renderer);

	struct balls tabBalls[nbBalls];
	SDL_Texture *  ballTexGen = getTextureFromImage("ball.png", renderer);
	for (int i = 0; i < nbBalls; i++)
	{
		tabBalls[i].texture = ballTexGen;
		tabBalls[i].position.x = rand()%640;
		tabBalls[i].position.y = rand()%400;	
		tabBalls[i].position.w = 64;	
		tabBalls[i].position.h = 64;
		tabBalls[i].friction = (rand()%10) + 5;
		tabBalls[i].delay = 0;
		tabBalls[i].vx = (rand()%4) - 2;
		tabBalls[i].vy = (rand()%4) - 2;
		if (tabBalls[i].vx == 0) tabBalls[i].vx = 1;
		if (tabBalls[i].vy == 0) tabBalls[i].vy = 1;		
	}

	int fin = 0;
	int nbreBoucle = 0;
	float cumulFps = 0;
	float moyenneFps = 0;
	while (!fin) 
	{
		Uint64 start = SDL_GetPerformanceCounter();		
		
		SDL_RenderClear(renderer);

		// Boucle principale
		if (SDL_PollEvent(&event)) 
		{ 												// Scrute sans cesse les evenements et renvoie 1
			switch (event.type) 
			{
				case SDL_QUIT: 			// Evenement fermeture de la fenetre
				fin=1;
				break;
				
				case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE) fin = 1;					
				break;
			}
		}

		int nbCollisions = 0;
		int nbTestsCollisions = 0;
		for (int i = 0; i < nbBalls; i++)
		{
			tabBalls[i].delay ++;
			if(tabBalls[i].delay > tabBalls[i].friction)
			{
				tabBalls[i].delay = 0;
				tabBalls[i].position.x += tabBalls[i].vx;
				tabBalls[i].position.y += tabBalls[i].vy;
			}
			SDL_RenderCopy(renderer, tabBalls[i].texture,NULL,&tabBalls[i].position);
			if (tabBalls[i].position.x < 0) tabBalls[i].vx *= -1;
			if (tabBalls[i].position.x + 64 > tailleFenetreW) tabBalls[i].vx *= -1;
			if (tabBalls[i].position.y < 0) tabBalls[i].vy *= -1;
			if (tabBalls[i].position.y + 64 > tailleFenetreH) tabBalls[i].vy *= -1;
		}
		
		/*
			Gerons ici les colissions et a chaque collision ajouter 1 à la variable nbCollisions
			Exemple avec la fonction de la SDL nbCollisions += SDL_HasIntersection(&shipPosition,&tabBalls[i].position);
		*/

		// Fin de l'appel de votre algo de collision
		
		Uint64 end= SDL_GetPerformanceCounter() ;
		float elapseTime = (end - start) / 1000000000.f;		
		cumulFps += 1.0f / elapseTime;
		nbreBoucle++;
		if (nbreBoucle > 1000)
		{
			moyenneFps = cumulFps / nbreBoucle;
			nbreBoucle = 0;
			cumulFps = 0;
		}
		sprintf(letemps ,"%.f %d %d",moyenneFps, nbCollisions, nbTestsCollisions);
		texte = TTF_RenderText_Solid(font, letemps, blanc) ;
		SDL_Texture * message = SDL_CreateTextureFromSurface(renderer, texte);
		SDL_RenderCopy(renderer, message,NULL,&textPosition);

		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyWindow(mafenetre);
	SDL_FreeSurface(texte);
	TTF_CloseFont(font);
    TTF_Quit();
	SDL_Quit();
	exit(0);
}
