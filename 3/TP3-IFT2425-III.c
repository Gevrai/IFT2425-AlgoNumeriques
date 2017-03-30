//------------------------------------------------------
// module  : Tp-IFT2425-III.c
// author  : Gevrai Jodoin-Tremblay
// date    : 28 Mars 2017
// version : 1.0
// language: C++
//------------------------------------------------------

//------------------------------------------------
// FICHIERS INCLUS -------------------------------
//------------------------------------------------
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

//-------------------------//
//---- Fonction Pour TP ---//
//-------------------------//

//----------------------------------------------------------
// Fonction (2) de l'énoncé
//----------------------------------------------------------
inline double fonction2(float u, double x){
  return u*x*(1.0-x);
}

//----------------------------------------------------------
// Fonction approximant la valeur de Pi avec la suite logistique
// de valeur u = 4.0. Bien que cette fonction renvoie un
// float, elle utilise des 'doubles' à l'interne car l'utilisation
// de float menait à un underflow pour la plupart des 'x_init'.
// Ceci est effectué suite à une recommandation de M.Mignotte.
//----------------------------------------------------------
float pi_approximation(const double x_init, const int nbIter){
  float u = 4.0;
  double xn = x_init;
  double sum = 0.0;
  for (int i=0; i<nbIter;i++){
    xn = fonction2(u, xn);
    sum += sqrt(xn);
  }
  float result = 2.0 * (double)nbIter / sum;
  printf("[%.2f:>%.10f]\n", x_init, result);
  return result;
}

//----------------------------------------------------------
//----------------------------------------------------------
// PROGRAMME PRINCIPAL -------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
int main(int argc,char** argv)
{
 //>Var
 int NbInt;

 if (argc>1) NbInt=atoi(argv[1]);
 else NbInt = 1e7;

 pi_approximation(0.2, NbInt);
 pi_approximation(0.4, NbInt);
 pi_approximation(0.6, NbInt);

 return 0;
 }
