//------------------------------------------------------
// module  : Tp-IFT2425-I.3.c
// author  : Gevrai Jodoin-Tremblay
// date    : 27/01/17
// version : 1.0
// language: C
// note    :
//------------------------------------------------------
//  

//------------------------------------------------
// FICHIERS INCLUS -------------------------------
//------------------------------------------------
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <new>
#include <time.h>

//------------------------------------------------
// DEFINITIONS -----------------------------------
//------------------------------------------------
#define CARRE(X) ((X)*(X))
#define CUBE(X)  ((X)*(X)*(X))

//-------------------------
//--- Windows -------------
//-------------------------
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

Display   *display;
int	  screen_num;
int 	  depth;
Window	  root;
Visual*	  visual;
GC	  gc;

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
/* Crée une XImage à partir d'un tableau de float                           */
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

//-------------------------//
//-- Matrice de Flottant --//
//-------------------------//
//---------------------------------------------------------
//  alloue de la memoire pour une matrice 1d de float
//----------------------------------------------------------
float* fmatrix_allocate_1d(int hsize)
 {
  float* matrix;
  matrix=new float[hsize]; return matrix; }

//----------------------------------------------------------
//  alloue de la memoire pour une matrice 2d de float
//----------------------------------------------------------
float** fmatrix_allocate_2d(int vsize,int hsize)
 {
  float** matrix;
  float *imptr;

  matrix=new float*[vsize];
  imptr=new  float[(hsize)*(vsize)];
  for(int i=0;i<vsize;i++,imptr+=hsize) matrix[i]=imptr;
  return matrix;
 }

//----------------------------------------------------------
// libere la memoire de la matrice 1d de float
//----------------------------------------------------------
void free_fmatrix_1d(float* pmat)
{ delete[] pmat; }

//----------------------------------------------------------
// libere la memoire de la matrice 2d de float
//----------------------------------------------------------
void free_fmatrix_2d(float** pmat)
{ delete[] (pmat[0]);
  delete[] pmat;}

//----------------------------------------------------------
// Sauvegarde de l'image de nom <name> au format pgm                        
//----------------------------------------------------------                
void SaveImagePgm(char* bruit,char* name,float** mat,int lgth,int wdth)
{
 int i,j;
 char buff[300];
 FILE* fic;

  //--extension--
  strcpy(buff,bruit);
  strcat(buff,name);
  strcat(buff,".pgm");

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

//----------------------------------------------------------
// Recal                                                    *
//----------------------------------------------------------
void Recal(float** mat,int lgth,int wdth)
{
 int i,j;
 float max,min,tmp;;


 //Recherche du min
  min=mat[0][0];
  for(i=0;i<lgth;i++) for(j=0;j<wdth;j++)
    if (mat[i][j]<min) min=mat[i][j];

  //plus min
  for(i=0;i<lgth;i++) for(j=0;j<wdth;j++) mat[i][j]-=min;
 
  //Recherche du max
  max=mat[0][0];
  for(i=0;i<lgth;i++) for(j=0;j<wdth;j++) 
    if (mat[i][j]>max) max=mat[i][j];

  //Recalibre la matrice
 for(i=0;i<lgth;i++) for(j=0;j<wdth;j++)
   mat[i][j]*=(255/max);
}

//----------------------------------------------------------
//  Egalisation Histogramme         
//----------------------------------------------------------
void Egalise(float** img,int lgth,int wdth,int thresh)
{
 int i,j;
 float tmp;
 float nb;
 float HistoNg[256];
 float FnctRept[256];

 //Calcul Histogramme Ng
 for(i=0;i<256;i++) HistoNg[i]=0.0;

 nb=0;
 for(i=0;i<lgth;i++) for(j=0;j<wdth;j++)
    { tmp=img[i][j];
      if (tmp>thresh) { HistoNg[(int)(tmp)]++; nb++; } } 
 
 for(i=0;i<256;i++)  HistoNg[i]/=(float)(nb);

 //Calcul Fnct Repartition
 for(i=0;i<256;i++) FnctRept[i]=0.0;

 for(i=0;i<256;i++)
    { if (i>0)  FnctRept[i]=FnctRept[i-1]+HistoNg[i];
      else      FnctRept[i]=FnctRept[i]; }

 for(i=0;i<256;i++) FnctRept[i]=(int)((FnctRept[i]*255)+0.5);

 //Egalise
 for(i=0;i<lgth;i++) for(j=0;j<wdth;j++)
   img[i][j]=FnctRept[(int)(img[i][j])];
}


//----------------------------------------------------------
//----------------------------------------------------------
// PROGRAMME PRINCIPAL -------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
int main(int argc,char** argv)
{
  int i,j,k;
 int flag_graph;
 int zoom;

 //Pour Xwindow
 //------------
 XEvent ev;
 Window win_ppicture;
 XImage *x_ppicture; 
 char   nomfen_ppicture[100]; 
 const int length = 512;
 const int width = 512;

 float** Graph2D=fmatrix_allocate_2d(length,width); 
 flag_graph=1;
 zoom=1;

 //Init
 for(i=0;i<length;i++) for(j=0;j<width;j++) Graph2D[i][j]=0.0;
 
//--------------------------------------------------------------------------------
// PROGRAMME ---------------------------------------------------------------------
//--------------------------------------------------------------------------------

 //Affichage dégradé de niveaux de gris dans Graph2D
 //for(int i=0;i<length;i++) for(int j=0;j<width;j++) Graph2D[i][j]=j/2.0;

  
   //---------------------------
   //Algorithme NEWTON
   //---------------------------
	float zr, zi, z1r,z1i,cr,ci;

	const int maxIter = 200;
	for(int i=0;i<length;i++){
		ci = 2.0 * ((float)i-(float)length/2.0)/((float)length-1);
	for(int j=0;j<width;j++){
		cr = 2.0 * ((float)j-(float)width/1.35)/((float)width-1);
		zr = 0;
		zi = 0;

		for (k=0; k<maxIter;k++) {
			z1r = zr*zr - zi*zi + cr;
			z1i = 2.0*zr*zi + ci;

			zr = z1r;
			zi = z1i;

			// Module plus grand que 2 ?
			if (zr*zr + zi*zi > 4){
				break;
			}
		}

		// Si le pixel n'appartient pas au mandelbrot set, on affiche la "trajectoire" des 
		// 10x10 sous-pixels. Ceci est evidemment une approximation, puisque tous les sous-pixels
		// ne font pas necessairement de l'ensemble...
		if(k != maxIter){
			int corr_i,corr_j;
			float pas = 0.1;
			float pas_i,pas_j;
			for(pas_i=(float)i;pas_i<i+1;pas_i += pas) {
				ci = 2.0 * (pas_i-(float)length/2.0)/((float)length-1);
			for(pas_j=(float)j;pas_j<j+1;pas_j += pas){
				cr = 2.0 * (pas_j-(float)width/1.35)/((float)width-1);
				zr = 0;
				zi = 0;

				for (k=0; k<maxIter;k++) {
					z1r = zr*zr - zi*zi + cr;
					z1i = 2.0*zr*zi + ci;

					zr = z1r;
					zi = z1i;

					// Module plus grand que 2 ?
					if (zr*zr + zi*zi > 4){
						break;
					}

					// Accumule le pixel correspondant!
					corr_j = zr*((float)width-1.0)/2.0 + (float)width/1.35 + 0.5;
					corr_i = zi*((float)length-1.0)/2.0 + (float)length/2.0 + 0.5;
					if (corr_j >= 0 && corr_j < width && corr_i >= 0 && corr_i < length){
						Graph2D[corr_j][corr_i] += 1.0;
					}

				}
			}
			}
		}
	}

	// Un semblant de barre de progres
	std::cout << "[";
	float progress = (float)i/length;
	int barWidth = 70;
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0) << " %\r";
	std::cout.flush();

	}

     
//--------------------------------------------------------------------------------
//---------------- visu sous XWINDOW ---------------------------------------------
//--------------------------------------------------------------------------------
  
//Recalage-Egalise le graph
Recal(Graph2D,length,width);
Egalise(Graph2D,length,width,0.0);

 if (flag_graph)
 {
 //ouverture session graphique
 if (open_display()<0) printf(" Impossible d'ouvrir une session graphique");
 sprintf(nomfen_ppicture,"Graphe : ","");
 win_ppicture=fabrique_window(nomfen_ppicture,10,10,width,length,zoom);
 x_ppicture=cree_Ximage(Graph2D,zoom,length,width);

 //Sauvegarde
 SaveImagePgm((char*)"",(char*)"FractalMandelbrot_accuInterne",Graph2D,length,width);
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
       
 //retour sans probleme 
 printf("\n Fini... \n\n\n");
 return 0;
 }
 


