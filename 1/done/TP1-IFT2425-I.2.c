//------------------------------------------------------
// module  : Tp-IFT2425-I.2.c
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

//------------------------------------------------
// Evaluation de l'equation (1) donne dans l'enonce,
// decomposee en f(cmv,yi) = a/b - 1/cmv - c/N
// pour une meilleure comprehension
//------------------------------------------------
float f(float cmv, float y[], int ysize){
    
    int i;
    float yi_pow_cmv, lnyi;
    float a = 0., b = 0., c = 0.;
    
    for (i=0; i<ysize;i++){
		yi_pow_cmv = pow (y[i], cmv);
		lnyi = log(y[i]);
		a += yi_pow_cmv*lnyi;
		b += yi_pow_cmv;
		c += lnyi;
    }

    return (a/b) - (1/cmv) - (c/(float)ysize);
}

int main(int argc,char** argv)
{
	float esp = 1e-5;

	int ysize = 10;
	float y[10] = {0.11, 0.24, 0.27, 0.52, 1.13, 1.54, 1.71, 1.84, 1.92, 2.01};
	
	float cmv = 0.25;

	// X = g(x) = X - F(x) / F'(x)
	int i = 1;
	while (1){
		float fx = f(cmv, y, ysize);

		// Tolerance sur fx
		if (abs(fx) <= 1e-6)
			break;
		// Calcul d'une approximation de la derivee
		float dfx = (
		   	- f(cmv+2*esp,y,ysize)
		   	+ 8*f(cmv+esp,y,ysize)
		   	- 8*f(cmv-esp,y,ysize)
		   	+ f(cmv-2*esp,y,ysize)) / (12*esp);
		// x = g(x)
		float x = cmv - fx/dfx;
		printf("IT %d: x = %f, f(x) =  %f, f'(x) =  %f\n",i,x,fx,dfx);

		// Tolerance sur x
		if (abs(x-cmv) <= 10e-6){
			cmv=x;
			break;
		}

		// Prochaine iteration
		cmv = x;
		i++;
    }
	
	printf("=========================\nResultat final: x=%f\n",cmv);
}

