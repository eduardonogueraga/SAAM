/**
 * SISTEMA DE ALARMA DE LA ALBERQUILLA (MUSIC)
 * Version: VE21R0
 * Leyenda: @develop  @PEND (Pendiente de adaptar a ESP32)
 *
 * POR HACER:
 *
 * -Probar que en caso de necesitar tlf y sms las tareas en segundo plano finalizan OK
 *- Añadir envios informativos sin saltar la alarma
 * -Añadir un envio SAAS adicional y posterior a mensaje y las llamadas
 * -Ajustar los requerimientos de SAAS
 * -Ajustar el modo sabotaje
 * -Modo inquieto
 * -Enriquecer el log con dia de la semana o temperatura
 * -Durante el modo reposo se hacen envioSAAS pero lo que entra por linea no se evalua ni guarda en el modelo Json
 *
 */



#include "Arduino.h"
#include "Alarma.h"

void EstadoInicio(){

	procesoCentral = ALARMA;
	estadoAlarma = ESTADO_REPOSO;
	sleepModeGSM = GSM_ON;

	saasCronEstado = ESPERA_ENVIO;

	//Acutalizamos el secuencial de los logs para esta ejecucion
	guardarFlagEE("LOG_SEQ", (leerFlagEEInt("LOG_SEQ")+1));

	//Actualizamos los intentos
	//guardarFlagEE("JSON_RETRY", 0);
	//guardarFlagEE("PACKAGE_ID", 1);

	//if(!MODO_DEFAULT)
	//printSystemInfo();


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
	  pantalla.iniciar();
	  fecha.iniciarRTC();

	  //Iniciar multiplexor
	  if (!mcp.begin_I2C(MCP_ADDR)) {
	    Serial.println("Error MUX MCP23017");
	    //while (1);
	  }

	  //Asignar el semaforo
	  semaphore = xSemaphoreCreateMutex();

	  if(registro.iniciar() == 0){
		  Serial.println(F("ERROR AL INICIAR SD"));
		  pantallaDeError(F("  SYSTM ERROR!  ERROR INICIAR SD"));
	  }


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

		//MODULO RS485
	    //mcp.pinMode(RS_CTL, OUTPUT);
	    pinMode(RS_CTL, OUTPUT);


		//PUERTOS INTERNOS
	    mcp.pinMode(BOCINA_PIN, OUTPUT);
	    mcp.pinMode(RELE_AUXILIAR, OUTPUT);
	    mcp.pinMode(LED_COCHERA, OUTPUT);
	    mcp.pinMode(RESETEAR,OUTPUT);
	    mcp.pinMode(WATCHDOG, OUTPUT);
	    mcp.pinMode(SENSOR_BATERIA_RESPALDO, INPUT);
	    //attachInterrupt(digitalPinToInterrupt(FALLO_BATERIA_PRINCIPAL), interrupcionFalloAlimentacion, FALLING); @PEND

	    //Configuracion de los puertos

	    mcp.digitalWrite(LED_COCHERA, LOW);
	    //mcp.digitalWrite(RS_CTL,LOW);
	    digitalWrite(RS_CTL,LOW);


	    //Activamos el modulo GSM
	    mcp.digitalWrite(GSM_PIN, HIGH);

	    EstadoInicio();
	    cargarEstadoPrevio();
	    checkearAlertasDetenidas();
	    chekearInterrupciones();

	    eventosJson.iniciarModeloJSON();
	    registro.registrarLogSistema("ALARMA INICIADA");
	    eventosJson.guardarLog(ALARMA_INICIADA_LOG);

	    //Serial.println(xPortGetCoreID());
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

	    // Iniciar el planificador de tareas
	    //vTaskStartScheduler();


	    //SIM800L
	    //comprobarConexionGSM(5000L);

	    //blinker.attach(4, blinkTEST);

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
	//enviarEnvioModeloSaas();

	//Pendiente de cierre
	vTaskSuspend(NULL);
	vTaskDelay(100);
	//Finalizar tras release
	envioServidorSaas = NULL;
	vTaskDelete(NULL);
}

void tareaNotificacionSaas(void *pvParameters){
	NotificacionSaas *datos = (NotificacionSaas *)pvParameters;

	Serial.println("Task Notificacion");
	//enviarNotificacionesSaas(datos->tipo, datos->contenido);
/*
	 while (1) {
		 Serial.println("Tarea jammed");
		 vTaskDelay(1000);
	 }
*/
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
	sleepMode();
	checkearSensorPuertaCochera();
	avisoLedPuertaCochera();
	resetearAlarma();
    checkearSms();
	resetAutomatico();
	checkearBateriaDeEmergencia();
	escucharGSM();
	//gestionarPilaDeTareas();
	//checkearEnvioSaas();
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

			mg.compruebaEstadoMG(mcp.digitalRead(MG_SENSOR));
			pir1.compruebaEstado(mcp.digitalRead(PIR_SENSOR_1));
			pir2.compruebaEstado(mcp.digitalRead(PIR_SENSOR_2));
			pir3.compruebaEstado(mcp.digitalRead(PIR_SENSOR_3));

			if(mg.disparador()){
				Serial.println(F("\nDisparador:  MG"));
				zona = MG;
				respuestaTerminal.resumen = "PUERTA COCHERA";
				setEstadoAlerta();
				break;
			}

			if(pir1.disparador()){
				Serial.println(F("\nDisparador: PIR1"));
				zona = PIR_1;
				respuestaTerminal.resumen = "COCHERA";
				setEstadoAlerta();
				break;
			}

			if(pir2.disparador()){
				Serial.println(F("\nDisparador: PIR2"));
				zona = PIR_2;
				respuestaTerminal.resumen = "PORCHE";
				Serial.print(respuestaTerminal.resumen);
				setEstadoAlerta();
				break;
			}

			if(pir3.disparador()){
				Serial.println(F("\nDisparador: PIR3"));
				zona = PIR_3;
				respuestaTerminal.resumen = "ALMACEN";
				setEstadoAlerta();
				break;
			}

			for (int i = 0; i < 1; i++) { //N_TERMINALES_LINEA
				respuestaTerminal = T_LIST[i]->evaluarDatosTerminal();

				if(respuestaTerminal.interpretacion != TERMINAL_OK){

					if(respuestaTerminal.interpretacion == BAD_COMM
					|| respuestaTerminal.interpretacion == BAD_REPLY
					|| respuestaTerminal.interpretacion == NO_REPLY){
						//Se avisara pero la alarma no saltara
						Serial.print(F("\nTerminal "));
						Serial.print(T_LIST[i]->getTerminalName());
						Serial.println(F(" supera el umbral de fallo en la comunicacion"));
						//Enviar notificacion al server TODO

						//Resetear strikes
						T_LIST[i]->limpiarStrikes();

					}else {
						//Constitutivo de aviso
						zona = PIR_1; //@PEND ADAPTAR

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

			if(INTENTOS_REACTIVACION < 3){

				INTENTOS_REACTIVACION++;
				if(configSystem.MODO_SENSIBLE){
					setMargenTiempo(tiempoSensible,TIEMPO_MODO_SENSIBLE, TIEMPO_MODO_SENSIBLE_TEST);
				}
				setEstadoGuardiaReactivacion();
			}else {
				//TODO cuando no queden reactivaciones hay que sacar a la alarma de aqui para que
				//el SAAS continue recibiendo informes de situacion
				setEstadoInquieto();
			}
		}

		if(INTENTOS_REACTIVACION < 3){
			//Si la alarma aun tiene opciones de reinicio examinamos el phantom
			mg.compruebaPhantom(mcp.digitalRead(MG_SENSOR),datosSensoresPhantom);
			pir1.compruebaPhantom(mcp.digitalRead(PIR_SENSOR_1),datosSensoresPhantom);
			pir2.compruebaPhantom(mcp.digitalRead(PIR_SENSOR_2),datosSensoresPhantom);
			pir3.compruebaPhantom(mcp.digitalRead(PIR_SENSOR_3),datosSensoresPhantom);

			//Linea
			for (int i = 0; i < 1; i++) { //N_TERMINALES_LINEA
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

		//TODO Evaluacion de datos y uso de bocina sin avisos
		/*
		for (int i = 0; i < 1; i++) { //N_TERMINALES_LINEA
			respuestaTerminal = T_LIST[i]->evaluarDatosTerminal();

			if(respuestaTerminal.interpretacion != TERMINAL_OK){

				if(respuestaTerminal.interpretacion != BAD_COMM
						|| respuestaTerminal.interpretacion != BAD_REPLY
						|| respuestaTerminal.interpretacion != NO_REPLY){

					//Constitutivo de aviso
					Serial.println(respuestaTerminal.resumen);
					Serial.println("Aviso inquieto accionando bocina");
				}

			}
		}*/

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

	limpiarSensores();
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


	limpiarSensores();
	lcd_clave_tiempo = millis();

	setMargenTiempo(tiempoBocina, (TIEMPO_BOCINA*0.5), TIEMPO_BOCINA_REACTIVACION_TEST);
	setMargenTiempo(tiempoMargen,(TIEMPO_ON*0.01));

	//Desabilitar puerta tras la reactivacion
	if(configSystem.SENSORES_HABLITADOS[0] && zona == MG ){
		flagPuertaAbierta = 1;
		guardarFlagEE("PUERTA_ABIERTA", 1);

		sensorHabilitado[0] = 0;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	//Se envian los mensajes de reactivacion
	mensaje.mensajeReactivacion(datosSensoresPhantom);
	datosSensoresPhantom.borraDatos();

	Serial.println("Datos phantom");
	Serial.println(T_COCHERA.generarInformeDatos()); //@TEST ONLY

	//Se limpia el resultado del phantom para dejar paso a la nueva ejecucion
	limpiarTerminalesLinea();

	registro.registrarLogSistema("ALARMA ACTIVADA AUTOMATICAMENTE");
	eventosJson.guardarLog(ALARMA_ACTIVADA_AUTOMATICAMENTE_LOG);
	eventosJson.guardarEntrada();
	vTaskResume(envioServidorSaas); //Continua la ejecucion SAAS
}

void setEstadoAlerta()
{
	Serial.println("\nIntrusismo detectado en " + nombreZonas[zona]);
	estadoAlarma = ESTADO_ALERTA;

	ACCESO_LISTAS = 0; //Blindamos las listas de datos
	guardarFlagEE("ESTADO_ALERTA", 1);
	guardarEstadoAlerta();

	lcd_clave_tiempo = millis();

	if(checkearMargenTiempo(tiempoSensible)){
		setMargenTiempo(tiempoMargen,TIEMPO_OFF, TIEMPO_OFF_TEST);
	}else{
		setMargenTiempo(tiempoMargen,(TIEMPO_OFF*TIEMPO_OFF_MODO_SENSIBLE));
	}

	//Liberamos la actividad en modulo
	vTaskSuspend(envioServidorSaas); //Pausa la ejecucion SAAS
	sleepModeGSM = GSM_ON;

	if(!modem.waitForNetwork(1000, true)){
		Serial.println(F("Modulo sin red refrescando"));
		refrescarModuloGSM(); //Si se refresca el modulo no puedes usar seguidamente http
	}

	//Llamamos a la tarea de notificacion
}

void setEstadoEnvio()
{
	Serial.println(F("\nTiempo acabado \nAVISO ENVIADO"));
	estadoAlarma = ESTADO_ENVIO;
	lcd_clave_tiempo = millis();
	setMargenTiempo(tiempoBocina, TIEMPO_BOCINA, TIEMPO_BOCINA_TEST);
	setMargenTiempo(tiempoMargen,TIEMPO_REACTIVACION, TIEMPO_REACTIVACION_TEST);

	//Desde aqui se envian los correspondientes avisos

	mensaje.mensajeAlerta(datosSensores);
	guardarFlagEE("ESTADO_ALERTA", 0);

	//Liberamos el acceso cuando los mensajes esten enviados y limpiamos para el analisis phantom

	Serial.println(T_COCHERA.generarInformeDatos()); //@TEST ONLY

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
	limpiarSensores();
	pararBocina();
	tiempoSensible = millis();
	estadoLlamada = TLF_1;
	desactivaciones ++;
	guardarFlagEE("ESTADO_GUARDIA", 0);
	guardarFlagEE("ESTADO_ALERTA", 0);

	guardarFlagEE("F_RESTAURADO", 0);
	guardarFlagEE("F_REACTIVACION", 0);


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


	//Rehabilitar sensor puerta
	if(flagPuertaAbierta){
		sensorHabilitado[0] = 1;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);

		guardarFlagEE("PUERTA_ABIERTA", 0);
	}

	//Eliminamos la informacion de los terminales
	limpiarSensores();
	limpiarTerminalesLinea();

	registro.registrarLogSistema("ALARMA DESACTIVADA AUTOMATICAMENTE");
	eventosJson.guardarEntrada();
	eventosJson.guardarLog(ALARMA_DESACTIVADA_AUTOMATICAMENTE_LOG);
}
