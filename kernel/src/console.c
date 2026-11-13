#include "console.h"
typedef unsigned char u8;
static volatile u8 *fb;
static u32 width, height, stride, format;
static u32 cx, cy, columns, rows;
static void pixel(u32 x,u32 y,u8 r,u8 g,u8 b){
 if(x>=width||y>=height)return;
 volatile u8*p=fb+((u64)y*stride+x)*4;
 if(format==NYROX_PIXEL_FORMAT_RGB){p[0]=r;p[1]=g;p[2]=b;}
 else{p[0]=b;p[1]=g;p[2]=r;}
 p[3]=0;
}
static void rect(u32 x,u32 y,u32 w,u32 h,u8 r,u8 g,u8 b){
 for(u32 yy=0;yy<h;yy++)
  for(u32 xx=0;xx<w;xx++)
   pixel(x+xx,y+yy,r,g,b);
}
static const u8 font[][7]={
 {14,17,17,31,17,17,17},
 {30,17,17,30,17,17,30},
 {14,17,16,16,16,17,14},
 {28,18,17,17,17,18,28},
 {31,16,16,30,16,16,31},
 {31,16,16,30,16,16,16}
};
static const u8 font2[][7]={
 {14,17,16,23,17,17,14},
 {17,17,17,31,17,17,17},
 {14,4,4,4,4,4,14},
 {7,2,2,2,18,18,12},
 {16,18,20,24,20,18,17},
 {16,16,16,16,16,16,31}
};
static const u8 font3[][7]={
 {17,27,21,21,17,17,17},
 {17,25,21,19,17,17,17},
 {14,17,17,17,17,17,14},
 {30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},
 {30,17,17,30,20,18,17}
};
static const u8 font4[][7]={
 {15,16,16,14,1,1,30},
 {31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},
 {17,17,17,17,17,10,4},
 {17,17,17,21,21,21,10},
 {17,17,10,4,10,17,17}
};
static const u8 font5[][7]={
 {17,17,10,4,4,4,4},
 {31,1,2,4,8,16,31},
 {0,0,0,0,0,0,0},
 {0,4,4,31,4,4,0},
 {0,0,0,0,0,0,4}
};
static u8 glyph(char c,u32 y){
 if(c>='a'&&c<='z')c-=32;
 if(c>='A'&&c<='F')return font[c-'A'][y];
 if(c>='G'&&c<='L')return font2[c-'G'][y];
 if(c>='M'&&c<='R')return font3[c-'M'][y];
 if(c>='S'&&c<='X')return font4[c-'S'][y];
 if(c=='Y')return font5[0][y];
 if(c=='Z')return font5[1][y];
 if(c==' ')return font5[2][y];
 if(c=='+')return font5[3][y];
 if(c=='.')return font5[4][y];
 return 0;
}
static void draw_char(char c,u32 x,u32 y){
 for(u32 j=0;j<7;j++){
  u8 bits=glyph(c,j);
  for(u32 i=0;i<5;i++){
   if(bits&(16>>i))pixel(x+i,y+j,255,255,255);
   else pixel(x+i,y+j,0,0,0);
  }
 }
}
int console_init(const NyroxBootInfo *info){
 if(!info||!info->framebuffer_base||!info->framebuffer_width||!info->framebuffer_height||info->framebuffer_pixels_per_scanline<info->framebuffer_width||info->framebuffer_pixel_format>NYROX_PIXEL_FORMAT_BGR)return -1;
 fb=(volatile u8*)info->framebuffer_base;
 width=info->framebuffer_width;
 height=info->framebuffer_height;
 stride=info->framebuffer_pixels_per_scanline;
 format=info->framebuffer_pixel_format;
 columns=width/8; rows=height/10;
 cx=0; cy=0;
 console_clear();
 return 0;
}
void console_clear(void){
 rect(0,0,width,height,0,0,0);
 cx=0; cy=0;
}
void console_putc(char c){
 if(c=='\r'){cx=0;return;}
 if(c=='\n'){cx=0;cy++;return;}
 if(cx>=columns){cx=0;cy++;}
 if(cy>=rows)cy=0;
 draw_char(c,cx*8,cy*10);
 cx++;
}
void console_write(const char *text){
 if(!text)return;
 while(*text)console_putc(*text++);
}
