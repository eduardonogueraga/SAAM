/*
 * EventosJson.h
 *
 *  Created on: 25 jun. 2023
 *      Author: Eduardo
 */

#ifndef SOURCE_EVENTOSJSON_H_
#define SOURCE_EVENTOSJSON_H_

#include "Arduino.h"
#include "Registro.h"
#include <ArduinoJson.h>
#include "Fecha.h"



extern Registro registro;
extern Fecha fecha;


extern byte MODO_DEFAULT;
extern byte INTENTOS_REACTIVACION;
extern EstadosAlarma estadoAlarma;

extern ConfigSystem configSystem;

class EventosJson {
private:
	StaticJsonDocument<MAX_SIZE_JSON> JSON_DOC;
	String SALIDA_JSON;
public:
	EventosJson();
	void iniciarModeloJSON();
	JsonObject crearNuevaEntrada(char reg[]);
	void guardarEvento(char eventName[],char reg[]);
	StaticJsonDocument<MAX_SIZE_JSON> crearNuevoModeloJson();
	void componerJSON();
	void purgarModeloJSON();
	void mostrarModeloJSON();

	void guardarEntrada();
	void guardarDeteccion(byte strikes, byte umbral, byte modo, byte id, byte estado,  byte valor = 1);
	void guardarNotificacion(byte tipo, byte asunto , char cuerpo[],byte tlf);
	void guardarLog(byte id);

	void comprobarMemoriaDisponible();
	void exportarFichero();
	void guardarJsonNVS();
};

#endif /* SOURCE_EVENTOSJSON_H_ */
