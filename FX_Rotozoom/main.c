﻿/* =============================================================================
                  ██╗   ██╗██╗  ██╗ ██████╗ ███╗   ██╗██╗  ██╗
                  ██║   ██║██║ ██╔╝██╔═══██╗████╗  ██║╚██╗██╔╝
                  ██║   ██║█████╔╝ ██║   ██║██╔██╗ ██║ ╚███╔╝
                  ██║   ██║██╔═██╗ ██║   ██║██║╚██╗██║ ██╔██╗
                  ╚██████╔╝██║  ██╗╚██████╔╝██║ ╚████║██╔╝ ██╗
                   ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝
  File name:    main.c
  Date:         23 07 2017
  Author:       Power.
  Description:  Oldschool demoeffect - FX Rotozoom.
============================================================================= */

/* =============================================================================
                                 DEBUG Section
============================================================================= */


/* =============================================================================
                                 Include Files
============================================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
/* -User specific- */
#include <math.h>
/* -User specific- */

/* =============================================================================
                          Private defines and typedefs
============================================================================= */
/* FX specific. */
#define SDL_SUBSYSTEM_TO_INIT   SDL_INIT_VIDEO
#define FX_NAME                 "FX_Rotozoom by UKONX"
#define SCREEN_WIDTH            ((uint16_t)800)
#define SCREEN_HIGH             ((uint16_t)600)
#define SCREEN_BPP              ((uint8_t)32)
#define YMAX                    ((uint16_t)(SCREEN_HIGH - (SCREEN_HIGH / 3)))
#define ALPHA_MASK              ((uint32_t)0xFF000000)
#define RED_MASK                ((uint32_t)0x00FF0000)
#define GREEN_MASK              ((uint32_t)0x0000FF00)
#define BLUE_MASK               ((uint32_t)0x000000FF)

// #define TRUE_FULLSCREEN
// #define DESKTOP_FULLSCREEN
#if defined(TRUE_FULLSCREEN)
  #define SDL_FLAG (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE)
#elif defined(DESKTOP_FULLSCREEN)
  #define SDL_FLAG (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_RESIZABLE)
#else
  #define SDL_FLAG (SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)
#endif
/* FX specific. */

/* -User specific- */
#define UINT8_MAX_VALUE         ((uint8_t)255)
#define MAKE_RGB(r,g,b)         ( ( (r)<<16u) | ((g)<<8) | (b) )
#define GET_RED(rgb)            ((uint8_t)(rgb>>16u))
#define GET_GREEN(rgb)          ((uint8_t)(rgb>>8u))
#define GET_BLUE(rgb)           ((uint8_t)(rgb))
#define MASK_16BITS             (0xFFFF)
#define BIT0                    (1<<0)
#define BIT1                    (1<<1)
#define BIT2                    (1<<2)
#define RANDOM_LINE_SIZE        ((uint8_t)(SCREEN_HIGH/48))
#define min(a)                  ((a) > 255 ? 255 : a)
#define TABLE_SIZE              256
#define TEXTURE_FILENAME        "./texture/ukonx.png"
/* -User specific- */


/* =============================================================================
                        Private constants and variables
============================================================================= */
static SDL_Window   *g_pSDL_Window                              = NULL;
static SDL_Renderer *g_pSDL_Renderer                            = NULL;
static SDL_Surface  *g_pSDL_Image                               = NULL;
static SDL_Surface  *g_pSDL_Screen                              = NULL;
static SDL_Texture  *g_pSDL_Texture                             = NULL;

/* -User specific- */
static uint8_t  *g_pau8Buffer                                   = NULL;
static uint32_t *g_pau32Buffer                                  = NULL;
static uint32_t *g_pau32Mini                                    = NULL;
/* -User specific- */


/* =============================================================================
                        Public constants and variables
============================================================================= */


/* =============================================================================
                        Private function declarations
============================================================================= */
static bool bSetup    (void);
static bool bLoop     (void);
static void vQuit     (void);
static bool bFxSetup  (void);
static void vFxQuit   (void);
static bool bFxLoop   (void);

/* -User specific- */
static void vMakeRotozoom (void);
static void vMakeAndDisplayMini (void);
/* -User specific- */

/* =============================================================================
                               Public functions
============================================================================= */


/*==============================================================================
Function    : main
Describe    : Program entry point.
Parameters  : Don't care.
Returns     : -1 on error else 0.
==============================================================================*/
int main (int argc, char* argv[])
{
  /* Locals variables declaration. */
  bool l_bReturn = false;

  /* Program initialization. */
  l_bReturn = bSetup ();
  if (true == l_bReturn)
  {
    /* Program loop. */
    while (true == bLoop () );
  }

  /* Program de-initialization. */
  vQuit ();

  return (l_bReturn == true ? 0 : -1);
}


/* =============================================================================
                               Private functions
============================================================================= */

/*==============================================================================
Function    :   bSetup
Describe    :   Program setup.
Parameters  :   None.
Returns     :   false on error.
==============================================================================*/
static bool bSetup (void)
{
  /* Locals variables declaration. */
  bool l_bReturn = false;

  /* Initialize the SDL library. */
  if (0 == SDL_Init (SDL_SUBSYSTEM_TO_INIT) )
  {
    /* Initialize PNG loader/parser. */
    if (IMG_INIT_PNG == (IMG_INIT_PNG & IMG_Init (IMG_INIT_PNG) ) )
    {
      /* Create window. */
      g_pSDL_Window = SDL_CreateWindow (FX_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HIGH, SDL_FLAG);
      if (NULL != g_pSDL_Window)
      {
        /* Create renderer. */
        g_pSDL_Renderer = SDL_CreateRenderer (g_pSDL_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (NULL != g_pSDL_Renderer)
        {
          /* Allocate a new RGB surface. */
          g_pSDL_Screen = SDL_CreateRGBSurface (0, SCREEN_WIDTH, SCREEN_HIGH, SCREEN_BPP, RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
          if (NULL != g_pSDL_Screen)
          {
            /* Create a texture for a rendering context. */
            g_pSDL_Texture = SDL_CreateTexture (g_pSDL_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HIGH);
            if (NULL != g_pSDL_Texture)
            {
              /* Fx setup. */
              l_bReturn = bFxSetup ();
            }
            else
            {
              printf ("SDL_CreateTexture failed: %s\n", SDL_GetError () );
            }
          }
          else
          {
            printf ("SDL_CreateRGBSurface failed: %s\n", SDL_GetError () );
          }
        }
        else
        {
          printf ("SDL_CreateWindow failed: %s\n", SDL_GetError () );
        }
      }
      else
      {
        printf ("SDL_CreateWindow failed: %s\n", SDL_GetError () );
      }
    }
    else
    {
      printf ("IMG_Init failed: %s\n", IMG_GetError () );
    }
  }
  else
  {
    printf ("SDL_Init failed: %s\n", SDL_GetError () );
  }

  return (l_bReturn);
}


/*==============================================================================
Function    :   bLoop
Describe    :   Program loop.
Parameters  :   None.
Returns     :   false to quit main loop or on error.
==============================================================================*/
static bool bLoop (void)
{
  /* Locals variables declaration. */
  SDL_Event l_sEvent;
  bool l_bReturn = true;

  /* SDL Event management */
  while(0 != SDL_PollEvent (&l_sEvent) )
  {
    switch (l_sEvent.type)
    {
      case SDL_KEYDOWN:
      {
        switch(l_sEvent.key.keysym.sym)
        {
          case SDLK_ESCAPE:
          {
            l_bReturn = false;
            break;
          }
          default:
          {
            break;
          }
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }

  /* Fx still running ? */
  if (true == l_bReturn)
  {
    /* Fx loop. */
    l_bReturn = bFxLoop ();
    if (true == l_bReturn)
    {
      /* Update SDL Texture */
      if (0 == SDL_UpdateTexture (g_pSDL_Texture, NULL, g_pSDL_Screen->pixels, g_pSDL_Screen->w * sizeof (uint32_t) ) )
      {
        /* Render new texture*/
        if (0 == SDL_RenderCopy (g_pSDL_Renderer, g_pSDL_Texture, NULL, NULL) )
        {
          /* Select new render */
          SDL_RenderPresent (g_pSDL_Renderer);
        }
        else
        {
          l_bReturn = false;
          printf ("SDL_RenderCopy failed: %s\n", SDL_GetError () );
        }
      }
      else
      {
        l_bReturn = false;
        printf ("SDL_UpdateTexture failed: %s\n", SDL_GetError () );
      }
    }
  }

  return (l_bReturn);
}


/*==============================================================================
Function    :   vQuit
Describe    :   Program quit.
Parameters  :   None.
Returns     :   None.
==============================================================================*/
static void vQuit (void)
{
  /* Fx Quit. */
  vFxQuit ();

  /* Free RGB surfaces.*/
  if (NULL != g_pSDL_Screen)
  {
    SDL_FreeSurface (g_pSDL_Screen);
  }

  if (NULL != g_pSDL_Image)
  {
    SDL_FreeSurface (g_pSDL_Image);
  }

  /* Destroy the rendering context and free associated textures. */
  if (NULL != g_pSDL_Renderer)
  {
    SDL_DestroyRenderer (g_pSDL_Renderer);
  }

  /* Destroy window.*/
  if (NULL != g_pSDL_Window)
  {
    SDL_DestroyWindow (g_pSDL_Window);
  }

  /* Destroy the texture. */
  if (NULL != g_pSDL_Texture)
  {
    SDL_DestroyTexture (g_pSDL_Texture);
  }

  /* Cleans up all dynamically loaded library handles, freeing memory. */
  IMG_Quit ();

  /* Clean up all initialized subsystems. */
  if (0UL != SDL_WasInit (SDL_INIT_EVERYTHING))
    SDL_Quit ();
}


/*==============================================================================
Function    :   bFxSetup
Describe    :   Fx setup.
Parameters  :   None.
Returns     :   false on error.
==============================================================================*/
static bool bFxSetup (void)
{
  /* Locals variables declaration. */
  bool l_bReturn = false;

  /* -User specific- */
  g_pau8Buffer = (uint8_t *)malloc (sizeof (uint8_t) * YMAX * SCREEN_WIDTH);
  if (NULL != g_pau8Buffer)
  {
    g_pau32Buffer = (uint32_t *)malloc (sizeof (uint32_t) * YMAX * SCREEN_WIDTH);
    if (NULL != g_pau32Buffer)
    {
      g_pau32Mini = (uint32_t *)malloc (sizeof (uint32_t) * YMAX * SCREEN_WIDTH);
      if (NULL != g_pau32Mini)
      {
        /* Load file for use as an image in a new surface */
        g_pSDL_Image = IMG_Load (TEXTURE_FILENAME);
        if (NULL != g_pSDL_Image)
        {
          l_bReturn = true;
        }
        else
        {
          printf ("IMG_Load failed: %s\n", IMG_GetError () );
        }
      }
    }
  }
  /* -User specific- */

  return (l_bReturn);
}


/*==============================================================================
Function    :   bFxQuit
Describe    :   Fx Quit.
Parameters  :   None.
Returns     :   None.
==============================================================================*/
static void vFxQuit (void)
{
  /* -User specific- */
  if (NULL != g_pau8Buffer)
  {
    free (g_pau8Buffer);
  }
  if (NULL != g_pau32Buffer)
  {
    free (g_pau32Buffer);
  }
  if (NULL != g_pau32Mini)
  {
    free (g_pau32Mini);
  }
  /* -User specific- */
}


/*==============================================================================
Function    :   bFxLoop
Describe    :   Fx loop.
Parameters  :   None.
Returns     :   false on error.
==============================================================================*/
static bool bFxLoop (void)
{
  /* Locals variables declaration. */
  bool l_bReturn = false;

  /* -User specific- */
  vMakeRotozoom ();
  vMakeAndDisplayMini ();

  /* Display effect. */
  for (uint16_t l_u16IndexY = 0u; l_u16IndexY < YMAX; l_u16IndexY++)
  {
    for (uint16_t l_u16IndexX = 0u; l_u16IndexX < SCREEN_WIDTH; l_u16IndexX++)
    {
      uint32_t *l_pau32Screen =   g_pSDL_Screen->pixels
                        + ( (l_u16IndexY + (SCREEN_HIGH/6u) ) * SCREEN_WIDTH * g_pSDL_Screen->format->BytesPerPixel)
                        + (l_u16IndexX * g_pSDL_Screen->format->BytesPerPixel);
      *l_pau32Screen = g_pau32Buffer[(l_u16IndexY * SCREEN_WIDTH) + l_u16IndexX];
    }
  }

  l_bReturn = true;
  /* -User specific- */

  return (l_bReturn);
}


/* -User specific- */
/*==============================================================================
Function    :   vMakeRotozoom
Describe    :   Make Fx rotozoom.
Parameters  :   None.
Returns     :   None.
==============================================================================*/
static void vMakeRotozoom (void)
{
  /* Locals variables declaration. */
  static float l_froto = 0.0f;
  static float l_fzoom = 0.8f;

  /* Compute rotozoom. */
  int32_t l_s32Vector_U0  = 0;
  int32_t l_s32Vector_V0  = 0;
  int32_t l_s32Vector_xx  = (int32_t)(cos (l_froto) * l_fzoom * 16384 * 2);
  int32_t l_s32Vector_yy  = (int32_t)(sin (l_froto) * l_fzoom * 16384 * 2);

  for (uint16_t l_u16IndexY = 0u; l_u16IndexY < YMAX ; l_u16IndexY++)
  {
    int32_t l_s32Vector_U1 = l_s32Vector_U0;
    int32_t l_s32Vector_V1 = l_s32Vector_V0;

    for (uint16_t l_u16IndexX = 0u; l_u16IndexX < SCREEN_WIDTH ; l_u16IndexX++)
    {
      l_s32Vector_U0 += l_s32Vector_xx;
      l_s32Vector_V0 += l_s32Vector_yy;

      uint8_t *p_texture =  (uint8_t *)g_pSDL_Image->pixels
                          + (uint8_t)( (l_s32Vector_V0 >> 15u) >> 2u) * g_pSDL_Image->pitch
                          + (uint8_t)( (l_s32Vector_U0 >> 15u) >> 2u) * g_pSDL_Image->format->BytesPerPixel;

      g_pau32Buffer[(l_u16IndexY * SCREEN_WIDTH) + l_u16IndexX] = MAKE_RGB(*(p_texture + 0u), *(p_texture + 1u), *(p_texture + 2u) );
    }

    l_s32Vector_U0 = l_s32Vector_U1 - l_s32Vector_yy;
    l_s32Vector_V0 = l_s32Vector_V1 + l_s32Vector_xx;
  }

  l_froto += 0.02f;
  l_fzoom = 3*cos (l_froto);
}


/*==============================================================================
Function    :   vMakeAndDisplayMini.
Describe    :   Display 3 miniatures with scroll down.
Parameters  :   None.
Returns     :   None.
==============================================================================*/
static void vMakeAndDisplayMini (void)
{
  /* Locals variables declaration. */
  static uint32_t l_u32ScrollYValue = 0;

  /* Create miniature. */
  for (uint16_t l_u16IndexY = 0u; l_u16IndexY < (YMAX>>2u); l_u16IndexY++)
  {
    for (uint16_t l_u16IndexX = 0u; l_u16IndexX < (SCREEN_WIDTH>>2u); l_u16IndexX++)
    {
      g_pau32Mini[(l_u16IndexY * (SCREEN_WIDTH>>2u) ) + l_u16IndexX] = g_pau32Buffer[ (  (l_u16IndexY * g_pSDL_Screen->format->BytesPerPixel) * SCREEN_WIDTH)
                                                                    + (l_u16IndexX * g_pSDL_Screen->format->BytesPerPixel)];
    }
  }

  /* Display miniatures. */
  for (uint16_t l_u16IndexY = 0u; l_u16IndexY < (YMAX>>2u); l_u16IndexY++)
  {
    for (uint16_t l_u16IndexX = 0u; l_u16IndexX < (SCREEN_WIDTH>>2u); l_u16IndexX++)
    {
      /* Video inversion. */
      uint32_t l_u32RGB     = g_pau32Mini[l_u16IndexY * (SCREEN_WIDTH>>2u) + l_u16IndexX];
      uint8_t l_u8SrcRed    = UINT8_MAX_VALUE - GET_RED(l_u32RGB);
      uint8_t l_u8SrcGreen  = UINT8_MAX_VALUE - GET_GREEN(l_u32RGB);
      uint8_t l_u8SrcBlue   = UINT8_MAX_VALUE - GET_BLUE(l_u32RGB);

      /* First miniature. */
      uint16_t l_u16MiniPosY  = (l_u16IndexY + l_u32ScrollYValue)%YMAX;
      l_u32RGB                = g_pau32Buffer[(l_u16MiniPosY*SCREEN_WIDTH)+(l_u16IndexX+8)];
      uint8_t l_u8DstRed      = min(l_u8SrcRed    + 3 * GET_RED(l_u32RGB)   )>>2u;
      uint8_t l_u8DstGreen    = min(l_u8SrcGreen  + 3 * GET_GREEN(l_u32RGB) )>>2u;
      uint8_t l_u8DstBlue     = min(l_u8SrcBlue   + 3 * GET_BLUE(l_u32RGB)  )>>2u;
      g_pau32Buffer[(l_u16MiniPosY*SCREEN_WIDTH)+(l_u16IndexX+8)] = MAKE_RGB(l_u8DstRed, l_u8DstGreen, l_u8DstBlue);

      /* Second miniature. */
      l_u16MiniPosY = (l_u16MiniPosY + (YMAX/3) ) % YMAX;
      l_u32RGB      = g_pau32Buffer[(l_u16MiniPosY * SCREEN_WIDTH) + (l_u16IndexX + 8)];
      l_u8DstRed    = min(l_u8SrcRed    + GET_RED(l_u32RGB)   )>>1u;
      l_u8DstGreen  = min(l_u8SrcGreen  + GET_GREEN(l_u32RGB) )>>1u;
      l_u8DstBlue   = min(l_u8SrcBlue   + GET_BLUE(l_u32RGB)  )>>1u;
      g_pau32Buffer[(l_u16MiniPosY*SCREEN_WIDTH)+(l_u16IndexX+8)] = MAKE_RGB(l_u8DstRed, l_u8DstGreen, l_u8DstBlue);

      /* Third miniature. */
      l_u16MiniPosY = (l_u16MiniPosY + (YMAX/3) ) % YMAX;
      l_u32RGB      = g_pau32Buffer[(l_u16MiniPosY * SCREEN_WIDTH) + (l_u16IndexX + 8)];
      l_u8DstRed    = min(3 * l_u8SrcRed    + GET_RED(l_u32RGB)   )>>2u;
      l_u8DstGreen  = min(3 * l_u8SrcGreen  + GET_GREEN(l_u32RGB) )>>2u;
      l_u8DstBlue   = min(3 * l_u8SrcBlue   + GET_BLUE(l_u32RGB)  )>>2u;
      g_pau32Buffer[(l_u16MiniPosY*SCREEN_WIDTH)+(l_u16IndexX+8)] = MAKE_RGB(l_u8DstRed, l_u8DstGreen, l_u8DstBlue);
    }
  }

  /* Display upper and lower border. */
  for (uint16_t l_u16IndexY = 0u; l_u16IndexY < (YMAX>>2u); l_u16IndexY++)
  {
    uint32_t l_u32RGB = MAKE_RGB(255,255,255);

    uint32_t l_u16BorderPosY = (l_u16IndexY + l_u32ScrollYValue) % YMAX;
    g_pau32Buffer[(l_u16BorderPosY * SCREEN_WIDTH) + 8] = l_u32RGB;
    g_pau32Buffer[(l_u16BorderPosY * SCREEN_WIDTH) + 8 + (SCREEN_WIDTH>>2u)]=l_u32RGB;

    l_u16BorderPosY = (l_u16BorderPosY + (YMAX/3))%YMAX;
    g_pau32Buffer[(l_u16BorderPosY * SCREEN_WIDTH) + 8] = l_u32RGB;
    g_pau32Buffer[(l_u16BorderPosY * SCREEN_WIDTH) + 8 + (SCREEN_WIDTH>>2u)]=l_u32RGB;

    l_u16BorderPosY = (l_u16BorderPosY + (YMAX/3))%YMAX;
    g_pau32Buffer[(l_u16BorderPosY * SCREEN_WIDTH) + 8] = l_u32RGB;
    g_pau32Buffer[(l_u16BorderPosY * SCREEN_WIDTH) + 8 + (SCREEN_WIDTH>>2u)]=l_u32RGB;
  }

  /* Display left and right border. */
  for (uint16_t l_u16IndexX=8;l_u16IndexX<(8 + (SCREEN_WIDTH>>2u));l_u16IndexX++)
  {
    uint32_t l_u32RGB         = MAKE_RGB(255,255,255);

    uint32_t l_u16BorderPosY  = l_u32ScrollYValue%YMAX;
    g_pau32Buffer[(l_u16BorderPosY * SCREEN_WIDTH) + l_u16IndexX] = l_u32RGB;
    g_pau32Buffer[( ( (l_u16BorderPosY + (YMAX>>2u) )%YMAX) * SCREEN_WIDTH) + l_u16IndexX] = l_u32RGB;

    l_u16BorderPosY = (l_u16BorderPosY + (YMAX/3) )%YMAX;
    g_pau32Buffer[(l_u16BorderPosY*SCREEN_WIDTH)+l_u16IndexX] = l_u32RGB;
    g_pau32Buffer[( ( (l_u16BorderPosY+(YMAX>>2u) )%YMAX) * SCREEN_WIDTH) + l_u16IndexX] = l_u32RGB;

    l_u16BorderPosY = (l_u16BorderPosY + (YMAX/3) )%YMAX;
    g_pau32Buffer[(l_u16BorderPosY*SCREEN_WIDTH) + l_u16IndexX] = l_u32RGB;
    g_pau32Buffer[( ( (l_u16BorderPosY+(YMAX>>2u) )%YMAX) * SCREEN_WIDTH) + l_u16IndexX] = l_u32RGB;
  }

  l_u32ScrollYValue++;
}
/* -User specific- */


