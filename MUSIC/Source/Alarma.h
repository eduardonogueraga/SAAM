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
#include <Ticker.h>

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


//VERSION (VE -> Version Estable VD -> Version Desarrollo)
const char* version[] = {"MUSIC VE21R0", "13/08/23"};

//RTOS
TaskHandle_t gestionLinea;
TaskHandle_t envioServidorSaas = NULL;
TaskHandle_t envioNotificacionSaas = NULL;
//Parametros para tareas
NotificacionSaas datosNotificacionSaas;

//Flags resultado tareas
byte resultadoEnvioServidorSaas;
byte resultadoEnvioNotificacionSaas;

byte accesoGestorPila = 1; //Abre y cierra el gestor
PilaTareaEstado estadoPila; //Aun no se usa para nada

//Cola para registro en tareas
QueueHandle_t colaRegistros;


#include "PilaTareas.h"

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

byte ACCESO_LISTAS = 1;

//Terminales en linea
Terminal T_COCHERA = Terminal(1,"CH");
Terminal T_PORCHE = Terminal(2,"PC");
//Terminal T_ALMACEN = Terminal("ALMACEN");

Terminal* T_LIST[] = { &T_COCHERA };

//Terminal check
RespuestaTerminal respuestaTerminal;

int flagTest = 1; //@TEST ONLY

//TIEMPOS MARGEN

const unsigned long TIEMPO_OFF = 120000; // (*0.1666) -> 20000 (*0.1666) -> 20000 sensible
const unsigned long TIEMPO_ON = 600000; //(*0.01666) -> 10000 (*0.01) -> 6000 en auto activacion
const unsigned long TIEMPO_REACTIVACION = 240000; // (*0.1) ->  24000
const unsigned long TIEMPO_MODO_SENSIBLE = 3600000; // (*0.0166)  -> 60000*
const unsigned long TIEMPO_BOCINA = 600000; // (*0.0333) -> 20000* //300000(*0.0666) ->20000
const unsigned long TIEMPO_PRORROGA_GSM = 1200000; // (*0.05) -> 60000
const unsigned short TIEMPO_CARGA_GSM = 10000;

const unsigned short TIEMPO_MAX_TAREA = 90000;
const unsigned short TIEMPO_ESPERA_REINTENTO_TAREA = 20000;
const unsigned short TIEMPO_REINCIO_PILA = 20000;

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
 Ticker blinker;
 void blinkTEST() {
	 Serial.println(F("\nBlinker")); //@TEST

   digitalWrite(TEST_PIN_RS, !digitalRead(TEST_PIN_RS));

   Serial.println(digitalRead(TEST_PIN_RS));

 }

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

		case GSM_REFRESH:

			if(checkearMargenTiempo(tiempoRefrescoGSM)){
				sleepModeGSM = GSM_ON;
				mcp.digitalWrite(GSM_PIN, HIGH);
			}else {
				mcp.digitalWrite(GSM_PIN, LOW);
			}

			break;
		}

	}

	void refrescarModuloGSM(){
		setMargenTiempo(tiempoRefrescoGSM, 1000);
		sleepModeGSM = GSM_REFRESH;
	}

	void checkearLimitesEnvios(){
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

			if(leerFlagEEInt("N_ALR_SEND") != 0){
				guardarFlagEE("N_ALR_SEND", 0);
				registro.registrarLogSistema("INTENTOS NOTIFICACION ALARMA DIARIAS RECUPERADAS");
				eventosJson.guardarLog(INTENTOS_NOT_ALR_DIARIOS_RECUPERADOS_LOG);
				Serial.println(F("Intentos notificaciones alr diarios recuperados"));

			}

			if(leerFlagEEInt("N_SYS_SEND") != 0){
				guardarFlagEE("N_SYS_SEND", 0);
				registro.registrarLogSistema("INTENTOS NOTIFICACION SYS DIARIAS RECUPERADAS");
				eventosJson.guardarLog(INTENTOS_NOT_SYS_DIARIOS_RECUPERADOS_LOG);
				Serial.println(F("Intentos notificaciones sys diarios recuperados"));

			}

			if(leerFlagEEInt("N_MOD_SEND") != 0){
				guardarFlagEE("N_MOD_SEND", 0);
				registro.registrarLogSistema("INTENTOS MODELO JSON DIARIOS RECUPERADOS");
				eventosJson.guardarLog(INTENTOS_MODELO_JSON_DIARIOS_RECUPERADOS_LOG);
				Serial.println(F("Intentos modelo json recuperados"));

			}

		}

	}

	void checkearColaLogsSubtareas(){
		/*Lee la cola para los registro provenietes de las tareas y los guarda*/
		RegistroLogTarea reg;
		TickType_t espera = pdMS_TO_TICKS(50);

		if (uxQueueMessagesWaiting(colaRegistros) > 0) {
			if (xQueueReceive(colaRegistros, &reg, espera) == pdTRUE) {
				//printf("Tipo de log: %d\n", reg.tipoLog);
				//printf("Mensaje: %s\n", reg.log);
				//printf("Id SAAS: %d\n", reg.saasLogid);

				if(reg.tipoLog == 0){
					//Log sistema
					registro.registrarLogSistema(reg.log);
				}else {
					//Log http
					registro.registrarLogHttpRequest(reg.log);
				}

				if(reg.saasLogid != 0){
					eventosJson.guardarLog(reg.saasLogid);
				}

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


		if (leerFlagEEInt("ESTADO_GUARDIA") == 1 || leerFlagEEInt("ERR_INTERRUPT") == 0) {
			estadoAlarma = ESTADO_GUARDIA;
			registro.registrarLogSistema("CARGADO ESTADO GUARDIA PREVIO");
			eventosJson.guardarLog(CARGADO_ESTADO_GUARDIA_PREVIO_LOG);

			//Informar de que se ha restaurado una entrada
			guardarFlagEE("F_RESTAURADO", 1);
		}


	}

	void checkearAlertasDetenidas(){ //TODO choca con el blindaje de los terminales
		if (leerFlagEEInt("ESTADO_ALERTA") == 1 && leerFlagEEInt("ERR_INTERRUPT") == 0) {

			eeDatosSalto = NVS_RestoreData<datos_saltos_t>("SALTO_DATA");

			int* datos = datosSensores.getDatos();
			arrCopy<int>(eeDatosSalto.DATOS_SENSOR,datos ,TOTAL_SENSORES); //Carga los datos EE
			zona = eeDatosSalto.ZONA;
			INTENTOS_REACTIVACION = eeDatosSalto.INTENTOS_REACTIVACION;

			char registroConjunto[50];
			snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", "CARGADO ESTADO ALERTA EN ", nombreZonas[zona]);

			registro.registrarLogSistema(registroConjunto);
			eventosJson.guardarLog(INTRUSISMO_RESTAURADO_LOG);

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

		//Liberamos la actividad en modulo
		vTaskSuspend(envioServidorSaas); //Pausa la ejecucion SAAS
		sleepModeGSM = GSM_ON;
		setMargenTiempo(tiempoMargen,TIEMPO_CARGA_GSM);

		if(!modem.waitForNetwork(1000, true)){
			Serial.println(F("Modulo sin red refrescando"));
			refrescarModuloGSM();
		}

	}

	void setEstadoErrorRealizarLlamadas(){
		estadoError = REALIZAR_LLAMADAS;
		setMargenTiempo(tiempoMargen,240000);
	}

	void setEstadoErrorEsperarAyuda(){
		Serial.println(F("Esperar ayuda"));

		//Encolamos una notificacion
		encolarNotificacionSaas(0, "Interrupcion por fallo en la alimentacion");

		estadoError = ESPERAR_AYUDA;
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


	void limpiarTerminalesLinea(){
		for (int i = 0; i < 1; i++) { //N_TERMINALES_LINEA
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

		Serial.print("\n");

		Serial.printf("SENSORES_HABILITADOS = {%d, %d, %d, %d}\n",
				  configSystem.SENSORES_HABLITADOS[0], configSystem.SENSORES_HABLITADOS[1],
				  configSystem.SENSORES_HABLITADOS[2], configSystem.SENSORES_HABLITADOS[3]);


		Serial.printf("DATOS SENSORES = {%d, %d, %d, %d}\n",
				eeDatosSalto.DATOS_SENSOR[0], eeDatosSalto.DATOS_SENSOR[1],
				eeDatosSalto.DATOS_SENSOR[2], eeDatosSalto.DATOS_SENSOR[3]);

		Serial.print("\n");

		Serial.printf("FLAG GUARDIA = %d\n", leerFlagEEInt("ESTADO_GUARDIA"));
		Serial.printf("FLAG ALERTA = %d\n", leerFlagEEInt("ESTADO_ALERTA"));
		Serial.printf("FLAG PUERTA ABIERTA = %d\n", leerFlagEEInt("PUERTA_ABIERTA"));
		Serial.printf("NUM SMS ENVIADOS = %d\n", leerFlagEEInt("N_SMS_ENVIADOS"));

		Serial.printf("NUM NOT SYS ENVIADOS = %d\n", leerFlagEEInt("N_SYS_SEND"));
		Serial.printf("NUM NOT ALR ENVIADOS = %d\n", leerFlagEEInt("N_ALR_SEND"));
		Serial.printf("NUM MODELOS ENVIADOS = %d\n", leerFlagEEInt("N_MOD_SEND"));

		Serial.print("\n");

		Serial.printf("ERR INTERRUPT = %d\n", leerFlagEEInt("ERR_INTERRUPT"));
		Serial.printf("ERR HISTORICO INTERRUPCIONES = %d\n", leerFlagEEInt("INTERUP_HIST"));
		Serial.printf("ERR SMS EMERGENCIA ENVIADO = %d\n", leerFlagEEInt("MENSAJE_EMERGEN"));
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

	ProveedorEstado coberturaRed() {

		static ProveedorEstado respuesta;
		static unsigned long tiempoAnterior = 0; // Variable estática para almacenar el tiempo de la última ejecución

		unsigned long tiempoActual = millis(); // Obtener el tiempo actual desde que se encendió Arduino

		// Verificar si han pasado al menos 10 segundos desde la última ejecución
		if (tiempoActual - tiempoAnterior >= 4000) { // 10000 milisegundos = 10 segundos
			tiempoAnterior = tiempoActual; // Actualizar el tiempo de la última ejecución

			respuesta.intensidadSignal = modem.getSignalQuality();
			respuesta.proveedor = modem.getOperator();

			if(respuesta.proveedor.isEmpty())
				respuesta.proveedor = "Sin servicio";

			return respuesta;
		}

		// Si no han pasado 10 segundos, no ejecutar la función y devolver un valor inválido (por ejemplo, -1)
		return respuesta;
	}

	void comprobarConexionGSM(unsigned long timeOut){
		Serial.print("Waiting for network...");
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGSM);
		if (!modem.waitForNetwork(timeOut, true)) {
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

		if(xPortGetCoreID() == 1)
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprs);

		if (!modem.gprsConnect(apn, gsmUser, gsmPass)) {
			Serial.println(" Error al conectar a la red GPRS.");
			if(xPortGetCoreID() == 1)
			pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprsFail);

			return false;
		}
		Serial.println(" Conexion a la red GPRS establecida.");
		if(xPortGetCoreID() == 1)
		pantalla.lcdLoadView(&pantalla, &Pantalla::sysConexionGprsOk);

		return true;
	}

	void cerrarConexionGPRS(){
		modem.gprsDisconnect();
		Serial.println(F("GPRS disconnected"));
	}

	void encolarEnvioModeloSaas(){
		//Se guarda en la cola el envio periodico al saas
		DatosTarea datosNodo;
		datosNodo.tipoPeticion = PAQUETE;

		//Comprobamos si quedan envios
		if(leerFlagEEInt("N_MOD_SEND") >= MAX_MODELO_JSON_DIARIOS){
			registro.registrarLogSistema("SUPERADO MAXIMO ENVIOS MODELO DIARIOS");
			return;
		}

		InsertarFinal(&listaTareas, datosNodo);

		guardarFlagEE("N_MOD_SEND", (leerFlagEEInt("N_MOD_SEND")+1));
	}


	void crearTareaEnvioModeloSaas(){
		xTaskCreatePinnedToCore(
				tareaSaas,
				"tareaSaas",
				(1024*10), //Buffer
				NULL, //Param
				1, //Prioridad
				&envioServidorSaas, //Task
				0);
	}

	byte enviarEnvioModeloSaas(){
		byte executionResult;

		if(modem.waitForNetwork(2000, true)){ //@TEST NO NEGAR EN PROD
			Serial.println(F("Hay cobertura se procede al envio"));
			executionResult = eventosJson.enviarInformeSaas();
		}else {
			Serial.println(F("No hay cobertura se aborta el envio"));
			executionResult = 0;
			//Refresco el modulo
			refrescarModuloGSM();
		}

		return executionResult;
	}

	void encolarNotificacionSaas(byte tipo, const char* contenido){
		//Se guarda en la cola la notificacion para ser procesada
		DatosTarea datosNodo;

		datosNodo.tipoPeticion = NOTIFICACION;
		datosNodo.notificacion.tipo = tipo;
		strcpy(datosNodo.notificacion.contenido, contenido);

		//Comprobamos si quedan envios
		if(leerFlagEEInt("N_SYS_SEND") >= MAX_NOTIFICACIONES_SYS_DIARIAS && datosNodo.notificacion.tipo == 0){
			registro.registrarLogSistema("SUPERADO MAXIMO NOTIFICACIONES_SYS_DIARIAS");
			return;
		}

		if(leerFlagEEInt("N_ALR_SEND") >= MAX_NOTIFICACIONES_ALARM_DIARIAS && datosNodo.notificacion.tipo == 1){
			registro.registrarLogSistema("SUPERADO MAXIMO NOTIFICACIONES_ALR_DIARIAS");
			return;
		}

		InsertarFinal(&listaTareas, datosNodo);

		if(datosNodo.notificacion.tipo == 1){
			guardarFlagEE("N_ALR_SEND", (leerFlagEEInt("N_ALR_SEND")+1));
		}else {
			guardarFlagEE("N_SYS_SEND", (leerFlagEEInt("N_SYS_SEND")+1));
		}
	}

	void crearTareaNotificacionSaas(byte tipo, const char* contenido){
		//Se definen los datos de la notificacion y se crea una tarea en segundo plano
		 datosNotificacionSaas.tipo = tipo;
		 strcpy(datosNotificacionSaas.contenido, contenido);

			xTaskCreatePinnedToCore(
					tareaNotificacionSaas,
					"tareaNotificacionSaas",
					(1024*10), //Buffer
					&datosNotificacionSaas, //Param
					1, //Prioridad
					&envioNotificacionSaas, //Task
					0);
	}

	byte enviarNotificacionesSaas(byte tipo, const char* contenido){

		byte resultado;

		if(modem.waitForNetwork(2000, true)){ //@TEST NO NEGAR EN PROD
			Serial.println(F("Hay cobertura se procede al envio"));
			resultado = eventosJson.enviarNotificacionSaas(tipo, contenido);
		}else {
			Serial.println(F("No hay cobertura se aborta el intento"));
			resultado = 0;
			//Refresco el modulo
			refrescarModuloGSM();
		}

		if (resultado == 1) {
			Serial.println(F("Notificacion enviada exitosamente."));
		} else {
			Serial.println(F("Fallo al enviar notificacion."));
		}

		return resultado;
	}

	void gestionarPilaDeTareas(){
		//Compruebo si se ha habilitado la gestion de tareas
		if(!accesoGestorPila)
			return;

		if(!checkearMargenTiempo(tiempoReinicioPila))
			return;

		//Compruebo si la lista esta vacia
		if(listaTareas.cabeza == NULL){
			//Serial.println("Pila vacia");
			return;
		}

		//Se controla que no pueda haber un desbordamiento
		if (listaLongitud(&listaTareas) > MAX_NODOS_EN_EJECUCION) {
			Serial.println("Overload en pila de tareas");
			EliminarPrincipio(&listaTareas);
		}

		TaskNodo* tarea;
		TaskHandle_t manejador;
		unsigned long tiempoReintento;

		//Comprobamos si hay alguna tarea en curso
		tarea = tareaEnCurso(&listaTareas);

		//if(tarea != NULL){ //@TEST ONLY
		//	Serial.println(tarea->data.tipoPeticion);
		//}

		if(tarea == NULL){
			//Serial.println("No hay tareas en curso buscamos una nueva");
			//Comprobar si alguno de los elementos de la lista esta ya listo para procesarse
			tarea = recuperarTareaProcesable(&listaTareas);

			if(tarea == NULL){
				//Serial.println("No hay nada en la pila listo para procesar");
				//Si ninguna de las tareas esta lista volvemos
				return;
			}
		}


		if(tarea->data.tipoPeticion == NOTIFICACION){
			manejador = envioNotificacionSaas;
		}else if(tarea->data.tipoPeticion == PAQUETE) {
			manejador = envioServidorSaas;
		}else {
			manejador = NULL;
		}

		//Definimos el tiempo de los reintentos
		tiempoReintento = TIEMPO_ESPERA_REINTENTO_TAREA;

		if(estadoAlarma == ESTADO_ALERTA){
			tiempoReintento = (TIEMPO_ESPERA_REINTENTO_TAREA*0.5);
		}


		//Si hay una tarea en cola compruebo si esta en ejecucion
		if(manejador == NULL){
			Serial.print("Tarea no creada, Reintentos: ");
			Serial.println(tarea->reintentos);

			//Creo la tarea conveniente en funcion del ipo de peticion
			if(tarea->data.tipoPeticion == NOTIFICACION){
				//Creo la tarea para notificaciones
				Serial.println("Creando tarea notificacion");
				crearTareaNotificacionSaas(tarea->data.notificacion.tipo, tarea->data.notificacion.contenido);
			}

			if(tarea->data.tipoPeticion == PAQUETE){
				//Creo la tarea para el envio de paquetes
				Serial.println("Creando tarea envio modelo json");
				crearTareaEnvioModeloSaas();
			}


			if(estadoAlarma == ESTADO_ALERTA || estadoAlarma == ESTADO_ENVIO){
				setMargenTiempo(tiempoTareaEnEjecucion,TIEMPO_MAX_TAREA);
			}else {
				setMargenTiempo(tiempoTareaEnEjecucion,(TIEMPO_MAX_TAREA*2));
			}

			estadoPila = PROCESANDO;

		}else {
			//La tarea existe compruebo su estado
			eTaskState estadoTarea = eTaskGetState(manejador);

			//Serial.print("Estado de la tarea eTaskState: ");
			//Serial.println(estadoTarea);

			if(estadoTarea == eSuspended){
				//Suspendemos cuando termina OK
				Serial.println("Tarea finalizada");
				//Libero la tarea para que ella sola se diriga hacia su irremediable final
				vTaskResume(manejador);

				//Compruebo el flag global de la tarea http
				byte resultadoTarea;

				if(tarea->data.tipoPeticion == NOTIFICACION){
					resultadoTarea = resultadoEnvioNotificacionSaas;
				}

				if(tarea->data.tipoPeticion == PAQUETE) {
					resultadoTarea = resultadoEnvioServidorSaas;
				}

				if(resultadoTarea){
					//Elimino el nodo
					Serial.println("Resultado OK borro la tarea");
					EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
				}else {

					if(tarea->reintentos == MAX_REINTENTOS_REPROCESO_TAREA){
						Serial.println("Tarea ko supera los reintnetos");
						EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
					}else {
						//Si no ha sido exitosa movemos al final con timeout
						Serial.println("Tarea ko reintento con timeout");
						tarea->reintentos++;
						EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
						InsertarFinal(&listaTareas, tarea->data, tarea->reintentos, (millis()+tiempoReintento));
					}

				}

				estadoPila = LIBRE;
				finalizarTareaEnCurso(&listaTareas);

			}else {
				//Dejamos trabajar a la tarea y controlamos el tiempo

				if(checkearMargenTiempo(tiempoTareaEnEjecucion)){
					Serial.println("Se supera el tiempo de ejecucion");

					//Cerramos la tareas directamente
					//eRunning 0 , eReady 1 , eBlocked 2, eSuspended 3, etc.
					if(tarea->data.tipoPeticion == NOTIFICACION){
						vTaskSuspend(envioNotificacionSaas);
						//Elimino la tarea
						vTaskDelete(envioNotificacionSaas);
						envioNotificacionSaas = NULL;

					}else if(tarea->data.tipoPeticion == PAQUETE) {
						vTaskSuspend(envioServidorSaas);
						//Elimino la tarea
						vTaskDelete(envioServidorSaas);
						envioServidorSaas = NULL;
					}

					if(tarea->reintentos == MAX_REINTENTOS_REPROCESO_TAREA){
						//Elimino la tarea
						Serial.println("Eliminando tarea por exceso de intentos");
						EliminarTareaEnPosicion(&listaTareas, tarea->posicion);
					}else {
						tarea->reintentos++;
						//Muevo la tarea al final
						Serial.println("Tarea pospuesta");
						EliminarTareaEnPosicion(&listaTareas,tarea->posicion);
						InsertarFinal(&listaTareas, tarea->data, tarea->reintentos, (millis()+tiempoReintento));
					}

					estadoPila = LIBRE;
					finalizarTareaEnCurso(&listaTareas);
				}

			}
		}
	}

	void rehabilitarEjecucionPila(){
		accesoGestorPila = 1; //Acceso a pila con tiempo de espera
		setMargenTiempo(tiempoReinicioPila,(TIEMPO_REINCIO_PILA));
	}

	void detenerEjecucionPila(){
		accesoGestorPila = 0;

		if(envioNotificacionSaas != NULL){
			vTaskDelete(envioNotificacionSaas);
			envioNotificacionSaas = NULL;
		}

		if(envioServidorSaas != NULL){
			vTaskDelete(envioServidorSaas);
			envioServidorSaas = NULL;
		}
	}

	void checkearEnvioSaas(){
		if(!configSystem.ENVIO_SAAS)
			return;

		static unsigned long lastExecutionTime = 0;
		//if (millis() - lastExecutionTime >= (((configSystem.ESPERA_SAAS_MULTIPLICADOR*5)+10)*60000)) { //600000 // @TEST
		if (millis() - lastExecutionTime >= 30000) { //600000

			//Encolar envio modelo
			encolarEnvioModeloSaas();
			lastExecutionTime = millis();
		}
	}

	RespuestaHttp realizarPeticionHttp(const char* metodo, const char* resource, byte auth = 1, const char* jsonData = nullptr){

		RespuestaHttp respuesta;

		/*TEST*/
		//respuesta.codigo= 200;
		//respuesta.respuesta = "OK";// "Id de paquete duplicado";
		//return respuesta;
		//vTaskDelay(1000);
		/*TEST*/

		if(establecerConexionGPRS()){
			int estadoHttp = 0;
			String respuestaHttp;

			//vTaskDelay(1000);

			//Formular peticion HTTP
			//http.connectionKeepAlive();  // Currently, this is needed for HTTPS
			http.beginRequest();

			if (strcmp(metodo, "GET") == 0) {
				estadoHttp = http.get(resource);
			} else if (strcmp(metodo, "POST") == 0) {
				if (jsonData) {
					estadoHttp= http.post(resource/*, "application/json", jsonData*/);
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

			//Adjuntamos el json en el body
			http.beginBody();
			http.print(jsonData);

			http.endRequest();


			//Gestionar respuesta HTTP
			respuesta.codigo = http.responseStatusCode();
			Serial.print(F("Response status code: "));
			Serial.println(respuesta.codigo);

			if (estadoHttp != 0 || respuesta.codigo <= 0) {
				Serial.println(F("failed to connect"));

				if(xPortGetCoreID() == 1)
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


			RegistroLogTarea reg;
			TickType_t espera = pdMS_TO_TICKS(10);
			respuestaHttp.toCharArray(reg.log, sizeof(reg.log));
			reg.tipoLog = 1; //http

			xQueueSend(colaRegistros, &reg, espera);


			cerrarConexionGPRS();

			//vTaskDelay(1000);
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



	RespuestaHttp postDatosSaas(String* modeloJson, SAAS_TIPO_HTTP_REQUEST tipoDatos){
		RespuestaHttp respuesta;
		const char* jsonData = modeloJson->c_str();

		if(tipoDatos == PAQUETE){
			 Serial.println("Envio de paquete");
			respuesta = realizarPeticionHttp("POST", postEventosJson, 1, jsonData);
		}else if(tipoDatos == NOTIFICACION){
			Serial.println("Envio de notificacion");
			respuesta = realizarPeticionHttp("POST", postNotificacionJson, 1, jsonData);
		}else {
			Serial.print("Error tipo de peticion http no reconocida");
		}

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
