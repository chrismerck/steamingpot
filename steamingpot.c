
/*****************************************************
SteamingPot draws streaming plots
(c) 2010 Stingworks
(c) 2014 Chris Merck
This program may be distributed under the terms of the
 GNU General Public License version 2 or later.
Compiles best on Linux (but may compile on other systems)
*****************************************************/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <SDL.h>
#include "SDL_ttf.h"
#include "SDL_gfxPrimitives.h"
#include "swmath.h"
#include "swgfx.h"
#include "SDL_net.h"

// define size of screen
#define XMAX 1000
#define YMAX 700

#define SAMPLE_CUT_TIME 1

#define UDP_PORT 12345

int
main(int argc, char *argv[])
{
  SDL_Surface    *screen;
  SDL_Event       event;

  int             kspace = false, kup = false, kdown = false, kleft = false, kright = false;
  int             my, mx;

  srand(1);                     /* Fetch a set of fresh random numbers */

  /* Initialize SDL, exit if there is an error. */
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Could not initialize SDL: %s\n",
            SDL_GetError());
    return -1;
  }
  TTF_Init();

  /* When the program is through executing, call SDL_Quit */
  atexit(SDL_Quit);

  /* Grab a surface on the screen */
  screen = SDL_SetVideoMode(XMAX, YMAX, 32, SDL_SWSURFACE | SDL_ANYFORMAT);
  if (!screen) {
    fprintf(stderr, "Couldn't create a surface: %s\n",
            SDL_GetError());
    return -1;
  }
  SDL_WM_SetCaption("SteamingPot - Real-Time Plotting of Glass Data",
                    "steamingpot.png");

  double          x = 0;
  double          y = 0;
  double          z = 0;
  double          rawx, rawy, rawz;
  int             ox = 0;
  int             oy = 0;
  int             oz = 0;
  float           t = 0;
  int             sample = 0;
  float           ot = 0;
  const float     smooth = .2;
  float           cds, btemp;
  float           vbatt = 0, vusb = 0;
  float           theta, psi, phi,stheta = 0, spsi = 0, sphi=0;
  int             b, b2;

  /* networking */
  char rxstr[1600+1];
  UDPsocket sd;
  UDPpacket *p;
  if (SDLNet_Init() < 0) {
    fprintf(stderr,"SDLNet_Init: %s\n",SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
  if (!(sd=SDLNet_UDP_Open(UDP_PORT))) {
    fprintf(stderr,"SDLNet_UDP_Open: %s\n",SDLNet_GetError());
    exit(EXIT_FAILURE);
  }
  if (!(p=SDLNet_AllocPacket(1600))) {
    fprintf(stderr,"SDLNet_AllocPacket: %s\n",SDLNet_GetError());
    exit(EXIT_FAILURE);
  }

#define HIST_SIZE XMAX/2
  double          hist[HIST_SIZE][3];
  int             i, j;
  for (i = 0; i < HIST_SIZE; i++)
    for (j = 0; j < 3; j++)
      hist[i][j] = 0;

  //initialize accelerometer filters
    SWParticle * xparticle = malloc(sizeof(SWParticle));
  SWParticle     *yparticle = malloc(sizeof(SWParticle));
  SWParticle     *zparticle = malloc(sizeof(SWParticle));
  swparticle_init(xparticle);
  swparticle_init(yparticle);
  swparticle_init(zparticle);


  //connect to device
  // TODO


  /* START OF MAIN PROGRAM LOOP */
  do {

  ot = t;
  ox = x;
  oy = y;
  oz = z;

  // if we receive packet from glass,
  //  then run processing
  if (!SDLNet_UDP_Recv(sd,p)) goto do_events;

  printf("Got Packet!\n");

  // format:
  //"Wed Jul 30 12:01:38 EDT 2014,1406736098971,192162356811523,MPL Rotation Vector,0.7809734,-0.039168295,-0.018553311,Other"

  // create null-terminated string from UDP packet
  memcpy(&rxstr[0],p->data,p->len);
  rxstr[p->len] = 0;
  // extract fields
  char datetime[100];
  long time1;
  long time2;
  char sensor[100];
  char class[100];
  sscanf(rxstr,"%[^','],%ld,%ld,%[^','],%lf,%lf,%lf,%s",
      datetime,&time1,&time2,sensor,
      &rawx,&rawy,&rawz,class);

  printf("Packet:%s\n",rxstr);
  sample++;

#if 0
  rawx = rawx;
  rawy = rawy;
  rawz = rawz;
#endif

  //apply particle filter
  /*
  x = swparticle_update(xparticle, rawx);
  y = swparticle_update(yparticle, rawy);
  z = swparticle_update(zparticle, rawz);
  */

  // convert from 3-component quaternion
  //  to nautical angles (zyx/321)
  double q1 = rawx;
  double q2 = rawy;
  double q3 = rawz;
  double q0 = sqrt(1.0-q1*q1-q2*q2-q3*q3);
  q0=isnan(q0)?0:q0;
  double sum = 1.0-q1*q1-q2*q2-q3*q3;
  printf("q = (%f,%f,%f,%f) %f\n",
      q0,q1,q2,q3,sum);
/* // FROM StackExchange
  x = atan2((q2*q3 + q0*q1),0.5-(q1*q1+q2*q2));
  y = asin(-2*(q1*q3+q0*q2));
  z = atan2((q1*q2+q0*q3),0.5-(q2*q2+q3*q3));
  */

#if 0
  // 3-2-1
  phi = atan2(-2*q1*q2 + 2*q0*q3,q1*q1+q0*q0-q3*q3-q2*q2);
  theta = asin(2*(q1*q3+q0*q2));
  psi = atan2(-2*q2*q3 + 2*q0*q1,q3*q3-q2*q2-q1*q1+q0*q0);
#endif

  // 1-2-3
  psi = atan2(2*q2*q3 + 2*q0*q1,-q1*q1+q0*q0+q3*q3-q2*q2);
  phi = asin(2*(q1*q3-q0*q2));
  theta = atan2(2*q1*q2 + 2*q0*q3,-q3*q3-q2*q2+q1*q1+q0*q0);

  /* correct for our sensibilities */
  phi = -phi; // roll inverted
  theta = -theta; // yaw inverted
  psi = psi - PI/2; // flat pitch = 0, up = positive, down = negative

  //phi = q1; theta = q2; psi = q3;

  /*
  phi = rawx;
  theta = rawy;
  psi = rawz;
  */
        

  double hscale = 0.5;
  hist[sample % HIST_SIZE][1] = x = phi * 180. / PI * hscale;
  hist[sample % HIST_SIZE][2] = y = theta * 180. / PI * hscale;
  hist[sample % HIST_SIZE][0] = z = psi * 180. / PI * hscale;

  //normalize acceleration vector
  printf("Pre (x,y,z)=(%f,%f,%f)\n",x,y,z);
  // rad -> deg
  // x = roll
  // y = yaw
  // z = pitch

  //compute pitch and roll

  stheta = weighted_angle_average(stheta, theta, smooth);
  sphi = weighted_angle_average(sphi, phi, smooth);
  spsi = weighted_angle_average(spsi, psi, smooth);

  // do graphics only every few samples
  if (sample % SAMPLE_CUT_TIME == 0) {
    SDL_Flip(screen);
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    int             pos;
    for (pos = 1; pos < HIST_SIZE; pos++) {
      //draw + -1 g lines
        for (i = -1; i <= 1; i++) {
        if (i == 0)
          continue;
        b = (sin(pos / 3.) + 1) * 30;
        setpixel(screen, pos, 130 + i * 32, 0, b, 0);
        setpixel(screen, pos, 330 + i * 32, 0, b, 0);
        setpixel(screen, pos, 530 + i * 32, 0, b, 0);
      }

      b = pos * 1024 / HIST_SIZE;
      if (b > 180)
        b2 = (b - 180) * 2;
      else
        b2 = 0;
      i = (sample - (HIST_SIZE - pos)) % HIST_SIZE;


      if (i < 1)
        i += HIST_SIZE - 1;
      setpixel(screen, pos, 130, 10, 100, 10);
      setpixel(screen, pos, 330, 10, 100, 10);
      setpixel(screen, pos, 530, 10, 100, 10);
      /*
       * setpixel(screen,pos,130-hist[i][0],b,b,b2);
       * 
       * setpixel(screen,pos,330-hist[i][1],b,b,b2);
       * setpixel(screen,pos,530-hist[i][2],b,b,b2);
       */
      aalineColor(screen,
                  pos - 1, 130 - hist[i - 1][0],
                  pos, 130 - hist[i][0],
                  0xFFFFFF00 + b);
      aalineColor(screen,
                  pos - 1, 330 - hist[i - 1][1],
                  pos, 330 - hist[i][1],
                  0xFFFFFF00 + b);
      aalineColor(screen,
                  pos - 1, 530 - hist[i - 1][2],
                  pos, 530 - hist[i][2],
                  0xFFFFFF00 + b);

      }

#define GLEFT   XMAX*5/8
#define GRIGHT  XMAX*7/8
#define GTOP    YMAX/6
#define GMID    YMAX/2
#define GLOW    YMAX*5/6
#define GSIZE   XMAX/10
                printf("%1.02f %1.02f %1.02f\n",
		spsi * 180. / PI,sphi*180./PI,stheta*180./PI);

      drawGuage(screen, GLEFT, GTOP, GSIZE,
                "pitch",
                -180, 180, 90, 10,
                1000,
                spsi * 180. / PI);

      drawGuage(screen, GRIGHT, GTOP, GSIZE,
                "roll",
                -180, 180, 90, 10,
                1000,
                sphi * 180. / PI);

      drawGuage(screen, GLEFT, GMID, GSIZE,
                "yaw",
                -180, 180, 90, 10,
                500,
                stheta * 180. / PI);
#if 0
      drawGuage(screen, GRIGHT, GMID, GSIZE,
                "BTEMP (C)",
                0, 79.9, 20, 5,
                90,
                btemp);

      drawGuage(screen, GLEFT, GLOW, GSIZE,
                "Vbatt (V)",
                0, 6.99, 1, .25,
                5.5,
                vbatt);

      drawGuage(screen, GRIGHT, GLOW, GSIZE,
                "Vext (V)",
                0, 6.99, 1, .25,
                5.5,
                vusb);
#endif

      drawTextColor(screen, "pitch", 12, 20, 130 + 15, GREEN, BLACK);
      drawTextColor(screen, "roll", 12, 20, 330 + 15, GREEN, BLACK);
      drawTextColor(screen, "yaw", 12, 20, 530 + 15, GREEN, BLACK);
      /*
       * drawGuage(screen, GRIGHT  ,GLOW    ,GSIZE, "Ibatt (mA)",
       * 0,699.9,100,10, 500, floor((vusb-vbatt)/1.*1000));
       */

    }
  //determine frame rate

do_events:
    SDL_PumpEvents();
  SDL_GetMouseState(&mx, &my);

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      printf("Got quit event!\n");
      return 0;
      break;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        printf("Got quit key!\n");
        return 0;
        break;
      case SDLK_UP:
        kup = true;
        break;
      case SDLK_DOWN:
        kdown = true;
        break;
      case SDLK_LEFT:
        kleft = true;
        break;
      case SDLK_RIGHT:
        kright = true;
        break;
      case SDLK_SPACE:
        kspace = true;
        break;
      case SDLK_TAB:
        break;
      default:
        break;
      }
      break;
    case SDL_KEYUP:
      switch (event.key.keysym.sym) {
      case SDLK_UP:
        kup = false;
        break;
      case SDLK_DOWN:
        kdown = false;
        break;
      case SDLK_LEFT:
        kleft = false;
        break;
      case SDLK_RIGHT:
        kright = false;
        break;
      case SDLK_SPACE:
        kspace = false;
        break;
      case SDLK_TAB:
        break;
      default:
        break;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      break;
    case SDL_MOUSEBUTTONUP:
      break;
    default:
      break;
    }
  }

  } while (1);

  printf("should never reach here\n");

  return 0;
}
/*
double max(double a, double b){
	if(a>b){
		return a;
	}
	return b;

}
*/
