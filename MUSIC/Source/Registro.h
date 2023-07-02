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
#include <ArduinoJson.h>


extern ConfigSystem configSystem;
extern byte SD_STATUS;



extern Fecha fecha;

class Registro {
private:
	File root;
	const char* directories[6] = {"/saa",
									"/saa/sys",
									"/saa/http",
									"/saa/sys/log",
									"/saa/sys/json",
									"/saa/http/log"};
	char rutaAbosuluta[60];
	char rutaAbosulutaTemporal[60];
	char nombreFicheroLog[40];
	char nombreFicheroJsonRequest[25] = "TEMP_JSON_RESQUEST.txt";
	RegistroDirectorios registroDirectorios;
public:
	Registro();
	byte iniciar();
	void registrarLogSistema(char descripcion[190]);
	void mostrarRegistro(RegistroDirectorios dir = DIR_LOGS);
	void listarRegistros(RegistroDirectorios dir = DIR_LOGS);
	void borrarRegistros(RegistroDirectorios dir = DIR_LOGS);
	byte exportarEventosJson(StaticJsonDocument<MAX_SIZE_JSON>* json);
	String extraerPrimerElemento(RegistroDirectorios dir = DIR_JSON_REQUEST);
};

#endif /* SOURCE_REGISTRO_H_ */
