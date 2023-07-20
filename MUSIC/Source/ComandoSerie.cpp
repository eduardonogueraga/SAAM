/*
 * ComandoSerie.cpp
 *
 *  Created on: 29 jul. 2021
 *      Author: isrev
 */

#include "ComandoSerie.h"


ComandoSerie::ComandoSerie() {
	// TODO Auto-generated constructor stub
}

void ComandoSerie::demonioSerie(){

	if (Serial.available() > 0 ){

		if(Serial.available() > 0)

		comandoRecibido = Serial.readStringUntil('\n');
		comandoRecibido.trim();
		comandoRecibido.toCharArray(data, sizeof(data));

		comprobarComando();
	}

}

boolean ComandoSerie::compararCadena(const char* data, const char* cadena) {
  return strcmp(data, cadena) == 0;
}

void ComandoSerie::nombreComando(const char* data){
	Serial.print("demonio@saa:~$	");
	Serial.println(data);
}

void ComandoSerie::comprobarComando() {


	if (compararCadena(data, "set on")) {
		nombreComando(data);
		setEstadoGuardia();
	}

	if (compararCadena(data, "set off")) {
		nombreComando(data);
		setEstadoReposo();
		//desactivarAlarma();
	}

	if (compararCadena(data, "set mode")) {
		nombreComando(data);

		if(MODO_DEFAULT){
			Serial.println("Alarma en modo de pruebas");
			MODO_DEFAULT = 0;
		}else {
			Serial.println("Alarma en modo default");
			MODO_DEFAULT = 1;
		}
	}

	if (compararCadena(data, "menu")) {
		nombreComando(data);
		if(procesoCentral == ALARMA){
			procesoCentral = MENU;

			return;
		}

		if(procesoCentral == MENU){

			procesoCentral = ALARMA;
			return;
		}

	}

	if (compararCadena(data, "pir1")) {
		nombreComando(data);
		pir1.pingSensor();
	}

	if (compararCadena(data, "pir2")) {
		nombreComando(data);
		pir2.pingSensor();
	}

	if (compararCadena(data, "pir3")) {
		nombreComando(data);
		pir3.pingSensor();
	}

	if (compararCadena(data, "mg")) {
		nombreComando(data);
		mg.pingSensor();
	}

	if (compararCadena(data, "ch puerta")) {
		nombreComando(data);
		sensorHabilitado[0] = !sensorHabilitado[0];
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);

		if(!sensorHabilitado[0]){
			Serial.println("Sensor puerta deshabilitado" );

		}else{
			Serial.println("Sensor puerta habilitado" );
		}
	}

	if (compararCadena(data, "mail")) {
		nombreComando(data);
		setEstadoEnvio();
	}


	if (compararCadena(data, "ls")){
		nombreComando(data);
		registro.listarRegistros();
	}

	if (compararCadena(data, "log")){
		nombreComando(data);
		Serial.println("Mostrando contenido");
		registro.mostrarRegistro();
	}


	if (compararCadena(data, "clear")){
		nombreComando(data);
		registro.borrarRegistros();
	}


	if (compararCadena(data, "json -h")){
		nombreComando(data);

		Serial.println(registro.extraerPrimerElemento());
	}

	if (compararCadena(data, "clear json")){
		nombreComando(data);
		registro.borrarRegistros(DIR_JSON_REQUEST);
	}


	if (compararCadena(data, "json")){
		nombreComando(data);
		eventosJson.mostrarModeloJSON();
	}

	if (compararCadena(data, "json -p")){
		nombreComando(data);
		eventosJson.purgarModeloJSON();
	}


	if (compararCadena(data, "json -make")){
		nombreComando(data);
		eventosJson.componerJSON();
	}

	if (compararCadena(data, "cat json")){
		nombreComando(data);
		registro.mostrarRegistro(DIR_JSON_REQUEST);
	}

	if (compararCadena(data, "ls json")){
		nombreComando(data);
		registro.listarRegistros(DIR_JSON_REQUEST);
	}

	if (compararCadena(data, "json -e")){
		nombreComando(data);
		eventosJson.exportarFichero();
	}

	if (compararCadena(data, "json -save")){
		nombreComando(data);
		eventosJson.guardarJsonNVS();
	}

	if (compararCadena(data, "json -load")){
		nombreComando(data);
		eventosJson.cargarJsonNVS();
	}

	if (compararCadena(data, "json -t")){
			nombreComando(data);
			eventosJson.purgarModeloJSON();
			eventosJson.componerJSON();
			eventosJson.guardarJsonNVS();
			eventosJson.cargarJsonNVS();
		}


	if (compararCadena(data, "json -send")){
		nombreComando(data);
		eventosJson.enviarInformeSaas();
	}

	if (compararCadena(data, "rs -t")){
		nombreComando(data);
		linea.testUart();
		delay(5);
	}


	if (compararCadena(data, "t -add")){
		nombreComando(data);
		 byte myArray[] = {1, 0, 0, 0, 0, 0, 0, 0};
		 byte myArray2[] = {0, 0};
		T_COCHERA.guardarDatosTerminal(myArray, myArray2);
	}

	if (compararCadena(data, "t -show")){
		nombreComando(data);
		T_COCHERA.recorrerDatosTerminal();

		Serial.println(T_COCHERA.getDatosFotosensor());
/*
		for (int i = 0; i < 2; i++) {
			Serial.print(T_COCHERA.datosControlLineas[i]);
			Serial.print(" ");
		}
*/
	}


	if (compararCadena(data, "t -df")){
		nombreComando(data);
		T_COCHERA.borrarPrimerElemento();
	}


	if (compararCadena(data, "t -dl")){
		nombreComando(data);
		T_COCHERA.borrarUltimoElemento();
	}

	if(compararCadena(data, "at")){
		nombreComando(data);
		UART_GSM.println("AT");
	}


	if(compararCadena(data, "speed -c")){
		nombreComando(data);
		UART_GSM.println("AT+IPR=115200");
		UART_GSM.println("AT&W");
		UART_GSM.println("AT+CFUN=1,1");
	}


	if(compararCadena(data, "power")){
		nombreComando(data);
		interrupcionFalloAlimentacion();
	}

	if(compararCadena(data, "d")){
		nombreComando(data);
		Serial.println(datosSensores.imprimeDatos());
	}

	if(compararCadena(data, "info")){
		nombreComando(data);
		printSystemInfo();
	}

	if(compararCadena(data, "clock")){
		nombreComando(data);
		configSystem.MODULO_RTC = !configSystem.MODULO_RTC;
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	if(compararCadena(data, "bye")){
		nombreComando(data);
		resetear();
	}

}











