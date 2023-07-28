/*
 * Alarma.h
 *
 *  Created on: 3 ago. 2021
 *      Author: isrev
 */

#ifndef SOURCE_ALARMA_H_
#define SOURCE_ALARMA_H_

#define TINY_GSM_MODEM_SIM800 //Definimos el modem

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Preferences.h>
#include <Adafruit_MCP23X17.h>
#include <HardwareSerial.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

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
#include "ComunicacionLinea.h"
#include "Terminal.h"

//#include "MUXMCP23X17.h"
#include "RecursosCompartidosRTOS.h"


//VERSION (VE -> Version Estable VD -> Version Desarrollo)
const char* version[] = {"MUSIC VE21R0", "27/07/23"};

//RTOS
TaskHandle_t tareaLoopDos;

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

// Crea un objeto TinyGsm para comunicarse con el módulo SIM800L
TinyGsm modem(UART_GSM);
TinyGsmClient client(modem); // @suppress("Abstract class cannot be instantiated")
//TinyGsmClientSecure client(modem);
HttpClient http(client, serverUrl, portHttp); // @suppress("Abstract class cannot be instantiated")


//MUX
Adafruit_MCP23X17 mcp;

SemaphoreHandle_t semaphore;
//MUXMCP23X17 mcp(&semaphore);

RecursosCompartidosRTOS rcomp0(&semaphore,&mcp);
RecursosCompartidosRTOS rcomp1(&semaphore,&mcp);


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
SAAS_CRON_ENVIOS  saasCronEstado;


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
Mensajes mensaje(UART_GSM/*, &modem, &client*/);
Menu menu;
Fecha fecha;
Registro registro;
EventosJson eventosJson;
ComunicacionLinea linea;

//Terminales en linea
Terminal T_COCHERA = Terminal("COCHERA");
//Terminal T_PORCHE = Terminal("PORCHE");
//Terminal T_ALMACEN = Terminal("ALMACEN");

Terminal* T_LIST[] = { &T_COCHERA };

int flagTest = 1; //@TEST ONLY

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

 void pantallaDeError(String mensaje){
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
		rcomp1.digitalWrite(WATCHDOG, !rcomp1.digitalRead(WATCHDOG));
	}

	void sleepMode(){

		switch(sleepModeGSM){

		case GSM_ON:
			rcomp1.digitalWrite(GSM_PIN, HIGH);
			break;

		case GSM_OFF:
			rcomp1.digitalWrite(GSM_PIN, LOW);
			break;

		case GSM_TEMPORAL:

			if(checkearMargenTiempo(prorrogaGSM)){
				rcomp1.digitalWrite(GSM_PIN, LOW);
			}else {
				rcomp1.digitalWrite(GSM_PIN, HIGH);
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
			rcomp1.digitalWrite(RESETEAR, HIGH);
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


		if (leerFlagEEInt("ESTADO_GUARDIA") == 1 || leerFlagEEInt("ERR_INTERRUPT") == 0) {
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
		alertsInfoLcd[INFO_FALLO_BATERIA] = !rcomp1.digitalRead(SENSOR_BATERIA_RESPALDO);

	    if(rcomp1.digitalRead(SENSOR_BATERIA_RESPALDO) != sensorBateriaAnterior){

			if(rcomp1.digitalRead(SENSOR_BATERIA_RESPALDO) == LOW){
				registro.registrarLogSistema("BATERIA DE EMERGENCIA ACTIVADA");
				eventosJson.guardarLog(BATERIA_EMERGENCIA_ACTIVADA_LOG);
			} else{
				registro.registrarLogSistema("BATERIA DE EMERGENCIA DESACTIVADA");
				eventosJson.guardarLog(BATERIA_EMERGENCIA_DESACTIVADA_LOG);
			}
		}

	    sensorBateriaAnterior = rcomp1.digitalRead(SENSOR_BATERIA_RESPALDO);
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
				//sleepModeGSM = GSM_TEMPORAL;
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
		//sleepModeGSM = GSM_TEMPORAL;

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
			rcomp1.digitalWrite(LED_COCHERA, LOW);
		}else{

			if(!checkearMargenTiempo(tiempoMargen)){
				if(!rcomp1.digitalRead(MG_SENSOR)){
					rcomp1.digitalWrite(LED_COCHERA, HIGH);
				}else{
					rcomp1.digitalWrite(LED_COCHERA, LOW);
				}

			}else{
				rcomp1.digitalWrite(LED_COCHERA, LOW);
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

	void escucharGSM(){

		if (UART_GSM.available() > 0) { // Verificamos si hay datos disponibles para leer

			char tramaRecibida[200] = "";
			size_t byteCount = UART_GSM.readBytesUntil('\n', tramaRecibida, sizeof(tramaRecibida) - 1); //read in data to buffer
			tramaRecibida[byteCount] = NULL;	//put an end character on the data


			Serial.print("UART@GSM-> ");
			Serial.println(tramaRecibida);
			UART_GSM.flush();

		}

	}

	int coberturaRed() {
	  static unsigned long tiempoAnterior = 0; // Variable estática para almacenar el tiempo de la última ejecución

	  unsigned long tiempoActual = millis(); // Obtener el tiempo actual desde que se encendió Arduino

	  // Verificar si han pasado al menos 10 segundos desde la última ejecución
	  if (tiempoActual - tiempoAnterior >= 10000) { // 10000 milisegundos = 10 segundos
	    tiempoAnterior = tiempoActual; // Actualizar el tiempo de la última ejecución
	    return modem.getSignalQuality();
	  }

	  // Si no han pasado 10 segundos, no ejecutar la función y devolver un valor inválido (por ejemplo, -1)
	  return -1;
	}

	void comprobarConexionGSM(){
		Serial.print("Waiting for network...");
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGSM);
		if (!modem.waitForNetwork()) {
			Serial.println(" fail");
			pantallaDeError(F("  SYSTM ERROR!   SIN RED MOVIL  "));
			return;
		}
		Serial.println(" success");
		if (modem.isNetworkConnected()) { Serial.println("Network connected"); }


		String operatorName = modem.getOperator();
		Serial.print("Proveedor de servicios: ");
		Serial.println(operatorName);

		int csq = modem.getSignalQuality();
		Serial.println("Signal quality: " + String(csq));

		pantallaDeError(fixedLengthString(operatorName, 16)+"Calidad red:"+(csq));

	}

	bool establecerConexionGPRS(){
		// Intenta conectar al servicio de red GPRS
		Serial.println("Conectando a la red GPRS...");
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprs);

		if (!modem.gprsConnect(apn, gsmUser, gsmPass)) {
			Serial.println(" Error al conectar a la red GPRS.");
			pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprsFail);
			return false;
		}
		Serial.println(" Conexion a la red GPRS establecida.");
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprsOk);

		return true;
	}

	void cerrarConexionGPRS(){
		modem.gprsDisconnect();
		Serial.println(F("GPRS disconnected"));
	}

	void checkearEnvioSaas(){

		static unsigned long lastExecutionTime = 0;
		static int retryCount = 0;
		byte executionResult;


		switch (saasCronEstado) {
		case ESPERA_ENVIO:
			if (millis() - lastExecutionTime >= 15000) { //600000
				saasCronEstado = ENVIO;
				retryCount = 0; // Reset retry count when starting a new execution
			}
			break;

		case ENVIO:
			Serial.println(F("Enviando datos al servidor"));
			executionResult = eventosJson.enviarInformeSaas();
			if (executionResult == 1) {
				saasCronEstado = ESPERA_ENVIO;
				lastExecutionTime = millis();
			} else {
				Serial.println("Reintento");
				lastExecutionTime = millis();
				saasCronEstado = ESPERA_REINTENTO;
			}
			break;

		case ESPERA_REINTENTO:
			if (retryCount < 2) {
				if (millis() - lastExecutionTime >= 6000) { //120000
					saasCronEstado = ENVIO;
					retryCount++;
				}
			} else {
				Serial.println("Demasiados reintentos se espera al siguente ciclo");
				lastExecutionTime = millis();
				saasCronEstado = ESPERA_ENVIO;
			}
			break;

		}
	}

	RespuestaHttp realizarPeticionHttp(const char* metodo, const char* resource, byte auth = 1, const char* jsonData = nullptr){

		RespuestaHttp respuesta;

		/*TEST*/
		respuesta.codigo= 200;
		respuesta.respuesta = "OK";// "Id de paquete duplicado";
		return respuesta;
		/*TEST*/

		if(establecerConexionGPRS()){
			int estadoHttp = 0;
			String respuestaHttp;

			//Formular peticion HTTP
			//http.connectionKeepAlive();  // Currently, this is needed for HTTPS
			http.beginRequest();

			if (strcmp(metodo, "GET") == 0) {
				estadoHttp = http.get(resource);
			} else if (strcmp(metodo, "POST") == 0) {
				if (jsonData) {
					estadoHttp= http.post(resource, "application/json", jsonData);
					http.sendHeader("Content-Type", "application/json");
					http.sendHeader("Content-Length", strlen(jsonData));
				} else {
					estadoHttp = http.post(resource);
				}
			}
			if(auth){
				Serial.println("Autorizando peticion...");
				String SAAS_TOKEN = leerCadenaEE("SAAS_TOKEN");
				Serial.println(SAAS_TOKEN);
				http.sendHeader("Authorization", String("Bearer ") + SAAS_TOKEN);
			}

			http.endRequest();

			//Gestionar respuesta HTTP
			respuesta.codigo = http.responseStatusCode();
			Serial.print(F("Response status code: "));
			Serial.println(respuesta.codigo);

			if (estadoHttp != 0 || respuesta.codigo <= 0) {
				Serial.println(F("failed to connect"));
				pantallaDeError(F("  SYSTM ERROR!  PETICION HTTP KO"));
				respuestaHttp += "Fallo en la peticion HTTP error API: "
						+ String(estadoHttp)
						+ " error HTTP:"
						+ String(respuesta.codigo) +"\n";
			}else {
				respuestaHttp += "Codigo respuesta servidor: " + String(respuesta.codigo) + "\n";

				byte linesCount = 0;
				Serial.println(F("Response Headers:"));
				while (http.headerAvailable() && linesCount < 15) {
					String headerName  = http.readHeaderName();
					String headerValue = http.readHeaderValue();
					Serial.println("    " + headerName + " : " + headerValue);
					respuestaHttp += "    " + headerName + " : " + headerValue + "\n";

				}

				int length = http.contentLength();

				if (length >= 0) {
					Serial.print(F("Content length is: "));
					Serial.println(length);
					respuestaHttp += "Content length :"+ (String)length + "\n";
				}

				if (http.isResponseChunked()) {
					Serial.println(F("The response is chunked"));
					respuestaHttp += "The response is chunked\n";
				}

				String body = http.responseBody();
				Serial.println(F("Response:"));
				Serial.println(body);

				//Comprobamos si la respuesta es plana o json
				DynamicJsonDocument respuetaJson(100);

				if (body.charAt(0) == '{' && body.charAt(body.length() - 1) == '}') {

					Serial.println("Respuesta JSON");
					DeserializationError error = deserializeJson(respuetaJson, body);

					if(!error){
						String msgJson = respuetaJson["msg"];
						String errorJson  = respuetaJson["error"];

						respuesta.respuesta = (respuesta.codigo == 200) ? msgJson : errorJson;
						respuestaHttp += "Respuesta servidor: " + String(respuesta.respuesta) + "\n";

					}else {
						Serial.println("Error deserializando JSON");
						respuestaHttp += "Respuesta servidor: Error saa durante la extraccion JSON\n";
					}

				} else {
					Serial.println("Respuesta NO JSON");
					respuesta.respuesta = body;
					respuestaHttp += "Respuesta servidor: " + String(respuesta.respuesta) + "\n";

				}

				Serial.print(F("Body length is: "));
				Serial.println(body.length());
				respuestaHttp += "Body length is: " + String(body.length()) + "\n";

				http.stop(); // Shutdown
				Serial.println(F("Server disconnected"));
			}

			registro.registrarLogHttpRequest(&respuestaHttp);
			cerrarConexionGPRS();
		}
		return respuesta;
	}


	int getIdPaqueteSaas(){
		RespuestaHttp respuesta;
	    respuesta = realizarPeticionHttp("GET", getUltimoPaquete);

	    Serial.print("Codigo: ");
	    Serial.println(respuesta.codigo);
	    Serial.print("Contenido: ");
	    Serial.println(respuesta.respuesta);

	    if(respuesta.codigo == 200){
	    	int id = respuesta.respuesta.toInt();
	    	guardarFlagEE("PACKAGE_ID", id);
	    }

	    return respuesta.codigo;
	}

	int generarTokenSaas(){
		RespuestaHttp respuesta;
		respuesta = realizarPeticionHttp("POST", postTokenSanctum, 0);

	    Serial.print("Codigo: ");
	    Serial.println(respuesta.codigo);
	    Serial.print("Contenido: ");
	    Serial.println(respuesta.respuesta);

		if(respuesta.codigo == 200){
			guardarCadenaEE("SAAS_TOKEN", &respuesta.respuesta);
		}

		return respuesta.codigo;
	}



	RespuestaHttp postPaqueteSaas(String* modeloJson){
		RespuestaHttp respuesta;
		const char* jsonData = modeloJson->c_str();
		respuesta = realizarPeticionHttp("POST", postEventosJson, 1, jsonData);

	    Serial.print("Codigo: ");
	    Serial.println(respuesta.codigo);
	    Serial.print("Contenido: ");
	    Serial.println(respuesta.respuesta);

		return respuesta;
	}

	String fixedLengthString(String& original, size_t fixedLength) {
		if (original.length() < fixedLength) {
			byte espacios = fixedLength - original.length();
			for (int i = 0;   i < espacios; ++  i) {
				original += " ";
			}
			return original;
		}
		else {
			return original.substring(0, fixedLength);
		}
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


	String leerCadenaEE(const char* key) {
		NVSMemory.begin("SAA_DATA", false);
		String value = NVSMemory.getString(key);
		NVSMemory.end();

		return value;
	}


	void guardarCadenaEE(const char* key, String* value) {

			NVSMemory.begin("SAA_DATA", false);
			String previo = NVSMemory.getString(key);

			if(previo.isEmpty() || previo != *value){
				NVSMemory.putString(key, *value);
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
