//------------------------------------------------------
// module  : Tp-IFT2425-I.c
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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <new>
#include <time.h>

//-------------------------
//--- Windows -------------
//-------------------------
#include <X11/Xutil.h>

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
//  Alloue de la memoire pour une matrice 1d de float
//----------------------------------------------------------
float* fmatrix_allocate_1d(int hsize)
 {
  float* matrix;
  matrix=new float[hsize]; return matrix; }

//----------------------------------------------------------
//  Alloue de la memoire pour une matrice 2d de float
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
// Libere la memoire de la matrice 1d de float
//----------------------------------------------------------
void free_fmatrix_1d(float* pmat)
{ delete[] pmat; }

//----------------------------------------------------------
// Libere la memoire de la matrice 2d de float
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

//-------------------------//
//---- Fonction Pour TP ---//
//-------------------------//

//----------------------------------------------------------
// Fonction (1) non-intégrée de l'énoncé
//----------------------------------------------------------
inline float fonction1(const float x){
  return 4.0 * sqrt(1 - x*x);
}
//----------------------------------------------------------
// Imprime les résultats comme demandé dans le Barème,
// calculant par le fait même l'erreur
//----------------------------------------------------------
void prettyPrint(char const* msg, const float estimate, const double realValue){
  float error = abs(estimate - realValue);
  float logerr = log10(error);
  printf("[%s:]\tPi=%.10f\tEr=%.10f\tLogEr=%.2f\n",
             msg,    estimate, error,       logerr);
}
//----------------------------------------------------------
// Sommation en cascade des éléments de v par récursion
//----------------------------------------------------------
float cascadeSum(float const* v, const int startInd, const int endInd){
  int diff = endInd - startInd;
  if (diff < 0) {
    printf("ERROR: start=%d end=%d", startInd, endInd);
    exit(1);
  }
  if (diff == 0) return v[startInd];
  if (diff == 1) return v[startInd] + v[endInd];

  int middleInd = (startInd + endInd)/2;
  return cascadeSum(v, startInd, middleInd) + cascadeSum(v, middleInd+1, endInd);
}
//----------------------------------------------------------
// Calcul de la somme des éléments dans 'v' par la somme
// compensée de Kahan présentée dans l'énoncé
//----------------------------------------------------------
float kahamCompSum(float const*v, const int nbInter){
  float e = 0.0;
  float s = 0.0;
  float temp, y;
  for (int i=0; i <= nbInter; i++){
    temp = s;
    y = v[i] + e;
    s = temp + y;
    e = (temp - s) + y;
  }
  return s;
}
//----------------------------------------------------------
//----------------------------------------------------------
// PROGRAMME PRINCIPAL -------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
int main(int argc,char** argv)
{
 int   i,j,k,l;
 int   flag_graph;
 int   zoom;

 //Pour Xwindow
 //------------
 XEvent ev;
 Window win_ppicture;
 XImage *x_ppicture;
 char   nomfen_ppicture[100];
 int    length,width;

 length=width=4096;
 float** Graph2D=fmatrix_allocate_2d(length,width);
 flag_graph=1;
 zoom=-16;

 //Affichage Axes
 for(i=0;i<length;i++) for(j=0;j<width;j++) Graph2D[i][j]=190.0;


//--------------------------------------------------------------------------------
// PROGRAMME ---------------------------------------------------------------------
//--------------------------------------------------------------------------------

 //>Var
 float result;
 double result_;

 //>Cst
 const double PI=3.14159265358979323846264338;
 int NBINTERV=5000000;
 int NbInt=NBINTERV;
 if (argc>1)  { NbInt=atoi(argv[1]); }
 float* VctPts=fmatrix_allocate_1d(NbInt+1);

 //Programmer ici

 float start = 0.0;
 float end = 1.0;
 float h = (end - start)/(float)NBINTERV;

 // Remplissage du tableau pour les trois versions
 VctPts[0] = fonction1(start);
 VctPts[NBINTERV] = fonction1(end);
 for (int i = 1; i < NBINTERV; i++){
   VctPts[i] = 2.0 * fonction1(i*h);
 }

 // 1- Méthode d'intégration par des trapèzes
 // h/2 * (f(0) + 2f(1) + 2f(2) + ... + 2f(n-1) + f(n))
 float sum = 0.0;
 for (int i = 0; i <= NBINTERV; i++){
   sum += VctPts[i];
 }
 result = h*sum/2.0;
 prettyPrint("1>Given_Order", result, PI);

 // 2- Somme par cascade récursive
 result = h/2.0 * cascadeSum(VctPts, 0, NBINTERV);
 prettyPrint("2>PairwiseSum", result, PI);

 // 2b- Somme compensée de Kahan
 result = h/2.0 * kahamCompSum(VctPts, NBINTERV);
 prettyPrint("3>KahanSummat", result, PI);

 exit(0);
 //End


//--------------------------------------------------------------------------------
//---------------- visu sous XWINDOW ---------------------------------------------
//--------------------------------------------------------------------------------
 if (flag_graph)
 {
 //ouverture session graphique
 if (open_display()<0) printf(" Impossible d'ouvrir une session graphique");
 sprintf(nomfen_ppicture,"Graphe : ","");
 win_ppicture=fabrique_window(nomfen_ppicture,10,10,width,length,zoom);
 x_ppicture=cree_Ximage(Graph2D,zoom,length,width);

 //Sauvegarde
 //SaveImagePgm((char*)"",(char*)"Graphe",Graph2D,length,width); //Pour sauvegarder l'image
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
