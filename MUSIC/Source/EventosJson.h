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
#include <Preferences.h>
#include "Fecha.h"
#include "Macros.h"



extern Registro registro;
extern Fecha fecha;
extern Preferences NVSMemory;


extern byte MODO_DEFAULT;
extern byte INTENTOS_REACTIVACION;
extern EstadosAlarma estadoAlarma;

extern ConfigSystem configSystem;

extern RespuestaHttp postPaqueteSaas(String* modeloJson);
extern int getIdPaqueteSaas();
extern int generarTokenSaas();

class EventosJson {
private:
	StaticJsonDocument<MAX_SIZE_JSON> JSON_DOC;
	String SALIDA_JSON;
	void guardarJsonNVS(StaticJsonDocument<MAX_SIZE_JSON>& jsonDoc);
	byte cargarJsonNVS(StaticJsonDocument<MAX_SIZE_JSON>& jsonDoc);
	String asignarIdPaquete();
	SAAS_GESTION_ENVIO_R gestionarEnvioPaquete(String* modeloJson);

	SAAS_GESTION_ENVIO gestionPaquete;


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
	void cargarJsonNVS();
	void actualizarCabecera();
	byte enviarInformeSaas();
	String* getSalidaJsonPointer() { //@TEST ONLY

		SALIDA_JSON.clear();
		serializeJsonPretty(JSON_DOC, SALIDA_JSON);

		return &SALIDA_JSON;
	}

};

#endif /* SOURCE_EVENTOSJSON_H_ */
