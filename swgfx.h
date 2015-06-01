// swgfx.h
//
// several useful graphics utilities
//
// requires sdl, sdl_gfx, and sdl_ttf
//

//RGBA colors
#define WHITE 		0xFFFFFFFF
#define RED   		0xFF0000FF
#define DARK_RED   	0x770000FF
#define GREEN 		0x00FF00FF
#define BLUE  		0x0000FFFF
#define GREY  		0x777777FF
#define DARK_GREEN 	0x007700FF
#define BLACK 		0x00000000
#define UNIT_GREY 	0x01010100
#define OPAQUE 		0x000000FF



void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
  Uint8 *ubuff8;
  Uint16 *ubuff16;
  Uint32 *ubuff32;
  Uint32 color;
  char c1, c2, c3;
 
  if (x < 0 || y < 0 || x > screen->w || y > screen->h) return;

  /* Lock the screen, if needed */
  if(SDL_MUSTLOCK(screen)) {
    if(SDL_LockSurface(screen) < 0) 
      return;
  }
 
  /* Get the color */
  color = SDL_MapRGB( screen->format, r, g, b );
  
  /* How we draw the pixel depends on the bitdepth */
  switch(screen->format->BytesPerPixel) 
    {
    case 1: 
      ubuff8 = (Uint8*) screen->pixels;
      ubuff8 += (y * screen->pitch) + x; 
      *ubuff8 = (Uint8) color;
      break;

    case 2:
      ubuff8 = (Uint8*) screen->pixels;
      ubuff8 += (y * screen->pitch) + (x*2);
      ubuff16 = (Uint16*) ubuff8;
      *ubuff16 = (Uint16) color; 
      break;  

    case 3:
      ubuff8 = (Uint8*) screen->pixels;
      ubuff8 += (y * screen->pitch) + (x*3);
      

      if(SDL_BYTEORDER == SDL_LIL_ENDIAN) {
  c1 = (color & 0xFF0000) >> 16;
  c2 = (color & 0x00FF00) >> 8;
  c3 = (color & 0x0000FF);
      } else {
  c3 = (color & 0xFF0000) >> 16;
  c2 = (color & 0x00FF00) >> 8;
  c1 = (color & 0x0000FF);  
      }

      ubuff8[0] = c3;
      ubuff8[1] = c2;
      ubuff8[2] = c1;
      break;
      
    case 4:
      ubuff8 = (Uint8*) screen->pixels;
      ubuff8 += (y*screen->pitch) + (x*4);
      ubuff32 = (Uint32*)ubuff8;
      *ubuff32 = color;
      break;
      
    default:
      fprintf(stderr, "Error: Unknown bitdepth!\n");
    }
  
  /* Unlock the screen if needed */
  if(SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
}



void drawTextColor(SDL_Surface* screen,
              char* string,
              int size,
              int x, int y,
              uint32_t fc,
              uint32_t bc)
{
   SDL_Color foregroundColor = {(fc >> 24), (fc >> 16) % 256, (fc >> 8) % 256};
   SDL_Color backgroundColor = {(bc >> 24), (bc >> 16) % 256, (bc >> 8) % 256};
   TTF_Font* font = TTF_OpenFont("FreeSans.ttf", size);
   SDL_Surface* textSurface = TTF_RenderText_Shaded(font, string,
      foregroundColor, backgroundColor);
   SDL_Rect textLocation = { x-textSurface->w/2, y-textSurface->h/2, 0, 0 }; //hack!
   SDL_BlitSurface(textSurface, NULL, screen, &textLocation);
   SDL_FreeSurface(textSurface);
   TTF_CloseFont(font);
}


void drawGuage(SDL_Surface* screen, 
		int xpos, int ypos, int r,  //r=radius
		char* units, 
		double min, double max, double major_tick, double minor_tick,
		double redline,
		double value)
{
	int i;

	// define the unused angle 
	//  at the bottom of the gauge
	float theta0 = 0; 

	float tick_len = 10;
	float text_gap = -30;
	float text_size = 22*r/175;
	float needle_width = 5;

	uint32_t color;

	char buf[100];

	// draw minor tick marks
	for ( i = 0; i <= (max-min)/minor_tick; i++) {
		float theta = theta0/2 + (i*minor_tick/(max-min)+min)*(2*PI-theta0);
		float valx = xpos - r * sin(theta);
		float valy = ypos + r * cos(theta);
		float valx2 = xpos - (r-tick_len) * sin(theta);
		float valy2 = ypos + (r-tick_len) * cos(theta);
		color = DARK_GREEN;
		if (i*minor_tick > redline)
			color = DARK_RED;
		aalineColor(screen, valx, valy, valx2, valy2, color);
	}


	// draw major tick marks and numbers
	for ( i = 0; i <= (max-min)/major_tick; i++) {
		float theta = theta0/2 + (i*major_tick/(max-min) + min)*(2*PI-theta0);
		float valx = xpos - r * sin(theta);
		float valy = ypos + r * cos(theta);
		float valx2 = xpos - (r-tick_len) * sin(theta);
		float valy2 = ypos + (r-tick_len) * cos(theta);
		float valx3 = xpos - (r+text_gap) * sin(theta);
		float valy3 = ypos + (r+text_gap) * cos(theta);

		color = GREEN;
		if (i*major_tick > redline)
			color = RED;
		aalineColor(screen, valx, valy, valx2, valy2, color);

		if (!(i == 0 && theta0 == 0)) {
			sprintf(buf,"%1.0f",i*major_tick+min);	
			drawTextColor(screen,buf,text_size,valx3,valy3,GREEN,BLACK);
		}
	}



	// draw needle
	float theta = theta0/2 + ((value-min)/(max-min))*(2*PI-theta0);
	float valx = xpos - r * sin(theta);
	float valy = ypos + r * cos(theta);
	float valx2 = xpos - needle_width * sin(theta-PI/2);
	float valy2 = ypos + needle_width * cos(theta-PI/2);
	float valx3 = xpos - needle_width * sin(theta+PI/2);
	float valy3 = ypos + needle_width * cos(theta+PI/2);
	//filledTrigonColor(screen,valx,valy,valx2,valy2,valx3,valy3,RED);
	aalineColor(screen,valx,valy,valx2,valy2,RED);
	aalineColor(screen,valx2,valy2,valx3,valy3,RED);
	aalineColor(screen,valx,valy,valx3,valy3,RED);
	filledTrigonColor(screen,valx,valy,valx2,valy2,valx3,valy3,RED);

	// units
	drawTextColor(screen,units,text_size,xpos,ypos+r/3,GREEN,BLACK);
	

	// digital
	sprintf(buf,"%1.2f",value);
	drawTextColor(screen,buf,text_size,xpos,ypos+r/5,GREEN,BLACK);

}


