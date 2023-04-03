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
#include "PCF8575.h"


#include "Autenticacion.h"
#include "Pantalla.h"
#include "ComandoSerie.h"
#include "InterStrike.h"
#include "Datos.h"
#include "Bocina.h"
#include "Teclado.h"
#include "Env.h"
#include "Macros.h"


/*

#include <Wire.h>
#include <SoftwareSerial.h>

#include "Mensajes.h"
#include "Menu.h"
#include "Registro.h"
#include "Fecha.h"

*/


//VERSION (VE -> Version Estable VD -> Version Desarrollo)
const char* version[] = {"MUSIC VE21R0", "02/04/23"};

//VARIABLES GLOBALES
ConfigSystem configSystem;
EE_DatosSalto eeDatosSalto;

byte MODO_DEFAULT = 0;  //@develop
byte INTENTOS_REACTIVACION = 0;

//SENSORES
byte sensorHabilitado[4] = {1,1,1,1};

//INSTANCIAS
PCF8575 pcf8575(0x27); //Multiplexor
Preferences NVSMemory; //Memoria

ProcesoCentral procesoCentral;
EstadosAlarma estadoAlarma;
EstadosError estadoError;
SLEEPMODE_GSM sleepModeGSM;
LLAMADAS_GSM estadoLlamada;
CODIGO_ERROR codigoError;

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


/*
SoftwareSerial SIM800L(GSM_TX,GSM_RX);
Mensajes mensaje;
Menu menu;
Registro registro;
Fecha fecha;
*/

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
					//EEPROM.update(EE_ERROR_INTERRUPCION,0);
					//EEPROM.update(EE_MENSAJE_EMERGENCIA,0);
					//EEPROM.update(EE_LLAMADA_EMERGENCIA,0);
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
		pcf8575.digitalWrite(WATCHDOG, !pcf8575.digitalRead(WATCHDOG));
	}

	void sleepMode(){

		switch(sleepModeGSM){

		case GSM_ON:

			pcf8575.digitalWrite(GSM_PIN, HIGH);
			break;

		case GSM_OFF:

			pcf8575.digitalWrite(GSM_PIN, LOW);
			break;

		case GSM_TEMPORAL:

			if(checkearMargenTiempo(prorrogaGSM)){
				pcf8575.digitalWrite(GSM_PIN, LOW);
			}else {
				pcf8575.digitalWrite(GSM_PIN, HIGH);
			}
			break;
		}

	}

	void checkearSms(){
		if(!configSystem.MODULO_RTC){
			return;
		}
/*
		if(fecha.comprobarHora(0, 0)){
			if(EEPROM.read(MENSAJES_ENVIADOS) != 0){ @PEND
				EEPROM.write(MENSAJES_ENVIADOS,0);
				insertQuery(&sqlIntentosRecuperados);
				Serial.println(F("Intentos diarios recuperados"));
			}
		}
*/
	}

	void resetear(){
			Serial.println(F("\nReseteando"));
			//insertQuery(&sqlReset);
			delay(200);
			pcf8575.digitalWrite(RESETEAR, HIGH);
		}

	void resetAutomatico(){

		if(!configSystem.MODULO_RTC){
			return;
		}
/*
		if(fecha.comprobarFecha(fecha.getFechaReset())){

			alertsInfoLcd[INFO_RESET_AUTO] = 1;

			if(fecha.comprobarHora(16, 30)){
				Serial.println(F("\nReset programado"));
				resetear();
			}
		}else {
			alertsInfoLcd[INFO_RESET_AUTO] = 0;
		}*/
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
		   flagPuertaAbierta = leerFlagEE("PUERTA_ABIERTA") == 1;


		if (leerFlagEE("ESTADO_GUARDIA") == 1 && leerFlagEE("ERR_INTERRUPT") == 0) {
			estadoAlarma = ESTADO_GUARDIA;
			//insertQuery(&sqlUpdateEntradaRestaurada);
		}
	}

	void checkearAlertasDetenidas(){
		if (leerFlagEE("ESTADO_ALERTA") == 1 && leerFlagEE("ERR_INTERRUPT") == 0) {

			//EEPROM_RestoreData(EE_DATOS_SALTOS, eeDatosSalto);
			eeDatosSalto = NVS_RestoreData<datos_saltos_t>("SALTO_DATA");

			int* datos = datosSensores.getDatos();
			arrCopy<int>(eeDatosSalto.DATOS_SENSOR,datos ,TOTAL_SENSORES); //Carga los datos EE @PEND
			zona = eeDatosSalto.ZONA;
			INTENTOS_REACTIVACION = eeDatosSalto.INTENTOS_REACTIVACION;

			//insertQuery(&sqlUpdateSaltoRestaurado);

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
		//EEPROM_SaveData(EE_DATOS_SALTOS, eeDatosSalto);
	}

	void guardarEstadoInterrupcion(){

		//EEPROM.update(EE_ERROR_INTERRUPCION,1);
		guardarFlagEE("ERR_INTERRUPT", 1);

		//EEPROM.update(EE_INTERRUPCIONES_HISTORICO, (EEPROM.read(EE_INTERRUPCIONES_HISTORICO)+1));
		guardarFlagEE("INTERUP_HIST", (leerFlagEE("INTERUP_HIST") + 1));

		configSystem.MODULO_RTC = 0;
		//EEPROM_SaveData(EE_CONFIG_STRUCT, configSystem); //Apagar RTC durante las interrupciones @PEND
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	void checkearBateriaDeEmergencia(){

		alertsInfoLcd[INFO_FALLO_BATERIA] = !digitalRead(SENSOR_BATERIA_RESPALDO);

		if(digitalRead(SENSOR_BATERIA_RESPALDO) != sensorBateriaAnterior){

			if(digitalRead(SENSOR_BATERIA_RESPALDO) == HIGH){
				//insertQuery(&sqlBateriaEmergenciaActivada);
			} else{
				//insertQuery(&sqlBateriaEmergenciaDesactivada);
			}
		}

		sensorBateriaAnterior = digitalRead(SENSOR_BATERIA_RESPALDO);
	}

	void realizarLlamadas(){

		if(!MODO_DEFAULT)
			return;

		static byte estadoAnterior;

		switch(estadoLlamada){

		case TLF_1:

			if(millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.95) && millis() < tiempoMargen - (TIEMPO_REACTIVACION*0.90)){
				//mensaje.llamarTlf((char*)telefonoLlamada_1); @PEND
				//insertQuery(&sqlLlamadas);
				estadoLlamada = COLGAR;
				estadoAnterior = TLF_1;
			}

			break;

		case TLF_2:

			if(millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.80)){
				//mensaje.llamarTlf((char*)telefonoLlamada_2); @PEND
				//insertQuery(&sqlLlamadas);
				estadoLlamada = COLGAR;
				estadoAnterior = TLF_2;
			}

			break;

		case COLGAR:

			if((millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.85))){
				if(estadoAnterior == TLF_1){
					//mensaje.colgarLlamada(); @PEND
					estadoLlamada = TLF_2;
				}
			}

			if((millis() > tiempoMargen - (TIEMPO_REACTIVACION*0.70))){
				if(estadoAnterior == TLF_2){
					//mensaje.colgarLlamada(); @PEND
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

				//EEPROM_RestoreData(EE_DATOS_SALTOS, eeDatosSalto);
				eeDatosSalto = NVS_RestoreData<datos_saltos_t>("SALTO_DATA");
				//int* datos = datosSensores.getDatos(); @PEND
				//arrCopy<int>(eeDatosSalto.DATOS_SENSOR,datos ,TOTAL_SENSORES); @PEND

				estadoError = COMPROBAR_DATOS;
				sleepModeGSM = GSM_TEMPORAL;
			}
		}
	}

	void interrupcionFalloAlimentacion(){
		Serial.println(F("\nInterrupcion por fallo en la alimentacion"));
		codigoError = ERR_FALLO_ALIMENTACION;
		//insertQuery(&sqlError);
		procesoCentral = ERROR;
		//EEPROM.update(EE_CODIGO_ERROR, ERR_FALLO_ALIMENTACION);
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
			//insertQuery(&sqlUpdateEntradaModoAuto);
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
				//mensaje.mensajeError(datosSensores); @PEND
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
				//EEPROM.update(EE_LLAMADA_EMERGENCIA, 1);
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
			pcf8575.digitalWrite(LED_COCHERA, LOW);
		}else{

			if(!checkearMargenTiempo(tiempoMargen)){

				if(!pcf8575.digitalRead(MG_SENSOR)){
					pcf8575.digitalWrite(LED_COCHERA, HIGH);
				}else{
					pcf8575.digitalWrite(LED_COCHERA, LOW);
				}

			}else{
				pcf8575.digitalWrite(LED_COCHERA, LOW);
			}

		}
	}

	//MANEJO DE MEMORIA NVS
	uint8_t leerFlagEE(const char* key) {

		NVSMemory.begin("SAA_DATA", false);
		uint8_t value = NVSMemory.getUChar(key);
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
