/* 
 * File:   dial.cpp
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2019/02/10, 22:13
 */


#include <arduino.h>
#include "display.h"
#include "graph.h"
#include "dial_font.h"
#include "dial.h"
#include "dial_prm.h"

extern uint8_t  R_GRAM[Nx][Ny];
extern uint8_t  B_GRAM[Nx][Ny];
extern uint8_t  G_GRAM[Nx][Ny];

int freq_tick = 1000;
long Dial_font[26][13];
int fontpitch;
float xoff_font, yoff_font;
float xoff_point;
int D_center, D_left, D_right;
int yry[Nx][4];
int D_R_inside;
float reso;
float reso_main;

int H_sub1, H_sub5, H_sub10;
int L_sub1, L_sub5, L_sub10;

int H_main1, H_main5, H_main10;
int L_main1, L_main5, L_main10;


/*-------------------------------------------------------------------------------------------------


--------------------------------------------------------------------------------------------------*/
void init_Dial(void) {
  int xg; //yg;
  float xf, yf, arc_main, arc_sub;
  int D_R_tmp;


  if (f_dispmode == 0) {
    D_left = 0; D_right = Nx - 1;
    D_center = (Nx >> 1);
  } else {
    D_left = 0; D_right = Ny - 1;
    D_center = (Ny >> 1);
  }

  D_R_inside = D_R - Dial_space;

  if (f_main_outside == 1) {
    reso = (float)tick_pitch / (float)D_R_inside;
    reso_main = (float)tick_pitch_main / (float)D_R;
    if ( D_R < (Nx / 2) ) arc_main = 1.6 * (float)D_R; else arc_main = 1.6 * 0.5 * (float)Nx;
    if ( D_R_inside < (Nx / 2) ) arc_sub = 1.6 * (float)D_R_inside; else arc_sub = 1.6 * 0.5 * (float)Nx;
  } else {
    reso = (float)tick_pitch / (float)D_R;
    reso_main = (float)tick_pitch_main / (float)D_R_inside;
    if ( D_R < (Nx / 2) ) arc_sub = 1.6 * (float)D_R; else arc_sub = 1.6 * 0.5 * (float)Nx;
    if ( D_R_inside < (Nx / 2) ) arc_main = 1.6 * (float)D_R_inside; else arc_main = 1.6 * 0.5 * (float)Nx;
  }

  reso = 0.1 * reso;
  reso_main = 0.1 * reso_main;


  H_main1 = (int)(  (arc_main / ( 0.1 * (float)tick_pitch_main ) ) * 2.0  );
  L_main1 = (int)(  (arc_main / ( 0.1 * (float)tick_pitch_main ) ) * -1.0 );


  H_main5 = (int)(  (arc_main / ( 0.1 * (float)tick_pitch_main) / 5.0 ) * 2.0  );
  L_main5 = (int)(  (arc_main / ( 0.1 * (float)tick_pitch_main) / 5.0 ) * 0.75 );

  L_main5 /= 2; L_main5++;
  L_main5 *= 2; L_main5++;
  L_main5 *= -1;


  H_main10 = (int)(  (arc_main / ( 0.1 * (float)tick_pitch_main) / 10.0) * 2.0  );
  L_main10 = (int)(  (arc_main / ( 0.1 * (float)tick_pitch_main) / 10.0) * 0.75 );

  L_main10 /= 2; L_main10++;
  L_main10 *= 2;
  L_main10 *= -1;


  H_sub1 = (int)(  (arc_sub / ( 0.1 * (float)tick_pitch) ) * 2.0  );
  L_sub1 = (int)(  (arc_sub / ( 0.1 * (float)tick_pitch) ) * -1.0 );


  H_sub5 = (int)(  (arc_sub / ( 0.1 * (float)tick_pitch) / 5.0) * 2.0  );
  L_sub5 = (int)(  (arc_sub / ( 0.1 * (float)tick_pitch) / 5.0) * 0.75 );

  L_sub5 /= 2; L_sub5++;
  L_sub5 *= 2; L_sub5++;
  L_sub5 *= -1;


  H_sub10 = (int)(  (arc_sub / ( 0.1 * (float)tick_pitch) / 10.0) * 2.0  );
  L_sub10 = (int)(  (arc_sub / ( 0.1 * (float)tick_pitch) / 10.0) * 0.75 );

  L_sub10 /= 2; L_sub10++;
  L_sub10 *= 2;
  L_sub10 *= -1;



  // Region settings  yry[][]--------------------------------------------------------
  for (xg = D_left; xg <= D_right; xg++) {

    xf = (float)xg;
    yf = (float)(D_R * D_R) -
         (xf - (float)D_center) * (xf - (float)D_center);

    if (yf > 0) {
      yry[xg][0] = (int)( 0.5 + sqrt(yf) - (float)D_R + (float)D_height );
      if (yry[xg][0] < 0) yry[xg][0] = 0;
    } else {
      yry[xg][0] = 0;
    }

  }

  if (f_main_outside == 1) D_R_tmp = D_R - tick_main10;
  else D_R_tmp = D_R - tick10;


  for (xg = D_left; xg <= D_right; xg++) {

    xf = (float)xg;
    yf = (float)(D_R_tmp * D_R_tmp) -
         (xf - (float)D_center) * (xf - (float)D_center);

    if (yf > 0) {
      yry[xg][1] = (int)( 0.5 + sqrt(yf) - (float)D_R + (float)D_height );
      if (yry[xg][1] < 0) yry[xg][1] = 0;
    } else {
      yry[xg][1] = 0;
    }

  }


  for (xg = D_left; xg <= D_right; xg++) {

    xf = (float)xg;
    yf = (float)( (D_R_inside + 1) * (D_R_inside + 1) ) -
         (xf - (float)D_center) * (xf - (float)D_center);

    if (yf > 0) {
      yry[xg][2] = (int)( 0.5 + sqrt(yf) - (float)D_R + (float)D_height );
      if (yry[xg][2] < 0) yry[xg][2] = 0;
    } else {
      yry[xg][2] = 0;
    }

  }

  if (f_main_outside == 1) D_R_tmp = D_R_inside - tick10;
  else D_R_tmp = D_R_inside - tick_main10;

  for (xg = D_left; xg <= D_right; xg++) {

    xf = (float)xg;
    yf = (float)(D_R_tmp * D_R_tmp) -
         (xf - (float)D_center) * (xf - (float)D_center);

    if (yf > 0) {
      yry[xg][3] = (int)( 0.5 + sqrt(yf) - (float)D_R + (float)D_height );
      if (yry[xg][3] < 0) yry[xg][3] = 0;
    } else {
      yry[xg][3] = 0;
    }

  }
  //------------------------------------------------------------------------

  Sel_font12();
  //if(f_FONT==0) Sel_font12();
  if (f_FONT == 1) Sel_font14();
  if (f_FONT == 2) Sel_font16();
}
/*-------------------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------------------*/






void Sel_font12(void) {
  int k, j;
  fontpitch = 8.5;
  xoff_font = 2.5;
  yoff_font = 10.0;
  xoff_point = 0.5;

  for (k = 0; k < 10; k++) {
    for (j = 0; j < 13; j++) {
      Dial_font[k+16][j] = Dial_font12[k][j];
    }
  }

}




void Sel_font14(void) {
  int k, j;
  fontpitch = 9;
  xoff_font = 1.5;
  yoff_font = 9.0;
  xoff_point = 0.5;

  for (k = 0; k < 10; k++) {
    for (j = 0; j < 13; j++) {
      Dial_font[k+16][j] = Dial_font14[k][j];
    }
  }

}



void Sel_font16(void) {
  int k, j;
  fontpitch = 10;
  xoff_font = 2.0;
  yoff_font = 8.0;
  xoff_point = 0;

  for (k = 0; k < 10; k++) {
    for (j = 0; j < 13; j++) {
      Dial_font[k+16][j] = Dial_font16[k][j];
    }
  }

}




/*-------------------------------------------------------------------------------
*
--------------------------------------------------------------------------------*/
void Dial(long freq)
{
  int i, k;
  int xg, yg, xi, yi;
  float x, y, angle, a, s, c, xf, yf;
#define ZERO_rad 128
  float sin_[ZERO_rad * 2], cos_[ZERO_rad * 2];

  float xr, yr, fsign;
  int d, dg, dgmax;
  float dgf;
  long fdisp;
  long fx, fy;
  int D_R_tmp;

  if(f_rev==1) freq=-freq;

  if (freq < 0) {
    freq = - freq;
    fsign = -1.0;
  } else {
    fsign = 1.0;
  }

  //----------------------------------------------------
  if (f_dispmode == 0) {

    for (xg = D_left; xg <= D_right; xg++) {
      yg = yry[xg][0];
      for (i = 0; i <= yg; i++) {
        R_GRAM[xg][i] = 0;
        G_GRAM[xg][i] = 0;
        B_GRAM[xg][i] = 0;
      }
    }

  } else {

    for (xg = D_left; xg <= D_right; xg++) {
      yg = yry[xg][0];
      for (i = 0; i <= yg; i++) {
        R_GRAM[Nx - 1 - i][xg] = 0;
        G_GRAM[Nx - 1 - i][xg] = 0;
        B_GRAM[Nx - 1 - i][xg] = 0;
      }
    }

  }



  //------------------------------------------------------------------------
  //    SUB DIAL 
  //------------------------------------------------------------------------

  angle = -(float)( freq % (freq_tick * 10) ) * reso / (float)freq_tick;
  angle *= fsign;


  // Rotation matrix----------------------------------------------
  for (i = -ZERO_rad + 1; i <= ZERO_rad - 1; i++) {
    a = angle + i * reso;
    sin_[i + ZERO_rad] = sin(a); cos_[i + ZERO_rad] = cos(a);
  }


  if (f_main_outside == 1) D_R_tmp = D_R_inside;
  else D_R_tmp = D_R;


  if (f_subtick10 == 1) {

    // Tick 10kHz----------------------------------------
    if (f_dispmode == 0) {

      for (i = L_sub10; i <= H_sub10; i++) {

        // every 10points
        if (fsign > 0) k = i * 10 + ZERO_rad; else k = -i * 10 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick10 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            x = x + (float)D_center;
            y = y - (float)D_R + (float)D_height;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height )
              dot(x, y);
          }
        }
      }

    } else {

      for (i = L_sub10; i <= H_sub10; i++) {

        // every 10points
        if (fsign > 0) k = i * 10 + ZERO_rad; else k = -i * 10 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick10 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            y = (float)(Nx - 1) - ( y - (float)D_R + (float)D_height);
            x = x + (float)D_center;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= Nx - 1 - D_height && yi <= Nx - 1 )
              dot(y, x);
          }
        }
      }

    }

  }



  if (f_subtick5 == 1) {

    // Tick 5kHz----------------------------------------
    if (f_dispmode == 0) {

      for (i = L_sub5; i <= H_sub5; i += 2) {

        // every 5points
        if (fsign > 0) k = i * 5 + ZERO_rad; else k = -i * 5 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick5 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            x = x + (float)D_center;
            y = y - (float)D_R + (float)D_height;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height )
              dot(x, y);
          }
        }
      }

    } else {

      for (i = L_sub5; i <= H_sub5; i += 2) {

        if (fsign > 0) k = i * 5 + ZERO_rad; else k = -i * 5 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick5 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            y = (float)(Nx - 1) - ( y - (float)D_R + (float)D_height);
            x = x + (float)D_center;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= Nx - 1 - D_height && yi <= Nx - 1 )
              dot(y, x);
          }
        }
      }

    }

  }


  if (f_subtick1 == 1) {

    //Tick 1kHz---------------------------------------------------------------------
    if (f_dispmode == 0) {

      for (i = L_sub1; i <= H_sub1; i++) {

        if (i % 5 != 0 ) {

          if (fsign > 0) k = i + ZERO_rad; else k = -i + ZERO_rad;

          s = sin_[k]; c = cos_[k];

          for (xg = -tick_width; xg <= 0; xg++) {
            for (yg = 1 + (D_R - D_R_tmp); yg < tick1 + (D_R - D_R_tmp); yg++) {
              x = c * (float)xg - s * ((float)D_R - (float)yg);
              y = s * (float)xg + c * ((float)D_R - (float)yg);

              x = x + (float)D_center;
              y = y - (float)D_R + (float)D_height;

              xi = (int)x; yi = (int)y;
              if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height ) dot(x, y);
            }
          }
        }
      }

    } else {

      for (i = L_sub1; i <= H_sub1; i++) {

        if (i % 5 != 0 ) {

          if (fsign > 0) k = i + ZERO_rad; else k = -i + ZERO_rad;

          s = sin_[k]; c = cos_[k];

          for (xg = -tick_width; xg <= 0; xg++) {
            for (yg = 1 + (D_R - D_R_tmp); yg < tick1 + (D_R - D_R_tmp); yg++) {
              x = c * (float)xg - s * ((float)D_R - (float)yg);
              y = s * (float)xg + c * ((float)D_R - (float)yg);

              y = (float)(Nx - 1) - ( y - (float)D_R + (float)D_height);
              x = x + (float)D_center;

              xi = (int)x; yi = (int)y;
              if ( xi >= D_left && xi <= D_right && yi >= Nx - 1 - D_height && yi <= Nx - 1 ) dot(y, x);
            }
          }
        }
      }

    }

  }



  if (f_subnum == 1) {

    // Number(kHz)------------------------------------------
    for (i = L_sub10; i <= H_sub10; i++) //
    {
      fdisp = freq + i * (10 * freq_tick);

      if (fdisp >= 0) {

        fdisp /= (10 * freq_tick);
        fdisp *= 10;

        // every 10kHz
        if (fsign > 0) k = i * 10 + ZERO_rad; else k = -i * 10 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        dgmax = 3;
        if (freq_tick_main == 10000) dgmax = 2;


        //-------------------------------------------------------------------------------------
        for (dg = 0; dg < dgmax; dg++)
        {
          d = fdisp % 10;

          //for( xg=0; xg<13; xg++)  // Scanning 13bit
          for ( xg = 0; xg < 9; xg++)
          {
            fx = Dial_font[d + 0x10][xg];
            //for( yg=0; yg<24; yg++)  // Scanning 24bit
            for ( yg = 10; yg < 24; yg++)
            {
              fy = (long)( 1 << ( 23 - yg) );

              if (f_dispmode == 0) {

                if ( ( fx & fy ) == fy )
                {
                  if (dgmax == 1) xr = (float)xg - 6.0 + xoff_font; // (13-1)/2 = 6
                  if (dgmax == 2) xr = (float)xg - 6.0 + xoff_font - ( (float)(dg) - 0.5 ) * (float)fontpitch;
                  if (dgmax == 3) xr = (float)xg - 6.0 + xoff_font - ( (float)(dg) - 1.0 ) * (float)fontpitch;

                  yr = (float)( D_R_tmp - (yg + TNCL) ) + yoff_font;

                  xf = c * (xr) - s * (yr);
                  yf = s * (xr) + c * (yr);

                  xf = xf + (float)D_center;
                  yf = yf - (float)D_R + (float)D_height;

                  xi = (int)xf; yi = (int)yf;
                  if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height ) dot(xf, yf);

                }

              } else {

                if ( ( fx & fy ) == fy )
                {
                  if (dgmax == 1) xr = (float)( (xg + TNCL) - D_R_tmp );
                  if (dgmax == 2) xr = (float)( (xg + TNCL) - D_R_tmp ) - ( (float)(dg) - 1.0 ) * (float)fontpitch;
                  if (dgmax == 3) xr = (float)( (xg + TNCL) - D_R_tmp ) - ( (float)(dg) - 2.0 ) * (float)fontpitch;

                  yr = (float)(12 - yg) + 0.5 * yoff_font;

                  xf = c * (xr) - s * (yr);
                  yf = s * (xr) + c * (yr);

                  xf = xf + (float)D_R - (float)D_height + (float)(Nx - 1);
                  yf = yf + (float)D_center;

                  xi = (int)xf; yi = (int)yf;
                  if ( yi >= D_left && yi <= D_right && xi >= Nx - 1 - D_height && xi <= Nx - 1 ) dot(xf, yf);
                }


              }

            }
          }

          fdisp /= 10;
        }
      }
    }

  }




  //------------------------------------------------------------------------------------
  //   MAIN DIAL 
  //------------------------------------------------------------------------------------

  if (f_main_outside == 1) D_R_tmp = D_R;
  else D_R_tmp = D_R_inside;

  angle = -(float)( freq % (freq_tick_main * 10) ) * reso_main / (float)freq_tick_main;
  angle *= fsign;


  // Rotation matrix -------------------------------
  for (i = -ZERO_rad + 1; i <= ZERO_rad - 1; i++) {
    a = angle + i * reso_main;
    sin_[i + ZERO_rad] = sin(a); cos_[i + ZERO_rad] = cos(a);
  }

  if (f_maintick10 == 1) {

    // Tick 10--------------------------------------
    if (f_dispmode == 0) {

      for (i = L_main10; i <= H_main10; i++) {

        if (fsign > 0) k = i * 10 + ZERO_rad; else k = -i * 10 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick_main10 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            x = x + (float)D_center;
            y = y - (float)D_R + (float)D_height;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height )
              dot(x, y);
          }
        }
      }

    } else {


      for (i = L_main10; i <= H_main10; i++) {

        if (fsign > 0) k = i * 10 + ZERO_rad; else k = -i * 10 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick_main10 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            y = (float)(Nx - 1) - ( y - (float)D_R + (float)D_height);
            x = x + (float)D_center;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= Nx - 1 - D_height && yi <= Nx - 1 )
              dot(y, x);
          }
        }
      }


    }

  }


  if (f_maintick5 == 1) {

    // Tick 5 ----------------------------------------
    if (f_dispmode == 0) {

      for (i = L_main5; i <= H_main5; i += 2) {

        if (fsign > 0) k = i * 5 + ZERO_rad; else k = -i * 5 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick_main5 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            x = x + (float)D_center;
            y = y - (float)D_R + (float)D_height;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height )
              dot(x, y);
          }
        }
      }

    } else {

      for (i = L_main5; i <= H_main5; i += 2) {

        if (fsign > 0) k = i * 5 + ZERO_rad; else k = -i * 5 + ZERO_rad;

        s = sin_[k]; c = cos_[k];

        for (xg = -1 - tick_width; xg <= 1; xg++) {
          for (yg = 1 + (D_R - D_R_tmp); yg < tick_main5 + (D_R - D_R_tmp); yg++) {
            x = c * (float)xg - s * ((float)D_R - (float)yg);
            y = s * (float)xg + c * ((float)D_R - (float)yg);

            y = (float)(Nx - 1) - ( y - (float)D_R + (float)D_height);
            x = x + (float)D_center;

            xi = (int)x; yi = (int)y;
            if ( xi >= D_left && xi <= D_right && yi >= Nx - 1 - D_height && yi <= Nx - 1 )
              dot(y, x);
          }
        }
      }

    }

  }


  if (f_maintick1 == 1) {

    // Tick 1---------------------------------------------
    if (f_dispmode == 0) {

      for (i = L_main1; i <= H_main1; i++) {

        if (i % 5 != 0 ) {
          if (fsign > 0) k = i + ZERO_rad; else k = -i + ZERO_rad;
          s = sin_[k]; c = cos_[k];

          for (xg = -tick_width; xg <= 0; xg++) {
            for (yg = 1 + (D_R - D_R_tmp); yg < tick_main1 + (D_R - D_R_tmp); yg++) {
              x = c * (float)xg - s * ((float)D_R - (float)yg);
              y = s * (float)xg + c * ((float)D_R - (float)yg);

              x = x + (float)D_center;
              y = y - (float)D_R + (float)D_height;

              xi = (int)x; yi = (int)y;
              if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height )
                dot(x, y);
            }
          }
        }
      }

    } else {

      for (i = L_main1; i <= H_main1; i++) {

        if (i % 5 != 0 ) {
          if (fsign > 0) k = i + ZERO_rad; else k = -i + ZERO_rad;
          s = sin_[k]; c = cos_[k];

          for (xg = -tick_width; xg <= 0; xg++) {
            for (yg = 1 + (D_R - D_R_tmp); yg < tick_main1 + (D_R - D_R_tmp); yg++) {
              x = c * (float)xg - s * ((float)D_R - (float)yg);
              y = s * (float)xg + c * ((float)D_R - (float)yg);

              y = (float)(Nx - 1) - ( y - (float)D_R + (float)D_height);
              x = x + (float)D_center;

              xi = (int)x; yi = (int)y;
              if ( xi >= D_left && xi <= D_right && yi >= Nx - 1 - D_height && yi <= Nx - 1 )
                dot(y, x);
            }
          }
        }
      }

    }

  }

  // Number -----------------------------------------------
  if (f_mainnum == 1) {


    for (i = L_main10; i <= H_main10; i++)
    {
      fdisp = freq + i * (10 * freq_tick_main);

      if (fdisp >= 0)
      {

        fdisp /= (10 * freq_tick_main);

        if (fsign > 0) k = i * 10 + ZERO_rad; else k = -i * 10 + ZERO_rad; //Step 10 point
        s = sin_[k]; c = cos_[k];


        dgmax = 1;
        if (fdisp < 10) {
          if (freq_tick_main == 10000) dgmax = 2;
        } else if (fdisp < 100) {
          dgmax = 2;
        } else if (fdisp < 1000) {
          dgmax = 3;
        } else {
          dgmax = 4;
        }

        //-------------------------------------------------
        for (dg = 0; dg < dgmax; dg++)
        {
          d = fdisp % 10;

          //for( xg=0; xg<13; xg++)  // Scanning 13bit
          for ( xg = 0; xg < 9; xg++)
          {
            fx = Dial_font[d + 0x10][xg];
            //for( yg=0; yg<24; yg++)  // Scanning 24bit
            for ( yg = 10; yg < 24; yg++)
            {
              fy = (long)( 1 << ( 23 - yg) ); //23

              if (f_dispmode == 0) {

                if ( ( fx & fy ) == fy )
                {
                  dgf = (float)dg;

                  if (dg == 0 && freq_tick_main == 10000)
                    dgf = (float)dg - 0.6;

                  if (dgmax == 1) xr = (float)xg - 6.0 + xoff_font; // (13-1)/2 = 6
                  if (dgmax == 2) xr = (float)xg - 6.0 + xoff_font - ( dgf - 0.5 ) * (float)fontpitch;
                  if (dgmax == 3) xr = (float)xg - 6.0 + xoff_font - ( dgf - 1.0 ) * (float)fontpitch;
                  if (dgmax == 4) xr = (float)xg - 6.0 + xoff_font - ( dgf - 1.5 ) * (float)fontpitch;

                  yr = (float)( D_R_tmp - (yg + TNCL_main) ) + yoff_font;

                  xf = c * (xr) - s * (yr);
                  yf = s * (xr) + c * (yr);

                  xf = xf + (float)D_center;
                  yf = yf - (float)D_R + (float)D_height;

                  xi = (int)xf; yi = (int)yf;
                  if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height ) dot(xf, yf);
                }

              } else {

                if ( ( fx & fy ) == fy )
                {
                  dgf = (float)dg;

                  if (dg == 0 && freq_tick_main == 10000)
                    dgf = (float)dg - 0.6;

                  if (dgmax == 1) xr = (float)( (xg + TNCL_main) - D_R_tmp ); // (13-1)/2 = 6
                  if (dgmax == 2) xr = (float)( (xg + TNCL_main) - D_R_tmp ) - (dgf - 1.0 ) * (float)fontpitch;
                  if (dgmax == 3) xr = (float)( (xg + TNCL_main) - D_R_tmp ) - (dgf - 2.0 ) * (float)fontpitch;
                  if (dgmax == 4) xr = (float)( (xg + TNCL_main) - D_R_tmp ) - (dgf - 3.0 ) * (float)fontpitch;

                  yr = (float)(12 - yg) + 0.5 * yoff_font;

                  xf = c * (xr) - s * (yr);
                  yf = s * (xr) + c * (yr);

                  xf = xf + (float)D_R - (float)D_height + (float)(Nx - 1);
                  yf = yf + (float)D_center;

                  xi = (int)xf; yi = (int)yf;
                  if ( yi >= D_left && yi <= D_right && xi >= Nx - 1 - D_height && xi <= Nx - 1 ) dot(xf, yf);
                }

              }
            }
          }


          // Decimal point---------------------------------------
          if (dg == 0)
          {
            if (freq_tick_main == 10000)
            {
              for (xg = -5; xg <= -4; xg++) {
                for (yg = 21; yg <= 22; yg++) {

                  if (
                    //(xg==-6 && yg==20)||
                    //(xg==-6 && yg==22)||
                    //(xg==-4 && yg==20)||
                    //(xg==-4 && yg==22)
                    0
                  )
                    {} else
                  {
                    if (f_dispmode == 0) {
                      if (dgmax == 1) xr = (float)xg + 0.29 * (float)fontpitch + xoff_point;
                      if (dgmax == 2) xr = (float)xg + 0.69 * (float)fontpitch + xoff_point;
                      if (dgmax == 3) xr = (float)xg + 1.29 * (float)fontpitch + xoff_point;
                      if (dgmax == 4) xr = (float)xg + 1.69 * (float)fontpitch + xoff_point;

                      yr = (float)( D_R_tmp - (yg + TNCL_main) ) + yoff_font;


                      xf = c * (xr) - s * (yr);
                      yf = s * (xr) + c * (yr);

                      xf = xf + (float)D_center;
                      yf = yf - (float)D_R + (float)D_height;

                      xi = (int)xf; yi = (int)yf;
                      if ( xi >= D_left && xi <= D_right && yi >= 0 && yi <= D_height ) {
                        dot(xf, yf);
                        if (tick_width == 1) dot(xf, yf + 0.3);
                      }


                    } else {
                      if (dgmax == 1) xr = (float)( (xg + TNCL_main) - D_R_tmp - 1) + 0.7 * (float)fontpitch + xoff_point;
                      if (dgmax == 2) xr = (float)( (xg + TNCL_main) - D_R_tmp - 1) + 1.7 * (float)fontpitch + xoff_point;
                      if (dgmax == 3) xr = (float)( (xg + TNCL_main) - D_R_tmp - 1) + 2.7 * (float)fontpitch + xoff_point;
                      if (dgmax == 4) xr = (float)( (xg + TNCL_main) - D_R_tmp - 1) + 3.7 * (float)fontpitch + xoff_point;

                      yr = (float)(12 - yg) + 0.5 * yoff_font;

                      xf = c * (xr) - s * (yr);
                      yf = s * (xr) + c * (yr);

                      xf = xf + (float)D_R - (float)D_height + (float)(Nx - 1);
                      yf = yf + (float)D_center;

                      xi = (int)xf; yi = (int)yf;
                      if ( yi >= D_left && yi <= D_right && xi >= Nx - 1 - D_height && xi <= Nx - 1 ) {
                        dot(xf, yf);
                        if (tick_width == 1) dot(xf + 0.3, yf);
                      }
                    }
                  }

                }
              }
            }

          }

          fdisp /= 10;
        }
      }
    }

  }




  //--------------------------------------------------------------------
  //   coloring
  //--------------------------------------------------------------------
  unsigned int cR, cG, cB;
  unsigned int dcR, dcG, dcB;
  int ccR, ccG, ccB;
  float kido;


  dcR = (cl_DIAL_BG >> 16) & 0xFF;
  dcG = (cl_DIAL_BG >> 8) & 0xFF;
  dcB = (cl_DIAL_BG) & 0xFF;



  // Outside Tick
  if (f_main_outside == 1) {
    cR = (cl_TICK_main >> 16) & 0xFF;
    cG = (cl_TICK_main >> 8 ) & 0xFF;
    cB = cl_TICK_main & 0xFF;
  } else {
    cR = (cl_TICK >> 16) & 0xFF;
    cG = (cl_TICK >> 8 ) & 0xFF;
    cB = cl_TICK & 0xFF;
  }
  
    
  if (f_dispmode == 0){
	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = yry[xg][1]; i <= yry[xg][0]; i++) {

			if ( R_GRAM[xg][i] != 0) {
			  kido = (float)R_GRAM[xg][i] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5 );
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5 );
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0 );
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[xg][i] = (unsigned char)ccR;
			  G_GRAM[xg][i] = (unsigned char)ccG;
			  B_GRAM[xg][i] = (unsigned char)ccB;
			}
			
		}
	  }
  
  }else{

	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = yry[xg][1]; i <= yry[xg][0]; i++) {


			if ( R_GRAM[Nx - 1 - i][xg] != 0) {
			  kido = (float)R_GRAM[Nx - 1 - i][xg] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5);
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5);
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0);
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[Nx - 1 - i][xg] = (unsigned char)ccR;
			  G_GRAM[Nx - 1 - i][xg] = (unsigned char)ccG;
			  B_GRAM[Nx - 1 - i][xg] = (unsigned char)ccB;
			}

		}
	  }
	  
  }
  
  
  

  //Outside Number---------------------
  if (f_main_outside == 1) {
    cR = (cl_NUM_main >> 16) & 0xFF;
    cG = (cl_NUM_main >> 8 ) & 0xFF;
    cB = cl_NUM_main & 0xFF;
  } else {
    cR = (cl_NUM >> 16) & 0xFF;
    cG = (cl_NUM >> 8 ) & 0xFF;
    cB = cl_NUM & 0xFF;
  }
 
  if (f_dispmode == 0){  
	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = yry[xg][2]; i < yry[xg][1]; i++) {

			if (R_GRAM[xg][i] != 0) {
			  kido = (float)R_GRAM[xg][i] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5);
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5);
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0);
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[xg][i] = (unsigned char)ccR;
			  G_GRAM[xg][i] = (unsigned char)ccG;
			  B_GRAM[xg][i] = (unsigned char)ccB;
			}

		}
	  }
	  
  }else{
	  	  
	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = yry[xg][2]; i < yry[xg][1]; i++) {

			if (R_GRAM[Nx - 1 - i][xg] != 0) {
			  kido = (float)R_GRAM[Nx - 1 - i][xg] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5);
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5);
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0);
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[Nx - 1 - i][xg] = (unsigned char)ccR;
			  G_GRAM[Nx - 1 - i][xg] = (unsigned char)ccG;
			  B_GRAM[Nx - 1 - i][xg] = (unsigned char)ccB;
			}
		}
	  }
  }
  
  
  

  //Inside Tick---------------
  if (f_main_outside == 1) {
    cR = (cl_TICK >> 16) & 0xFF;
    cG = (cl_TICK >> 8 ) & 0xFF;
    cB = cl_TICK & 0xFF;
  } else {
    cR = (cl_TICK_main >> 16) & 0xFF;
    cG = (cl_TICK_main >> 8 ) & 0xFF;
    cB = cl_TICK_main & 0xFF;
  }

  
  if (f_dispmode == 0) {
	  		  
	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = yry[xg][3]; i < yry[xg][2]; i++) {
	 
			if (R_GRAM[xg][i] != 0) {
			  kido = (float)R_GRAM[xg][i] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5);
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5);
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0);
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[xg][i] = (unsigned char)ccR;
			  G_GRAM[xg][i] = (unsigned char)ccG;
			  B_GRAM[xg][i] = (unsigned char)ccB;
			}

		}
	  }
	  
  }else{

	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = yry[xg][3]; i < yry[xg][2]; i++) {

			if (R_GRAM[Nx - 1 - i][xg] != 0) {
			  kido = (float)R_GRAM[Nx - 1 - i][xg] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5);
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5);
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0);
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[Nx - 1 - i][xg] = (unsigned char)ccR;
			  G_GRAM[Nx - 1 - i][xg] = (unsigned char)ccG;
			  B_GRAM[Nx - 1 - i][xg] = (unsigned char)ccB;
			}

		}
	  }
  }
	  
 
  

  //Inside Number--------------------------
  if (f_main_outside == 1) {
    cR = (cl_NUM >> 16) & 0xFF;
    cG = (cl_NUM >> 8 ) & 0xFF;
    cB = cl_NUM & 0xFF;
  } else {
    cR = (cl_NUM_main >> 16) & 0xFF;
    cG = (cl_NUM_main >> 8 ) & 0xFF;
    cB = cl_NUM_main & 0xFF;
  }
  
  
  if (f_dispmode == 0) {
		  
	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = 0; i < yry[xg][3]; i++) {

			if (R_GRAM[xg][i] != 0) {
			  kido = (float)R_GRAM[xg][i] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5);
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5);
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0);
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[xg][i] = (unsigned char)ccR;
			  G_GRAM[xg][i] = (unsigned char)ccG;
			  B_GRAM[xg][i] = (unsigned char)ccB;
			}

		}
	  }
	  
  }else{
  
	  for (xg = D_left; xg <= D_right; xg++) {
		for (i = 0; i < yry[xg][3]; i++) {


			if (R_GRAM[Nx - 1 - i][xg] != 0) {
			  kido = (float)R_GRAM[Nx - 1 - i][xg] / (float)255.0;
			  ccR = (int)(kido * (float)cR + (1.0 - kido) * (float)dcR + 0.5);
			  ccG = (int)(kido * (float)cG + (1.0 - kido) * (float)dcG + 0.5);
			  ccB = (int)(kido * (float)cB + (1.0 - kido) * (float)dcB + 1.0);
			  if (ccR > 0xFF) ccR = 0xFF;
			  if (ccG > 0xFF) ccG = 0xFF;
			  if (ccB > 0xFF) ccB = 0xFF;
			  R_GRAM[Nx - 1 - i][xg] = (unsigned char)ccR;
			  G_GRAM[Nx - 1 - i][xg] = (unsigned char)ccG;
			  B_GRAM[Nx - 1 - i][xg] = (unsigned char)ccB;
			}

		}
	  }
  }
	  


  //----DIAL base-----------------------------------------------------------------------

  if (f_dispmode == 0) {

    for (xg = D_left; xg <= D_right; xg++) {
      yg = yry[xg][0];
      for (i = 0; i <= yg; i++) {

        if ((R_GRAM[xg][i] == 0) && (G_GRAM[xg][i] == 0) && (B_GRAM[xg][i] == 0))
        {
          R_GRAM[xg][i] = dcR;
          G_GRAM[xg][i] = dcG;
          B_GRAM[xg][i] = dcB;
        }
      }
    }

  } else {

    for (xg = D_left; xg <= D_right; xg++) {
      yg = yry[xg][0];
      for (i = 0; i <= yg; i++) {

        if ((R_GRAM[Nx - 1 - i][xg] == 0) && (G_GRAM[Nx - 1 - i][xg] == 0) && (B_GRAM[Nx - 1 - i][xg] == 0))
        {
          R_GRAM[Nx - 1 - i][xg] = dcR;
          G_GRAM[Nx - 1 - i][xg] = dcG;
          B_GRAM[Nx - 1 - i][xg] = dcB;
        }
      }
    }

  }



  //-- Dial pointer ---------------------------------------------------------------
  int ypt = D_height + DP_pos - DP_len;
  if (ypt < 0) ypt = 0;

  if (f_dispmode == 0) {

    for (xg = D_center - (DP_width - 1); xg <= D_center + (DP_width - 1); xg++) {
      for (yg = ypt; yg < (D_height + DP_pos); yg++) {
        R_GRAM[xg][yg] = (cl_POINTER >> 16) & 0xFF;
        G_GRAM[xg][yg] = (cl_POINTER >> 8) & 0xFF;
        B_GRAM[xg][yg] = (cl_POINTER) & 0xFF;
      }
    }

  } else {

    for (xg = D_center - (DP_width - 1); xg <= D_center + (DP_width - 1); xg++) {
      for (yg = ypt; yg < (D_height + DP_pos); yg++) {
        R_GRAM[Nx - 1 - yg][xg] = (cl_POINTER >> 16) & 0xFF;
        G_GRAM[Nx - 1 - yg][xg] = (cl_POINTER >> 8) & 0xFF;
        B_GRAM[Nx - 1 - yg][xg] = (cl_POINTER) & 0xFF;
      }
    }

  }



}
/*---------------------------------------------------------------------------
 *  End of  void Dial(long freq) 
---------------------------------------------------------------------------- */


void dot(float x, float y)
{
  int xd, yd, xu, yu;
  float Rxu, Rxd, Ryu, Ryd;
  unsigned int dat;

  if(x>0 && y>0)
  { 
	  if (f_dispmode == 0) x = x + 0.5 * (float)(tick_width);
	  else y = y + 0.5 * (float)(tick_width);

	  xd = (int)x; yd = (int)y;

	  if ( xd >= 0 && xd < Nx - 1 && yd >= 0 && yd < Ny - 1) {
		xu = xd + 1; yu = yd + 1;

		Rxd = ( (float)xu - x );
		Rxu = ( x - (float)xd );
		Ryd = ( (float)yu - y );
		Ryu = ( y - (float)yd );


		dat = (int)R_GRAM[xd][yd] + (int)(Rxd * Ryd * 256.0);
		if (dat > 0xFF) dat = 0xFF;
		R_GRAM[xd][yd] = (unsigned char)dat;

		dat = (unsigned int)R_GRAM[xu][yd] + (unsigned int)(Rxu * Ryd * 256.0);
		if (dat > 0xFF) dat = 0xFF;
		R_GRAM[xu][yd] = (unsigned char)dat;

		dat = (unsigned int)R_GRAM[xd][yu] + (unsigned int)(Rxd * Ryu * 256.0);
		if (dat > 0xFF) dat = 0xFF;
		R_GRAM[xd][yu] = (unsigned char)dat;

		dat = (unsigned int)R_GRAM[xu][yu] + (unsigned int)(Rxu * Ryu * 256.0);
		if (dat > 0xFF) dat = 0xFF;
		R_GRAM[xu][yu] = (unsigned char)dat;
	  }
  }
}

/*
void dot(float x, float y)
{
  R_GRAM[(int)x][(int)y]=255;
}
*/
