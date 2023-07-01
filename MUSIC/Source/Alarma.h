/*
 * Alarma.h
 *
 *  Created on: 3 ago. 2021
 *      Author: isrev
 */

#ifndef SOURCE_ALARMA_H_
#define SOURCE_ALARMA_H_

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Preferences.h>
#include <Adafruit_MCP23X17.h>
#include <HardwareSerial.h>

#include "Autenticacion.h"
#include "Pantalla.h"
#include "ComandoSerie.h"
#include "InterStrike.h"
#include "Datos.h"
#include "Bocina.h"
#include "Teclado.h"
#include "Env.h"
#include "Macros.h"
#include "Mensajes.h"
#include "Menu.h"
#include "Fecha.h"
#include "Registro.h"
#include "EventosJson.h"


//VERSION (VE -> Version Estable VD -> Version Desarrollo)
const char* version[] = {"MUSIC VE21R0", "25/06/23"};

//VARIABLES GLOBALES
ConfigSystem configSystem;
EE_DatosSalto eeDatosSalto;

byte MODO_DEFAULT = 0;  //@develop
byte INTENTOS_REACTIVACION = 0;
byte SD_STATUS = 0; //Comprueba si la escritura en SD esta OK

//SENSORES
byte sensorHabilitado[4] = {1,1,1,1};

//INSTANCIAS

//UART
HardwareSerial UART_GSM(1);
HardwareSerial UART_RS(2);

//MUX
Adafruit_MCP23X17 mcp;

//NVS
Preferences NVSMemory; //Memoria

//TYPE DEF
ProcesoCentral procesoCentral;
EstadosAlarma estadoAlarma;
EstadosError estadoError;
SLEEPMODE_GSM sleepModeGSM;
LLAMADAS_GSM estadoLlamada;
CODIGO_ERROR codigoError;
SAAS_LITERAL_LOGS saaLiteralLogs;

//CLASES
Autenticacion auth;
Pantalla pantalla;
ComandoSerie demonio;
Bocina bocina;
Datos datosSensores;
Datos datosSensoresPhantom;

InterStrike mg = InterStrike(0, 1, datosSensores);
InterStrike pir1 = InterStrike(1, 1, datosSensores, 5000, 60000);
InterStrike pir2 = InterStrike(2, 2, datosSensores, 7000, 20000);
InterStrike pir3 = InterStrike(3, 2, datosSensores, 5000, 21000);
Mensajes mensaje;
Menu menu;
Fecha fecha;
Registro registro;
EventosJson eventosJson;


//TIEMPOS MARGEN

const unsigned long TIEMPO_OFF = 120000; // (*0.1666) -> 20000 (*0.1666) -> 20000 sensible
const unsigned long TIEMPO_ON = 600000; //(*0.01666) -> 10000 (*0.01) -> 6000 en auto activacion
const unsigned long TIEMPO_REACTIVACION = 240000; // (*0.1) ->  24000
const unsigned long TIEMPO_MODO_SENSIBLE = 3600000; // (*0.0166)  -> 60000*
const unsigned long TIEMPO_BOCINA = 600000; // (*0.0333) -> 20000* //300000(*0.0666) ->20000
const unsigned long TIEMPO_PRORROGA_GSM = 1200000; // (*0.05) -> 60000
const unsigned short TIEMPO_CARGA_GSM = 10000;

unsigned long tiempoMargen;

//TIEMPO MODO SENSIBLE
unsigned long tiempoSensible;
//TIEMPO SLEEPMODE
unsigned long prorrogaGSM;
//TIEMPO BOCINA
unsigned long tiempoBocina;

//TIEMPO CLAVE
unsigned long lcd_clave_tiempo;
//LCD ALERTS
byte alertsInfoLcd[NUMERO_ALERTAS];
static byte alertInfoCached[NUMERO_ALERTAS];

unsigned long lcd_info_tiempo;
static byte tiempoFracccion;


//ESTADO GUARDIA
 String nombreZonas[4] {"PUERTA COCHERA","COCHERA","PORCHE","ALMACEN"};
 byte zona;

//DATOS SMS
 byte desactivaciones = 0;
 byte mensajesEnviados = 0;

 //Control bateria
 bool sensorBateriaAnterior; //Compara el estado de la bateria

 //FLAG PUERTA
 byte flagPuertaAbierta = 0;

 //FUNCIONES//
 void leerEntradaTeclado(){
	 key = keypad.getKey();
	 auth.comprobarEntrada();
 }

 byte mostrarLcdAlerts(){

	 if(lcd_info_tiempo < millis()){

		 tiempoFracccion = 0;
		 alertInfoCached[INFO_FALLO_BATERIA] = 0;
		 alertInfoCached[INFO_SENSOR_PUERTA_OFF] = 0;
		 alertInfoCached[INFO_RESET_AUTO] = 0;
		 alertInfoCached[INFO_BLUETOOH] = 0;

		 return 0;
	 }

	 byte alertNum = arrSum<byte>(alertsInfoLcd, NUMERO_ALERTAS);

	 if(alertNum){
		 if(alertsInfoLcd[INFO_FALLO_BATERIA] && (millis() >= (lcd_info_tiempo - ((TIEMPO_ALERT_LCD/alertNum)*(alertNum-tiempoFracccion))))){

			 if(alertInfoCached[INFO_FALLO_BATERIA] == 0){
				 pantalla.lcdLoadView(&pantalla, &Pantalla::lcdFalloBateria);
				 tiempoFracccion++;
				 alertInfoCached[INFO_FALLO_BATERIA] = 1;
			 }

		 }

		 if(alertsInfoLcd[INFO_SENSOR_PUERTA_OFF] && (millis() >= (lcd_info_tiempo - ((TIEMPO_ALERT_LCD/alertNum)*(alertNum-tiempoFracccion))))){

			 if(alertInfoCached[INFO_SENSOR_PUERTA_OFF] == 0){
				 pantalla.lcdLoadView(&pantalla, &Pantalla::lcdSensorPuertaDesconectado);
				 tiempoFracccion++;
				 alertInfoCached[INFO_SENSOR_PUERTA_OFF] = 1;
			 }
		 }

		 if(alertsInfoLcd[INFO_RESET_AUTO] && (millis() >= (lcd_info_tiempo - ((TIEMPO_ALERT_LCD/alertNum)*(alertNum-tiempoFracccion))))){

			 if(alertInfoCached[INFO_RESET_AUTO] == 0){
				 pantalla.lcdLoadView(&pantalla, &Pantalla::lcdAvisoResetAuto);
				 tiempoFracccion++;
				 alertInfoCached[INFO_RESET_AUTO] = 1;
			 }
		 }

		 if(alertsInfoLcd[INFO_BLUETOOH] && (millis() >= (lcd_info_tiempo - ((TIEMPO_ALERT_LCD/alertNum)*(alertNum-tiempoFracccion))))){

			 if(alertInfoCached[INFO_BLUETOOH] == 0){
				 pantalla.lcdLoadView(&pantalla, &Pantalla::lcdAvisoBluetooh);
				 tiempoFracccion++;
				 alertInfoCached[INFO_BLUETOOH] = 1;
			 }
		 }
	 }

	 return alertNum;
 }

 bool isLcdInfo(){

	 if(lcd_clave_tiempo > millis()){
		 pantalla.lcdLoadView(&pantalla, &Pantalla::lcdClave);
		 return true;
	 }

	 return mostrarLcdAlerts();
 }

 void pantallaDeErrorInicial(String mensaje){
		String *errLcd = &pantalla.getErrorTexto();
		*errLcd = mensaje;
		pantalla.lcdLoadView(&pantalla, &Pantalla::lcdError);
		delay(2000);
 }

	void setMargenTiempo(unsigned long &tiempoMargen, const unsigned long tiempo, float porcentaje = 1.0F){

		if(MODO_DEFAULT == 1){
			porcentaje = 1.0F;
		}

		tiempoMargen = millis() + (tiempo * porcentaje);
	}

	bool checkearMargenTiempo(unsigned long tiempoMargen){

		return millis() > tiempoMargen;
	}

	void desactivarAlarma(){
		if(auth.isPasswordCached()){
			if (key != NO_KEY){
				if(key == '#'){
					setEstadoReposo();
				}
			}
		}
	}

	void desactivarEstadoDeError(){
		if(auth.isPasswordCached()){
			if (key != NO_KEY){
				if(key == '#'){
					estadoError = GUARDAR_DATOS;
					guardarFlagEE("ERR_INTERRUPT", 0);
					guardarFlagEE("MENSAJE_EMERGEN", 0);
					guardarFlagEE("LLAMADA_EMERGEN", 0);

					procesoCentral = ALARMA;
				}
			}
		}
	}
	void sonarBocina(){
		if(!checkearMargenTiempo(tiempoBocina)){
			bocina.sonarBocina();
		}else{
			bocina.stopBocina();
		}
	}

	void pararBocina()
	{
		bocina.stopBocina();
		tiempoBocina = 0;
	}

	void limpiarSensores(){

		mg.setStart();
		pir1.setStart();
		pir2.setStart();
		pir3.setStart();

	}

	void watchDog(){
		mcp.digitalWrite(WATCHDOG, !mcp.digitalRead(WATCHDOG));
	}

	void sleepMode(){

		switch(sleepModeGSM){

		case GSM_ON:
			mcp.digitalWrite(GSM_PIN, HIGH);
			break;

		case GSM_OFF:
			mcp.digitalWrite(GSM_PIN, LOW);
			break;

		case GSM_TEMPORAL:

			if(checkearMargenTiempo(prorrogaGSM)){
				mcp.digitalWrite(GSM_PIN, LOW);
			}else {
				mcp.digitalWrite(GSM_PIN, HIGH);
			}
			break;
		}

	}

	void checkearSms(){
		if(!configSystem.MODULO_RTC){
			return;
		}

		if(fecha.comprobarHora(0, 0)){
			if(leerFlagEEInt("N_SMS_ENVIADOS") != 0){
				guardarFlagEE("N_SMS_ENVIADOS", 0);
				registro.registrarLogSistema("INTENTOS SMS DIARIOS RECUPERADOS");
				eventosJson.guardarLog(INTENTOS_SMS_DIARIOS_RECUPERADOS_LOG);
				Serial.println(F("Intentos diarios recuperados"));

			}
		}

	}

	void resetear(){
			Serial.println(F("\nReseteando"));
			if(alertsInfoLcd[INFO_RESET_AUTO] == 1){
				registro.registrarLogSistema("RESET AUTOMATICO");
				eventosJson.guardarLog(RESET_AUTOMATICO_LOG);
			}else {
				registro.registrarLogSistema("RESET MANUAL");
				eventosJson.guardarLog(RESET_MANUAL_LOG);
			}

			delay(200);
			mcp.digitalWrite(RESETEAR, HIGH);
		}

	void resetAutomatico(){

		if(!configSystem.MODULO_RTC){
			return;
		}

		if(fecha.comprobarFecha(fecha.getFechaReset())){

			alertsInfoLcd[INFO_RESET_AUTO] = 1;

			if(fecha.comprobarHora(16, 30)){
				Serial.println(F("\nReset programado"));
				resetear();
			}
		}else {
			alertsInfoLcd[INFO_RESET_AUTO] = 0;
		}
	}

	void resetearAlarma(){
		if(auth.isPasswordCached()){
			if (key != NO_KEY){
				if(key == '0'){
					setEstadoReposo();
					resetear();
				}
			}
		}
	}

	void cargarEstadoPrevio(){
		   flagPuertaAbierta = leerFlagEEInt("PUERTA_ABIERTA") == 1;


		if (leerFlagEEInt("ESTADO_GUARDIA") == 1 && leerFlagEEInt("ERR_INTERRUPT") == 0) {
			estadoAlarma = ESTADO_GUARDIA;
			registro.registrarLogSistema("CARGADO ESTADO GUARDIA PREVIO");
			//eventosJson.guardarLog(RESET_MANUAL_LOG); @PEND

			//Informar de que se ha restaurado una entrada
			guardarFlagEE("F_RESTAURADO", 1);
		}


	}

	void checkearAlertasDetenidas(){
		if (leerFlagEEInt("ESTADO_ALERTA") == 1 && leerFlagEEInt("ERR_INTERRUPT") == 0) {

			eeDatosSalto = NVS_RestoreData<datos_saltos_t>("SALTO_DATA");

			int* datos = datosSensores.getDatos();
			arrCopy<int>(eeDatosSalto.DATOS_SENSOR,datos ,TOTAL_SENSORES); //Carga los datos EE
			zona = eeDatosSalto.ZONA;
			INTENTOS_REACTIVACION = eeDatosSalto.INTENTOS_REACTIVACION;

			char registroConjunto[50];
			snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", "CARGADO ESTADO ALERTA EN ", nombreZonas[zona]);

			registro.registrarLogSistema(registroConjunto);
			//eventosJson.guardarLog(RESET_MANUAL_LOG); @PEND

			Serial.println("\nIntrusismo restaurado en " + nombreZonas[zona]);
			estadoAlarma = ESTADO_ALERTA;
			sleepModeGSM = GSM_ON;
			setMargenTiempo(tiempoMargen,15000);
		}
	}

	void guardarEstadoAlerta(){
		int* datos = datosSensores.getDatos();
		arrCopy<int>(datos, eeDatosSalto.DATOS_SENSOR,TOTAL_SENSORES);
		eeDatosSalto.ZONA = zona;
		eeDatosSalto.INTENTOS_REACTIVACION = INTENTOS_REACTIVACION;

		NVS_SaveData<datos_saltos_t>("SALTO_DATA", eeDatosSalto);
	}

	void guardarEstadoInterrupcion(){

		guardarFlagEE("ERR_INTERRUPT", 1);

		guardarFlagEE("INTERUP_HIST", (leerFlagEE("INTERUP_HIST") + 1));

		configSystem.MODULO_RTC = 0;
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	void checkearBateriaDeEmergencia(){
		alertsInfoLcd[INFO_FALLO_BATERIA] = !mcp.digitalRead(SENSOR_BATERIA_RESPALDO);

	    if(mcp.digitalRead(SENSOR_BATERIA_RESPALDO) != sensorBateriaAnterior){

			if(mcp.digitalRead(SENSOR_BATERIA_RESPALDO) == LOW){
				registro.registrarLogSistema("BATERIA DE EMERGENCIA ACTIVADA");
				eventosJson.guardarLog(BATERIA_EMERGENCIA_ACTIVADA_LOG);
			} else{
				registro.registrarLogSistema("BATERIA DE EMERGENCIA DESACTIVADA");
				eventosJson.guardarLog(BATERIA_EMERGENCIA_DESACTIVADA_LOG);
			}
		}

	    sensorBateriaAnterior = mcp.digitalRead(SENSOR_BATERIA_RESPALDO);
	}

	void realizarLlamadas(){

		if(MODO_DEFAULT) //@develop !MODO_DEFAULT
			return;

		static byte estadoAnterior;

		switch(estadoLlamada){

		case TLF_1:

			if(millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.95) && millis() < tiempoMargen - (TIEMPO_REACTIVACION*0.90)){
				mensaje.llamarTlf((char*)telefonoLlamada_1);
				registro.registrarLogSistema("LLAMANDO A MOVIL");
				eventosJson.guardarLog(LLAMANDO_A_MOVIL_LOG);
				estadoLlamada = COLGAR;
				estadoAnterior = TLF_1;

				eventosJson.guardarNotificacion(0, 0,"X", TLF_NUM_2);
			}

			break;

		case TLF_2:

			if(millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.80)){
				mensaje.llamarTlf((char*)telefonoLlamada_2);
				registro.registrarLogSistema("LLAMANDO A MOVIL");
				eventosJson.guardarLog(LLAMANDO_A_MOVIL_LOG);
				estadoLlamada = COLGAR;
				estadoAnterior = TLF_2;

				eventosJson.guardarNotificacion(0, 0,"X", TLF_NUM_3);

			}

			break;

		case COLGAR:

			if((millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.85))){
				if(estadoAnterior == TLF_1){
					mensaje.colgarLlamada();
					estadoLlamada = TLF_2;
				}
			}

			if((millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.70))){
				if(estadoAnterior == TLF_2){
					mensaje.colgarLlamada();
					estadoLlamada = TLF_1;
				}
			}
			break;
		}
	}

	void chekearInterrupciones(){
		if(leerFlagEE("ERR_INTERRUPT") == 1){

			procesoCentral = ERROR;
			codigoError = static_cast<CODIGO_ERROR>(leerFlagEE("CODIGO_ERROR"));

			if(leerFlagEE("MENSAJE_EMERGEN") == 1){

				if(leerFlagEE("LLAMADA_EMERGEN") == 0){
					Serial.println(F("Vuelve a por las llamadas"));
					estadoError = REALIZAR_LLAMADAS;
					setMargenTiempo(tiempoMargen,240000);
				}else {
					Serial.println(F("Vuelve a esperar ayuda"));
					estadoError = ESPERAR_AYUDA;
				}

			}else {
				Serial.println(F("Vuelve desde el principio"));

				eeDatosSalto = NVS_RestoreData<datos_saltos_t>("SALTO_DATA");
				int* datos = datosSensores.getDatos();
				arrCopy<int>(eeDatosSalto.DATOS_SENSOR,datos ,TOTAL_SENSORES);

				estadoError = COMPROBAR_DATOS;
				sleepModeGSM = GSM_TEMPORAL;
			}
		}
	}

	void interrupcionFalloAlimentacion(){
		Serial.println(F("\nInterrupcion por fallo en la alimentacion"));
		codigoError = ERR_FALLO_ALIMENTACION;
		registro.registrarLogSistema("INTERRUPCION POR FALLO EN LA ALIMENTACION");
		eventosJson.guardarLog(FALLO_ALIMENTACION_LOG);
		procesoCentral = ERROR;
		guardarFlagEE("CODIGO_ERROR", ERR_FALLO_ALIMENTACION);
		guardarEstadoInterrupcion();
	}

	void setEstadoErrorComprobarDatos(){
		Serial.println(F("Guardando datos "));
		estadoError = COMPROBAR_DATOS;
		guardarEstadoAlerta();
	}

	void setEstadoErrorEnviarAviso(){

		Serial.println(F("Comprobando datos "));
		estadoError = ENVIAR_AVISO;
		sleepModeGSM = GSM_ON;
		setMargenTiempo(tiempoMargen,TIEMPO_CARGA_GSM);
	}

	void setEstadoErrorRealizarLlamadas(){
		estadoError = REALIZAR_LLAMADAS;
		setMargenTiempo(tiempoMargen,240000);
	}

	void setEstadoErrorEsperarAyuda(){
		Serial.println(F("Esperar ayuda"));
		estadoError = ESPERAR_AYUDA;

		setMargenTiempo(prorrogaGSM, TIEMPO_PRORROGA_GSM, TIEMPO_PRORROGA_GSM_TEST);
		sleepModeGSM = GSM_TEMPORAL;

	}

	void procesoError(){

		switch(estadoError){

		case GUARDAR_DATOS:
			setEstadoErrorComprobarDatos();
			setEstadoReposo(); //Desactiva
			break;

		case COMPROBAR_DATOS:

			if(!isLcdInfo())
			pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			setEstadoErrorEnviarAviso();
			break;

		case ENVIAR_AVISO:

			if(!isLcdInfo())
				pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			if(checkearMargenTiempo(tiempoMargen)){
				mensaje.mensajeError(datosSensores);
				setEstadoErrorRealizarLlamadas();
			}
			desactivarEstadoDeError();

			break;

		case REALIZAR_LLAMADAS:

			if(!isLcdInfo())
				pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			realizarLlamadas();

			if(checkearMargenTiempo(tiempoMargen)){
				setEstadoErrorEsperarAyuda();
				guardarFlagEE("LLAMADA_EMERGEN", 1);
			}

			desactivarEstadoDeError();
			break;
		case ESPERAR_AYUDA:

			if(!isLcdInfo())
				pantalla.lcdLoadView(&pantalla, &Pantalla::errorEmergencia);

			desactivarEstadoDeError();
			break;
		}
	}

	void checkearSensorPuertaCochera(){
		alertsInfoLcd[INFO_SENSOR_PUERTA_OFF] = !configSystem.SENSORES_HABLITADOS[0];
	}

	void avisoLedPuertaCochera(){

		if(estadoAlarma != ESTADO_GUARDIA){
			mcp.digitalWrite(LED_COCHERA, LOW);
		}else{

			if(!checkearMargenTiempo(tiempoMargen)){
				if(!mcp.digitalRead(MG_SENSOR)){
					mcp.digitalWrite(LED_COCHERA, HIGH);
				}else{
					mcp.digitalWrite(LED_COCHERA, LOW);
				}

			}else{
				mcp.digitalWrite(LED_COCHERA, LOW);
			}

		}
	}

	void printSystemInfo(){

		Serial.print("INFORMACION DEL SISTEMA\n");
		Serial.print("\n");

		Serial.printf("MODO_SENSIBLE = %d\n", configSystem.MODO_SENSIBLE);
		Serial.printf("MODULO SD = %d\n", configSystem.MODULO_SD);
		Serial.printf("RTC = %d\n", configSystem.MODULO_RTC);
		Serial.printf("SMS HISTORICO = %d\n", configSystem.SMS_HISTORICO);
		Serial.printf("FECHA_SMS_HISTORICO = %s\n", configSystem.FECHA_SMS_HITORICO);

		Serial.print("\n");

		Serial.printf("SENSORES_HABILITADOS = {%d, %d, %d, %d}\n",
				  configSystem.SENSORES_HABLITADOS[0], configSystem.SENSORES_HABLITADOS[1],
				  configSystem.SENSORES_HABLITADOS[2], configSystem.SENSORES_HABLITADOS[3]);


		Serial.printf("DATOS SENSORES = {%d, %d, %d, %d}\n",
				eeDatosSalto.DATOS_SENSOR[0], eeDatosSalto.DATOS_SENSOR[1],
				eeDatosSalto.DATOS_SENSOR[2], eeDatosSalto.DATOS_SENSOR[3]);

		Serial.print("\n");

		Serial.printf("FLAG GUARDIA = %d\n", leerFlagEE("ESTADO_GUARDIA"));
		Serial.printf("FLAG ALERTA = %d\n", leerFlagEE("ESTADO_ALERTA"));
		Serial.printf("FLAG PUERTA ABIERTA = %d\n", leerFlagEE("PUERTA_ABIERTA"));
		Serial.printf("NUM SMS ENVIADOS = %d\n", leerFlagEE("N_SMS_ENVIADOS"));

		Serial.print("\n");

		Serial.printf("ERR INTERRUPT = %d\n", leerFlagEE("ERR_INTERRUPT"));
		Serial.printf("ERR HISTORICO INTERRUPCIONES = %d\n", leerFlagEE("INTERUP_HIST"));
		Serial.printf("ERR SMS EMERGENCIA ENVIADO = %d\n", leerFlagEE("MENSAJE_EMERGEN"));
	}

	//MANEJO DE MEMORIA NVS
	uint8_t leerFlagEE(const char* key) {

		NVSMemory.begin("SAA_DATA", false);
		uint8_t value = NVSMemory.getUChar(key);
		NVSMemory.end();

		return value;
	}

	int leerFlagEEInt(const char* key) {

			NVSMemory.begin("SAA_DATA", false);
			int value = NVSMemory.getInt(key);
			NVSMemory.end();

			return value;
	}


	void guardarFlagEE(const char* key, uint8_t value) {

		NVSMemory.begin("SAA_DATA", false);
		uint8_t previo = NVSMemory.getUChar(key);

		if(previo != value || value == NULL){
			NVSMemory.putUChar(key, value);
		}

		NVSMemory.end();
	}


	void guardarFlagEE(const char* key, int value) {

			NVSMemory.begin("SAA_DATA", false);
			int previo = NVSMemory.getUChar(key);

			if(previo != value || value == NULL){
				NVSMemory.putInt(key, value);
			}

			NVSMemory.end();
		}


	template <typename T> void NVS_SaveData(const char* key, T value) {

		NVSMemory.begin("SAA_DATA", false);
		NVSMemory.putBytes(key, &value, sizeof(value));
		NVSMemory.end();
	}

	template <typename T> T NVS_RestoreData(const char* key) {

	  NVSMemory.begin("SAA_DATA", false);
	  T value; //Struct generico

	  NVSMemory.getBytes(key, &value, sizeof(value));
	  NVSMemory.end();

	  // Devolvemos el struct leído
	  return value;
	}

#endif /* SOURCE_ALARMA_H_ */
