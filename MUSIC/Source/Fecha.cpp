/*
 * Fecha.cpp
 *
 *  Created on: 16 ago. 2021
 *      Author: isrev
 */

#include "Fecha.h"

Fecha::Fecha() {
}

byte Fecha::iniciarRTC(){

	if(!configSystem.MODULO_RTC)
		return 1;

	if (! rtc.begin()) {
		Serial.println("Modulo RTC no encontrado !");
		pantallaDeError(F("  SYSTM ERROR!  ERR INICIAR  RTC"));
		return 0;
	}else {
		//rtc.adjust(DateTime(__DATE__, __TIME__));  //Establecer tiempo

		//Sincronizamos el tiempo con el RTC local
		rtcLocal.setTime(rtc.now().unixtime());
		return 1;
	}

}


DateTime Fecha::obtenerTiempoActual(){
	  DateTime dateTime = DateTime(rtcLocal.getEpoch());
	  return dateTime;
}

String Fecha::imprimeFecha(byte local, DateTime paramFecha){

	if(!configSystem.MODULO_RTC)
		return "01/01/2099 00:00:00";

	if(local)
	 return rtcLocal.getTime("%d-%m-%Y %H:%M:%S");

	(paramFecha > 0)? fecha = paramFecha : fecha = obtenerTiempoActual();
	char buffer[] = "DD/MM/YYYY hh:mm:ss";
	return fecha.toString(buffer);
}

String Fecha::imprimeFechaSimple(DateTime paramFecha){

	if(!configSystem.MODULO_RTC)
		return "01/01/2099";

	(paramFecha > 0)? fecha = paramFecha : fecha = obtenerTiempoActual();
	char buffer[] = "DD/MM/YYYY";
	return fecha.toString(buffer);

}

String Fecha::imprimeFechaSQL(){
	return rtcLocal.getTime("%Y-%m-%d %H:%M:%S");
}


String Fecha::imprimeFechaJSON(byte local, DateTime paramFecha){

	if(!configSystem.MODULO_RTC)
		return "2099-01-01T00:00:00";

	if(local)
	return rtcLocal.getTime("%Y-%m-%dT%H:%M:%S");


	(paramFecha > 0)? fecha = paramFecha : fecha = obtenerTiempoActual();
	char buffer[] = "YYYY-MM-DDThh:mm:ss";
	return fecha.toString(buffer);

}



const char* Fecha::imprimeFechaFichero(byte local){

	if(!configSystem.MODULO_RTC)
		return "XXXX_XXXX";

	String fechaString;

	if(local){
		fechaString = rtcLocal.getTime("%d%m%Y_%H%M%S");
	}else{
		fecha = obtenerTiempoActual();
		char buffer[] = "DDMMYYYY_hhmmss";
		fechaString = fecha.toString(buffer);
	}

	return fechaString.c_str();

}

String Fecha::imprimeHora(){
	return rtcLocal.getTime("%H:%M");
}

void Fecha::establecerFechaReset(byte dia, byte hora, byte minuto, byte segundo){
	DateTime fechaFutura (obtenerTiempoActual() + TimeSpan(dia,hora,minuto,segundo));
	fechaReset = fechaFutura;

}

DateTime Fecha::getFechaReset(){
	return fechaReset;
}


bool Fecha::comprobarFecha(DateTime paramFecha){

	if(obtenerTiempoActual() >= paramFecha){
		return true;
	}else{
		return false;
	}

}
bool Fecha::comprobarHora(byte horas, byte minutos){ //Hora concreta

	fecha = obtenerTiempoActual();
	if(horas == rtcLocal.getHour(true)){

		if (minutos == rtcLocal.getMinute()){
			return true;
		}else {
			return false;
		}

	}else {
		return false;
	}

}

bool Fecha::comprobarRangoHorario(byte hora_inicio, byte hora_fin,  byte min_inicio, byte min_fin){ //Rango de horas

	fecha = obtenerTiempoActual();

	int aux=0;

	if(hora_inicio > hora_fin) { //Si hay diferencia entre dias entrara aqui

		if((int)rtcLocal.getHour(true) >= hora_inicio) {

			hora_fin = hora_fin +24; //Hasta las 12 contempla horas posteriores como mayores(+24 horas)

		}

		if((int)rtcLocal.getHour(true) <= 23){ //Cuando son menos de las 12 contempla la hora de inicio como inferior (-24 horas)

			hora_inicio = hora_inicio - 24;

		}

		aux = 24; //Ajusta el tiempo las primeras horas del ultimo dia
	}else {
		aux = 0;
	}


	if((int)rtcLocal.getHour(true) == hora_inicio+aux) {

		if(((int)rtcLocal.getHour(true) >= hora_inicio && (int)rtcLocal.getMinute() >= min_inicio) && (hora_fin > (int)rtcLocal.getHour(true))) {

			return true;
		}else {
			return false;
		}


	}else if((int)rtcLocal.getHour(true) == hora_fin) {

		if(((int)rtcLocal.getHour(true) >= hora_inicio) && (hora_fin >= (int)rtcLocal.getHour(true) && (int)rtcLocal.getMinute() < min_fin )) {

			return true;
		}else {
			return false;
		}

	}else if ((int)rtcLocal.getHour(true) != hora_inicio && (int)rtcLocal.getHour(true) != hora_fin){

		if(((int)rtcLocal.getHour(true) >= hora_inicio ) && (hora_fin > (int)rtcLocal.getHour(true))) {

			return true;
		}else {
			return false;
		}
	}else {
		return false;
	}

}
