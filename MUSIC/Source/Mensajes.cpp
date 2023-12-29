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
}


void Mensajes::mensajeAlerta()  {

	this->tipoMensaje = SMS_TIPO_SALTO;
	//Preparamos el asunto del mensaje
	asuntoAlerta();

	this->cuerpoMensaje += "Informacion sobre la deteccion: \n";

	//Datos puerta
	if(respuestaTerminal.idTerminal == 0 && respuestaTerminal.idSensorDetonante == 3){
		this->cuerpoMensaje += "Puerta de la cochera abierta.\n";
	}

	//Datos terminales
	for (int i = 0; i < N_TERMINALES_LINEA; i++) {
		this->cuerpoMensaje += T_LIST[i]->generarInformeDatos();
	}

	this->pieMensaje = "\nIntentos restantes: "+ (String)(3-INTENTOS_REACTIVACION);

	//pieFechaBateria(); //Incompatibilidad hardware

	this->enviarSMS();

	char registroConjunto[60];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", Mensajes::getAsuntoMensaje().c_str()," MENSAJE ENVIADO");
	registro.registrarLogSistema(registroConjunto);
	eventosJson.guardarNotificacion(1, this->literalAsuntoSaas, this->getFullSMS(), TLF_NUM_1);

	limpiarContenido();
}

void Mensajes::mensajeReactivacion(){

	this->tipoMensaje = SMS_TIPO_INFO;
	this->asuntoMensaje = "ALARMA REACTIVADA CON EXITO";

	this->cuerpoMensaje += "Informe de movimientos: \n";

	//Datos terminal
	for (int i = 0; i < N_TERMINALES_LINEA; i++) {
		this->cuerpoMensaje += T_LIST[i]->generarInformeDatos();
	}

	if(flagPuertaAbierta)
		this->pieMensaje += "\nLa puerta esta abierta";

	//pieFechaBateria(); //Incompatibilidad hardware

	this->enviarSMS();
	registro.registrarLogSistema("ALARMA REACTIVADA MENSAJE ENVIADO");
	eventosJson.guardarNotificacion(1, ALARMA_REACTIVADA_EXITO, this->getFullSMS(), TLF_NUM_1);

	limpiarContenido();
}



void Mensajes::mensajeError(){

	this->tipoMensaje = SMS_TIPO_ERROR;
	switch(codigoError){

	case ERR_FALLO_ALIMENTACION:

		if(respuestaTerminal.interpretacion == SABOTAJE){
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

	//Datos terminal
	for (int i = 0; i < N_TERMINALES_LINEA; i++) {
		this->cuerpoMensaje += T_LIST[i]->generarInformeDatos();
	}

	if(flagPuertaAbierta)
		this->pieMensaje += "\nLa puerta esta abierta";

	//pieFechaBateria(); //Incompatibilidad hardware

	this->enviarSMSEmergencia();
	char registroConjunto[60];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", Mensajes::getAsuntoMensaje().c_str()," MENSAJE ENVIADO");
	registro.registrarLogSistema(registroConjunto);
	eventosJson.guardarNotificacion(1, this->literalAsuntoSaas, this->getFullSMS(), TLF_NUM_1);

	limpiarContenido();
}

void Mensajes::enviarSMS(){

#ifdef ALARMA_EN_MODO_DEBUG
	if(MODO_DEFAULT) //@develop !MODO_DEFAULT
		return;
#else
	if(!MODO_DEFAULT)
		return;
#endif

	procesarSMS();

	char registroConjunto[50];
	snprintf(registroConjunto, sizeof(registroConjunto), "%s%d", "SMS ENVIADO NUMERO:",leerFlagEEInt("N_SMS_ENVIADOS"));
	registro.registrarLogSistema(registroConjunto);

	limpiarContenido();

}

void Mensajes::enviarSMSEmergencia(){

#ifdef ALARMA_EN_MODO_DEBUG
	if(MODO_DEFAULT) //@develop !MODO_DEFAULT
		return;
#else
	if(!MODO_DEFAULT)
		return;
#endif

	if(leerFlagEEInt("MENSAJE_EMERGEN") == 1)
	return;

	procesarSMS(); //@develop

	guardarFlagEE("MENSAJE_EMERGEN", 1);

}

void Mensajes::procesarSMS(){

	if(leerFlagEEInt("N_SMS_ENVIADOS") >= LIMITE_MAXIMO_SMS){
		Serial.println(F("Intentos diarios acabados")); //No se enviaran mas mensajes
		registro.registrarLogSistema("INTENTOS SMS DIARIOS ACABADOS");
		eventosJson.guardarLog(INTENTOS_SMS_DIARIOS_ACABADOS_LOG);

		return;
	}
	//UART_GSM

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
	guardarFlagEE("N_SMS_ENVIADOS", (leerFlagEEInt("N_SMS_ENVIADOS")+1));

}

void Mensajes::llamarTlf(char* tlf){

#ifdef ALARMA_EN_MODO_DEBUG
	if(MODO_DEFAULT) //@develop !MODO_DEFAULT
		return;
#else
	if(!MODO_DEFAULT)
		return;
#endif

	if(leerFlagEEInt("N_SMS_ENVIADOS") >= (LIMITE_MAXIMO_SMS-7)){
		Serial.println(F("Intentos diarios acabados")); //No se haran mas llamadas
		return;
	}

	Serial.println("Llamando "+(String)tlf);
	llamarTlfAPI(tlf);

}

void Mensajes::colgarLlamada(){
	Serial.println("Llamada finalizada");
	colgarLlamadaAPI();
}

void Mensajes::asuntoAlerta(){

	if(respuestaTerminal.interpretacion == DETECCION){
		this->asuntoMensaje += "INTRUSISMO DETECTADO EN " + String(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]);
	}else if(respuestaTerminal.interpretacion == DETECCION_FOTOSENIBLE){
		this->asuntoMensaje += "LUZ DETECTADA EN " + String(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]);
	}else if(respuestaTerminal.interpretacion == AVERIA){
		this->asuntoMensaje += "AVERIA: " + String(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]);
	}else if(respuestaTerminal.interpretacion == SABOTAJE){
		this->asuntoMensaje += "SABOTAJE EN " + String(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]);
	}else {
		this->asuntoMensaje += "INTRUSISMO DETECTADO EN " + String(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]);
	}

	//Por ahora solo el CORE
	if(respuestaTerminal.idTerminal == 0){
		this->literalAsuntoSaas = nombreZonasSaas[respuestaTerminal.idSensorDetonante];
	}


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


byte Mensajes::getTipoMensaje() const {
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

 void Mensajes::limpiarContenido(){
	 this->cuerpoMensaje = "";
	 this->asuntoMensaje = "";
	 this->pieMensaje = "";
 }
