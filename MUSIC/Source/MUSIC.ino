/**
 * SISTEMA DE ALARMA DE LA ALBERQUILLA (MUSIC)
 * Version: VE21R0
 * Leyenda: @develop  @PEND (Pendiente de adaptar a ESP32)
 *
 * POR HACER:
 *
 * - Controlar el reset GSM en Response status code: -4 (FLAG)
 * - Mas metricas y envio FTP
 *
 * - Enriquecer el log con dia de la semana o temperatura
 * - Probar que en caso de necesitar tlf y sms las tareas en segundo plano finalizan OK
 * - Controlar a futuro el impacto entre terminales core
 *
 * INCIDENCIAS
 *
 */



#include "Arduino.h"
#include "Alarma.h"

void EstadoInicio(){

	procesoCentral = ALARMA;
	estadoAlarma = ESTADO_REPOSO;


	saasCronEstado = ESPERA_ENVIO;

	//Acutalizamos el secuencial de los logs para esta ejecucion
	guardarFlagEE("LOG_SEQ", (leerFlagEEInt("LOG_SEQ")+1));


#ifdef ALARMA_EN_MODO_DEBUG
	printSystemInfo();
#endif

	if(configSystem.MODULO_RTC){
		fecha.establecerFechaReset(10);
	}
}

void setup()
{
	  Serial.begin(115200);
	  mensaje.inicioGSM();
	  UART_RS.begin(115200, SERIAL_8N1, RS_RX, RS_TX);    //RX TX  (H1 = RX5 TX 18) PUERTO RS
	  UART_RS.setTimeout(10);

	  Serial.println(version[0]);

	  //Restaurar configuracion almacenada
	  configSystem = NVS_RestoreData<configuracion_sistema_t>("CONF_SYSTEM");


	  //Inicio de perifericos
	  iniciarTecladoI2C();
	  //Iniciar pantalla
	  pantalla.iniciar();
	  //Iniciar reloj
	  if(!fecha.iniciarRTC()){
		  registro.registrarLogSistema("Error! No se inicializo del modulo RTC");
	  }else {
		  registro.registrarLogSistema("Modulo RTC OK");
	  }
	  //Iniciar multiplexor
	  if (!mcp.begin_I2C(MCP_ADDR)) {
	    Serial.println("Error MUX MCP23017");
	    pantallaDeError(F("  SYSTM ERROR!  ERROR MUX INICIO"));
	    registro.registrarLogSistema("Error! No se inicializo el modulo multiplexor");
	  }else {
		  registro.registrarLogSistema("Multiplexor OK");
	  }

	  //Asignar el semaforo
	  semaphore = xSemaphoreCreateMutex();

	  if(registro.iniciar() == 0){
		  Serial.println(F("ERROR AL INICIAR SD"));
		  registro.registrarLogSistema("Error! No se inicializo la tarjeta SD");
		  pantallaDeError(F("  SYSTM ERROR!  ERROR INICIAR SD"));
	  }else {
		  registro.registrarLogSistema("Almacenamiento SD OK");

	  }

	  //Definir el padding AES
	  aesLib.set_paddingmode(paddingMode::CMS);

	  pantalla.lcdLoadView(&pantalla, &Pantalla::lcdInicio);
	  delay(1000);


	  //Declaracion de los puertos I/O
	  	//PIR
	    mcp.pinMode(PIR_SENSOR_1, INPUT);
	    mcp.pinMode(PIR_SENSOR_2, INPUT);
	    mcp.pinMode(PIR_SENSOR_3, INPUT);
	    mcp.pinMode(MG_SENSOR, INPUT);

	    //MODULO GSM
	    mcp.pinMode(GSM_PIN, OUTPUT);
	    mcp.digitalWrite(GSM_PIN, LOW); //Siempre BAJO (ALTO = TIERRA EN RESET)

		//MODULO RS485
	    pinMode(RS_CTL, OUTPUT);


		//PUERTOS INTERNOS
	    mcp.pinMode(BOCINA_PIN, OUTPUT);
	    mcp.pinMode(RELE_AUXILIAR, OUTPUT);
	    mcp.pinMode(LED_COCHERA, OUTPUT);
	    mcp.pinMode(RESETEAR,OUTPUT);
	    mcp.pinMode(WATCHDOG, OUTPUT);
	    mcp.pinMode(FALLO_BATERIA_PRINCIPAL, INPUT);
	    //mcp.pinMode(SENSOR_BATERIA_RESPALDO, INPUT); //No soportado por hardware

	    //Configuracion de los puertos
	    mcp.digitalWrite(LED_COCHERA, LOW);
	    digitalWrite(RS_CTL,LOW);

	    //Respaldo modelo JSON
	    eventosJson.iniciarModeloJSON();

	    EstadoInicio();
	    cargarEstadoPrevio();
	    checkearAlertasDetenidas();
	    chekearInterrupciones();

	    registro.registrarLogSistema("ALARMA INICIADA");
	    eventosJson.guardarLog(ALARMA_INICIADA_LOG);


	    //Hilo 0
	    xTaskCreatePinnedToCore(
	    		tareaLinea,
				"tareaLinea",
				(1024*5),
				NULL,
				1,
				&gestionLinea,
				1);

	    disableCore0WDT(); //Quito el watchdog en 0 que Dios me perdone
	    colaRegistros = xQueueCreate(10, sizeof(RegistroLogTarea));

	    if(mcp.digitalRead(FALLO_BATERIA_PRINCIPAL) == HIGH){
	    	 Serial.println(F("ERROR NO 12V"));
	    }else {
	    	 Serial.println(F("12V OK"));
	    }

	    //TEST WIFI
		#ifdef WIFI_PUERTO_SERIE
	    WiFi.mode(WIFI_STA);
	   	    WiFi.begin(ssidWifi, passwordWifi);
	   	    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
	   	    	Serial.printf("WiFi Failed!\n");
	   	    	//return;
	   	    }else {
	   	    	Serial.print("IP Address: ");
	   	    	Serial.print(WiFi.localIP());
	   	    	Serial.println("/webserial");
	   	    	// WebSerial is accessible at "<IP Address>/webserial" in browser
	   	    	WebSerial.begin(&serverDos);
	   	    	WebSerial.msgCallback(recvMsg);
	   	    	serverDos.begin();
	   	    }
		#endif
	   //TEST WIFI

#ifndef ALARMA_EN_MODO_DEBUG
	    comprobarConexionGSM(20000L);
#endif
}

void loop()
{
	leerEntradaTeclado();
	demonio.demonioSerie();
	procesosSistema();
	procesosPrincipales();

}


void tareaSaas(void *pvParameters) {

	Serial.println("Task Paquete datos");
	resultadoEnvioServidorSaas = 0;
	resultadoEnvioServidorSaas = enviarEnvioModeloSaas();
	vTaskDelay(100);

	//Pendiente de cierre
	vTaskSuspend(NULL);
	vTaskDelay(100);
	//Finalizar tras release
	envioServidorSaas = NULL;
	vTaskDelete(NULL);
}

void tareaNotificacionSaas(void *pvParameters){
	NotificacionSaas *datos = (NotificacionSaas *)pvParameters;

	Serial.println("Task Notificacion Tiempo medio 15 segundos");
	resultadoEnvioNotificacionSaas = 0;
	resultadoEnvioNotificacionSaas = enviarNotificacionesSaas(datos->tipo, datos->contenido);
	vTaskDelay(100);

	//Pendiente de cierre
	vTaskSuspend(NULL);
	vTaskDelay(100);
	//Finalizar tras release
	envioNotificacionSaas = NULL;
	vTaskDelete(NULL);
}

void tareaLinea(void *pvParameters){
	  while (1) {
		linea.mantenerComunicacion();
		vTaskDelay(300);
	}
}


void procesosSistema(){


	watchDog();
	checkearResetModuloGSM();
	checkearSensorPuertaCochera();
	avisoLedPuertaCochera();
	resetearAlarma();
    checkearLimitesEnvios();
	resetAutomatico();
	//checkearBateriaDeEmergencia(); //TODO Hardware actual incompatible
	checkearFalloEnAlimientacion();
	//escucharGSM();

	//Quitadas por pruebas

	gestionarPilaDeTareas();
	checkearEnvioSaas();
	checkearColaLogsSubtareas();
}

void procesosPrincipales()
{
	switch(procesoCentral){

	case ALARMA:
		procesoAlarma();
		break;

	case MENU:
		menu.procesoMenu();
		break;

	case ERROR:
		procesoError();
		break;
	}
}


void procesoAlarma(){

	switch(estadoAlarma){

	case ESTADO_REPOSO:

		if(!isLcdInfo())
		pantalla.lcdLoadView(&pantalla, &Pantalla::lcdReposo);

		if(auth.isPasswordCached()){
			if (key != NO_KEY){
				if(key == '*'){
					setEstadoGuardia();
				}

				if(key == 'B'){ //Tecla menu
					procesoCentral = MENU;
				}
			}
		}

		break;

	case ESTADO_GUARDIA:

		if(!isLcdInfo())
		pantalla.lcdLoadView(&pantalla, &Pantalla::lcdGuardia);

		if(checkearMargenTiempo(tiempoMargen)){

			comprobarSensoresCore();

			//Sensor MG
			if(sensorCore.sensorMG){
				Serial.println(F("\nDisparador:  MG"));
				respuestaTerminal.resumen = "PUERTA COCHERA ABIERTA";
				respuestaTerminal.idTerminal = 0;
				respuestaTerminal.idSensorDetonante = 3;
				sensorCore.notificadoMG = 0;
				setEstadoAlerta();
				break;
			}

			//Terminales
			for (int i = 0; i < N_TERMINALES_LINEA; i++) {
				respuestaTerminal = T_LIST[i]->evaluarDatosTerminal();

				if(respuestaTerminal.interpretacion != TERMINAL_OK){

					if(respuestaTerminal.interpretacion == BAD_COMM
					|| respuestaTerminal.interpretacion == BAD_REPLY
					|| respuestaTerminal.interpretacion == NO_REPLY){
						//Se avisara pero la alarma no saltara
						Serial.print(F("\nTerminal "));
						Serial.print(T_LIST[i]->getTerminalName());
						Serial.println(F(" supera el umbral de fallo en la comunicacion"));

						char contenidoCola[200];

						sprintf(contenidoCola, "\nTerminal %s supera el umbral de fallo en la comunicacion: %s",
								T_LIST[i]->getTerminalName(),
								(respuestaTerminal.interpretacion == BAD_COMM)? "Mala comunicacion":
								(respuestaTerminal.interpretacion == BAD_REPLY)? "Mala respuesta": "Sin respueta"
								);

						encolarNotificacionSaas(0, contenidoCola);

						//Resetear strikes
						T_LIST[i]->limpiarStrikes();

					}else {
						//Constitutivo de aviso
						respuestaTerminal.resumen  = String(T_LIST[i]->getTerminalName()).substring(0, 4)  + "-" + respuestaTerminal.resumen;
						Serial.println(respuestaTerminal.resumen);
						setEstadoAlerta();
						break;
					}

				}
			}

		}

		sonarBocina();
		desactivarAlarma();

		break;

	case ESTADO_ALERTA:

		if(!isLcdInfo())
		pantalla.lcdLoadView(&pantalla, &Pantalla::lcdAlerta);

		if(checkearMargenTiempo(tiempoMargen)){

			setEstadoEnvio();
		}

		desactivarAlarma();

		break;

	case ESTADO_ENVIO:

		if(!isLcdInfo())
		pantalla.lcdLoadView(&pantalla, &Pantalla::lcdAvisoEnviado);

		if(checkearMargenTiempo(tiempoMargen)){

			if(INTENTOS_REACTIVACION < 1){

				INTENTOS_REACTIVACION++;
				if(configSystem.MODO_SENSIBLE){
					setMargenTiempo(tiempoSensible,TIEMPO_MODO_SENSIBLE, TIEMPO_MODO_SENSIBLE_TEST);
				}
				sensorCore.notificadoMG = 0;
				setEstadoGuardiaReactivacion();
			}else {
				//Cuando no queden reactivaciones hay que sacar a la alarma de aqui para que
				//el SAAS continue recibiendo informes de situacion
				sensorCore.notificadoMG = 0;
				setEstadoInquieto();
				break;
			}
		}

		if(INTENTOS_REACTIVACION < MAX_INTENTOS_REACTIVACION){
			//Si la alarma aun tiene opciones de reinicio examinamos el phantom
			comprobarSensoresCore();

			//Terminales
			for (int i = 0; i < N_TERMINALES_LINEA; i++) {
				T_LIST[i]->evaluarPhantomTerminal();
			}
		}

		realizarLlamadas();
		sonarBocina();
		desactivarAlarma();

		break;

	case ESTADO_INQUIETO:

		if(!isLcdInfo())
			pantalla.lcdLoadView(&pantalla, &Pantalla::lcdInquieto);

		comprobarSensoresCore();

		for (int i = 0; i < N_TERMINALES_LINEA; i++) {
			respuestaTerminal = T_LIST[i]->evaluarDatosTerminal();

			if(respuestaTerminal.interpretacion != TERMINAL_OK){
				if(respuestaTerminal.interpretacion != BAD_COMM
						|| respuestaTerminal.interpretacion != BAD_REPLY
						|| respuestaTerminal.interpretacion != NO_REPLY){

					//Constitutivo de aviso
					Serial.println("Aviso inquieto accionando bocina");
					encolarNotifiacionIntrusismo();

					if(checkearMargenTiempo(tiempoBocina)){
						//Si la bocina anterior ha terminado sirvo otra racion
						Serial.println("Mas tiempo bocina");
						int intentosRestantes = leerFlagEEInt("TICKET_BOCINA");

						if(intentosRestantes <= MAX_TOQUES_BOCINA_RESTANTES){
							setMargenTiempo(tiempoBocina, 120000, 0.15);
							intentosRestantes++;
							guardarFlagEE("TICKET_BOCINA", intentosRestantes);
						}

					}

					//Volvemos al estado anterior
					respuestaTerminal.interpretacion == TERMINAL_OK;
					limpiarTerminalesLinea();
				}
			}
		}

		desactivarAlarma();
		sonarBocina();
		break;

	}
}


void setEstadoGuardia()
{
	Serial.println(F("\nAlarma Activada"));
	estadoAlarma = ESTADO_GUARDIA;
	guardarFlagEE("ESTADO_GUARDIA", 1);


	//Eliminamos la informacion de los terminales
	limpiarTerminalesLinea();

	lcd_clave_tiempo = millis();
	lcd_info_tiempo = millis() + TIEMPO_ALERT_LCD;
	setMargenTiempo(tiempoMargen,TIEMPO_ON, TIEMPO_ON_TEST);

	registro.registrarLogSistema("ALARMA ACTIVADA MANUALMENTE");
	eventosJson.guardarEntrada();
	eventosJson.guardarLog(ALARMA_ACTIVADA_MANUALMENTE_LOG);
}

void setEstadoGuardiaReactivacion()
{
	Serial.println("\nAlarma Reactivada. Intentos realizados: "+ (String)INTENTOS_REACTIVACION);
	estadoAlarma = ESTADO_GUARDIA;
	guardarFlagEE("ESTADO_GUARDIA", 1);
	guardarFlagEE("F_REACTIVACION", 1);


	lcd_clave_tiempo = millis();

	setMargenTiempo(tiempoBocina, (TIEMPO_BOCINA*0.5), TIEMPO_BOCINA_REACTIVACION_TEST);
	setMargenTiempo(tiempoMargen,(TIEMPO_ON*0.01));

	//Desabilitar puerta tras la reactivacion
	if(configSystem.SENSORES_HABLITADOS[0] && (respuestaTerminal.idTerminal == 0 && respuestaTerminal.idSensorDetonante == 3) ){
		flagPuertaAbierta = 1;
		guardarFlagEE("PUERTA_ABIERTA", 1);

		sensorHabilitado[0] = 0;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	//Se envian los mensajes de reactivacion
	mensaje.mensajeReactivacion();

	encolarNotificacionSaas(1, "Alarma reactivada con exito");
	encolarEnvioModeloSaas(); //Encolamos otro modelo tras el envio de la alarma

	rehabilitarEjecucionPila();

	//Se limpia el resultado del phantom para dejar paso a la nueva ejecucion
	limpiarTerminalesLinea();

	registro.registrarLogSistema("ALARMA ACTIVADA AUTOMATICAMENTE");
	eventosJson.guardarLog(ALARMA_ACTIVADA_AUTOMATICAMENTE_LOG);
	eventosJson.guardarEntrada();

}

void setEstadoAlerta()
{

	Serial.print("\nIntrusismo detectado en ");
	Serial.println(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]);

	estadoAlarma = ESTADO_ALERTA;

	ACCESO_LISTAS = 0; //Blindamos las listas de datos
	guardarFlagEE("ESTADO_ALERTA", 1);
	guardarEstadoAlerta();
	encolarNotifiacionIntrusismo();

	lcd_clave_tiempo = millis();

	if(checkearMargenTiempo(tiempoSensible)){
		setMargenTiempo(tiempoMargen,TIEMPO_OFF, TIEMPO_OFF_TEST);
	}else{
		setMargenTiempo(tiempoMargen,(TIEMPO_OFF*TIEMPO_OFF_MODO_SENSIBLE));
	}

}

void setEstadoEnvio()
{
	Serial.println(F("\nTiempo acabado \nAVISO ENVIADO"));
	estadoAlarma = ESTADO_ENVIO;
	lcd_clave_tiempo = millis();
	setMargenTiempo(tiempoBocina, TIEMPO_BOCINA, TIEMPO_BOCINA_TEST);
	setMargenTiempo(tiempoMargen,TIEMPO_REACTIVACION, TIEMPO_REACTIVACION_TEST);


	//Cerramos la pila de tareas y terminamos la ejecucion si quedase alguna tarea ejecutandose
	detenerEjecucionPila();

	//Desde aqui se envian los correspondientes avisos
	mensaje.mensajeAlerta();


	limpiarEstadoAlerta(); //Se limpia la memoria si la alerta ha sido restaurada
	guardarFlagEE("ESTADO_ALERTA", 0);

	//Liberamos el acceso cuando los mensajes esten enviados y limpiamos para el analisis phantom
	ACCESO_LISTAS = 1;
	limpiarTerminalesLinea();
}

void setEstadoReposo()
{
	Serial.println(F("\nAlarma Desactivada"));
	estadoAlarma = ESTADO_REPOSO;

	lcd_clave_tiempo = millis();
	lcd_info_tiempo = millis() + TIEMPO_ALERT_LCD;

	INTENTOS_REACTIVACION = 0;
	ACCESO_LISTAS = 1; //Habilita la escritura de la linea
	pararBocina();
	tiempoSensible = millis();
	estadoLlamada = TLF_1;
	desactivaciones ++;
	guardarFlagEE("ESTADO_GUARDIA", 0);
	guardarFlagEE("ESTADO_ALERTA", 0);

	guardarFlagEE("F_RESTAURADO", 0);
	guardarFlagEE("F_REACTIVACION", 0);
	guardarFlagEE("TICKET_BOCINA", 0);

	rehabilitarEjecucionPila();

	//Rehabilitar sensor puerta
	if(flagPuertaAbierta){
		sensorHabilitado[0] = 1;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);


		guardarFlagEE("PUERTA_ABIERTA", 0);
	}

	//Eliminamos la informacion de los terminales
	limpiarTerminalesLinea();

	registro.registrarLogSistema("ALARMA DESACTIVADA MANUALMENTE");
	eventosJson.guardarEntrada();
	eventosJson.guardarLog(ALARMA_DESACTIVADA_MANUALMENTE_LOG);
}


void setEstadoInquieto()
{
	Serial.println(F("\nAlarma Desactivada Automaticamente"));
	estadoAlarma = ESTADO_INQUIETO;

	lcd_clave_tiempo = millis();
	lcd_info_tiempo = millis() + TIEMPO_ALERT_LCD;

	tiempoSensible = millis();
	estadoLlamada = TLF_1;

	guardarFlagEE("ESTADO_GUARDIA", 0);
	guardarFlagEE("ESTADO_ALERTA", 0);
	guardarFlagEE("F_RESTAURADO", 0);
	guardarFlagEE("F_REACTIVACION", 0);


	rehabilitarEjecucionPila();

	//Rehabilitar sensor puerta
	if(flagPuertaAbierta){
		sensorHabilitado[0] = 1;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);

		guardarFlagEE("PUERTA_ABIERTA", 0);
	}

	//Eliminamos la informacion de los terminales
	limpiarTerminalesLinea();

	registro.registrarLogSistema("ALARMA DESACTIVADA AUTOMATICAMENTE");
	eventosJson.guardarEntrada();
	eventosJson.guardarLog(ALARMA_DESACTIVADA_AUTOMATICAMENTE_LOG);
}
