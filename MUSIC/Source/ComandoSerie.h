/*
 * ComandoSerie.h
 *
 *  Created on: 29 jul. 2021
 *      Author: isrev
 */

#ifndef SOURCE_COMANDOSERIE_H_
#define SOURCE_COMANDOSERIE_H_
#include "Arduino.h"
#include "InterStrike.h"
#include "Registro.h"
#include "Macros.h"
#include "Datos.h"
#include "EventosJson.h"

extern void setEstadoGuardia();
extern void setEstadoReposo();
extern void setEstadoEnvio();
extern void resetear();
extern void desactivarAlarma();

extern byte MODO_DEFAULT;
extern ProcesoCentral procesoCentral;
extern ConfigSystem configSystem;
extern InterStrike pir1, pir2, pir3, mg;
extern Datos datosSensores;
extern EventosJson eventosJson;



extern Registro registro;

extern byte sensorHabilitado[];

extern void interrupcionFalloAlimentacion();

class ComandoSerie {

private:
	String data ="";
	void comprobarComando();

public:
	ComandoSerie();
	void demonioSerie();
};

#endif /* SOURCE_COMANDOSERIE_H_ */
