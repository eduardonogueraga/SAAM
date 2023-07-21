/**
 * SISTEMA DE ALARMA DE LA ALBERQUILLA (MUSIC)
 * Version: VE21R0
 * Leyenda: @develop  @PEND (Pendiente de adaptar a ESP32)
 *
 * POR HACER:
 * -Enriquecer el log con dia de la semana o temperatura
 * -Ajustar los requerimientos de SAAS
 *
 */



#include "Arduino.h"
#include "Alarma.h"

void EstadoInicio(){

	procesoCentral = ALARMA;
	estadoAlarma = ESTADO_REPOSO;
	sleepModeGSM = GSM_ON;

	//Acutalizamos el secuencial de los logs para esta ejecucion
	guardarFlagEE("LOG_SEQ", (leerFlagEEInt("LOG_SEQ")+1));


	//Actualizamos los intentos
	//guardarFlagEE("JSON_RETRY", 0);
	//guardarFlagEE("JSON_SEQ", 1);

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
	    mcp.pinMode(RS_CTL, OUTPUT);

		//PUERTOS INTERNOS
	    mcp.pinMode(BOCINA_PIN, OUTPUT);
	    mcp.pinMode(RELE_AUXILIAR, OUTPUT);
	    mcp.pinMode(LED_COCHERA, OUTPUT);
	    mcp.pinMode(RESETEAR,OUTPUT);
	    mcp.pinMode(WATCHDOG, OUTPUT);
	    mcp.pinMode(SENSOR_BATERIA_RESPALDO, INPUT);
	    //attachInterrupt(digitalPinToInterrupt(FALLO_BATERIA_PRINCIPAL), interrupcionFalloAlimentacion, FALLING); @PEND

	    //Configuracion de los puertos

	    rcomp1.digitalWrite(LED_COCHERA, LOW);
	    rcomp1.digitalWrite(RS_CTL,LOW);

	    //Activamos el modulo GSM
	    rcomp1.digitalWrite(GSM_PIN, HIGH);

	    EstadoInicio();
	    cargarEstadoPrevio();
	    checkearAlertasDetenidas();
	    chekearInterrupciones();

	    eventosJson.iniciarModeloJSON();
	    registro.registrarLogSistema("ALARMA INICIADA");
	    eventosJson.guardarLog(ALARMA_INICIADA_LOG);

	    //Hilo 0
	    xTaskCreatePinnedToCore(loop2,"loop_2",5000,NULL,1,&tareaLoopDos,0);

	    // Iniciar el planificador de tareas
	    //vTaskStartScheduler();


	    //SIM800L
	    if(MODO_DEFAULT)
	    comprobarConexionGSM();

}


void loop()
{

	leerEntradaTeclado();
	demonio.demonioSerie();

	procesosSistema();
	procesosPrincipales();
	linea.mantenerComunicacion();
	//rcomp1.test();

}


void loop2(void *parameter){
	for(;;){

		/*
		Serial.print("hola: ");
		Serial.println(xPortGetCoreID());
		Serial.println(rcomp0.digitalRead(SENSOR_BATERIA_RESPALDO));
		delay(1000);
		 */
		//rcomp0.digitalRead(SENSOR_BATERIA_RESPALDO);
		//rcomp0.digitalWrite(RS_CTL, HIGH);
		//delay(100);

		//linea.mantenerComunicacion();
		//rcomp0.test();
		vTaskDelay(10);

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

			mg.compruebaEstadoMG(rcomp1.digitalRead(MG_SENSOR));
			pir1.compruebaEstado(rcomp1.digitalRead(PIR_SENSOR_1));
			pir2.compruebaEstado(rcomp1.digitalRead(PIR_SENSOR_2));
			pir3.compruebaEstado(rcomp1.digitalRead(PIR_SENSOR_3));


			if(mg.disparador()){
				Serial.println(F("\nDisparador:  MG"));
				zona = MG;
				setEstadoAlerta();
			}

			if(pir1.disparador()){
				Serial.println(F("\nDisparador: PIR1"));
				zona = PIR_1;
				setEstadoAlerta();
			}

			if(pir2.disparador()){
				Serial.println(F("\nDisparador: PIR2"));
				zona = PIR_2;
				setEstadoAlerta();
			}

			if(pir3.disparador()){
				Serial.println(F("\nDisparador: PIR3"));
				zona = PIR_3;
				setEstadoAlerta();
			}

			//Terminal check
			InterpretacionTerminal interpretacion;

			for (int i = 0; i < 1; i++) { //N_TERMINALES_LINEA
				interpretacion = T_LIST[i]->evaluarDatosTerminal();

				if(interpretacion != TERMINAL_OK){
					zona = PIR_1; //@PEND ADAPTAR
					setEstadoAlerta();
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
			}

		}

		mg.compruebaPhantom(rcomp1.digitalRead(MG_SENSOR),datosSensoresPhantom);
		pir1.compruebaPhantom(rcomp1.digitalRead(PIR_SENSOR_1),datosSensoresPhantom);
		pir2.compruebaPhantom(rcomp1.digitalRead(PIR_SENSOR_2),datosSensoresPhantom);
		pir3.compruebaPhantom(rcomp1.digitalRead(PIR_SENSOR_3),datosSensoresPhantom);


		for (int i = 0; i < 1; i++) { //N_TERMINALES_LINEA
			T_LIST[i]->evaluarPhantomTerminal();
		}


		realizarLlamadas();
		sonarBocina();
		desactivarAlarma();

		break;
	}
}


void setEstadoGuardia()
{
	Serial.println(F("\nAlarma Activada"));
	estadoAlarma = ESTADO_GUARDIA;
	guardarFlagEE("ESTADO_GUARDIA", 1);

	//sleepModeGSM = GSM_OFF;
	limpiarSensores();
	lcd_clave_tiempo = millis();
	lcd_info_tiempo = millis() + TIEMPO_ALERT_LCD;
	setMargenTiempo(tiempoMargen,TIEMPO_ON, TIEMPO_ON_TEST);

	registro.registrarLogSistema("ALARMA ACTIVADA MANUALMENTE");
	eventosJson.guardarEntrada();
	//eventosJson.guardarLog(); @PEND
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
	setMargenTiempo(prorrogaGSM, TIEMPO_PRORROGA_GSM, TIEMPO_PRORROGA_GSM_TEST);
	//sleepModeGSM = GSM_TEMPORAL;


	//Desabilitar puerta tras la reactivacion
	if(configSystem.SENSORES_HABLITADOS[0] && zona == MG ){
		flagPuertaAbierta = 1;
		guardarFlagEE("PUERTA_ABIERTA", 1);

		sensorHabilitado[0] = 0;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	mensaje.mensajeReactivacion(datosSensoresPhantom);
	datosSensoresPhantom.borraDatos();

	for (int i = 0; i < 1; i++) { //N_TERMINALES_LINEA
		T_LIST[i]->limpiarResultadoPhantom();
	}

	registro.registrarLogSistema("ALARMA ACTIVADA AUTOMATICAMENTE");
	//eventosJson.guardarLog(); @PEND
	eventosJson.guardarEntrada();
}

void setEstadoAlerta()
{
	Serial.println("\nIntrusismo detectado en " + nombreZonas[zona]);
	estadoAlarma = ESTADO_ALERTA;
	guardarFlagEE("ESTADO_ALERTA", 1);
	guardarEstadoAlerta();

	lcd_clave_tiempo = millis();

	if(checkearMargenTiempo(tiempoSensible)){
		setMargenTiempo(tiempoMargen,TIEMPO_OFF, TIEMPO_OFF_TEST);
	}else{
		setMargenTiempo(tiempoMargen,(TIEMPO_OFF*TIEMPO_OFF_MODO_SENSIBLE));
	}

	sleepModeGSM = GSM_ON;
}

void setEstadoEnvio()
{
	Serial.println(F("\nTiempo acabado \nAVISO ENVIADO"));
	estadoAlarma = ESTADO_ENVIO;
	lcd_clave_tiempo = millis();
	setMargenTiempo(tiempoBocina, TIEMPO_BOCINA, TIEMPO_BOCINA_TEST);
	setMargenTiempo(tiempoMargen,TIEMPO_REACTIVACION, TIEMPO_REACTIVACION_TEST);

	mensaje.mensajeAlerta(datosSensores);
	guardarFlagEE("ESTADO_ALERTA", 0);
}

void setEstadoReposo()
{
	Serial.println(F("\nAlarma Desactivada"));
	estadoAlarma = ESTADO_REPOSO;

	lcd_clave_tiempo = millis();
	lcd_info_tiempo = millis() + TIEMPO_ALERT_LCD;

	INTENTOS_REACTIVACION = 0;
	limpiarSensores();
	pararBocina();
	tiempoSensible = millis();
	//sleepModeGSM = GSM_OFF;
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

	registro.registrarLogSistema("ALARMA DESACTIVADA MANUALMENTE");
	eventosJson.guardarEntrada();
	//eventosJson.guardarLog(); @PEND
}
