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

extern ConfigSystem configSystem;
extern byte SD_STATUS;

#define sck 14
#define miso 2
#define mosi 15
#define ss 13

class Registro {
private:
	File root;
	String saaLogs = "/sys_logs";
	char rutaAbosuluta[40];
public:
	Registro();
	byte iniciar();
	void registrarLogSistema(char descripcion[190]);
};

#endif /* SOURCE_REGISTRO_H_ */
