//------------------------------------------------------
// module  : Tp4-IFT2425-2.c
// author  :
// date    :
// version : 1.0
// language: C++
// note    :
//------------------------------------------------------
//

//------------------------------------------------
// FICHIERS INCLUS -------------------------------
//------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <new>

#include <string>
#include <iostream>

/************************************************************************/
/* WINDOWS						          	*/
/************************************************************************/
#include <X11/Xutil.h>

Display   *display;
int	  screen_num;
int 	  depth;
Window	  root;
Visual*	  visual;
GC	  gc;

//------------------------------------------------
// DEFINITIONS -----------------------------------
//------------------------------------------------
#define CARRE(X) ((X)*(X))

#define OUTPUT_FILE "Tp4-Img-II"
#define VIEW_PGM    "xv"

#define DEBUG 1
#define TROIS 3

//-Cst-Modele
#define X_1 0.0
#define Y_1 1.0
#define X_2 -1.0/sqrt(2.0)
#define Y_2 -1.0/2.0
#define X_3 +1.0/2*sqrt(2.0)
#define Y_3 -1.0/2.0

#define C 0.25
#define R 0.1
#define D 0.3


//-Cst-Runge-Kutta
#define H            0.1
#define T_0          0.0
#define T_F          20.0
#define NB_INTERV (T_F-T_0)/H

 //-Cst-Image
#define WIDTH  256
#define HEIGHT 256
#define MAX_X  4.0
#define MAX_Y  4.0
#define EVOL_GRAPH 3000

#define WHITE     255
#define GREYWHITE 230
#define GREY      200
#define GREYDARK  120
#define BLACK       0

//------------------------------------------------
// GLOBAL CST ------------------------------------
//------------------------------------------------
float Xmin=-(MAX_X/2.0);
float Xmax=+(MAX_X/2.0);
float Ymin=-(MAX_Y/2.0);
float Ymax=+(MAX_Y/2.0);

float xx_1=((WIDTH/MAX_X)*X_1)+(WIDTH/2);
float yy_1=(-(HEIGHT/MAX_Y)*Y_1)+(HEIGHT/2);
float xx_2=((WIDTH/MAX_X)*X_2)+(WIDTH/2);
float yy_2=(-(HEIGHT/MAX_Y)*Y_2)+(HEIGHT/2);
float xx_3=((WIDTH/MAX_X)*X_3)+(WIDTH/2);
float yy_3=(-(HEIGHT/MAX_Y)*Y_3)+(HEIGHT/2);

float X_1_INI;
float X_2_INI;
float X_3_INI;
float X_4_INI;

/************************************************************************/
/* OPEN_DISPLAY()							*/
/************************************************************************/
int open_display()
{
  if ((display=XOpenDisplay(NULL))==NULL)
   { printf("Connection impossible\n");
     return(-1); }

  else
   { screen_num=DefaultScreen(display);
     visual=DefaultVisual(display,screen_num);
     depth=DefaultDepth(display,screen_num);
     root=RootWindow(display,screen_num);
     return 0; }
}

/************************************************************************/
/* FABRIQUE_WINDOW()							*/
/* Cette fonction crée une fenetre X et l'affiche à l'écran.	        */
/************************************************************************/
Window fabrique_window(char *nom_fen,int x,int y,int width,int height,int zoom)
{
  Window                 win;
  XSizeHints      size_hints;
  XWMHints          wm_hints;
  XClassHint     class_hints;
  XTextProperty  windowName, iconName;

  char *name=nom_fen;

  if(zoom<0) { width/=-zoom; height/=-zoom; }
  if(zoom>0) { width*=zoom;  height*=zoom;  }

  win=XCreateSimpleWindow(display,root,x,y,width,height,1,0,255);

  size_hints.flags=PPosition|PSize|PMinSize;
  size_hints.min_width=width;
  size_hints.min_height=height;

  XStringListToTextProperty(&name,1,&windowName);
  XStringListToTextProperty(&name,1,&iconName);
  wm_hints.initial_state=NormalState;
  wm_hints.input=True;
  wm_hints.flags=StateHint|InputHint;
  class_hints.res_name=nom_fen;
  class_hints.res_class=nom_fen;

  XSetWMProperties(display,win,&windowName,&iconName,
                   NULL,0,&size_hints,&wm_hints,&class_hints);

  gc=XCreateGC(display,win,0,NULL);

  XSelectInput(display,win,ExposureMask|KeyPressMask|ButtonPressMask|
               ButtonReleaseMask|ButtonMotionMask|PointerMotionHintMask|
               StructureNotifyMask);

  XMapWindow(display,win);
  return(win);
}

/****************************************************************************/
/* CREE_XIMAGE()							    */
/* Crée une XImage à partir d'un tableau de float                          */
/* L'image peut subir un zoom.						    */
/****************************************************************************/
XImage* cree_Ximage(float** mat,int z,int length,int width)
{
  int lgth,wdth,lig,col,zoom_col,zoom_lig;
  float somme;
  unsigned char	 pix;
  unsigned char* dat;
  XImage* imageX;

  /*Zoom positiv*/
  /*------------*/
  if (z>0)
  {
   lgth=length*z;
   wdth=width*z;

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<lgth;lig=lig+z) for(col=0;col<wdth;col=col+z)
   {
    pix=(unsigned char)mat[lig/z][col/z];
    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
      {
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+0)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+1)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+2)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+3)]=pix;
       }
    }
  } /*--------------------------------------------------------*/

  /*Zoom negatifv*/
  /*------------*/
  else
  {
   z=-z;
   lgth=(length/z);
   wdth=(width/z);

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<(lgth*z);lig=lig+z) for(col=0;col<(wdth*z);col=col+z)
   {
    somme=0.0;
    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
     somme+=mat[lig+zoom_lig][col+zoom_col];

     somme/=(z*z);
     dat[((lig/z)*wdth*4)+((4*(col/z))+0)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+1)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+2)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+3)]=(unsigned char)somme;
   }
  } /*--------------------------------------------------------*/

  imageX=XCreateImage(display,visual,depth,ZPixmap,0,(char*)dat,wdth,lgth,16,wdth*4);
  return (imageX);
}

/****************************************************************************/
/* CREE_XIMAGECOUL()							    */
/* Crée une XImage à partir d'un tableau 3 d de float                       */
/* L'image peut subir un zoom.						    */
/****************************************************************************/
XImage* cree_XimageCoul(float*** matRVB,int z,int length,int width)
{
  int i;
  int lgth,wdth,lig,col,zoom_col,zoom_lig;
  float somme;
  float sum[3];
  unsigned char	 pixR,pixV,pixB,pixN;
  unsigned char* dat;
  XImage* imageX;

  /*Zoom positif*/
  /*------------*/
  if (z>0)
  {
   lgth=length*z;
   wdth=width*z;

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<lgth;lig=lig+z) for(col=0;col<wdth;col=col+z)
   {
    pixR=(unsigned char)matRVB[0][lig/z][col/z];
    pixV=(unsigned char)matRVB[1][lig/z][col/z];
    pixB=(unsigned char)matRVB[2][lig/z][col/z];
    somme=(1.0/3.0)*(pixR+pixV+pixB);
    pixN=(unsigned char)somme;

    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
      {
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+0)]=pixB;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+1)]=pixV;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+2)]=pixR;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+3)]=0;
       }
    }
  } /*--------------------------------------------------------*/

  /*Zoom negatif*/
  /*------------*/
  else
  {
   z=-z;
   lgth=(length/z);
   wdth=(width/z);

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<(lgth*z);lig=lig+z) for(col=0;col<(wdth*z);col=col+z)
   {
    sum[0]=sum[1]=sum[2]=0.0;

    for(i=0;i<3;i++)
    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
     sum[i]+=matRVB[i][lig+zoom_lig][col+zoom_col];

    for(i=0;i<3;i++)  sum[i]/=(z*z);

     dat[((lig/z)*wdth*4)+((4*(col/z))+0)]=(unsigned char)sum[1];
     dat[((lig/z)*wdth*4)+((4*(col/z))+1)]=(unsigned char)sum[1];
     dat[((lig/z)*wdth*4)+((4*(col/z))+2)]=(unsigned char)sum[1];
     dat[((lig/z)*wdth*4)+((4*(col/z))+3)]=(unsigned char)sum[1];
   }
  } /*--------------------------------------------------------*/

  imageX=XCreateImage(display,visual,depth,ZPixmap,0,(char*)dat,wdth,lgth,16,wdth*4);
  return (imageX);
}

//------------------------------------------------
// FUNCTIONS -------------------------------------
//------------------------------------------------
//-------------------------//
//-- Matrice de Double ----//
//-------------------------//
//---------------------------------------------------------
// Alloue de la memoire pour une matrice 1d de float
//----------------------------------------------------------
float* dmatrix_allocate_1d(int hsize)
 {
  float* matrix;
  matrix=new float[hsize]; return matrix; }

//----------------------------------------------------------
// Alloue de la memoire pour une matrice 2d de float
//----------------------------------------------------------
float** dmatrix_allocate_2d(int vsize,int hsize)
 {
  float** matrix;
  float *imptr;

  matrix=new float*[vsize];
  imptr=new float[(hsize)*(vsize)];
  for(int i=0;i<vsize;i++,imptr+=hsize) matrix[i]=imptr;
  return matrix;
 }

//----------------------------------------------------------
// alloue de la memoire pour une matrice 3d de float
//----------------------------------------------------------
float*** dmatrix_allocate_3d(int dsize,int vsize,int hsize)
 {
  float*** matrix;

  matrix=new float**[dsize];

  for(int i=0;i<dsize;i++)
    matrix[i]=dmatrix_allocate_2d(vsize,hsize);
  return matrix;
 }

//----------------------------------------------------------
// Libere la memoire de la matrice 1d de float
//----------------------------------------------------------
void free_dmatrix_1d(float* pmat)
{ delete[] pmat; }

//----------------------------------------------------------
// Libere la memoire de la matrice 2d de float
//----------------------------------------------------------
void free_dmatrix_2d(float** pmat)
{ delete[] (pmat[0]);
  delete[] pmat;}

//----------------------------------------------------------
// libere la memoire de la matrice 3d de float
//----------------------------------------------------------
void free_dmatrix_3d(float*** pmat,int dsize)
{
 for(int i=0;i<dsize;i++)
  {
   delete[] (pmat[i][0]);
   delete[] (pmat[i]);
   }
 delete[] (pmat);
}

//----------------------------------------------------------
// Sauvegarde de l'image de nom <name> au format ppm
//----------------------------------------------------------
void SaveImagePpm(char* Name,float*** matrvb,int wdth,int lgth)
 {
  int i,j;
  char buff[200];
  FILE* fuser;

  //extension
  strcpy(buff,Name);
  strcat(buff,".ppm");

  //ouverture fichier
  fuser=fopen(buff,"w");
    if (fuser==NULL)
        { printf(" probleme dans la sauvegarde de %s",buff);
          exit(-1); }

  //affichage
  printf("\n  Sauvegarde de %s au format %s",buff,".ppm");
  fflush(stdout);

  //sauvegarde de l'entete
  fprintf(fuser,"P6");
  fprintf(fuser,"\n# IMG Module");
  fprintf(fuser,"\n%d %d",lgth,wdth);
  fprintf(fuser,"\n255\n");

  //enregistrement
  for(i=0;i<wdth;i++) for(j=0;j<lgth;j++)
    {
     fprintf(fuser,"%c",(char)matrvb[0][i][j]);
     fprintf(fuser,"%c",(char)matrvb[1][i][j]);
     fprintf(fuser,"%c",(char)matrvb[2][i][j]);
    }

  //fermeture fichier
   fclose(fuser);
 }

//------------------------------------------------------------------------
// plot_point
//
// Affiche entre x dans [-MAX_X/2  MAX_X/2]
//               y dans [-MAX_Y/2  MAX_Y/2]
//------------------------------------------------------------------------
void plot_point(float** MatPts,float** MatPict,int NbPts)
{
 int x_co,y_co;
 int i,j,k;

 //Init
 for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++)  MatPict[i][j]=GREYWHITE;

 for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++)
   { if ((fabs(i-yy_1)+fabs(j-xx_1))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_2)+fabs(j-xx_2))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_3)+fabs(j-xx_3))<10) MatPict[i][j]=GREYDARK; }

 //Loop
 for(k=0;k<NbPts;k++)
    { x_co=(int)((WIDTH/MAX_X)*MatPts[k][0]);
      y_co=-(int)((HEIGHT/MAX_Y)*MatPts[k][1]);
      y_co+=(HEIGHT/2);
      x_co+=(WIDTH/2);
      if (DEBUG) printf("[%d::%d]",x_co,y_co);
      if ((x_co<WIDTH)&&(y_co<HEIGHT)&&(x_co>0)&&(y_co>0))
	 MatPict[y_co][x_co]=BLACK;
    }
}

//------------------------------------------------------------------------
// Fill_Pict
//------------------------------------------------------------------------
void Fill_Pict(float** MatPts,float** MatPict,int PtsNumber,int NbPts)
{
 int i,j;
 int x_co,y_co;
 int k,k_Init,k_End;

 //Init
 for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++)
   { if (MatPict[i][j]!=GREYWHITE) MatPict[i][j]=GREY;
     if ((fabs(i-yy_1)+fabs(j-xx_1))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_2)+fabs(j-xx_2))<10) MatPict[i][j]=GREYDARK;
     if ((fabs(i-yy_3)+fabs(j-xx_3))<10) MatPict[i][j]=GREYDARK; }

 //Loop
 k_Init=PtsNumber;
 k_End=(k_Init+EVOL_GRAPH)%NbPts;
 for(k=k_Init;k<k_End;k++)
    { k=(k%NbPts);
      x_co=(int)((WIDTH/MAX_X)*MatPts[k][0]);
      y_co=-(int)((HEIGHT/MAX_Y)*MatPts[k][1]);
      y_co+=(HEIGHT/2);
      x_co+=(WIDTH/2);
      if ((x_co<WIDTH)&&(y_co<HEIGHT)&&(x_co>0)&&(y_co>0))
         MatPict[y_co][x_co]=BLACK; }
}


//------------------------------------------------
// FONCTIONS TPs----------------------------------
//------------------------------------------------

// Util function showing a progress bar on screen, prints only when there is a change
int lastProgress = -1;
void resetProgressBar(){lastProgress = -1;}

void showProgressBar(int current, int total, int barWidth){
	float progress = (float)current/(float)(total-1);
	//Don't reprint if not necessary
	if (lastProgress == (int)(progress*100))
		return;
	else
		lastProgress = (int)(progress*100);

	std::string bar;
	int i = 0;
	// Progress filed with =
	for (i = 0; i < (int)(barWidth*progress); ++i)
		bar += '=';
	// Cute > at current position
	bar += '>';
	// Rest filled with spaces
	for (; i < barWidth; ++i)
		bar += ' ';

	//Print bar
	std::cout << "[" << bar << "] " << (int)(progress*100) << "%\r" << std::flush;
	fflush(stdout);
}

//   x'(t) and y'(t) = f(t,x,y,z) = z
inline float f(float t, float x, float y, float z){
	return z;
}

//   zx'(t) = gx(t,x,y,z) = -R*zx + sum(x) - C*x
inline float gx(float t, float x, float y, float z) {
	float sum = ((X_1-x) / pow(CARRE(X_1-x) + CARRE(Y_1-y) + CARRE(D), 3.0/2.0))
		+ ((X_2-x) / pow(CARRE(X_2-x) + CARRE(Y_2-y) + CARRE(D), 3.0/2.0))
		+ ((X_3-x) / pow(CARRE(X_3-x) + CARRE(Y_3-y) + CARRE(D), 3.0/2.0));
	return (-R*z + sum - C*x);
}

//   zy'(t) = gy(t,x,y,z) = -R*zy + sum(y) - C*y
inline float gy(float t, float x, float y, float z) {
	float sum = ((Y_1-y) / pow(CARRE(X_1-x) + CARRE(Y_1-y) + CARRE(D), 3.0/2.0))
		+ ((Y_2-y) / pow(CARRE(X_2-x) + CARRE(Y_2-y) + CARRE(D), 3.0/2.0))
		+ ((Y_3-y) / pow(CARRE(X_3-x) + CARRE(Y_3-y) + CARRE(D), 3.0/2.0));
	return (-R*z + sum - C*y);
}

inline int isCloseToAMagnet(float x, float y, float distanceMax){
	if (sqrt(CARRE(X_1-x) + CARRE(Y_1-y)) < distanceMax)
			return 1;
	if (sqrt(CARRE(X_2-x) + CARRE(Y_2-y)) < distanceMax)
		return 2;
	if (sqrt(CARRE(X_3-x) + CARRE(Y_3-y)) < distanceMax)
		return 3;
	return 0;
}

/* IMPORTANT this matrix should be 2 rows of length nbIntervals
 * MatPts[k][0] -> x axis
 * MatPts[k][1] -> y axis
 *
 * This function fills the MatPts matrix with (x,y) values of the trajectory
 * computed with the Runge-Kutta method
 */
int RungeKutta(float x_init, float dx_init, float y_init, float dy_init, int nbIntervals){

	const float distance_forConvergence = 0.5;
	const int nbIntervals_forConvergence = 20;
	int counter_forConvergence = 0;

  float kx1,kx2,kx3,kx4,kx5,kx6;
  float ky1,ky2,ky3,ky4,ky5,ky6;
  float lx1,lx2,lx3,lx4,lx5,lx6;
  float ly1,ly2,ly3,ly4,ly5,ly6;
  float tn, xn, yn, zxn, zyn;
	float _t,_x,_y,_zx, _zy;

	_x = x_init;
	_y = y_init;
  _zx = dx_init;
  _zy = dy_init;

  for(int i=1;i<(int)(nbIntervals);i+=1) {

		// Initialize this iteration
    tn = H * i;
    xn = _x;
    yn = _y;
		zxn = _zx;
		zyn = _zy;

		//========== K1 and L1 =================================
    kx1 = H * f (tn,xn,yn,zxn);
    lx1 = H * gx(tn,xn,yn,zxn);
    ky1 = H * f (tn,xn,yn,zyn);
    ly1 = H * gy(tn,xn,yn,zyn);

		//========== K2 and L2 =================================
		_t  =  tn + H/4.0;
		_x  =  xn + kx1/4.0;
		_y  =  yn + ky1/4.0;
		_zx = zxn + lx1/4.0;
		_zy = zyn + ly1/4.0;

    kx2 = H * f(_t, _x, _y, _zx);
    lx2 = H *gx(_t, _x, _y, _zx);
    ky2 = H * f(_t, _x, _y, _zy);
    ly2 = H *gy(_t, _x, _y, _zy);

		//========== K3 and L3 =================================
		_t  =  tn + H*(3.0/8.0);
		_x  =  xn + 3.0/32.0*kx1 + 9.0/32.0*kx2;
		_y  =  yn + 3.0/32.0*ky1 + 9.0/32.0*ky2;
		_zx = zxn + 3.0/32.0*lx1 + 9.0/32.0*lx2;
		_zy = zyn + 3.0/32.0*ly1 + 9.0/32.0*ly2;

    kx3 = H * f(_t, _x, _y, _zx);
    lx3 = H *gx(_t, _x, _y, _zx);
    ky3 = H * f(_t, _x, _y, _zy);
    ly3 = H *gy(_t, _x, _y, _zy);

		//========== K4 and L4 =================================
		_t  =  tn + H*(12.0/13.0);
		_x  =  xn + 1932.0/2197.0*kx1 - 7200.0/2197.0*kx2 + 7296.0/2197.0*kx3;
		_y  =  yn + 1932.0/2197.0*ky1 - 7200.0/2197.0*ky2 + 7296.0/2197.0*ky3;
		_zx = zxn + 1932.0/2197.0*lx1 - 7200.0/2197.0*lx2 + 7296.0/2197.0*lx3;
		_zy = zyn + 1932.0/2197.0*ly1 - 7200.0/2197.0*ly2 + 7296.0/2197.0*ly3;

    kx4 = H * f(_t, _x, _y, _zx);
    lx4 = H *gx(_t, _x, _y, _zx);
    ky4 = H * f(_t, _x, _y, _zy);
    ly4 = H *gy(_t, _x, _y, _zy);

		//========== K5 and L5 =================================
		_t  =  tn + H;
		_x  =  xn + 439.0/216.0*kx1 - 8.0*kx2 + 3680.0/513.0*kx3 - 845.0/4104.0*kx4;
		_y  =  yn + 439.0/216.0*ky1 - 8.0*ky2 + 3680.0/513.0*ky3 - 845.0/4104.0*ky4;
		_zx = zxn + 439.0/216.0*lx1 - 8.0*lx2 + 3680.0/513.0*lx3 - 845.0/4104.0*lx4;
		_zy = zyn + 439.0/216.0*ly1 - 8.0*ly2 + 3680.0/513.0*ly3 - 845.0/4104.0*ly4;

    kx5 = H * f(_t, _x, _y, _zx);
    lx5 = H *gx(_t, _x, _y, _zx);
    ky5 = H * f(_t, _x, _y, _zy);
    ly5 = H *gy(_t, _x, _y, _zy);

		//========== K6 and L6 =================================
		_t  =  tn + H/2.0;
		_x  =  xn - 8.0/27.0*kx1 + 2.0*kx2 - 3544.0/2565.0*kx3 + 1859.0/4104.0*kx4 - 11.0/40.0*kx5;
		_y  =  yn - 8.0/27.0*ky1 + 2.0*ky2 - 3544.0/2565.0*ky3 + 1859.0/4104.0*ky4 - 11.0/40.0*ky5;
		_zx = zxn - 8.0/27.0*lx1 + 2.0*lx2 - 3544.0/2565.0*lx3 + 1859.0/4104.0*lx4 - 11.0/40.0*lx5;
		_zy = zxn - 8.0/27.0*ly1 + 2.0*ly2 - 3544.0/2565.0*ly3 + 1859.0/4104.0*ly4 - 11.0/40.0*ly5;

    kx6 = H * f(_t, _x, _y, _zx);
    lx6 = H *gx(_t, _x, _y, _zx);
    ky6 = H * f(_t, _x, _y, _zy);
    ly6 = H *gy(_t, _x, _y, _zy);

		//========== Results for this iteration ==========
    _x  =  xn + (16.0/135.0*kx1 + 6656.0/12825.0*kx3 + 28561.0/56430.0*kx4 - 9.0/50.0*kx5 + 2.0/55.0*kx6);
    _zx = zxn + (16.0/135.0*lx1 + 6656.0/12825.0*lx3 + 28561.0/56430.0*lx4 - 9.0/50.0*lx5 + 2.0/55.0*lx6);
    _y  =  yn + (16.0/135.0*ky1 + 6656.0/12825.0*ky3 + 28561.0/56430.0*ky4 - 9.0/50.0*ky5 + 2.0/55.0*ky6);
    _zy = zyn + (16.0/135.0*ly1 + 6656.0/12825.0*ly3 + 28561.0/56430.0*ly4 - 9.0/50.0*ly5 + 2.0/55.0*ly6);

		if(int magnet = isCloseToAMagnet(_x, _y, distance_forConvergence)){
			counter_forConvergence++;
			if(counter_forConvergence >= nbIntervals_forConvergence){
				// For magnet we landed on -> 1,2 or 3
				/* return magnet; */
				// For nb of iterations for convergence
				return i;
			}
		} else counter_forConvergence = 0;
  }
	return nbIntervals;
}

//----------------------------------------------------------
//----------------------------------------------------------
// MAIN
//----------------------------------------------------------
//----------------------------------------------------------
int main (int argc, char **argv)
{
  int i,j,k;
  int flag_graph;
  int zoom;

  XEvent ev;
  Window win_ppicture;
  XImage *x_ppicture;
  char   nomfen_ppicture[100];
  char BufSystVisu[100];

  //>AllocMemory
  float*** MatPict=dmatrix_allocate_3d(TROIS,HEIGHT,WIDTH);
  float** MatPts=dmatrix_allocate_2d((int)(NB_INTERV),2);

  //>Init
  for(k=0;k<TROIS;k++) for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) MatPict[k][i][j]=0;
  for(i=0;i<2;i++) for(j=0;j<(int)(NB_INTERV);j++) MatPts[i][j]=0.0;
  flag_graph=1;
  zoom=1;


  //---------------------------------------------------------------------
  //>Question 2
  //---------------------------------------------------------------------

  for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) {
			showProgressBar(i*HEIGHT + j, WIDTH*HEIGHT, 70);

			float x = Xmin + (float)j/WIDTH * (Xmax - Xmin);
			float y = Ymax - (float)i/HEIGHT * (Ymax - Ymin);
			// Shade between 0 and 255
			/* float color = (float)RungeKutta(x,0.0,y,0.0,NB_INTERV)/NB_INTERV * 255.0; */
			/* // GREY FOR LIFE */
			/* MatPict[0][i][j]=color; */
			/* MatPict[1][i][j]=color; */
			/* MatPict[2][i][j]=color; */

			int magnet = RungeKutta(x,0.0,y,0.0,NB_INTERV);
			if(magnet == 1)
				MatPict[0][i][j]=255;
			if(magnet == 2)
				MatPict[1][i][j]=255;
			if(magnet == 3)
				MatPict[2][i][j]=255;
		}


   //--Fin Question 2-----------------------------------------------------


  //>Save&Visu de MatPict
  SaveImagePpm((char*)OUTPUT_FILE,MatPict,HEIGHT,WIDTH);


  //---------------------------

  //>Affiche Statistique
  printf("\n\n Stat:  Xmin=[%.2f] Xmax=[%.2f] Ymin=[%.2f] Ymax=[%.2f]\n",Xmin,Xmax,Ymin,Ymax);

 //--------------------------------------------------------------------------------
 //-------------- visu sous XWINDOW -----------------------------------------------
 //--------------------------------------------------------------------------------
 if (flag_graph)
 {
 //>Uuverture Session Graphique
 if (open_display()<0) printf(" Impossible d'ouvrir une session graphique");
 sprintf(nomfen_ppicture,"Évolution du Graphe");
 win_ppicture=fabrique_window(nomfen_ppicture,10,10,HEIGHT,WIDTH,zoom);
 x_ppicture=cree_XimageCoul(MatPict,zoom,HEIGHT,WIDTH);

 printf("\n\n Pour quitter,appuyer sur la barre d'espace");
 fflush(stdout);

  //boucle d'evenements
  for(;;)
     {
      XNextEvent(display,&ev);
       switch(ev.type)
        {
	 case Expose:

         XPutImage(display,win_ppicture,gc,x_ppicture,0,0,0,0,x_ppicture->width,x_ppicture->height);
         break;

         case KeyPress:
         XDestroyImage(x_ppicture);

         XFreeGC(display,gc);
         XCloseDisplay(display);
         flag_graph=0;
         break;
         }
   if (!flag_graph) break;
   }
 }

 //>Retour
 printf("\n Fini... \n\n\n");
 return 0;
}
