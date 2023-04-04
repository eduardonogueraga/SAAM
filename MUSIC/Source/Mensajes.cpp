/*
 * Mensajes.cpp
 *
 *  Created on: 06 ago. 2020
 *      Author: isrev
 *
 */

#include "Mensajes.h"
#include "Datos.h"
//#include <SoftwareSerial.h>
#include "Env.h"
#include "Registro.h"

extern Registro registro;

Mensajes::Mensajes(){
	this->tipoMensaje = SMS_TIPO_SALTO;
}

/*
void Mensajes::inicioSIM800(SoftwareSerial &SIM800L){
	SIM800L.begin(9600); 					//Inicializamos la segunda comunicacion Serial.
	SIM800L.println("AT+CMGF=1"); 			//Vamos utilizar los SMS.
	delay(100);
	SIM800L.println("AT+CNMI=1,2,0,0,0"); 	//Configurar el SIM800L p/ que muestre msm por com. serie.
 }
*/

void Mensajes::mensajeAlerta(Datos &datos){
	this->tipoMensaje = SMS_TIPO_SALTO;
	this->asuntoMensaje = this->asuntoAlerta(datos);
	this->cuerpoMensaje = datos.imprimeDatos();

	this->pieMensaje = "Intentos restantes: "+ (String)(3-INTENTOS_REACTIVACION);
	if(flagPuertaAbierta)
		this->pieMensaje += "\nLa puerta esta abierta";

	pieFechaBateria();
	this->enviarSMS();

	//insertQuery(&sqlMensajes);
	//insertQuery(&sqlUpdateSalto);
	char registroConjunto[50];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", Mensajes::getAsuntoMensaje()," MENSAJE ENVIADO");
	registro.registrarLogSistema(registroConjunto);
}

void Mensajes::mensajeReactivacion(Datos &datos){

	this->tipoMensaje = SMS_TIPO_INFO;
	this->asuntoMensaje = "ALARMA REACTIVADA CON EXITO";
	this->cuerpoMensaje = datos.imprimeDatos();
	this->pieMensaje = "";

	pieFechaBateria();

	this->enviarSMS();
	//insertQuery(&sqlMensajes);
	//insertQuery(&sqlUpdateEntrada);
	registro.registrarLogSistema("ALARMA REACTIVADA MENSAJE ENVIADO");
}


void Mensajes::mensajeError(Datos &datos){

	this->tipoMensaje = SMS_TIPO_ERROR;
	switch(codigoError){

	case ERR_FALLO_ALIMENTACION:

		if(datos.comprobarDatos()){
			this->asuntoMensaje = "MOVIMIENTO DETECTADO Y SABOTAJE EN LA ALIMENTACION";
		}else{
			this->asuntoMensaje = "FALLO EN LA ALIMENTACION PRINCIPAL";
		}
		break;


	case ERR_FALLO_SENSOR:
		//
		break;
	}

	this->cuerpoMensaje = datos.imprimeDatos();

	pieFechaBateria();

	this->enviarSMSEmergencia();
	//insertQuery(&sqlMensajes);
	//insertQuery(&sqlUpdateErrores);
	char registroConjunto[50];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", Mensajes::getAsuntoMensaje()," MENSAJE ENVIADO");
	registro.registrarLogSistema(registroConjunto);

}

void Mensajes::enviarSMS(){

	if(MODO_DEFAULT) //@develop !MODO_DEFAULT
		return;


	procesarSMS();

	char registroConjunto[50];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%d", "SMS ENVIADO NUMERO:",leerFlagEE("N_SMS_ENVIADOS"));
	registro.registrarLogSistema(registroConjunto);
	//insertQuery(&sqlSmsIntentosRealizados);

}

void Mensajes::enviarSMSEmergencia(){

	if(MODO_DEFAULT) //@develop !MODO_DEFAULT
		return;

	if(leerFlagEE("MENSAJE_EMERGEN") == 1)
	return;

	/*Serial.println(this->asuntoMensaje+"\n");
	Serial.println(this->cuerpoMensaje);
	Serial.println(this->pieMensaje);*/

	procesarSMS();
	//EEPROM.update(EE_MENSAJE_EMERGENCIA, 1);
	guardarFlagEE("MENSAJE_EMERGEN", 1);

}

void Mensajes::procesarSMS(){

	if(leerFlagEE("N_SMS_ENVIADOS") >= LIMITE_MAXIMO_SMS){
		Serial.println(F("Intentos diarios acabados")); //No se enviaran mas mensajes
		registro.registrarLogSistema("INTENTOS SMS DIARIOS ACABADOS");

		return;
	}

	Serial.println("AT+CMGF=1"); //@PEND
	delay(200);
	Serial.println("AT+CMGS=\"+34"+(String)telefonoPrincipal+"\""); //@PEND
	delay(200);

	Serial.print(this->asuntoMensaje+"\n");//@PEND
	Serial.println(this->cuerpoMensaje); //@PEND
	Serial.println(this->pieMensaje); //@PEND

	delay(200);
	Serial.print((char)26); //@PEND
	delay(200);
	Serial.println(""); //@PEND
	delay(200);

	mensajesEnviados++;
	configSystem.SMS_HISTORICO++;
	//EEPROM_SaveData(EE_CONFIG_STRUCT, configSystem);
	//EEPROM.update(MENSAJES_ENVIADOS, (EEPROM.read(MENSAJES_ENVIADOS)+1));
	NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	guardarFlagEE("N_SMS_ENVIADOS", (leerFlagEE("N_SMS_ENVIADOS")+1));

}

void Mensajes::llamarTlf(char* tlf){

	if(MODO_DEFAULT) //@develop  !MODO_DEFAULT
	return;

	if(leerFlagEE("N_SMS_ENVIADOS") >= (LIMITE_MAXIMO_SMS-7)){
			Serial.println(F("Intentos diarios acabados")); //No se haran mas llamadas
			return;
		}

	Serial.println("Llamando "+(String)tlf);
	Serial.println("AT"); //@PEND
	delay(200);
	Serial.println("ATD+ +34"+(String)tlf+';'); //@PÊND
	delay(200);
}

void Mensajes::colgarLlamada(){
	Serial.println("Llamada finalizada");
	Serial.println("ATH"); //@PEND

}

String Mensajes::asuntoAlerta(Datos &datos){

	int* mapSensor= datos.getDatos();
	cont=0; //Reinicia el contador

	for (int i = 0; i < 4; i++) { //TODO enlaza con tam el tamano de los array

		if(mapSensor[i] == MAX_SALTO[i]) {
			//Salto principal
			asuntoMensaje = "AVISO ALARMA:";
			if(nombreZonas[i] == ("PUERTA COCHERA")) {
				asuntoMensaje += " PUERTA ABIERTA EN COCHERA";
			}else {
				asuntoMensaje += " MOVIMIENTO DETECTADO EN "+nombreZonas[i];
			}

			for (int j= 0; j < 4; j++) {
				if((mapSensor[j] != 0)&&(mapSensor[j] != MAX_SALTO[j])) {
					//Saltos secundarios

					if(cont == 0) {
						asuntoMensaje += " JUNTO CON MOVIMIENTO EN "+nombreZonas[j];
						cont++;

					}else {
						asuntoMensaje += " Y "+nombreZonas[j];
					}
				}
			}
		}
	}

	return asuntoMensaje;
}

void Mensajes::pieFechaBateria(){
	if(configSystem.MODULO_RTC)
		//this->pieMensaje = fecha.imprimeFecha(); @PEND

	if(pcf8575.digitalRead(SENSOR_BATERIA_RESPALDO) == LOW){
		this->pieMensaje += " Bateria de emergencia desactivada";
	}
}


const String& Mensajes::getAsuntoMensaje() const {
	return asuntoMensaje;
}

const String& Mensajes::getCuerpoMensaje() const {
	return cuerpoMensaje;
}

byte  Mensajes::getTipoMensaje() const {
		return tipoMensaje;
	}
