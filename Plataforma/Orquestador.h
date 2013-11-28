/*
 * orquestador.h
 *
 *  Created on: 27/10/2013
 *      Author: utnso
 */

#ifndef ORQUESTADOR_H_
#define ORQUESTADOR_H_

#include "Plataforma.h"
#include <wait.h>
#include <signal.h>

/* Nodos de la lista Ganadores */
typedef struct{
	char personaje;
}jugadorGanador;

/* Nodos de la lista de planificadores */
typedef struct{
	pthread_t idHilo;
}nodoPlanificador;

/* Registro que necesito el orquestador
 * al ser llamado por la plataforma
 */typedef struct tinfoOrquestador{
	 t_list *listaNiveles;
	 char * cfg;
 }infoOrquestador;

 /* Registro del archivo del orquestador
  */typedef struct{
	 char * ip;
	 char * koopa;
	 char * script;
	 int puerto;
 }cfgOrquestador;

void cerrarTodo(int);
void borrarTodoNivel(void*);
void finalizarTodo(t_list*,t_list*,t_list*,int);
void *orquestador(void* info);
void  nivelNuevo(handshake handshakeNivel,  int socketNivel, t_list* hilosPlanificadores);
void clienteNuevo(handshake handshakeJugador,int socketJugador);
void clienteViejo(handshake handshakeJugador, t_list *ganadores);
void responderError(int socketJugador);
nodoNivel *validarNivel(char nombreNivel[13]);
nodoNivel *buscarNivelEnSistema(char nombreNivel[13]);
void crearTanda(nuevo** lista);
void reconectarNivel(nodoNivel *nodo,int nid);
void agregarNivel(handshake handshakeNivel,int socketNivel, t_list* hilosPlanificadores);
bool _hay_jugadores(nodoNivel *nivel);
bool chequearKoopa(t_list *ganadores);
void crearHiloPlanificador(nodoNivel *nivel,t_list* hilosPlanificadores);
int _matar_hilo(nodoPlanificador *planificador);
void matarHilos(t_list* hilosPlanificadores);
void activarKoopa(t_list* hilosPlanificadores, char * koopa, char * script);
int selectGRID_orquestador(int fdmax, fd_set original, int tiempo);
void koopaWarning(int fdmax, fd_set original, t_list *hilosPlanificadores,t_list *ganadores, char * koopa, char * script);

cfgOrquestador cargarOrquestador(char *path);
int puertoPlataforma( t_config * cfgPlataforma);
char * pathKoopaPlataforma( t_config * cfgPlataforma);
char * pathScriptPlataforma( t_config * cfgPlataforma);
char * ipPlataforma( t_config * cfgPlataforma);

#endif /* ORQUESTADOR_H_ */
