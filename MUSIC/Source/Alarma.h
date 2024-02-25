/*
 * Alarma.h
 *
 *  Created on: 3 ago. 2021
 *      Author: isrev
 */

#ifndef SOURCE_ALARMA_H_
#define SOURCE_ALARMA_H_

#define TINY_GSM_MODEM_SIM7600 //Definimos el modem

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Preferences.h>
#include <Adafruit_MCP23X17.h>
#include <HardwareSerial.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <AESLib.h>
#include "base64.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

#include "Autenticacion.h"
#include "Pantalla.h"
#include "ComandoSerie.h"
#include "Bocina.h"
#include "AlarmaUtil/Teclado.h"
#include "Env.h"
#include "AlarmaUtil/Macros.h"
#include "Mensajes.h"
#include "Menu.h"
#include "Fecha.h"
#include "Registro.h"
#include "EventosJson.h"
#include "ComunicacionLinea.h"
#include "Terminal.h"

//TEST WIFI
#ifdef WIFI_PUERTO_SERIE
AsyncWebServer serverDos(80);

void recvMsg(uint8_t *data, size_t len){
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
}
#endif
//TEST WIFI

//VERSION (VE -> Version Estable VD -> Version Desarrollo)
const char* version[] = {"MUSIC VE21R1", "25/02/24"};

//RTOS
TaskHandle_t gestionLinea;
TaskHandle_t envioServidorSaas = NULL;
TaskHandle_t envioNotificacionSaas = NULL;
TaskHandle_t envioFtpSaas = NULL;
//Parametros para tareas
NotificacionSaas datosNotificacionSaas;

//Flags resultado tareas
byte resultadoEnvioServidorSaas;
byte resultadoEnvioNotificacionSaas;
byte resultadoEnvioFtpSaas;

byte accesoGestorPila = 1; //Abre y cierra el gestor
byte accesoAlmacenamientoSD = 1; //Abre y cierra en acceso a los logs del sistema
byte paradaDeEmergenciaPila = 0;
PilaTareaEstado estadoPila; //Aun no se usa para nada

//Cola para registro en tareas
QueueHandle_t colaRegistros;

//VARIABLES GLOBALES
ConfigSystem configSystem;
EE_DatosSalto eeDatosSalto;

#ifdef ALARMA_EN_MODO_DEBUG
byte MODO_DEFAULT = 0;  //@develop
#else
byte MODO_DEFAULT = 1;
#endif

byte INTENTOS_REACTIVACION = 0;
byte SD_STATUS = 0; //Comprueba si la escritura en SD esta OK

//SENSORES
byte sensorHabilitado[4] = {1,1,1,1};

//INSTANCIAS

//Cifrado AES
AESLib aesLib;

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


//NVS
Preferences NVSMemory; //Memoria

//TYPE DEF
ProcesoCentral procesoCentral;
EstadosAlarma estadoAlarma;
EstadosError estadoError;
LLAMADAS_GSM estadoLlamada;
CODIGO_ERROR codigoError;
SAAS_LITERAL_LOGS saaLiteralLogs;
SAAS_CRON_ENVIOS  saasCronEstado;
InterStrikeCore sensorCore;

//CLASES
Autenticacion auth;
Pantalla pantalla;
ComandoSerie demonio;
Bocina bocina;
Mensajes mensaje(UART_GSM);
Menu menu;
Fecha fecha;
Registro registro;
EventosJson eventosJson;
ComunicacionLinea linea;

byte ACCESO_LISTAS = 1;

//Terminales en linea
int tCoreMapeo[] = {3, 2, 4};
Terminal T_CORE = Terminal(0,"TC", 0,0,3, tCoreMapeo);

Terminal T_COCHERA = Terminal(1,"CH");
//Terminal T_PORCHE = Terminal(2,"PC");
//Terminal T_ALMACEN = Terminal(3,"AL");

Terminal* T_LIST[] = { &T_CORE, /*&T_COCHERA, &T_ALMACEN*/ };

//Terminal check
RespuestaTerminal respuestaTerminal;

const char* literalesZonas[2][MAX_DATOS_SUB_TRAMA] = {
    {"COCHERA", "PORCHE", "ALMACEN", "PUERTA COCHERA", "", "", "", ""},
    {"PORCHE A", "PORCHE B", "PORCHE C", "PATIO NORTE", "PATIO SUR", "TENDEDERO", "PUERTA A", "PUERTA B"}
};

//TIEMPOS MARGEN

const unsigned long TIEMPO_OFF = 120000; // (*0.1666) -> 20000 (*0.1666) -> 20000 sensible
const unsigned long TIEMPO_ON = 600000; //(*0.01666) -> 10000 (*0.01) -> 6000 en auto activacion
const unsigned long TIEMPO_REACTIVACION = 240000; // (*0.1) ->  24000
const unsigned long TIEMPO_MODO_SENSIBLE = 3600000; // (*0.0166)  -> 60000*
const unsigned long TIEMPO_BOCINA = 600000; // (*0.0333) -> 20000* //300000(*0.0666) ->20000
const unsigned long TIEMPO_PRORROGA_GSM = 1200000; // (*0.05) -> 60000
const unsigned long TIEMPO_CARGA_GSM = 10000;

const unsigned long TIEMPO_MAX_TAREA = 90000;
const unsigned long TIEMPO_MAX_TAREA_FTP = 300000; //5 min
const unsigned long TIEMPO_ESPERA_REINTENTO_TAREA = 25000;
const unsigned long TIEMPO_REINCIO_PILA = 20000;

unsigned long tiempoMargen;

//TIEMPO MODO SENSIBLE
unsigned long tiempoSensible;
//TIEMPO SLEEPMODE
unsigned long tiempoRefrescoGSM;
//TIEMPO BOCINA
unsigned long tiempoBocina;
//TIEMPO DE EJECUCION DE TAREAS
unsigned long tiempoTareaEnEjecucion;
unsigned long tiempoReinicioPila;


//TIEMPO CLAVE
unsigned long lcd_clave_tiempo;
//LCD ALERTS
byte alertsInfoLcd[NUMERO_ALERTAS];
static byte alertInfoCached[NUMERO_ALERTAS];

unsigned long lcd_info_tiempo;
static byte tiempoFracccion;

//DATOS SMS
 byte desactivaciones = 0;
 byte mensajesEnviados = 0;

 //Control bateria
 bool sensorBateriaAnterior; //Compara el estado de la bateria

 //FLAG PUERTA
 byte flagPuertaAbierta = 0;
//FLAG ALERTA RESTAURADA
 byte flagAlertaRestaurada = 0;

//ENVIOS FALLIDOS
 byte enviosHttpFallidos = 0;


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



	void watchDog(){
		mcp.digitalWrite(WATCHDOG, !mcp.digitalRead(WATCHDOG));
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

			mcp.digitalWrite(GSM_PIN, LOW); //Reseteo el modulo GSM
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


	void limpiarEstadoAlerta(){
		if(!flagAlertaRestaurada){
			return;
		}

		char temp[2048];
		strcpy(eeDatosSalto.LISTADOS_TERMINALES.terminalCoreJson, temp);
		NVS_SaveData<datos_saltos_t>("SALTO_DATA", eeDatosSalto);

		flagAlertaRestaurada = 0;
	}


	void realizarLlamadas(){

		#ifdef ALARMA_EN_MODO_DEBUG
			if(MODO_DEFAULT) //@develop !MODO_DEFAULT
				return;
		#else
			if(!MODO_DEFAULT)
				return;
		#endif

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


	void comprobarSensoresCore(){

		bool sensor;
		byte valoresSensoresCore[MAX_DATOS_SUB_TRAMA] = {0};
		byte ctlLineas[2] = {0};


		//Comprobamos sensor puerta
		if(configSystem.SENSORES_HABLITADOS[0]){
			sensor = mcp.digitalRead(sensorCore.dirSensor[0]);
			sensorCore.sensorMG = sensor;
#ifdef ALARMA_EN_MODO_DEBUG
			    if (sensor == HIGH && !sensorCore.notificadoMG) { // @develop("Cambiado a HIGH para evitar saltos en sensor MG")
#else
				if (sensor == LOW && !sensorCore.notificadoMG) {
#endif
					Serial.println("Puerta abierta");
					registro.registrarLogSistema("DETECCION APERTURA DE PUERTA");
					eventosJson.guardarDeteccion(1,
							1,
							(estadoAlarma ==ESTADO_GUARDIA)? P_MODO_NORMAL: P_MODO_PHANTOM,
									0, //id terminal core
									3, //id sensor
									(configSystem.SENSORES_HABLITADOS[0]? P_ESTADO_ONLINE : P_ESTADO_OFFLINE)
					);

					sensorCore.notificadoMG = 1;
				}
			}

		//Comprobamos sensores pir
		for (int i = 1; i < 4; ++i) {

			sensor = mcp.digitalRead(sensorCore.dirSensor[i]);

			if (sensor && !sensorCore.pirSensorAnt[i]) {
				if(configSystem.SENSORES_HABLITADOS[i]){
					valoresSensoresCore[i-1] = 1;
					Serial.print("Movimiento detectado ");
					Serial.println(i);
					Serial.print("Lectura: ");
					Serial.println(sensor);
					Serial.print("Lectura Anterior: ");
					Serial.println(sensorCore.pirSensorAnt[i]);
				}else {
					Serial.print("Sensor: ");
					Serial.print(i);
					Serial.print("deshabilitado");
				}

			}

			sensorCore.pirSensorAnt[i] = sensor;

		}

		if(arrSum<byte>(valoresSensoresCore, MAX_DATOS_SUB_TRAMA)){

			Serial.println("Enviado al terminal:");

			for (int i = 0; i < MAX_DATOS_SUB_TRAMA; i++) {
				Serial.print(valoresSensoresCore[i]);
				Serial.print(",");

			}
			Serial.println("\n");

			T_CORE.guardarDatosTerminal(valoresSensoresCore, ctlLineas);
			//Serial.println(T_CORE.generarInformeDatos());
		}

	}

	void limpiarTerminalesLinea(){
		for (int i = 0; i < N_TERMINALES_LINEA; i++) {
			T_LIST[i]->limpiarDatosTerminal();
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

		Serial.print("\nEnvios al servidor SAAS\n");
		Serial.printf("ENVIO DE MODELO PERIODICO = %d\n", configSystem.ENVIO_SAAS);
		Serial.printf("ENVIO DE NOTIFICACIONES = %d\n", configSystem.ENVIO_SAAS_NOTIFICACION);
		Serial.printf("ENVIO FICHEROS POR FTP = %d\n", configSystem.ENVIO_FTP);

		Serial.print("\n");

		Serial.printf("SENSORES_HABILITADOS = {%d, %d, %d, %d}\n",
				  configSystem.SENSORES_HABLITADOS[0], configSystem.SENSORES_HABLITADOS[1],
				  configSystem.SENSORES_HABLITADOS[2], configSystem.SENSORES_HABLITADOS[3]);

		Serial.print("\n");

		Serial.printf("FLAG GUARDIA = %d\n", leerFlagEEInt("ESTADO_GUARDIA"));
		Serial.printf("FLAG ALERTA = %d\n", leerFlagEEInt("ESTADO_ALERTA"));
		Serial.printf("FLAG PUERTA ABIERTA = %d\n", leerFlagEEInt("PUERTA_ABIERTA"));
		Serial.printf("NUM SMS ENVIADOS = %d\n", leerFlagEEInt("N_SMS_ENVIADOS"));
		Serial.printf("NUM TOQUES BOCINA = %d\n", leerFlagEEInt("TICKET_BOCINA"));

		Serial.printf("NUM NOT SYS ENVIADOS = %d\n", leerFlagEEInt("N_SYS_SEND"));
		Serial.printf("NUM NOT ALR ENVIADOS = %d\n", leerFlagEEInt("N_ALR_SEND"));
		Serial.printf("NUM MODELOS ENVIADOS = %d\n", leerFlagEEInt("N_MOD_SEND"));
		Serial.printf("NUM ENVIOS FTP DIARIOS = %d\n", leerFlagEEInt("FTP_DIARIO"));

		Serial.print("\n");

		Serial.printf("ERR INTERRUPT = %d\n", leerFlagEEInt("ERR_INTERRUPT"));
		Serial.printf("ERR HISTORICO INTERRUPCIONES = %d\n", leerFlagEEInt("INTERUP_HIST"));
		Serial.printf("ERR SMS EMERGENCIA ENVIADO = %d\n", leerFlagEEInt("MENSAJE_EMERGEN"));
	}


	void interrupcionFalloAlimentacion(){
#ifdef WIFI_PUERTO_SERIE
		WebSerial.println(F("\nInterrupcion por fallo en la alimentacion"));
#endif
		codigoError = ERR_FALLO_ALIMENTACION;
		registro.registrarLogSistema("INTERRUPCION POR FALLO EN LA ALIMENTACION");
		eventosJson.guardarLog(FALLO_ALIMENTACION_LOG);

		procesoCentral = ERROR;
		guardarFlagEE("CODIGO_ERROR", ERR_FALLO_ALIMENTACION);
		guardarEstadoInterrupcion();
	}

#include "AlarmaUtil/Utiles.h"
#include "AlarmaUtil/PilaTareas.h"
#include "AlarmaUtil/Ftp.h"
#include "AlarmaUtil/HttpRequest.h"
#include "AlarmaUtil/Restauracion.h"
#include "AlarmaUtil/ProcesoError.h"
#include "AlarmaUtil/Checker.h"


#endif /* SOURCE_ALARMA_H_ */
