//------------------------------------------------------
// module  : Tp-IFT2425-I.c
// author  : Gevrai Jodoin-Tremblay
// date    : 21 Mars 2017
// version : 1.0
// language: C++
//------------------------------------------------------

//------------------------------------------------
// FICHIERS INCLUS -------------------------------
//------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <new>

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
// Libere la memoire de la matrice 1d de float
//----------------------------------------------------------
void free_fmatrix_1d(float* pmat)
{ delete[] pmat; }

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
 //>Var
 float result;
 double result_;

 //>Cst
 const double PI=3.14159265358979323846264338;
 int NBINTERV=5000000;
 int NbInt=NBINTERV;
 if (argc>1)  { NbInt=atoi(argv[1]); }
 float* VctPts=fmatrix_allocate_1d(NbInt+1);

 //----------------------------------------------------------
 //Programmer ici
 //----------------------------------------------------------

 float start = 0.0;
 float end = 1.0;
 float h = (end - start)/(float)NbInt;

 // Remplissage du tableau pour la méthode du trapèze
 // h/2 * (f(0) + 2f(1) + 2f(2) + ... + 2f(n-1) + f(n))
 VctPts[0] = fonction1(start);
 for (int i = 1; i < NbInt; i++)
   VctPts[i] = 2.0 * fonction1(i*h);
 VctPts[NbInt] = fonction1(end);

 // 1- Somme simple
 float sum = 0.0;
 for (int i = 0; i <= NbInt; i++){
   sum += VctPts[i];
 }
 result = h*sum/2.0;
 prettyPrint("1>Given_Order", result, PI);

 // 2- Somme par cascade récursive
 result = h/2.0 * cascadeSum(VctPts, 0, NbInt);
 prettyPrint("2>PairwiseSum", result, PI);

 // 2b- Somme compensée de Kahan
 result = h/2.0 * kahamCompSum(VctPts, NbInt);
 prettyPrint("3>KahanSummat", result, PI);

 // Terminé
 free_fmatrix_1d(VctPts);
 return 0;
 }
