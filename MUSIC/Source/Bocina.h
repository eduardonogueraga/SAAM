/*
 * Bocina.h
 *
 *  Created on: 26 jun. 2020
 *      Author: isrev
 */

#ifndef PROYECTO_BOCINA_H_
#define PROYECTO_BOCINA_H_

#include "Arduino.h"
#include "Macros.h"
//#include <Adafruit_MCP23X17.h>
//extern Adafruit_MCP23X17 mcp;

#include "RecursosCompartidosRTOS.h"
extern RecursosCompartidosRTOS rcomp1;


class Bocina {

private:

	unsigned short bocinaIntervalo = 5000;
    unsigned long bocinaTiempoAnt = 0;

public:
	Bocina();
	void sonarBocina(unsigned short bocinaIntervalo = 5000);
	void stopBocina();
	void bocinaAlert();
};

#endif /* PROYECTO_BOCINA_H_ */
