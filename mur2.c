/*****************************************************************************/
/*                                                                           */
/*                           mur0.c                                          */
/*                                                                           */
/*  Programa inicial d'exemple per a les practiques 2 de FSO.                */
/*                                                                           */
/*  Compilar i executar:                                                     */
/*     El programa invoca les funcions definides a "winsuport.c", les        */
/*     quals proporcionen una interficie senzilla per crear una finestra     */
/*     de text on es poden escriure caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':                                   */
/*                                                                           */
/*       $ gcc -Wall -c winsuport.c -o winsuport.o                           */
/*       $ gcc -Wall mur0.c winsuport.o -o mur0 -lcurses                     */
/*                                                                           */
/*  Al tenir una orientació vertical cal tenir un terminal amb prou files.   */
/*  Per exemple:                                                             */
/*               xterm -geometry 80x50                                       */
/*               gnome-terminal --geometry 80x50                             */
/*                                                                           */
/*****************************************************************************/

//#include <stdint.h>		/* intptr_t for 64bits machines */

#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "winsuport.h"		/* incloure definicions de funcions propies */
#include <pthread.h>		/* incloure threads */
#include <stdint.h>
#include <time.h>

/* definicio de constants */
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
#define PARAMETROS_PELOTAS 6			/* variables globals */
char *descripcio[] = {
	"\n",
	"Aquest programa implementa una versio basica del joc Arkanoid o Breakout:\n",
	"generar un camp de joc rectangular amb una porteria, una paleta que s\'ha\n",
	"de moure amb el teclat per a cobrir la porteria, i una pilota que rebota\n",
	"contra les parets del camp, a la paleta i els blocs. El programa acaba si\n",
	"la pilota surt per la porteria o no queden mes blocs. Tambe es pot acabar\n",
	"amb la tecla RETURN.\n",
	"\n",
	"  Arguments del programa:\n",
	"\n",
	"       $ ./mur0 fitxer_config [retard]\n",
	"\n",
	"     El primer argument ha de ser el nom d\'un fitxer de text amb la\n",
	"     configuracio de la partida, on la primera fila inclou informacio\n",
	"     del camp de joc (valors enters), i la segona fila indica posicio\n",
	"     i velocitat de la pilota (valors reals):\n",
	"          num_files  num_columnes  mida_porteria\n",
    "          pos_fil_pal pelotas[0][3]ol_pal mida_pal\n"
	"          pos_fila   pelotas[0][3]olumna   pelotas[0][4]ila  pelotas[0][5]olumna\n",
	"\n",
	"     on els valors minims i maxims admesos son els seguents:\n",
	"          MIN_FIL <= num_files     <= MAX_FIL\n",
	"          MIN_COL <= num_columnes  <= MAX_COL\n",
	"          0 < mida_porteria < num_files-2\n",
	"        1.0 <= pos_fila     <= num_files-3\n",
	"        1.0 <= pelotas[0][3]olumna  <= num_columnes\n",
	"       -1.0 <= pelotas[0][4]ila     <= 1.0\n",
	"       -1.0 <= pelotas[0][5]olumna  <= 1.0\n",
	"\n",
	"     Alternativament, es pot donar el valor 0 a num_files i num_columnes\n",
	"     per especificar que es vol que el tauler ocupi tota la pantalla. Si\n",
	"     tambe fixem mida_porteria a 0, el programa ajustara la mida d\'aquesta\n",
	"     a 3/4 de l\'altura del camp de joc.\n",
	"\n",
	"     A mes, es pot afegir un segon argument opcional per indicar el\n",
	"     retard de moviment del joc en mil.lisegons; el valor minim es 10,\n",
	"     el valor maxim es 1000, i el valor per defecte d'aquest parametre\n",
	"     es 100 (1 decima de segon).\n",
	"\n",
	"  Codis de retorn:\n",
	"     El programa retorna algun dels seguents codis:\n",
	"	0  ==>  funcionament normal\n",
	"	1  ==>  numero d'arguments incorrecte\n",
	"	2  ==>  no s\'ha pogut obrir el fitxer de configuracio\n",
	"	3  ==>  algun parametre del fitxer de configuracio es erroni\n",
	"	4  ==>  no s\'ha pogut crear el camp de joc (no pot iniciar CURSES)\n",
	"\n",
	"   Per a que pugui funcionar aquest programa cal tenir instal.lada la\n",
	"   llibreria de CURSES (qualsevol versio).\n",
	"\n",
	"*"
};				/* final de la descripcio */

int n_fil, n_col;		/* numero de files i columnes del taulell */
int m_por;			/* mida de la porteria (en caracters) */
int f_pal, c_pal;		/* posicio del primer caracter de la paleta */
int m_pal;
float pelotas[MAX_PELOTAS][PARAMETROS_PELOTAS];
int nblocs = 0;
int dirPaleta = 0;
int retard;			/* valor del retard de moviment, en mil.lisegons */

int fi2 = 0;
int fi1 = 0;
int pfi2[MAX_PELOTAS];
int indexPilota = 0;
bool compr = false;
int numPilotes = 0; //pilotes actives que hi ha al joc
int numPilotesreal = 0; 
int lines;

pthread_t tid[MAX_THREADS];

bool comprova_num_pilota();



pthread_mutex_t mutex_varg = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pant = PTHREAD_MUTEX_INITIALIZER;

void llegeixPilotes(FILE * fit, const char * filename);
void conta_linees(const char *filename);


char strin[LONGMISS];			/* variable per a generar missatges de text */

/* funcio per carregar i interpretar el fitxer de configuracio de la partida */
/* el parametre ha de ser un punter a fitxer de text, posicionat al principi */
/* la funcio tanca el fitxer, i retorna diferent de zero si hi ha problemes  */
//int carrega_configuracio(FILE * fit)
int carrega_configuracio(FILE * fit, const char * filename)
{


	int ret = 0;

	fscanf(fit, "%d %d %d\n", &n_fil, &n_col, &m_por);	/* camp de joc */
	fscanf(fit, "%d %d %d\n", &f_pal, &c_pal, &m_pal);	/* paleta */
	fscanf(fit, "%f %f %f %f\n", &pelotas[0][2], &pelotas[0][3], &pelotas[0][4], &pelotas[0][5]);	/* pilota */
	llegeixPilotes(fit,filename);
	if ((n_fil != 0) || (n_col != 0)) {	/* si no dimensions maximes */
		if ((n_fil < MIN_FIL) || (n_fil > MAX_FIL) || (n_col < MIN_COL) || (n_col > MAX_COL))
			ret = 1;
		else if (m_por > n_col - 3)
			ret = 2;
        else if ((m_pal > n_col - 3) || (m_pal < 1) || (f_pal > n_fil-1) || (f_pal < 1) || (c_pal + m_pal > n_col -1 || c_pal < 1 ))
            ret = 3;
		else if ((pelotas[0][2] < 1) || (pelotas[0][2] >= n_fil - 3) || (pelotas[0][3] < 1)
			 || (pelotas[0][3] > n_col - 1))	/* tres files especials: línia d'estat, porteria i paleta */
			ret = 4;
	}
	if ((pelotas[0][4] < -1.0) || (pelotas[0][4] > 1.0) || (pelotas[0][5] < -1.0) || (pelotas[0][5] > 1.0))
		ret = 5;

	if (ret != 0) {		/* si ha detectat algun error */
		fprintf(stderr, "Error en fitxer de configuracio:\n");
		switch (ret) {
		case 1:
			fprintf(stderr,
				"\tdimensions del camp de joc incorrectes:\n");
			fprintf(stderr, "\tn_fil= %d \tn_col= %d\n", n_fil,
				n_col);
			break;
		case 2:
			fprintf(stderr, "\tmida de la porteria incorrecta:\n");
			fprintf(stderr, "\tm_por= %d\n", m_por);
			break;
        case 3:
            fprintf(stderr,"\tmida de la paleta incorrecta:\n");
            fprintf(stderr, "\tf_pal= %d \tc_pal= %d \t m_pal=%d\n", f_pal,c_pal,m_pal);
            break;
		case 4:
			fprintf(stderr, "\tposicio de la pilota incorrecta:\n");
			fprintf(stderr, "\tpos_f= %.2f \tpelotas[0][3]= %.2f\n", pelotas[0][2],
				pelotas[0][3]);
			break;
		case 5:
			fprintf(stderr,
				"\tvelocitat de la pilota incorrecta:\n");
			fprintf(stderr, "\tpelotas[0][4]= %.2f \tpelotas[0][5]= %.2f\n", pelotas[0][4],
				pelotas[0][5]);
			break;
		}
	}
	fclose(fit);
	return (ret);
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
/* retorna diferent de zero si hi ha algun problema */
int inicialitza_joc(void)
{
	int i, retwin;
	int i_port, f_port;	/* inici i final de porteria */
	int c, nb, offset;
	
	

	retwin = win_ini(&n_fil, &n_col, '+', INVERS);	/* intenta crear taulell */

	if (retwin < 0) {	/* si no pot crear l'entorn de joc amb les curses */
		fprintf(stderr, "Error en la creacio del taulell de joc:\t");
		switch (retwin) {
		case -1:
			fprintf(stderr, "camp de joc ja creat!\n");
			break;
		case -2:
			fprintf(stderr,
				"no s'ha pogut inicialitzar l'entorn de curses!\n");
			break;
		case -3:
			fprintf(stderr,
				"les mides del camp demanades son massa grans!\n");
			break;
		case -4:
			fprintf(stderr, "no s'ha pogut crear la finestra!\n");
			break;
		}
		return (retwin);
	}

	if (m_por > n_col - 2)
		m_por = n_col - 2;	/* limita valor de la porteria */
	if (m_por == 0)
		m_por = 3 * (n_col - 2) / 4;	/* valor porteria per defecte */

	i_port = n_col / 2 - m_por / 2 - 1;	/* crea el forat de la porteria */
	f_port = i_port + m_por - 1;
	for (i = i_port; i <= f_port; i++)
		win_escricar(n_fil - 2, i, ' ', NO_INV);

	n_fil = n_fil - 1;	/* descompta la fila de missatges */

	for (i = 0; i < m_pal; i++)	/* dibuixar paleta inicial */
		win_escricar(f_pal, c_pal + i, '0', INVERS);

	/* generar la pilota */
	if (pelotas[0][2] > n_fil - 1)
		pelotas[0][2] = n_fil - 1;	/* limita posicio inicial de la pilota */
	if (pelotas[0][3] > n_col - 1)
		pelotas[0][3] = n_col - 1;
	pelotas[0][0] = pelotas[0][2];
	pelotas[0][1] = pelotas[0][3];		/* dibuixar la pilota inicialment */
	win_escricar(pelotas[0][0], pelotas[0][1], '1', INVERS);

	// generar els blocs 
	nb = 0;
	nblocs = n_col / (BLKSIZE + BLKGAP) - 1;
	offset = (n_col - nblocs * (BLKSIZE + BLKGAP) + BLKGAP) / 2;
	for (i = 0; i < nblocs; i++) {
		for (c = 0; c < BLKSIZE; c++) {
			win_escricar(3, offset + c, FRNTCHAR, INVERS);
			nb++;
			win_escricar(4, offset + c, BLKCHAR, NO_INV);
			nb++;
			win_escricar(5, offset + c, FRNTCHAR, INVERS);
			nb++;
		}
		offset += BLKSIZE + BLKGAP;
	}
	nblocs = nb / BLKSIZE;
	// generar les defenses
	nb = n_col / (BLKSIZE + 2 * BLKGAP) - 2;
	offset = (n_col - nb * (BLKSIZE + 2 * BLKGAP) + BLKGAP) / 2;
	for (i = 0; i < nb; i++) {
		for (c = 0; c < BLKSIZE + BLKGAP; c++) {
			win_escricar(6, offset + c, WLLCHAR, NO_INV);
		}
		offset += BLKSIZE + 2 * BLKGAP;
	}

	sprintf(strin,
		"Tecles: \'%c\'-> Esquerra, \'%c\'-> Dreta, RETURN-> sortir\n",
		TEC_ESQUER, TEC_DRETA);
	win_escristr(strin);
	return (0);
}

/* funcio que escriu un missatge a la línia d'estat i tanca les curses */
void mostra_final(char *miss)
{	int lmarge;
	char marge[LONGMISS];

	/* centrar el misssatge */
	lmarge=(n_col+strlen(miss))/2;
	sprintf(marge,"%%%ds",lmarge);

	sprintf(strin, marge,miss);
	win_escristr(strin);

	/* espera tecla per a que es pugui veure el missatge */
	getchar();
}

/* Si hi ha una col.lisió pilota-bloci esborra el bloc */
void * mou_pilota(void * index);
void comprovar_bloc(int f, int c)
{
	int col;
	pthread_mutex_lock(&mutex_pant);
	char quin = win_quincar(f, c);
	pthread_mutex_unlock(&mutex_pant);

	if (quin == BLKCHAR || quin == FRNTCHAR) {
		col = c;
		pthread_mutex_lock(&mutex_pant);
		while (win_quincar(f, col) != ' ') {
		
			win_escricar(f, col, ' ', NO_INV);
			
			col++;
		}
		pthread_mutex_unlock(&mutex_pant);
		col = c - 1;
		pthread_mutex_lock(&mutex_pant);
		while (win_quincar(f, col) != ' ') {
			win_escricar(f, col, ' ', NO_INV);
			col--;
		}
		pthread_mutex_unlock(&mutex_pant);
		if (quin == BLKCHAR && numPilotesreal <= lines){
			pthread_mutex_lock(&mutex_varg);
			indexPilota++; //incrementem el index de la pilota
			pthread_mutex_unlock(&mutex_varg);
			pthread_mutex_lock(&mutex_varg);
			pthread_create(&tid[indexPilota+1],NULL,mou_pilota,(void *)(intptr_t) indexPilota);
			numPilotes++; //incrementem el numero de pilotes
			numPilotesreal++;
			pthread_mutex_unlock(&mutex_varg);
	
		/* generar nova pilota ? */
		}
		
        	pthread_mutex_lock(&mutex_varg);
		nblocs--;
		pthread_mutex_unlock(&mutex_varg);
	}
}

/* funcio per a calcular rudimentariament els efectes amb la pala */
/* no te en compta si el moviment de la paleta no és recent */
/* cal tenir en compta que després es calcula el rebot */
void control_impacte(void) {
	if (dirPaleta == TEC_DRETA) {
		if (pelotas[0][5] <= 0.0){	/* pilota cap a l'esquerra */
			pthread_mutex_lock(&mutex_varg);
			pelotas[0][5] = -pelotas[0][5] - 0.2;	/* xoc: canvi de sentit i reduir velocitat */
			pthread_mutex_unlock(&mutex_varg);
			}
		else {	/* a favor: incrementar velocitat */
			if (pelotas[0][5] <= 0.8){
				pthread_mutex_lock(&mutex_varg);
				pelotas[0][5] += 0.2;
				pthread_mutex_unlock(&mutex_varg);
				}
		}
	} else {
		if (dirPaleta == TEC_ESQUER) {
			if (pelotas[0][5] >= 0.0){	/* pilota cap a la dreta */
				pthread_mutex_lock(&mutex_varg);
				pelotas[0][5] = -pelotas[0][5] + 0.2;	/* xoc: canvi de sentit i reduir la velocitat */
				pthread_mutex_unlock(&mutex_varg);
				}
			else {	/* a favor: incrementar velocitat */
				if (pelotas[0][5] >= -0.8){
					pthread_mutex_lock(&mutex_varg);
					pelotas[0][5] -= 0.2;
					pthread_mutex_unlock(&mutex_varg);
					}
			}
		} else {	/* XXX trucs no documentats */
			if (dirPaleta == TEC_AMUNT){
				pthread_mutex_lock(&mutex_varg);
				pelotas[0][5] = 0.0;	/* vertical */
				pthread_mutex_unlock(&mutex_varg);
				}
			else {
				if (dirPaleta == TEC_AVALL)
					if (pelotas[0][4] <= 1.0){
						pthread_mutex_lock(&mutex_varg);
						pelotas[0][4] -= 0.2;	/* frenar */
						pthread_mutex_unlock(&mutex_varg);
						}
			}
		}
	}
	pthread_mutex_lock(&mutex_varg);
	dirPaleta=0;	/* reset perque ja hem aplicat l'efecte */
	pthread_mutex_unlock(&mutex_varg);
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

/* funcio per moure la pilota: retorna un 1 si la pilota surt per la porteria,*/
/* altrament retorna un 0 */
void * mou_pilota(void * index)
{	
	int f_h, c_h;
	char rh, rv, rd;
	
	int index2=(intptr_t) index;
	pthread_mutex_lock(&mutex_varg);
	pfi2[index2] = 0; //inicialitzem
	pthread_mutex_unlock(&mutex_varg);

  do{
  
    win_retard(retard);
    
	f_h = pelotas[index2][2] + pelotas[index2][4];	/* posicio hipotetica de la pilota (entera) */
	c_h = pelotas[index2][3] + pelotas[index2][5];
	rh = rv = rd = ' ';
	if ((f_h != pelotas[index2][0]) || (c_h != pelotas[index2][1])) {
	/* si posicio hipotetica no coincideix amb la posicio actual */
		if (f_h != pelotas[index2][0]) {	/* provar rebot vertical */
			pthread_mutex_lock(&mutex_pant);
			rv = win_quincar(f_h, pelotas[index2][1]);	/* veure si hi ha algun obstacle */
			pthread_mutex_unlock(&mutex_pant);
			if (rv != ' ') {	/* si hi ha alguna cosa */
				comprovar_bloc(f_h, pelotas[index2][1]);
				if (rv == '0'){	/* col.lisió amb la paleta? */
//					
					pthread_mutex_lock(&mutex_varg);	
					pelotas[index2][5] = control_impacte2(pelotas[index2][1], pelotas[index2][5]);
					pthread_mutex_unlock(&mutex_varg);
					}
					
				pthread_mutex_lock(&mutex_varg);	
				pelotas[index2][4] = -pelotas[index2][4];	/* canvia sentit velocitat vertical */
				pthread_mutex_unlock(&mutex_varg);
				f_h = pelotas[index2][2] + pelotas[index2][4];	/* actualitza posicio hipotetica */
			}
		}
		if (c_h != pelotas[index2][1]) {	/* provar rebot horitzontal */
			pthread_mutex_lock(&mutex_pant);
			rh = win_quincar(pelotas[index2][0], c_h);	/* veure si hi ha algun obstacle */
			pthread_mutex_unlock(&mutex_pant);
			if (rh != ' ') {	/* si hi ha algun obstacle */
				comprovar_bloc(pelotas[index2][0], c_h);
				/* TODO?: tractar la col.lisio lateral amb la paleta */
				pthread_mutex_lock(&mutex_varg);
				pelotas[index2][5] = -pelotas[index2][5];	/* canvia sentit vel. horitzontal */
				pthread_mutex_unlock(&mutex_varg);
				c_h = pelotas[index2][3] + pelotas[index2][5];	/* actualitza posicio hipotetica */
			}
		}
		if ((f_h != pelotas[index2][0]) && (c_h != pelotas[index2][1])) {	/* provar rebot diagonal */
			pthread_mutex_lock(&mutex_pant);
			rd = win_quincar(f_h, c_h);
			pthread_mutex_unlock(&mutex_pant);
			if (rd != ' ') {	/* si hi ha obstacle */
				comprovar_bloc(f_h, c_h);
				pthread_mutex_lock(&mutex_varg);
				pelotas[index2][4] = -pelotas[index2][4];
				pthread_mutex_unlock(&mutex_varg);
				pthread_mutex_lock(&mutex_varg);
				pelotas[index2][5] = -pelotas[index2][5];	/* canvia sentit velocitats */
				pthread_mutex_unlock(&mutex_varg);
				f_h = pelotas[index2][2] + pelotas[index2][4];
				c_h = pelotas[index2][3] + pelotas[index2][5];	/* actualitza posicio entera */
			}
		}
		/* mostrar la pilota a la nova posició */
		pthread_mutex_lock(&mutex_pant);
		if (win_quincar(f_h, c_h) == ' ') {	/* verificar posicio definitiva *//* si no hi ha obstacle */
			pthread_mutex_unlock(&mutex_pant);
			pthread_mutex_lock(&mutex_pant);
			win_escricar(pelotas[index2][0], pelotas[index2][1], ' ', NO_INV);	/* esborra pilota */
			pthread_mutex_unlock(&mutex_pant);
			pthread_mutex_lock(&mutex_varg);
			pelotas[index2][2] += pelotas[index2][4];
			pthread_mutex_unlock(&mutex_varg);
			pthread_mutex_lock(&mutex_varg);
			pelotas[index2][3] += pelotas[index2][5];
			pthread_mutex_unlock(&mutex_varg);
			pthread_mutex_lock(&mutex_varg);
			pelotas[index2][0] = f_h;
			pthread_mutex_unlock(&mutex_varg);
			pthread_mutex_lock(&mutex_varg);
			pelotas[index2][1] = c_h;	/* actualitza posicio actual */
			pthread_mutex_unlock(&mutex_varg);
			if (pelotas[index2][0] != n_fil - 1){	/* si no surt del taulell, */
				pthread_mutex_lock(&mutex_pant);
				win_escricar(pelotas[index2][0], pelotas[index2][1], '1', INVERS);	/* imprimeix pilota */
				pthread_mutex_unlock(&mutex_pant);
				}
			else{
				pthread_mutex_lock(&mutex_varg);
				pfi2[index2] = 1; //surt fora del camp el fiquem a 1
				pthread_mutex_unlock(&mutex_varg);
				
				pthread_mutex_lock(&mutex_varg);
				numPilotes--; //decrementem el numero de pilotes actives
				pthread_mutex_unlock(&mutex_varg);
				}
		}
		pthread_mutex_unlock(&mutex_pant);
	} else {	/* posicio hipotetica = a la real: moure */
		pthread_mutex_lock(&mutex_varg);
		pelotas[index2][2] += pelotas[index2][4];
		pthread_mutex_unlock(&mutex_varg);
		pthread_mutex_lock(&mutex_varg);
		pelotas[index2][3] += pelotas[index2][5];
		pthread_mutex_unlock(&mutex_varg);
	}

	
	  } while (!fi1 && !pfi2[index2]); 
  pthread_exit(0);
}



/* funcio per moure la paleta segons la tecla premuda */
/* retorna un boolea indicant si l'usuari vol acabar */
void * mou_paleta(void * nul)
{
	int tecla;
  do{
   win_retard(retard);
   pthread_mutex_lock(&mutex_varg);
	tecla = win_gettec();
	pthread_mutex_unlock(&mutex_varg);
	
	if (tecla != 0) {
		if ((tecla == TEC_DRETA)
			&& ((c_pal + m_pal) < n_col - 1)) {
				pthread_mutex_lock(&mutex_pant);
				win_escricar(f_pal, c_pal, ' ', NO_INV);	/* esborra primer bloc */
				pthread_mutex_unlock(&mutex_pant);
				pthread_mutex_lock(&mutex_varg);
				c_pal++;	/* actualitza posicio */
				pthread_mutex_unlock(&mutex_varg);
				pthread_mutex_lock(&mutex_pant);
				win_escricar(f_pal, c_pal + m_pal - 1, '0', INVERS);	/*esc. ultim bloc */
				pthread_mutex_unlock(&mutex_pant);
		}
		if ((tecla == TEC_ESQUER) && (c_pal > 1)) {
				pthread_mutex_lock(&mutex_pant);
				win_escricar(f_pal, c_pal + m_pal - 1, ' ', NO_INV);	/*esborra ultim bloc */
				pthread_mutex_unlock(&mutex_pant);
				pthread_mutex_lock(&mutex_varg);
				c_pal--;	/* actualitza posicio */
				pthread_mutex_unlock(&mutex_varg);
				pthread_mutex_lock(&mutex_pant);
				win_escricar(f_pal, c_pal, '0', INVERS);	/* escriure primer bloc */
				pthread_mutex_unlock(&mutex_pant);
		}
		if (tecla == TEC_RETURN){
			pthread_mutex_lock(&mutex_varg);
			fi1 = 1;	/* final per pulsacio RETURN */
			pthread_mutex_unlock(&mutex_varg);
			}
		pthread_mutex_lock(&mutex_varg);
		dirPaleta = tecla;	/* per a afectar al moviment de les pilotes */
		pthread_mutex_unlock(&mutex_varg);
	}

	
	
	  } while(!fi1);
  pthread_exit(0);
}


/* programa principal */
int main(int n_args, char *ll_args[])
{
	int i;
	int min, seg;
  time_t inicio, fin;
  char buffer[50];
  char buffer2[20];

	FILE *fit_conf;

	if ((n_args != 2) && (n_args != 3)) {	/* si numero d'arguments incorrecte */
		i = 0;
		do
			fprintf(stderr, "%s", descripcio[i++]);	/* imprimeix descripcio */
		while (descripcio[i][0] != '*');	/* mentre no arribi al final */
		exit(1);
	}

	fit_conf = fopen(ll_args[1], "rt");	/* intenta obrir el fitxer */
	if (!fit_conf) {
		fprintf(stderr, "Error: no s'ha pogut obrir el fitxer \'%s\'\n",
			ll_args[1]);
		exit(2);
	}

	if (carrega_configuracio(fit_conf,ll_args[1]) != 0)	/* llegir dades del fitxer  */
		exit(3);	/* aborta si hi ha algun problema en el fitxer */

	if (n_args == 3) {	/* si s'ha especificat parametre de retard */
		retard = atoi(ll_args[2]);	/* convertir-lo a enter */
		if (retard < 10)
			retard = 10;	/* verificar limits */
		if (retard > 1000)
			retard = 1000;
	} else
		retard = 100;	/* altrament, fixar retard per defecte */

	printf("Joc del Mur: prem RETURN per continuar:\n");
	getchar();

	if (inicialitza_joc() != 0){	/* intenta crear el taulell de joc */
		exit(4);	/* aborta si hi ha algun problema amb taulell */
		}
 // Open the file

  // Line counter (result)
	conta_linees(ll_args[1]);

 
pthread_mutex_init(&mutex_varg, NULL);
pthread_mutex_init(&mutex_pant, NULL);
pthread_create(&tid[0],NULL,mou_paleta, NULL);		/* creacio thread paleta usuari */
pthread_create(&tid[1],NULL,mou_pilota,(void *)(intptr_t) 0);		/* creacio thread pilota usuari */
numPilotes++;
numPilotesreal++;
inicio=time(NULL);
	do {
/********** bucle principal del joc **********/

		fin=time(NULL);
    seg=difftime(fin,inicio);
    min=seg/60;
    seg=seg%60;
    sprintf(buffer,"TIME: %2d:%2d", min, seg);
    pthread_mutex_lock(&mutex_pant);
    win_escristr(buffer);
    pthread_mutex_unlock(&mutex_pant);
	} while (!fi1 && numPilotes > 0);
	
	if (nblocs == 0){
	
		strcat(strcpy(buffer2, "YOU WIN !:::TOTAL "), buffer);
		mostra_final(buffer2);
		
		}
	else {
		
		strcat(strcpy(buffer2, "GAME OVER:::TOTAL "), buffer);
		mostra_final(buffer2);
	
		}

    for(i = 0; i < (indexPilota + 2);i++){
		pthread_join(tid[i],NULL);	
	}
  pthread_mutex_destroy(&mutex_varg);	
    pthread_mutex_destroy(&mutex_pant);
	win_fi();		/* tanca les curses */

	return (0);		/* retorna sense errors d'execucio */
}


void llegeixPilotes(FILE * fit,const char * filename){

	conta_linees(filename);

	for(int i = 1; i <= lines;i++){
		fscanf(fit, "%f %f %f %f\n", &pelotas[i][2], &pelotas[i][3], &pelotas[i][4], &pelotas[i][5]);
	}


	
}


void conta_linees(const char *filename){

	FILE*fp;
	
	fp = fopen(filename, "rt");
    	char c;
  
    
   	 for (c = getc(fp); c != EOF; c = getc(fp))
       		 if (c == '\n')
            		lines = lines + 1;
  
	lines=lines-3;
    	fclose(fp);

}








