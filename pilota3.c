#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */

#include <pthread.h>		/* incloure threads */
#include <stdint.h>
#include <time.h>

#include "memoria.h"
#include <unistd.h>
#include <sys/types.h>


#define MAX_THREADS	10
#define MAXBALLS	(MAX_THREADS-1)
#define MIN_FIL	10		/* dimensions del camp. Depenen del terminal ! */
#define MAX_FIL	50
#define MIN_COL	10
#define MAX_COL	80
#define BLKSIZE	3
#define BLKGAP	2
#define BLKCHAR 'B'
#define WLLCHAR '#'
#define FRNTCHAR 'A'
#define LONGMISS	65
#define MAX_PELOTAS 9
#define PARAMETROS_PELOTAS 6	

//float pelotas[MAX_PELOTAS][PARAMETROS_PELOTAS];
//poner parametros de la pelota
int f_pil, c_pil, n_col;		/* posicio de la pilota, en valor enter */
float pos_f, pos_c;		/* posicio de la pilota, en valor real */
float vel_f, vel_c;		/* velocitat de la pilota, en valor real */
int n_fil, retard;
int pfi2;

int ipc_numPilotes, ipc_fi1, ipc_mem,ipc_indexPilota;

int *p_mem, *p_fi1, *p_numPilotes, *p_indexPilota;


//FUNCION COMPROVAR_BLOC
int ipc_nblocs, ipc_numPilotesreal;
int *p_nblocs, *p_numPilotesreal;
int lines;

// control_impacte2
int c_pal, m_pal;




void comprovar_bloc(int f, int c)
{
	int col;
	
	char quin = win_quincar(f, c);
	

	if (quin == BLKCHAR || quin == FRNTCHAR) {
		col = c;
		
		while (win_quincar(f, col) != ' ') {
		
			win_escricar(f, col, ' ', NO_INV);
			
			col++;
		}
		
		col = c - 1;
		
		while (win_quincar(f, col) != ' ') {
			win_escricar(f, col, ' ', NO_INV);
			col--;
		}
		
		if (quin == BLKCHAR && (*p_numPilotesreal) <= lines){
			
			//*p_indexPilota = *p_indexPilota + 1; //incrementem el index de la pilota
			(*p_indexPilota)++;
			(*p_numPilotesreal)++;
			(*p_numPilotes)++;
			//*p_numPilotes = *p_numPilotes + 1; //*p_numPilotes++; //incrementem el numero de pilotes
			//*p_numPilotesreal = *p_numPilotesreal + 1; //*p_numPilotesreal++;//Aqui avisamos que hay que crear nueva pelota al MAIN
			//p_numPilotes++; //incrementem el numero de pilotes
			//p_numPilotesreal++;//Aqui avisamos que hay que crear nueva pelota al MAIN
			
			
		
		}
		
        	
		(*p_nblocs)--; 
		//*p_nblocs--;
		//p_nblocs--;
		
	}
}


float control_impacte2(int c_pil, float velc0) {
	int distApal;
	float vel_c;

	distApal = c_pil - c_pal;
	if (distApal >= 2*m_pal/3)	/* costat dreta */
		vel_c = 0.5;
	else if (distApal <= m_pal/3)	/* costat esquerra */
		vel_c = -0.5;
	else if (distApal == m_pal/2)	/* al centre */
		vel_c = 0.0;
	else /*: rebot normal */
		vel_c = velc0;
	return vel_c;
}











//PODRIA SER VOID???
int main(int n_args, char *ll_args[]){
{	
	int f_h, c_h;
	char rh, rv, rd;
	
	//int index2=(intptr_t) index;
	pfi2 = 0; //inicialitzem
	
	
	f_pil = atoi(ll_args[1]);
	c_pil = atoi(ll_args[2]);
	pos_f = atof(ll_args[3]);
	pos_c = atof(ll_args[4]);
	vel_f = atof(ll_args[5]); 
	vel_c = atof(ll_args[6]);
	n_fil = atoi(ll_args[7]);
	retard = atoi(ll_args[8]);
	ipc_numPilotes = atoi(ll_args[9]);
	ipc_fi1 = atoi(ll_args[10]);
	ipc_mem = atoi(ll_args[11]); 
	n_col = atoi(ll_args[12]);
	//funcion comprovar_bloc
	ipc_nblocs = atoi(ll_args[13]);
	ipc_numPilotesreal = atoi(ll_args[14]);
	lines = atoi(ll_args[15]);
	//control_impacte2
	c_pal = atoi(ll_args[16]);
	m_pal = atoi(ll_args[17]);
	ipc_indexPilota = atoi(ll_args[18]);
	
	
		
	//mapejem la referència de memòria compartida que conté el camp de joc, utilitzant la referència (identificador) que s'ha passat per argument des del procés pare
	p_mem=map_mem(ipc_mem);	
	p_fi1=map_mem(ipc_fi1);	
	p_numPilotes=map_mem(ipc_numPilotes);
	//funcion comprovar_bloc
	p_nblocs=map_mem(ipc_nblocs);
	p_numPilotesreal=map_mem(ipc_numPilotesreal);
	p_indexPilota = map_mem(ipc_indexPilota);
	
	//invoquem la funció win_set() amb la zona de memoria compartida i el numero de files i columnes
	
	win_set(p_mem, n_fil, n_col);	

  do{
  
    win_retard(retard);
    
	f_h = pos_f + vel_f;	/* posicio hipotetica de la pilota (entera) */
	c_h = pos_c + vel_c;
	rh = rv = rd = ' ';
	if ((f_h != f_pil) || (c_h != c_pil)) {
	/* si posicio hipotetica no coincideix amb la posicio actual */
		if (f_h != f_pil) {	/* provar rebot vertical */
			
			rv = win_quincar(f_h, c_pil);	/* veure si hi ha algun obstacle */
			
			if (rv != ' ') {	/* si hi ha alguna cosa */
				comprovar_bloc(f_h, c_pil);
				if (rv == '0'){	/* col.lisió amb la paleta? */
//					control_impacte();
						
					vel_c = control_impacte2(c_pil, vel_c);
					
					}
					
					
				vel_f = -vel_f;	/* canvia sentit velocitat vertical */
				
				f_h = pos_f + vel_f;	/* actualitza posicio hipotetica */
			}
		}
		if (c_h != c_pil) {	/* provar rebot horitzontal */
			
			rh = win_quincar(f_pil, c_h);	/* veure si hi ha algun obstacle */
			
			if (rh != ' ') {	/* si hi ha algun obstacle */
				comprovar_bloc(f_pil, c_h);
				/* TODO?: tractar la col.lisio lateral amb la paleta */
				
				vel_c = -vel_c;	/* canvia sentit vel. horitzontal */
				
				c_h = pos_c + vel_c;	/* actualitza posicio hipotetica */
			}
		}
		if ((f_h != f_pil) && (c_h != c_pil)) {	/* provar rebot diagonal */
			
			rd = win_quincar(f_h, c_h);
			
			if (rd != ' ') {	/* si hi ha obstacle */
				comprovar_bloc(f_h, c_h);
				vel_f = -vel_f;
				vel_c = -vel_c;	/* canvia sentit velocitats */
				f_h = pos_f + vel_f;
				c_h = pos_c + vel_c;	/* actualitza posicio entera */
			}
		}
		/* mostrar la pilota a la nova posició */
		if (win_quincar(f_h, c_h) == ' ') {	/* verificar posicio definitiva *//* si no hi ha obstacle */
			win_escricar(f_pil, c_pil, ' ', NO_INV);	/* esborra pilota */
			pos_f += vel_f;
			pos_c += vel_c;
			f_pil = f_h;
			c_pil = c_h;	/* actualitza posicio actual */
			if (f_pil != n_fil - 1){	/* si no surt del taulell, */
				win_escricar(f_pil, c_pil, '1', INVERS);	/* imprimeix pilota */
				}
			else{
				pfi2= 1; //surt fora del camp el fiquem a 1
				
				//fi2 = 1;
				
				//*p_numPilotes = *p_numPilotes - 1;//*p_numPilotes--; //decrementem el numero de pilotes actives
				(*p_numPilotes)--;
				//p_numPilotes--;
				
				}
		}
		
	} else {	/* posicio hipotetica = a la real: moure */
		
		pos_f += vel_f;
		
		
		pos_c += vel_c;
		
	}

	//compr = comprova_num_pilota();
	  } while (!*p_fi1 && !pfi2); //!fi2); //while (!fi1 && compr);//while (!fi1 && !pfi2[index2]); //
}

}
