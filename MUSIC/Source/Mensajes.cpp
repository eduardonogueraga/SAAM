/*
 * Mensajes.cpp
 *
 *  Created on: 06 ago. 2020
 *      Author: isrev
 *
 */

#include "Mensajes.h"

extern Registro registro;

Mensajes::Mensajes(HardwareSerial& serialInstance) : UART_GSM(serialInstance){
	this->tipoMensaje = SMS_TIPO_SALTO;
}


void Mensajes::inicioGSM(){
	UART_GSM.begin(115200, SERIAL_8N1, GSM_RX, GSM_TX); 	//RX TX  (H2 = RX23 TX19)
	UART_GSM.println("AT+CMGF=1"); 			//Vamos utilizar los SMS.
	delay(100);
	UART_GSM.println("AT+CNMI=1,2,0,0,0"); 	//Configurar el SIM800L p/ que muestre msm por com. serie.
}


void Mensajes::mensajeAlerta(Datos &datos)  {

	this->tipoMensaje = SMS_TIPO_SALTO;
	this->asuntoMensaje = this->asuntoAlerta(datos);
	this->cuerpoMensaje = datos.imprimeDatos();

	Serial.println("ASUNTO:");
	Serial.println(this->asuntoMensaje);


	this->pieMensaje = "Intentos restantes: "+ (String)(3-INTENTOS_REACTIVACION);
	if(flagPuertaAbierta)
		this->pieMensaje += "\nLa puerta esta abierta";

	pieFechaBateria();
	this->enviarSMS();

	char registroConjunto[50];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", Mensajes::getAsuntoMensaje()," MENSAJE ENVIADO");
	registro.registrarLogSistema(registroConjunto);
	eventosJson.guardarNotificacion(1, this->literalAsuntoSaas, this->getFullSMS(), TLF_NUM_1);

}

void Mensajes::mensajeReactivacion(Datos &datos){

	this->tipoMensaje = SMS_TIPO_INFO;
	this->asuntoMensaje = "ALARMA REACTIVADA CON EXITO";
	this->cuerpoMensaje = datos.imprimeDatos();
	this->pieMensaje = "";

	pieFechaBateria();

	this->enviarSMS();
	registro.registrarLogSistema("ALARMA REACTIVADA MENSAJE ENVIADO");
	eventosJson.guardarNotificacion(1, ALARMA_REACTIVADA_EXITO, this->getFullSMS(), TLF_NUM_1);
}


void Mensajes::mensajeError(Datos &datos){

	this->tipoMensaje = SMS_TIPO_ERROR;
	switch(codigoError){

	case ERR_FALLO_ALIMENTACION:

		if(datos.comprobarDatos()){
			this->asuntoMensaje = "MOVIMIENTO DETECTADO Y SABOTAJE EN LA ALIMENTACION";
			this->literalAsuntoSaas = MOVIMIENTO_DETEC_SABOTAJE_ALIMENTACION;
		}else{
			this->asuntoMensaje = "FALLO EN LA ALIMENTACION PRINCIPAL";
			this->literalAsuntoSaas = FALLO_ALIMENTACION_PRINCIPAL;
		}
		break;


	case ERR_FALLO_SENSOR:
		//
		break;
	}

	this->cuerpoMensaje = datos.imprimeDatos();

	pieFechaBateria();

	this->enviarSMSEmergencia();
	char registroConjunto[50];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", Mensajes::getAsuntoMensaje()," MENSAJE ENVIADO");
	registro.registrarLogSistema(registroConjunto);
	eventosJson.guardarNotificacion(1, this->literalAsuntoSaas, this->getFullSMS(), TLF_NUM_1);


}

void Mensajes::enviarSMS(){

	if(MODO_DEFAULT) //@develop !MODO_DEFAULT
		return;

	procesarSMS();

	char registroConjunto[50];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%d", "SMS ENVIADO NUMERO:",leerFlagEE("N_SMS_ENVIADOS"));
	registro.registrarLogSistema(registroConjunto);

}

void Mensajes::enviarSMSEmergencia(){ //@PEND Esto estaba terminado??

	if(MODO_DEFAULT) //@develop !MODO_DEFAULT
		return;

	if(leerFlagEE("MENSAJE_EMERGEN") == 1)
	return;

	/*Serial.println(this->asuntoMensaje+"\n");
	Serial.println(this->cuerpoMensaje);
	Serial.println(this->pieMensaje);*/

	procesarSMS();

	guardarFlagEE("MENSAJE_EMERGEN", 1);

}

void Mensajes::procesarSMS(){

	if(leerFlagEE("N_SMS_ENVIADOS") >= LIMITE_MAXIMO_SMS){
		Serial.println(F("Intentos diarios acabados")); //No se enviaran mas mensajes
		registro.registrarLogSistema("INTENTOS SMS DIARIOS ACABADOS");
		eventosJson.guardarLog(INTENTOS_SMS_DIARIOS_ACABADOS_LOG);

		return;
	}

	UART_GSM.println("AT+CMGF=1");
	delay(200);
	UART_GSM.println("AT+CMGS=\"+34"+(String)telefonoPrincipal+"\"");
	delay(200);

	UART_GSM.print(this->asuntoMensaje+"\n");
	UART_GSM.println(this->cuerpoMensaje);
	UART_GSM.println(this->pieMensaje);

	delay(200);
	UART_GSM.print((char)26);
	delay(200);
	UART_GSM.println("");
	delay(200);

	mensajesEnviados++;
	configSystem.SMS_HISTORICO++;
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
	UART_GSM.println("AT");
	delay(200);
	UART_GSM.println("ATD+ +34"+(String)tlf+';');
	delay(200);

}

void Mensajes::colgarLlamada(){
	Serial.println("Llamada finalizada");
	UART_GSM.println("ATH");

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
				this->literalAsuntoSaas = AVISO_ALARMA_PUERTA_COCHERA_ABIERTA;

			}else {
				asuntoMensaje += " MOVIMIENTO DETECTADO EN "+nombreZonas[i];
				this->literalAsuntoSaas = nombreZonasSaas[i];
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
		this->pieMensaje = fecha.imprimeFecha();

	if(mcp.digitalRead(SENSOR_BATERIA_RESPALDO) == LOW){
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

 char* Mensajes::getFullSMS(){

	 String mensaje = cuerpoMensaje;
	 mensaje += "\n";
	 mensaje += pieMensaje;

	  char* charArray = new char[mensaje.length() + 1]; // +1 para el carácter nulo

	    strcpy(charArray, mensaje.c_str());

	    return charArray;

}
