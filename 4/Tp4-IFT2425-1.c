//------------------------------------------------------
// module  : Tp4-IFT2425-1.c
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
#include <unistd.h>

#include <iostream>
#include <string>

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

#define OUTPUT_FILE "Tp4-Img-I.pgm"
#define VIEW_PGM    "xv"
#define DEBUG 0

//-Cst-Modele
#define X_1 0.0
#define Y_1 1.0
#define X_2 -1.0/sqrt(2.0)
#define Y_2 -1.0/2.0
#define X_3 +1.0/2*sqrt(2.0)
/* #define X_3 +1.0/sqrt(2.0) */
#define Y_3 -1.0/2.0
#define C 0.25
#define R 0.1
#define D 0.3

//-Valeurs initiales
#define X_1_INI 0.2
#define Y_1_INI -1.6
#define X_2_INI 0.0
#define Y_2_INI 0.0
#define X_3_INI -1.6
#define Y_3_INI 0.2
#define X_4_INI 1.0
#define Y_4_INI 1.0

//-Cst-Runge-Kutta
#define H            0.0001
#define T_0          0.0
#define T_F         30.0
#define NB_INTERV (T_F-T_0)/H

 //-Cst-Image
#define WIDTH  512
#define HEIGHT 512
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
// SaveImagePgm
//----------------------------------------------------------
void SaveImagePgm(char* name,float** mat,int lgth,int wdth)
{
 int i,j;
 char buff[300];
 FILE* fic;

  //--extension--
  strcpy(buff,name);

  //--ouverture fichier--
  fic=fopen(buff,"wb");
    if (fic==NULL)
        { printf("Probleme dans la sauvegarde de %s",buff);
          exit(-1); }
  printf("\n Sauvegarde de %s au format pgm\n",buff);

  //--sauvegarde de l'entete--
  fprintf(fic,"P5");
  fprintf(fic,"\n# IMG Module");
  fprintf(fic,"\n%d %d",wdth,lgth);
  fprintf(fic,"\n255\n");

  //--enregistrement--
  for(i=0;i<lgth;i++) for(j=0;j<wdth;j++)
	fprintf(fic,"%c",(char)mat[i][j]);

  //--fermeture fichier--
  fclose(fic);
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

/* IMPORTANT this matrix should be 2 rows of length nbIntervals
 * MatPts[k][0] -> x axis
 * MatPts[k][1] -> y axis
 *
 * This function fills the MatPts matrix with (x,y) values of the trajectory
 * computed with the Runge-Kutta method
 */
void RungeKutta(float x_init, float dx_init, float y_init, float dy_init, float** MatPts, int nbIntervals){

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

		showProgressBar(i, nbIntervals, 70);

		// Initialize this iteration
    tn = H * i;
    xn = _x;
    yn = _y;
		zxn = _zx;
		zyn = _zy;

		// Euler test
		if (true){
			_x   = xn + H* f(tn,xn,yn,zxn);
			_zx = zxn + H*gx(tn,xn,yn,zxn);
			_y   = yn + H* f(tn,xn,yn,zyn);
			_zy = zyn + H*gy(tn,xn,yn,zyn);

			MatPts[i][0] = _x;
			MatPts[i][1] = _y;
			continue;
		}

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

		MatPts[i][0] = _x;
		MatPts[i][1] = _y;
  }
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
  float** MatPict=dmatrix_allocate_2d(HEIGHT,WIDTH);
  float** MatPts=dmatrix_allocate_2d((int)(NB_INTERV),2);

  //>Init
  for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) MatPict[i][j]=GREYWHITE;
  for(i=0;i<2;i++) for(j=0;j<(int)(NB_INTERV);j++) MatPts[i][j]=0.0;
  flag_graph=1;
  zoom=1;


  //---------------------------------------------------------------------
  //>Question 1
  //---------------------------------------------------------------------

  /* Résolution de l'équation différentielle d'ordre 2 selon Runge-Kutta
  *   x'(t)  = f(t, x(t), y(t), zx(t))
  *   zx'(t) = gx(t, x(t), y(t), zx(t))
  *   y'(t)  = f(t, x(t), y(t), zy(t))
  *   zy'(t) = gy(t, x(t), y(t), zy(t))
  * qui devient ainsi
  *   zx'(t) = gx(t,x,y,zx) = 1/R * (-zx + sum - Cx)
  *   x'(t)  =  f(t,x,y,zx) = zx
  *   zy'(t) = gy(t,x,y,zy) = 1/R * (-zy + sum - Cy)
  *   y'(t)  =  f(t,x,y,zy) = zy
	*/

	//> Initial value of run
	float x_init = X_1_INI;
	float y_init = Y_1_INI;
	float dx_init = 0.0;
	float dy_init = 0.0;

	// Print values of model
	std::cout << "\nC = " << C << "\t";
	std::cout << "R = " << R << "\t";
	std::cout << "D = " << D << "\n";
	std::cout << "Magnet 1 at (" << X_1 << "," << Y_1 << ")\n";
	std::cout << "Magnet 2 at (" << X_2 << "," << Y_2 << ")\n";
	std::cout << "Magnet 3 at (" << X_3 << "," << Y_3 << ")\n";
	std::cout << "\nTime interval in seconds : [" << T_0 << "," << T_F << "]\n";
	std::cout << "with " << (int)NB_INTERV << " intervals (h=" << H << ")\n\n";
	std::cout << "Initial position (" << x_init << "," << y_init << ")";
	std::cout << "\nwith velocity (" << dx_init << "," << dy_init << "\n";

	// The meat!
	RungeKutta(x_init, dx_init, y_init, dy_init, MatPts, NB_INTERV);

  //--Fin Question 1-----------------------------------------------------


  //>Affichage des Points dans MatPict
  plot_point(MatPts,MatPict,(int)(NB_INTERV));

  //>Save&Visu de MatPict
  SaveImagePgm((char*)OUTPUT_FILE,MatPict,HEIGHT,WIDTH);
  strcpy(BufSystVisu,VIEW_PGM);
  strcat(BufSystVisu," ");
  strcat(BufSystVisu,OUTPUT_FILE);
  strcat(BufSystVisu," &");
  system(BufSystVisu);

  //>Affiche Statistique
  printf("\n\n Stat:  Xmin=[%.2f] Xmax=[%.2f] Ymin=[%.2f] Ymax=[%.2f]\n",Xmin,Xmax,Ymin,Ymax);


 //--------------------------------------------------------------------------------
 //-------------- visu sous XWINDOW de l'évolution de MatPts ----------------------
 //--------------------------------------------------------------------------------
 if (flag_graph)
 {
 //>Uuverture Session Graphique
 if (open_display()<0) printf(" Impossible d'ouvrir une session graphique");
 sprintf(nomfen_ppicture,"Évolution du Graphe");
 win_ppicture=fabrique_window(nomfen_ppicture,10,10,HEIGHT,WIDTH,zoom);
 x_ppicture=cree_Ximage(MatPict,zoom,HEIGHT,WIDTH);

 printf("\n\n Pour quitter,appuyer sur la barre d'espace");
 fflush(stdout);

 //>Boucle Evolution
  for(i=0;i<HEIGHT;i++) for(j=0;j<WIDTH;j++) MatPict[i][j]=GREYWHITE;
  for(k=0;;)
     {
       k=((k+EVOL_GRAPH)%(int)(NB_INTERV));
       Fill_Pict(MatPts,MatPict,k,(int)(NB_INTERV));
       XDestroyImage(x_ppicture);
       x_ppicture=cree_Ximage(MatPict,zoom,HEIGHT,WIDTH);
       XPutImage(display,win_ppicture,gc,x_ppicture,0,0,0,0,x_ppicture->width,x_ppicture->height);
       usleep(10000);  //si votre machine est lente mettre un nombre plus petit
     }
 }

 //>Retour
 printf("\n Fini... \n\n\n");
 return 0;
}
