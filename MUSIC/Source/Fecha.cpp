/*
 * Fecha.cpp
 *
 *  Created on: 16 ago. 2021
 *      Author: isrev
 */

#include "Fecha.h"

Fecha::Fecha() {
}

void Fecha::iniciarRTC(){

	if(!configSystem.MODULO_RTC)
		return;

	if (! rtc.begin()) {
		Serial.println("Modulo RTC no encontrado !");

		pantallaDeError(F("  SYSTM ERROR!  ERR INICIAR  RTC"));
	}else {
		//rtc.adjust(DateTime(__DATE__, __TIME__));  //Establecer tiempo

		//Sincronizamos el tiempo con el RTC local
		rtcLocal.setTime(rtc.now().unixtime());

	}


}


DateTime Fecha::obtenerTiempoActual(){
	return rtc.now();
}

String Fecha::imprimeFecha(byte local, DateTime paramFecha){

	if(!configSystem.MODULO_RTC)
		return "XX/XX/20XX 00:00:00";

	if(local)
	 return rtcLocal.getTime("%d-%m-%Y %H:%M:%S");

	(paramFecha > 0)? fecha = paramFecha : fecha = obtenerTiempoActual();
	char buffer[] = "DD/MM/YYYY hh:mm:ss";
	return fecha.toString(buffer);
}

String Fecha::imprimeFechaSimple(DateTime paramFecha){

	if(!configSystem.MODULO_RTC)
		return "XX/XX/20XX";

	(paramFecha > 0)? fecha = paramFecha : fecha = obtenerTiempoActual();
	char buffer[] = "DD/MM/YYYY";
	return fecha.toString(buffer);

}

String Fecha::imprimeFechaSQL(){
	fecha = obtenerTiempoActual();
	char buffer[] = "YYYY-MM-DD hh:mm:ss";

	return fecha.toString(buffer);
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
	fecha = obtenerTiempoActual();

	char buffer[] = "hh:mm";
	return fecha.toString(buffer);
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
	if(horas == fecha.hour()){

		if (minutos == fecha.minute()){
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

		if((int)fecha.hour() >= hora_inicio) {

			hora_fin = hora_fin +24; //Hasta las 12 contempla horas posteriores como mayores(+24 horas)

		}

		if((int)fecha.hour() <= 23){ //Cuando son menos de las 12 contempla la hora de inicio como inferior (-24 horas)

			hora_inicio = hora_inicio - 24;

		}

		aux = 24; //Ajusta el tiempo las primeras horas del ultimo dia
	}else {
		aux = 0;
	}


	if((int)fecha.hour() == hora_inicio+aux) {

		if(((int)fecha.hour() >= hora_inicio && (int)fecha.minute() >= min_inicio) && (hora_fin > (int)fecha.hour())) {

			return true;
		}else {
			return false;
		}


	}else if((int)fecha.hour() == hora_fin) {

		if(((int)fecha.hour() >= hora_inicio) && (hora_fin >= (int)fecha.hour() && (int)fecha.minute() < min_fin )) {

			return true;
		}else {
			return false;
		}

	}else if ((int)fecha.hour() != hora_inicio && (int)fecha.hour() != hora_fin){

		if(((int)fecha.hour() >= hora_inicio ) && (hora_fin > (int)fecha.hour())) {

			return true;
		}else {
			return false;
		}
	}else {
		return false;
	}

}
