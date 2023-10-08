/*
 * Checker.h
 *
 *  Created on: 2 sept. 2023
 *      Author: isrev
 *
 * En este fichero se organizan los metodos encargados de checkear distintos eventos
 * en la alarma
 *
 */

#ifndef MUSIC_SOURCE_ALARMAUTIL_CHECKER_H_
#define MUSIC_SOURCE_ALARMAUTIL_CHECKER_H_

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

void checkearAlertasDetenidas(){
		if (leerFlagEEInt("ESTADO_ALERTA") == 1 && leerFlagEEInt("ERR_INTERRUPT") == 0) {

			flagAlertaRestaurada =1;
			eeDatosSalto = NVS_RestoreData<datos_saltos_t>("SALTO_DATA");

			respuestaTerminal.idSensorDetonante = eeDatosSalto.ID_SENSOR;
			respuestaTerminal.idTerminal = eeDatosSalto.ID_TERMINAL;
			INTENTOS_REACTIVACION = eeDatosSalto.INTENTOS_REACTIVACION;

			//Restauramos la lista de saltos
			T_LIST[0]->deserializarListJson(eeDatosSalto.LISTADOS_TERMINALES.terminalCoreJson);

			char registroConjunto[50];
			snprintf(registroConjunto, sizeof(registroConjunto), "%s%s", "CARGADO ESTADO ALERTA EN ", String(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]));

			registro.registrarLogSistema(registroConjunto);
			eventosJson.guardarLog(INTRUSISMO_RESTAURADO_LOG);

			Serial.print("\nIntrusismo restaurado en ");
			Serial.println(literalesZonas[respuestaTerminal.idTerminal][respuestaTerminal.idSensorDetonante]);

			estadoAlarma = ESTADO_ALERTA;
			sleepModeGSM = GSM_ON;
			setMargenTiempo(tiempoMargen,15000);
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

			respuestaTerminal.idSensorDetonante = eeDatosSalto.ID_SENSOR;
			respuestaTerminal.idTerminal = eeDatosSalto.ID_TERMINAL;
			INTENTOS_REACTIVACION = eeDatosSalto.INTENTOS_REACTIVACION;

			//Restauramos la lista de saltos
			T_LIST[0]->deserializarListJson(eeDatosSalto.LISTADOS_TERMINALES.terminalCoreJson);


			estadoError = COMPROBAR_DATOS;
		}
	}
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

void checkearFalloEnAlimientacion(){
	if(mcp.digitalRead(FALLO_BATERIA_PRINCIPAL) == HIGH){
		interrupcionFalloAlimentacion();
	}
}


void checkearSensorPuertaCochera(){
	alertsInfoLcd[INFO_SENSOR_PUERTA_OFF] = !configSystem.SENSORES_HABLITADOS[0];
}


void checkearEnvioSaas(){
	if(!configSystem.ENVIO_SAAS)
		return;

	static unsigned long lastExecutionTime = 0;

#ifdef ALARMA_EN_MODO_DEBUG
	if (millis() - lastExecutionTime >= 30000) {
#else
    if (millis() - lastExecutionTime >= (((configSystem.ESPERA_SAAS_MULTIPLICADOR*5)+10)*60000)) { //600000
#endif
		//Encolar envio modelo
		encolarEnvioModeloSaas();
		lastExecutionTime = millis();
	}
}


#endif /* MUSIC_SOURCE_ALARMAUTIL_CHECKER_H_ */