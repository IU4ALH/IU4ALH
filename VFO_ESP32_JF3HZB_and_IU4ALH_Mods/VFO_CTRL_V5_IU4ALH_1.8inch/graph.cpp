/* 
 * File:   graph.cpp
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2017/12/08, 00:21
 */

 
#include <arduino.h>
#include "display.h"
#include "graph.h"
#include "font.h"

extern uint8_t  R_GRAM[Nx][Ny];
extern uint8_t  B_GRAM[Nx][Ny];
extern uint8_t  G_GRAM[Nx][Ny];


// Claer GRAM ------------------------------------------------------------------
void GRAM_clr(void)
{
    boxfill(0,0,Nx-1,Ny-1,0);
}

//--------------------------------------------------------------------------
void boxfill(int x_min, int y_min, int x_max, int y_max, unsigned long color)
{
    int k, j;
	union {
		unsigned long  lw[1];
		unsigned char  byte[4];		
	} cl_data;
	cl_data.lw[0]=color;
   
    if(x_min<0) x_min=0;
    if(y_min<0) y_min=0;
    if(x_max>=Nx) x_max=Nx-1;
    if(y_max>=Ny) y_max=Ny-1;   
    
    for(k=x_min;k<=x_max; k++){
        for(j=y_min; j<=y_max; j++){
			R_GRAM[k][j]=cl_data.byte[2];
			G_GRAM[k][j]=cl_data.byte[1];
			B_GRAM[k][j]=cl_data.byte[0];
		}
    }
}

//--------------------------------------------------------------------------
void line(int xs, int ys,int xe, int ye, unsigned long color)
{
	union {
		unsigned long  lw[1];
		unsigned char  byte[4];		
	} cl_data;
	cl_data.lw[0]=color;

    
	int dx = (xe > xs ? xe - xs : xs - xe);
	int xstep =  xe > xs ? 1 : -1;
	int dy = (ye > ys ? ye - ys : ys - ye);
	int ystep =  ye > ys ? 1 : -1;
	int j;
	
	if(dx==0 && dy==0){
		R_GRAM[xs][ys]=cl_data.byte[2];
		G_GRAM[xs][ys]=cl_data.byte[1];		
		B_GRAM[xs][ys]=cl_data.byte[0];		
		
	} else if(dx==0){
		if(ystep>0) for(j=ys; j<=ye; j++){
			R_GRAM[xs][j]=cl_data.byte[2];
			G_GRAM[xs][j]=cl_data.byte[1];
			B_GRAM[xs][j]=cl_data.byte[0];
		}
		if(ystep<0) for(j=ye; j<=ys; j++){
			R_GRAM[xs][j]=cl_data.byte[2];
			G_GRAM[xs][j]=cl_data.byte[1];
			B_GRAM[xs][j]=cl_data.byte[0];			
		}
	} else if(dy==0){
		if(xstep>0) for(j=xs; j<=xe; j++){
			R_GRAM[j][ys]=cl_data.byte[2];
			G_GRAM[j][ys]=cl_data.byte[1];
			B_GRAM[j][ys]=cl_data.byte[0];						
		}
		if(xstep<0) for(j=xe; j<=xs; j++){
			R_GRAM[j][ys]=cl_data.byte[2];
			G_GRAM[j][ys]=cl_data.byte[1];			
			B_GRAM[j][ys]=cl_data.byte[0];			
		}
	} else {
		int xx=xs, yy=ys;
		if(dx>dy){
			int t = - (dx >> 1);
			while (1)
			{
				R_GRAM[xx][yy]=cl_data.byte[2];
				G_GRAM[xx][yy]=cl_data.byte[1];
				B_GRAM[xx][yy]=cl_data.byte[0];
				if (xx == xe) break;
				xx += xstep;
				t  += dy;
				if (t >= 0){ yy += ystep; t-= dx;}
			} 
		} else {
			int t = - (dy >> 1);
			while (1)
			{
				R_GRAM[xx][yy]=cl_data.byte[2];
				G_GRAM[xx][yy]=cl_data.byte[1];
				B_GRAM[xx][yy]=cl_data.byte[0];
				if (yy == ye) break;
				yy += ystep;
				t  += dx;
				if (t >= 0){ xx += xstep; t-= dy;}
			} 
		}
	}
}


//--------------------------------------------------------------------------
void box(int x_min, int y_min, int x_max, int y_max, unsigned long color)
{
    int k,j;
    
    if(x_min<0) x_min=0;
    if(y_min<0) y_min=0;
    if(x_max>=Nx) x_max=Nx-1;
    if(y_max>=Ny) y_max=Ny-1;   
    
    line(x_min, y_min, x_max, y_min, color);
    line(x_min, y_max, x_max, y_max, color);
    line(x_min, y_min, x_min, y_max, color);
    line(x_max, y_min, x_max, y_max, color);

}


//----------------------------------------------------------------------------
//    Draw String
//----------------------------------------------------------------------------

//------font 5x7----------------------------------------
void disp_str8( char *s, int x, int y, unsigned long color)
{
	unsigned char c;
	int k,N;
	
	N=x;
	for(k=0; k<128; k++)
	{
		c=*(s+k);
		if(c==0) break;
		N=disp_chr8(c, N, y, color);
		N+=1;
	}
}


//------font12--------------------------------------------
void disp_str12( char *s, int x, int y, unsigned long color)
{
	unsigned char c;
	int k,N;
	
	N=x;
	for(k=0; k<128; k++)
	{
		c=*(s+k);
		if(c==0) break;
		N=disp_chr12(c,N,y,color);
		N+=1;
	}
}



//------font16--------------------------------------------
void disp_str16( char *s, int x, int y, unsigned long color)
{
	unsigned char c;
	int k,N;
	
	N=x;
	for(k=0; k<128; k++)
	{
		c=*(s+k);
		if(c==0) break;
		N=disp_chr16(c,N,y,color);
		N+=1;
	}
}

//------font20--------------------------------------------
void disp_str20( char *s, int x, int y, unsigned long color)
{
	unsigned char c;
	int k,N;
	
	N=x;
	for(k=0; k<128; k++)
	{
		c=*(s+k);
		if(c==0) break;
		N=disp_chr20(c,N,y,color);
		N+=1;
	}
}



//-----------------------------------------------------------------------------
//              Draw a character
//-----------------------------------------------------------------------------

//------font 5x7 -------------------------------------
int disp_chr8(char c, int x, int y, unsigned long color)
{
	union {
		unsigned long  lw[1];
		unsigned char  byte[4];		
	} cl_data;
	cl_data.lw[0]=color;
	
	int k, j;
    unsigned char f8;
	
	if(c=='\\') c=' ';
	
	for(k=0; k<5; k++){
        f8=(unsigned char)bitrev8(font[c-0x20][k]);
        if(x>=0){
            for(j=0; j<8; j++){
                if( (f8&0x01)==0x01 ){
					R_GRAM[x][y+j]=cl_data.byte[2];
					G_GRAM[x][y+j]=cl_data.byte[1];
					B_GRAM[x][y+j]=cl_data.byte[0];
				}
                f8>>=1;
            }
        }
		x++;
	}
	return(x);
}
//--------- bit reverse(8bit)-----------------
unsigned char bitrev8(unsigned char x)
	{	
		x = ((x & 0xaa) >> 1) | ((x & 0x55) << 1);
		x = ((x & 0xcc) >> 2) | ((x & 0x33) << 2);
		x = (x >> 4) | (x << 4);
		return(x);
	}


//------font12-----------------------------------------
int disp_chr12(char c, int x, int y, unsigned long color)
{
	union {
		unsigned long  lw[1];
		unsigned char  byte[4];		
	} cl_data;
	cl_data.lw[0]=color;
	
	int k, j, yj;
	unsigned short int f12;
	
	if(c=='\\') c=' ';
		
	for(k=0; k<24; k++){
		f12=(unsigned short int)font12[c-0x20][k];
		if(f12==0x0fff) break;
        if(x>=0){
            for(j=0; j<12; j++){
				yj=y+j;
                if( (f12&0x0001)==0x0001){
					if( x>=0 && x<Nx && yj>=0 && yj<Ny ){
					R_GRAM[x][yj]=cl_data.byte[2];
					G_GRAM[x][yj]=cl_data.byte[1];
					B_GRAM[x][yj]=cl_data.byte[0];
					}
				}
                f12>>=1;
            }
        }
		x++;
	}
	return(x+1);
}




//------font16-----------------------------------------
int disp_chr16(char c, int x, int y, unsigned long color)
{
	union {
		unsigned long  lw[1];
		unsigned char  byte[4];		
	} cl_data;
	cl_data.lw[0]=color;
	
	int k, j, yj;
	unsigned short int f16;
	
	if(c=='\\') c=' ';
		
	for(k=0; k<24; k++){
		f16=(unsigned short int)font16[c-0x20][k];
		if(f16==0xffff) break;
        if(x>=0){
            for(j=0; j<16; j++){
				yj=y+j;
                if( (f16&0x0001)==0x0001){
					if( x>=0 && x<Nx && yj>=0 && yj<Ny ){
					R_GRAM[x][yj]=cl_data.byte[2];
					G_GRAM[x][yj]=cl_data.byte[1];
					B_GRAM[x][yj]=cl_data.byte[0];
					}
				}
                f16>>=1;
            }
        }
		x++;
	}
	return(x+1);
}

//------font20-----------------------------------------
int disp_chr20(char c, int x, int y, unsigned long color)
{
	union {
		unsigned long  lw[1];
		unsigned char  byte[4];		
	} cl_data;
	cl_data.lw[0]=color;
	
	int k, j, yj;
	unsigned long f20;
	
	if(c=='\\') c=' ';
	
	for(k=0; k<24; k++){
		f20=(unsigned long)font20[c-0x20][k];
		if(f20==0xfffff) break;
        if(x>=0){
            for(j=0; j<20; j++){
				yj=y+j;
                if( (f20&0x00001)==0x00001){
					if( x>=0 && x<Nx && yj>=0 && yj<Ny ){
					R_GRAM[x][yj]=cl_data.byte[2];
					G_GRAM[x][yj]=cl_data.byte[1];
					B_GRAM[x][yj]=cl_data.byte[0];
					}
				}
                f20>>=1;
            }
        }
		x++;
	}
	return(x+1);
}
