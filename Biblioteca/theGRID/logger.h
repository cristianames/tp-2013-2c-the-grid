/*
 * logger.h
 *
 *  Created on: 02/05/2013
 *      Author: utnso
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#define DIRECTORIO_LOG_TRACE "/home/utnso/GRIDLogs/errorTrace.txt"
#define DIRECTORIO_LOG_DEBUG "/home/utnso/GRIDLogs/errorDebug.txt"
#define DIRECTORIO_LOG_WARNING "/home/utnso/GRIDLogs/errorWarning.txt"
#define DIRECTORIO_LOG_INFO "/home/utnso/GRIDLogs/errorInfo.txt"
#define DIRECTORIO_LOG_ERROR "/home/utnso/GRIDLogs/errorFatalityError.txt"



void newArchLogTrace(char* programa);
void newArchLogDebug(char* programa);
void newArchLogWarning(char* programa);
void newArchLogInfo(char* programa);
void newArchLogError(char* programa);
void loguearTrace (char* mensaje);
void loguearDebug (char* mensaje);
void loguearInfo (char* mensaje);
void loguearWarning (char* mensaje);
void loguearError (char* mensaje);
void finLogTrace ();
void finLogDebug ();
void finLogWarning ();
void finLogError ();
void finLogInfo ();
void cerrarLogs();






#endif /* LOGGER_H_ */
