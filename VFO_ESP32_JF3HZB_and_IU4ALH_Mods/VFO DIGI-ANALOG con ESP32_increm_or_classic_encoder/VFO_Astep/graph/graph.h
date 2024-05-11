/* 
 * File:   graph.h
 * Author: JF3HZB / T.UEBO
 *
 * Created on 2017/12/08, 00:21
 */


#ifndef _graph_
#define _graph_      

void GRAM_clr(void);
void line(int,int,int,int, unsigned long);
void boxfill(int, int, int, int, unsigned long);
void box(int, int, int, int, unsigned long);

void disp_str8(char*, int, int, unsigned long);
void disp_str12(char*, int, int, unsigned long);
void disp_str16(char*, int, int, unsigned long);
void disp_str20(char*, int, int, unsigned long);

int disp_chr8(char, int, int, unsigned long);
unsigned char bitrev8(unsigned char );
int disp_chr12(char, int, int, unsigned long);
int disp_chr16(char, int, int, unsigned long);
int disp_chr20(char, int, int, unsigned long);


#endif
