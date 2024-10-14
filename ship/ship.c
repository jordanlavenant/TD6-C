#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <time.h>
#include <SDL_ttf.h>

#define tailleFenetreH 500
#define tailleFenetreW 900
#define nbAsteroids 10
#define vitesseShip 1

int init(SDL_Window **mafenetre, SDL_Renderer **renderer)
{
	int res = 0;
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		res = 1;
	}
	*mafenetre = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, tailleFenetreW, tailleFenetreH, SDL_WINDOW_SHOWN);
	if (*mafenetre == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		res = 1;
	}
	*renderer = SDL_CreateRenderer(*mafenetre, -1, SDL_RENDERER_ACCELERATED);
	if (*renderer == NULL)
	{
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		res = 1;
	}
	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 255);
	SDL_RenderClear(*renderer);
	return res;
}

SDL_Texture *getTextureFromImage(const char *nomPic, SDL_Renderer *renderer)
{
	SDL_Surface *image = IMG_Load(nomPic);
	if (!image)
	{
		printf("Erreur de chargement de l'image : %s\n", SDL_GetError());
		return NULL;
	}

	SDL_Texture *texSprite = SDL_CreateTextureFromSurface(renderer, image);
	SDL_FreeSurface(image);

	return texSprite;
}

struct astro
{
	SDL_Texture *texture;
	SDL_Rect position;
	int friction;
	int delay;
};

int main()
{
	SDL_Window *mafenetre = NULL;
	SDL_Event event;
	SDL_Renderer *renderer = NULL;

	if (init(&mafenetre, &renderer) != 0)
	{
		printf("Failed to initialize!\n");
		return -1;
	}

	if (TTF_Init() == -1)
	{
		printf("TTF_Init: %s\n", TTF_GetError());
		return -1;
	}

	TTF_Font *font = TTF_OpenFont("time.ttf", 16);
	if (!font)
	{
		printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
		return -1;
	}

	SDL_Color blanc = {255, 255, 255};
	SDL_Surface *texte = NULL;
	SDL_Rect textPosition = {0, 0, 128, 32};
	char letemps[48];

	SDL_Texture *texShip = getTextureFromImage("ship.png", renderer);
	if (!texShip)
	{
		printf("Failed to load ship texture!\n");
		return -1;
	}

	SDL_Rect shipPosition = {256, 256, 64, 64};

	struct astro tabAstro[nbAsteroids];
	SDL_Texture *astroTexGen = getTextureFromImage("asteroid.png", renderer);
	if (!astroTexGen)
	{
		printf("Failed to load asteroid texture!\n");
		return -1;
	}

	for (int i = 0; i < nbAsteroids; i++)
	{
		tabAstro[i].texture = astroTexGen;
		tabAstro[i].position.x = rand() % tailleFenetreW;
		tabAstro[i].position.y = rand() % tailleFenetreH;
		tabAstro[i].position.w = 32;
		tabAstro[i].position.h = 32;
		tabAstro[i].friction = (rand() % 10) + 5;
		tabAstro[i].delay = 0;
	}

	int fin = 0;
	int nbreBoucle = 0;
	float cumulFps = 0;
	float moyenneFps = 0;
	int lastDirection = 0;	 // 1 for right, -1 for left
	int totalCollisions = 0; // Variable to store total collisions

	while (!fin)
	{
		Uint64 start = SDL_GetPerformanceCounter();

		SDL_RenderClear(renderer);

		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				fin = 1;
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_UP)
					shipPosition.y -= vitesseShip;
				if (event.key.keysym.sym == SDLK_DOWN)
					shipPosition.y += vitesseShip;
				if (event.key.keysym.sym == SDLK_RIGHT)
				{
					lastDirection = 1;
				}
				if (event.key.keysym.sym == SDLK_LEFT)
				{
					lastDirection = -1;
				}
				if (event.key.keysym.sym == SDLK_ESCAPE)
					fin = 1;
				break;
			}
		}

		// Move the ship in the last direction
		if (lastDirection == 1)
		{
			if (shipPosition.x + shipPosition.w + vitesseShip <= tailleFenetreW)
			{
				shipPosition.x += vitesseShip;
			}
		}
		else if (lastDirection == -1)
		{
			if (shipPosition.x - vitesseShip >= 0)
			{
				shipPosition.x -= vitesseShip;
			}
		}

		int nbCollisions = 0;
		for (int i = 0; i < nbAsteroids; i++)
		{
			tabAstro[i].delay++;
			if (tabAstro[i].delay > tabAstro[i].friction)
			{
				tabAstro[i].delay = 0;
				tabAstro[i].position.y++;
			}
			SDL_RenderCopy(renderer, tabAstro[i].texture, NULL, &tabAstro[i].position);
			if (tabAstro[i].position.y > tailleFenetreH)
				tabAstro[i].position.y = -32;

			nbCollisions += SDL_HasIntersection(&shipPosition, &tabAstro[i].position);
		}

		totalCollisions += nbCollisions; // Increment total collisions

		SDL_RenderCopy(renderer, texShip, NULL, &shipPosition);

		Uint64 end = SDL_GetPerformanceCounter();
		float elapseTime = (end - start) / 1000000000.f;
		cumulFps += 1.0f / elapseTime;
		nbreBoucle++;
		if (nbreBoucle > 1000)
		{
			moyenneFps = cumulFps / nbreBoucle;
			nbreBoucle = 0;
			cumulFps = 0;
		}
		sprintf(letemps, "%.f %d", moyenneFps, nbCollisions);
		texte = TTF_RenderText_Solid(font, letemps, blanc);
		SDL_Texture *message = SDL_CreateTextureFromSurface(renderer, texte);
		SDL_RenderCopy(renderer, message, NULL, &textPosition);
		SDL_FreeSurface(texte);
		SDL_DestroyTexture(message);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyWindow(mafenetre);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}