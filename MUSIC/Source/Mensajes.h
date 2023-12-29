/*
 * Mensajes.h
 *
 *  Created on: 23 jun. 2020
 *      Author: isrev
 *
 */

#ifndef PROYECTO_MENSAJES_H_
#define PROYECTO_MENSAJES_H_

#include "Arduino.h"
#include <HardwareSerial.h>
#include "EventosJson.h"
#include "Registro.h"
#include "Terminal.h"

#include "AlarmaUtil/Macros.h"
#include "Env.h"
#include "Fecha.h"
#include <Adafruit_MCP23X17.h>

extern EventosJson eventosJson;
extern Adafruit_MCP23X17 mcp;
extern byte INTENTOS_REACTIVACION;
extern byte MODO_DEFAULT;
extern byte mensajesEnviados;
extern ConfigSystem configSystem;
extern Fecha fecha;
extern CODIGO_ERROR codigoError;
extern byte flagPuertaAbierta;
extern const char* literalesZonas[2][MAX_DATOS_SUB_TRAMA];

extern RespuestaTerminal respuestaTerminal;
extern Terminal* T_LIST[];

extern void colgarLlamadaAPI();
extern void llamarTlfAPI(char* tlf);


class Mensajes {

private:

	HardwareSerial& UART_GSM;

	byte MAX_SALTO [4] = {1,1,2,2};
	byte cont = 0;

	byte tipoMensaje;
	String asuntoMensaje = "";
	String cuerpoMensaje = "";
	String pieMensaje = "";
	const byte LIMITE_MAXIMO_SMS = 15;
	void procesarSMS();
	void asuntoAlerta();
	void limpiarContenido();

	SAAS_LITERAL_NOTIFICACIONES saasNotificaciones;
	SAAS_LITERAL_NOTIFICACIONES_TLF saaNotificacionesTlf;
	byte literalAsuntoSaas;


	SAAS_LITERAL_NOTIFICACIONES nombreZonasSaas[4] = {
	  AVISO_ALARMA_MOVIMIENTO_COCHERA,
	  AVISO_ALARMA_MOVIMIENTO_PORCHE,
	  AVISO_ALARMA_MOVIMIENTO_ALMACEN,
	  AVISO_ALARMA_PUERTA_COCHERA_ABIERTA
	};

public:
	Mensajes(HardwareSerial& serialInstance);
	void inicioGSM();
	void enviarSMS();
	void enviarSMSEmergencia();
	void mensajeAlerta();
	void mensajeReactivacion();
	void mensajeError();
	void llamarTlf(char* tlf);
	void colgarLlamada();
	void pieFechaBateria();
	const String& getAsuntoMensaje() const;
	const String& getCuerpoMensaje() const;
	byte getTipoMensaje() const;
	 char* getFullSMS();

};

#endif /* PROYECTO_MENSAJES_H_ */
