/**
 * SISTEMA DE ALARMA DE LA ALBERQUILLA (MUSIC)
 * Version: VE21R0
 * Leyenda: @develop  @PEND (Pendiente de adaptar a ESP32)
 *
 * POR HACER:
 */

#include "Arduino.h"
#include "Alarma.h"

void EstadoInicio(){

	procesoCentral = ALARMA;
	estadoAlarma = ESTADO_REPOSO;
	sleepModeGSM = GSM_ON;

	//Acutalizamos el secuencial de los logs para esta ejecucion
	guardarFlagEE("LOG_SEQ", (leerFlagEEInt("LOG_SEQ")+1));
	registro.registrarLogSistema("ALARMA INICIADA");

	//if(!MODO_DEFAULT)
	//printSystemInfo();


	if(configSystem.MODULO_RTC){
		fecha.establecerFechaReset(10);
	}
}

void setup()
{
	  Serial.begin(115200);
	  MySerial2.begin(115200, SERIAL_8N1, 5, 18); //RX TX  (H1 = RX5 TX 18) PUERTO RS
	  MySerial.begin(115200, SERIAL_8N1, 23, 19); //RX TX  (H2 = RX23 TX19)
	  Serial.println(version[0]);

	  //Restaurar configuracion almacenada
	  //EEPROM_RestoreData(EE_CONFIG_STRUCT, configSystem);
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


	  if(registro.iniciar() == 0){
		  Serial.println(F("ERROR AL INICIAR SD"));
		  pantallaDeErrorInicial(F("  SYSTM ERROR!  ERROR INICIAR SD"));
	  }
	  //mensaje.inicioSIM800(SIM800L);

	  pantalla.lcdLoadView(&pantalla, &Pantalla::lcdInicio);
	  delay(2000);


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
	    //attachInterrupt(digitalPinToInterrupt(FALLO_BATERIA_PRINCIPAL), interrupcionFalloAlimentacion, FALLING);

	    //Configuracion de los puertos

	    mcp.digitalWrite(LED_COCHERA, LOW);
	    mcp.digitalWrite(RS_CTL,LOW);


	   EstadoInicio();
	   cargarEstadoPrevio();
	   checkearAlertasDetenidas();
	   chekearInterrupciones();


}


void loop()
{

	  //Serial.print("Estado de la entrada digital: ");
	  //Serial.println(digitalRead(SENSOR_BATERIA_RESPALDO));

	 // delay(1000); // Esperar 1 segundo

	 //Serial.println("###TRAZA|PRUEBA|TEST|FIN###");
	 // delay(2000);
/*
	 if (Serial.available()) {
		 while(Serial.available()){
			 // Leer el caracter y enviarlo al puerto Serial2
				    char c = Serial.read();
				    MySerial2.write(c);
		 }

	  }

	 if (MySerial.available()) {
	    // Leer el caracter y mostrarlo en el Monitor Serie
		 while(MySerial.available()){
			// delay(10);
			  char c = MySerial.read();
			  Serial.write(c);
		 }

	  }

 */
	//MySerial.println("¡Hola desde Arduino!");

	leerEntradaTeclado();
	demonio.demonioSerie();

	procesosSistema();
	procesosPrincipales();

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

		mg.compruebaPhantom(mcp.digitalRead(MG_SENSOR),datosSensoresPhantom);
		pir1.compruebaPhantom(mcp.digitalRead(PIR_SENSOR_1),datosSensoresPhantom);
		pir2.compruebaPhantom(mcp.digitalRead(PIR_SENSOR_2),datosSensoresPhantom);
		pir3.compruebaPhantom(mcp.digitalRead(PIR_SENSOR_3),datosSensoresPhantom);

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
	//EEPROM.update(EE_ESTADO_GUARDIA, 1);
	guardarFlagEE("ESTADO_GUARDIA", 1);

	sleepModeGSM = GSM_OFF;
	limpiarSensores();
	lcd_clave_tiempo = millis();
	lcd_info_tiempo = millis() + TIEMPO_ALERT_LCD;
	setMargenTiempo(tiempoMargen,TIEMPO_ON, TIEMPO_ON_TEST);

	//insertQuery(&sqlActivarAlarmaManual);
	registro.registrarLogSistema("ALARMA ACTIVADA MANUALMENTE");
}

void setEstadoGuardiaReactivacion()
{
	Serial.println("\nAlarma Reactivada. Intentos realizados: "+ (String)INTENTOS_REACTIVACION);
	estadoAlarma = ESTADO_GUARDIA;
	//EEPROM.update(EE_ESTADO_GUARDIA, 1);
	guardarFlagEE("ESTADO_GUARDIA", 1);



	limpiarSensores();
	lcd_clave_tiempo = millis();

	setMargenTiempo(tiempoBocina, (TIEMPO_BOCINA*0.5), TIEMPO_BOCINA_REACTIVACION_TEST);
	setMargenTiempo(tiempoMargen,(TIEMPO_ON*0.01));
	setMargenTiempo(prorrogaGSM, TIEMPO_PRORROGA_GSM, TIEMPO_PRORROGA_GSM_TEST);
	sleepModeGSM = GSM_TEMPORAL;


	//Desabilitar puerta tras la reactivacion
	if(configSystem.SENSORES_HABLITADOS[0] && zona == MG ){
		flagPuertaAbierta = 1;
		//EEPROM.update(EE_FLAG_PUERTA_ABIERTA, 1);
		guardarFlagEE("PUERTA_ABIERTA", 1);

		sensorHabilitado[0] = 0;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		//EEPROM_SaveData(EE_CONFIG_STRUCT, configSystem);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	mensaje.mensajeReactivacion(datosSensoresPhantom);
	datosSensoresPhantom.borraDatos();

	//insertQuery(&sqlActivarAlarmaAutomatico);
	registro.registrarLogSistema("ALARMA ACTIVADA AUTOMATICAMENTE");
}

void setEstadoAlerta()
{
	Serial.println("\nIntrusismo detectado en " + nombreZonas[zona]);
	estadoAlarma = ESTADO_ALERTA;
	//EEPROM.update(EE_ESTADO_ALERTA, 1);
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
	//EEPROM.update(EE_ESTADO_ALERTA, 0);
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
	sleepModeGSM = GSM_OFF;
	estadoLlamada = TLF_1;
	desactivaciones ++;
	//EEPROM.update(EE_ESTADO_GUARDIA, 0);
	//EEPROM.update(EE_ESTADO_ALERTA, 0);
	guardarFlagEE("ESTADO_GUARDIA", 0);
	guardarFlagEE("ESTADO_ALERTA", 0);


	//Rehabilitar sensor puerta
	if(flagPuertaAbierta){
		sensorHabilitado[0] = 1;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		//EEPROM_SaveData(EE_CONFIG_STRUCT, configSystem);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);


		//EEPROM.update(EE_FLAG_PUERTA_ABIERTA, 0);
		guardarFlagEE("PUERTA_ABIERTA", 0);
	}

	//insertQuery(&sqlDesactivarAlarma);
	registro.registrarLogSistema("ALARMA DESACTIVADA MANUALMENTE");
}
