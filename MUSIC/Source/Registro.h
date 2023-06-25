/*
 * Registro.h
 *
 *  Created on: 3 abr. 2023
 *      Author: Eduardo
 */

#ifndef SOURCE_REGISTRO_H_
#define SOURCE_REGISTRO_H_

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include <fcntl.h>
#include "Macros.h"
#include "Fecha.h"

extern ConfigSystem configSystem;
extern byte SD_STATUS;

#define sck 14
#define miso 2
#define mosi 15
#define ss 13

extern Fecha fecha;

class Registro {
private:
	File root;
	const char sysLog[15] = "/saa/sys/log";
	const char httpLog[15] = "/saa/http/log";
	const char* directories[5] = {"/saa", "/saa/sys","/saa/http", "/saa/sys/log", "/saa/http/log"};
	char rutaAbosuluta[60];
	char nombreFichero[40];
public:
	Registro();
	byte iniciar();
	void registrarLogSistema(char descripcion[190]);
	void mostrarRegistro();
	void listarRegistros();
	void borrarRegistros();
};

#endif /* SOURCE_REGISTRO_H_ */
