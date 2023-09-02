/*
 * Restauracion.h
 *
 *  Created on: 2 sept. 2023
 *      Author: Eduardo
 *
 *      Funciones encargadas de restuarar el estado previo tras reinicio
 *      inesperado
 */

#ifndef SOURCE_ALARMAUTIL_RESTAURACION_H_
#define SOURCE_ALARMAUTIL_RESTAURACION_H_


void cargarEstadoPrevio(){
	flagPuertaAbierta = leerFlagEEInt("PUERTA_ABIERTA") == 1;


	if (leerFlagEEInt("ESTADO_GUARDIA") == 1) {
		Serial.println("Estado de guardia restaurado");
		estadoAlarma = ESTADO_GUARDIA;
		registro.registrarLogSistema("CARGADO ESTADO GUARDIA PREVIO");
		eventosJson.guardarLog(CARGADO_ESTADO_GUARDIA_PREVIO_LOG);

		//Informar de que se ha restaurado una entrada
		guardarFlagEE("F_RESTAURADO", 1);
	}


}


void guardarEstadoInterrupcion(){
		guardarFlagEE("ERR_INTERRUPT", 1);
		guardarFlagEE("INTERUP_HIST", (leerFlagEE("INTERUP_HIST") + 1));
	}


void guardarEstadoAlerta(){

	eeDatosSalto.ID_SENSOR = respuestaTerminal.idSensorDetonante;
	eeDatosSalto.ID_TERMINAL = respuestaTerminal.idTerminal;
	eeDatosSalto.INTENTOS_REACTIVACION = INTENTOS_REACTIVACION;

	//Guardar lista de nodos
	char temp[2048];
	T_LIST[0]->serializarListaJson().toCharArray(temp, 2048);
	strcpy(eeDatosSalto.LISTADOS_TERMINALES.terminalCoreJson, temp);

	NVS_SaveData<datos_saltos_t>("SALTO_DATA", eeDatosSalto);

}

#endif /* SOURCE_ALARMAUTIL_RESTAURACION_H_ */
