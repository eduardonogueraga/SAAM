/*
 * Fecha.h
 *
 *  Created on: 16 ago. 2021
 *      Author: isrev
 */

#ifndef SOURCE_FECHA_H_
#define SOURCE_FECHA_H_
#include "Arduino.h"
#include <RTClib.h>
#include "AlarmaUtil/Macros.h"
#include <ESP32Time.h>
#include <regex>

extern ConfigSystem configSystem;
extern RespuestaHttp getTiempoServer();

class Fecha {
private:
	RTC_DS3231 rtc;
	ESP32Time rtcLocal;
	DateTime fecha;
	DateTime fechaReset;

	DateTime obtenerTiempoActual();
public:

	Fecha();
	byte iniciarRTC();
	byte ajustarFechaServidor();
	String imprimeFecha(byte local= 0, DateTime paramFecha = 0);
	String imprimeFechaSimple(DateTime paramFecha = 0);
	String imprimeFechaSQL();
	String imprimeFechaJSON(byte local = 0, DateTime paramFecha = 0);
	const char* imprimeFechaFichero(byte local= 0);
	String imprimeHora();
	void establecerFechaReset(byte dia, byte hora = 0, byte minuto = 0, byte segundo = 0);
	DateTime getFechaReset();
	bool comprobarFecha(DateTime paramFecha);
	bool comprobarHora(byte horas, byte minutos = 0);
	bool comprobarRangoHorario(byte hora_inicio, byte hora_fin, byte min_inicio = 0, byte min_fin = 0);
};

#endif /* SOURCE_FECHA_H_ */
