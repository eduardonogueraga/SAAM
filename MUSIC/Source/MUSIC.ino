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

	/*
	sleepModeGSM = GSM_ON;
	sleepModeBT = BT_OFF;

	fecha.establecerFechaReset(10);
	*/
}

void setup()
{
	  Serial.begin(115200);
	  Serial.println(version[0]);



	  //Restaurar configuracion almacenada
	  //EEPROM_RestoreData(EE_CONFIG_STRUCT, configSystem); @PEND

	  //Inicio de perifericos
	  iniciarTecladoI2C();
	  pantalla.iniciar();

	  pantalla.lcdLoadView(&pantalla, &Pantalla::lcdInicio);
	  delay(2000);

	  /*

		mensaje.inicioSIM800(SIM800L);
		registro.iniciar();
		fecha.iniciarRTC();


		*/

	  //Declaracion de los puertos I/O
	  	/*
	  	//PIR
	    pinMode(PIR_SENSOR_1, INPUT);
	    pinMode(PIR_SENSOR_2, INPUT);
	    pinMode(PIR_SENSOR_3, INPUT);
	    pinMode(MG_SENSOR, INPUT_PULLUP);

	    //MODULO GSM
		pinMode(GSM_PIN, OUTPUT);

		//MODULO RS485
		pinMode(RS_CTL, OUTPUT);

		//PUERTOS INTERNOS
		pinMode(BOCINA_PIN, OUTPUT);
		pinMode(LED_COCHERA, OUTPUT);
	    pinMode(RESETEAR,OUTPUT);
	    pinMode(WATCHDOG, OUTPUT);
        pinMode(SENSOR_BATERIA_RESPALDO, INPUT_PULLUP);
	    //attachInterrupt(digitalPinToInterrupt(FALLO_BATERIA_PRINCIPAL), interrupcionFalloAlimentacion, FALLING);

	  //Configuracion de los puertos
	    digitalWrite(LED_COCHERA, LOW);
	    digitalWrite(RS_CTL,LOW);
	  	 */
	    EstadoInicio();
	    //cargarEstadoPrevio();
	    //checkearAlertasDetenidas();
	    //chekearInterrupciones();

}


void loop()
{

	leerEntradaTeclado();
	demonio.demonioSerie();

	procesosSistema();
	procesosPrincipales();

}


void procesosSistema(){
	/*
	watchDog();
	sleepMode();
	checkearSms();
	resetAutomatico();
	checkearBateriaDeEmergencia();
	checkearSensorPuertaCochera();
	avisoLedPuertaCochera();
	resetearAlarma();
	*/
}

void procesosPrincipales()
{
	switch(procesoCentral){

	case ALARMA:
		procesoAlarma();
		break;

	case MENU:
		//menu.procesoMenu();
		break;

	case ERROR:
		//procesoError();
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

				if(key == '2'){ //Tecla menu
					procesoCentral = MENU;
				}
			}
		}

		break;

	case ESTADO_GUARDIA:

		if(!isLcdInfo())
		pantalla.lcdLoadView(&pantalla, &Pantalla::lcdGuardia);

		if(checkearMargenTiempo(tiempoMargen)){

			//mg.compruebaEstadoMG(digitalRead(MG_SENSOR));
			//pir1.compruebaEstado(digitalRead(PIR_SENSOR_1)); // @suppress("Ambiguous problem")
			//pir2.compruebaEstado(digitalRead(PIR_SENSOR_2)); // @suppress("Ambiguous problem")
			//pir3.compruebaEstado(digitalRead(PIR_SENSOR_3)); // @suppress("Ambiguous problem")
			/*
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
			*/
		}

		//sonarBocina();
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

		//mg.compruebaPhantom(digitalRead(MG_SENSOR),datosSensoresPhantom);
		//pir1.compruebaPhantom(digitalRead(PIR_SENSOR_1),datosSensoresPhantom);
		//pir2.compruebaPhantom(digitalRead(PIR_SENSOR_2),datosSensoresPhantom);
		//pir3.compruebaPhantom(digitalRead(PIR_SENSOR_3),datosSensoresPhantom);

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
	//EEPROM.update(EE_ESTADO_GUARDIA, 1); @PEND

	sleepModeGSM = GSM_OFF;
	limpiarSensores();
	lcd_clave_tiempo = millis();
	lcd_info_tiempo = millis() + TIEMPO_ALERT_LCD;
	setMargenTiempo(tiempoMargen,TIEMPO_ON, TIEMPO_ON_TEST);

	//insertQuery(&sqlActivarAlarmaManual);
}

void setEstadoGuardiaReactivacion()
{
	Serial.println("\nAlarma Reactivada. Intentos realizados: "+ (String)INTENTOS_REACTIVACION);
	estadoAlarma = ESTADO_GUARDIA;
	//EEPROM.update(EE_ESTADO_GUARDIA, 1); @PEND

	limpiarSensores();
	lcd_clave_tiempo = millis();

	setMargenTiempo(tiempoBocina, (TIEMPO_BOCINA*0.5), TIEMPO_BOCINA_REACTIVACION_TEST);
	setMargenTiempo(tiempoMargen,(TIEMPO_ON*0.01));
	setMargenTiempo(prorrogaGSM, TIEMPO_PRORROGA_GSM, TIEMPO_PRORROGA_GSM_TEST);
	sleepModeGSM = GSM_TEMPORAL;


	//Desabilitar puerta tras la reactivacion
	if(configSystem.SENSORES_HABLITADOS[0] && zona == MG ){
		flagPuertaAbierta = 1;
		//EEPROM.update(EE_FLAG_PUERTA_ABIERTA, 1); @PEND

		sensorHabilitado[0] = 0;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		EEPROM_SaveData(EE_CONFIG_STRUCT, configSystem);
	}

	//mensaje.mensajeReactivacion(datosSensoresPhantom); @PEND
	//datosSensoresPhantom.borraDatos(); @PEND

	//insertQuery(&sqlActivarAlarmaAutomatico);
}

void setEstadoAlerta()
{
	Serial.println("\nIntrusismo detectado en " + nombreZonas[zona]);
	estadoAlarma = ESTADO_ALERTA;
	//EEPROM.update(EE_ESTADO_ALERTA, 1);
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

	//mensaje.mensajeAlerta(datosSensores);
	//EEPROM.update(EE_ESTADO_ALERTA, 0); @PEND
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
	//EEPROM.update(EE_ESTADO_GUARDIA, 0); @PEND
	//EEPROM.update(EE_ESTADO_ALERTA, 0); @PEND

	//Rehabilitar sensor puerta
	if(flagPuertaAbierta){
		sensorHabilitado[0] = 1;
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		EEPROM_SaveData(EE_CONFIG_STRUCT, configSystem);

		//EEPROM.update(EE_FLAG_PUERTA_ABIERTA, 0); @PEND
	}

	//insertQuery(&sqlDesactivarAlarma);
}
