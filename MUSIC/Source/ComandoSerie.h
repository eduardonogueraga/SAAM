/*
 * ComandoSerie.h
 *
 *  Created on: 29 jul. 2021
 *      Author: isrev
 */

#ifndef SOURCE_COMANDOSERIE_H_
#define SOURCE_COMANDOSERIE_H_
#include "Arduino.h"
#include "Registro.h"
#include "AlarmaUtil/Macros.h"
#include "EventosJson.h"
#include <HardwareSerial.h>
#include "ComunicacionLinea.h"
#include "Terminal.h"
#include "Env.h"


extern void setEstadoGuardia();
extern void setEstadoReposo();
extern void setEstadoEnvio();
extern void setEstadoInquieto();
extern void resetear();
extern void desactivarAlarma();

extern byte MODO_DEFAULT;
extern ProcesoCentral procesoCentral;
extern ConfigSystem configSystem;
extern EventosJson eventosJson;
extern HardwareSerial UART_RS;
extern HardwareSerial UART_GSM;
extern ComunicacionLinea linea;

extern Terminal T_COCHERA;
extern Terminal T_CORE;

extern Registro registro;

extern byte sensorHabilitado[];

extern void interrupcionFalloAlimentacion();
extern int testHttpRequest();
extern int getIdPaqueteSaas();
extern void refrescarModuloGSM();
extern byte enviarNotificacionesSaas(byte tipo,  const char* contenido);

extern TaskHandle_t envioNotificacionSaas;
extern void tareaNotificacionSaas(void *pvParameters);
extern NotificacionSaas datosNotificacionSaas;
extern void crearTareaNotificacionSaas(byte tipo, const char* contenido);

extern void testTaskNodos();
extern void testTaskNodos2();
extern void testTaskNodosDelete();
extern void gestionarPilaDeTareas();
extern void testTaskNodosRecorrer();
extern void testTaskNodosMover();

extern void testTaskNodosTimeout();
extern void testTaskNodosRecuperarProcesable();

extern void rehabilitarEjecucionPila();

extern InterStrikeCore sensorCore;
extern void guardarEstadoAlerta();
extern void testEnvioFtp();
extern void pruebaCifrado();
extern bool establecerConexionGPRS();
extern void cerrarConexionGPRS();
extern String pwdFtp();
extern bool enviarFicheroFtp(const char* nombreArchivo, int bytes, const char* buffer);

extern bool iniciarServicioFtp();
extern bool abrirSesionFtp();
extern bool cambiarDirectorioTrabajoFtp();
extern bool cerrarConexionFtp();


class ComandoSerie {

private:
	String comandoRecibido;
	char data[100];
	void comprobarComando();
	boolean compararCadena(const char* data, const char* cadena);
	void nombreComando(const char* data);
	void esperarRespuestaUart2();
	void mostrarAyuda();

public:
	ComandoSerie();
	void demonioSerie();
};

#endif /* SOURCE_COMANDOSERIE_H_ */
