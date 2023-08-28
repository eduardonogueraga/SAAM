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


void ComandoSerie::esperarRespuestaUart2(){
	for (int i = 0; i < 10; i++)
	{
		// Verificamos si hay datos disponibles para leer

		char tramaRecibida[200] = "";
		size_t byteCount = UART_GSM.readBytesUntil('\n', tramaRecibida, sizeof(tramaRecibida) - 1); //read in data to buffer
		tramaRecibida[byteCount] = NULL;	//put an end character on the data


		Serial.print("UART@GSM-> ");
		Serial.println(tramaRecibida);
		UART_GSM.flush();
		i++;
	}
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

	if (compararCadena(data, "set in")) {
		nombreComando(data);
		setEstadoInquieto();
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
		byte myArray[] = {1, 0, 0, 0, 0, 0, 0, 0};
		byte myArray2[] = {0, 0};
		T_CORE.guardarDatosTerminal(myArray, myArray2);
	}

	if (compararCadena(data, "pir2")) {
		nombreComando(data);
		byte myArray[] = {0, 1, 0, 0, 0, 0, 0, 0};
		byte myArray2[] = {0, 0};
		T_CORE.guardarDatosTerminal(myArray, myArray2);
	}

	if (compararCadena(data, "pir3")) {
		nombreComando(data);
		byte myArray[] = {0, 0, 1, 0, 0, 0, 0, 0};
		byte myArray2[] = {0, 0};
		T_CORE.guardarDatosTerminal(myArray, myArray2);
	}

	if (compararCadena(data, "mg")) {
		nombreComando(data);
		sensorCore.sensorMG = 1;
		Serial.println("Comando pendiente de adpatacion");
	}

	//Sensores core

	if (compararCadena(data, "cp")){
		nombreComando(data);
		Serial.println(T_CORE.generarInformeDatos());
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

	if (compararCadena(data, "json -hl")){
		nombreComando(data);

		Serial.println(registro.leerPrimerElemento());
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

	if (compararCadena(data, "json -send not")){
		nombreComando(data);
		//Nivel de modelo
		eventosJson.enviarNotificacionSaas(0, "Movimiento en patio");
	}


	if (compararCadena(data, "note -t")){
		String t = "Movimiento en patio";
		//Nivel de composer
		enviarNotificacionesSaas(1, "Movimiento en patio");
	}

	if(compararCadena(data, "note -send")){
		nombreComando(data);
		//Nivel RTOS
		crearTareaNotificacionSaas(1, "Movimiento en patio 1,2,3");
	}


	if (compararCadena(data, "json -ch")){
		nombreComando(data);

		registro.actualizarUltimoElemento("retry");
		registro.actualizarUltimoElemento("id", 455);
	}


	if(compararCadena(data, "http -id")){
		nombreComando(data);

		Serial.println( leerFlagEEInt("PACKAGE_ID"));
		getIdPaqueteSaas();
		Serial.print("SALIDA->");
		Serial.println( leerFlagEEInt("PACKAGE_ID"));

	}


	if(compararCadena(data, "http -token")){
		nombreComando(data);

		Serial.println(leerCadenaEE("SAAS_TOKEN"));
		generarTokenSaas();
		Serial.println(leerCadenaEE("SAAS_TOKEN"));

	}

	if(compararCadena(data, "http -pack")){
		nombreComando(data);

		String json = R"(
		{
		  "version": "VE21R0",
		  "retry": "0",
		  "id": "58", 
		  "date": "2023-07-27T16:23:08",
		  "System": [
		    {
		      "action": "0",
		      "msen": "1",
		      "alive": "568962",
		      "traffic": "0|0|35|1",
		      "modules": "1|1|0", 
		      "sensors": "102;1|103;1|104;1|105;1",
		      "reset": "2023-08-06T16:13:45"
		    }
		  ],
		  "Entry": [
		    {
		      "isnew": true,
		      "reg": "1|0|0|0",
		      "date": "2023-07-27T16:23:15"
		    }, 
		    {
		      "isnew": true,
		      "reg": "0|0|0|0",
		      "date": "2023-07-27T16:23:18"
		    }
		  ]
		}
		)";


		postDatosSaas(&json, PAQUETE);

	}

	if(compararCadena(data, "http -not")){
		nombreComando(data);

		String json = R"(
			{
				"type": 1,
				"reg": "Movimiento detectado en PIR3",
				"date": "2023-08-10T18:50:52"
			}
			)";

		postDatosSaas(&json, NOTIFICACION);
	}

	if(compararCadena(data, "token")){
		nombreComando(data);
		Serial.println(leerCadenaEE("SAAS_TOKEN"));
	}


	if (compararCadena(data, "ls http")){
		nombreComando(data);
		registro.listarRegistros(DIR_HTTP_LOG);
	}

	if (compararCadena(data, "cat http")){
		nombreComando(data);
		registro.mostrarRegistro(DIR_HTTP_LOG);
	}

	if (compararCadena(data, "purge http")){
		nombreComando(data);
		registro.borrarRegistros(DIR_HTTP_LOG);
	}

	if (compararCadena(data, "make http")){
			nombreComando(data);


			 char httpResponse[] = "HTTP/1.1 200 OK\r\n"
			                          "Content-Type: application/json\r\n"
			                          "\r\n"
			                          "{\r\n"
			                          "    \"id\": 123,\r\n"
			                          "    \"nombre\": \"Juan Pérez\",\r\n"
			                          "    \"correo\": \"juan@example.com\",\r\n"
			                          "    \"edad\": 30\r\n"
			                          "}\r\n";

			registro.registrarLogHttpRequest(httpResponse);
		}

	if (compararCadena(data, "encolar reg")){

		RegistroLogTarea reg;
		TickType_t espera = pdMS_TO_TICKS(50);
		snprintf(reg.log, sizeof(reg.log), "Soy un alocado registro de prueba");
		reg.tipoLog = 0; //systema
		xQueueSend(colaRegistros, &reg, espera);

	}


	if (compararCadena(data, "go saas")){
		nombreComando(data);

		configSystem.ENVIO_SAAS = !configSystem.ENVIO_SAAS;
		Serial.print("Envio SAAS:");
		Serial.println(configSystem.ENVIO_SAAS);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}


	if (compararCadena(data, "go ter")){
		nombreComando(data);
		configSystem.ESCUCHAR_LINEA = !configSystem.ESCUCHAR_LINEA;
		Serial.print("Conexion linea on");
		Serial.println(configSystem.ESCUCHAR_LINEA);
		NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}

	if (compararCadena(data, "rs -t")){
		nombreComando(data);
		linea.testUart();
		delay(5);
	}

	if (compararCadena(data, "t -print")){
		nombreComando(data);
		Serial.println(T_COCHERA.generarInformeDatos());
	}

	if (compararCadena(data, "t -add")){
		nombreComando(data);
		 byte myArray[] = {1, 0, 0, 0, 1, 0, 0, 0};
		 byte myArray2[] = {1, 0};
		T_COCHERA.guardarDatosTerminal(myArray, myArray2);
	}

    if (compararCadena(data, "t -addr")) {
        nombreComando(data);

        byte myArray[8];
        byte myArray2[2];

        for (int i = 0; i < 8; i++) {
            myArray[i] = random(2); // Generar números aleatorios entre 0 y 1
        }

        for (int i = 0; i < 2; i++) {
            myArray2[i] = random(2); // Generar números aleatorios entre 0 y 1
        }

        T_COCHERA.guardarDatosTerminal(myArray, myArray2);
    }


	if (compararCadena(data, "t -show")){
		nombreComando(data);
		T_CORE.recorrerDatosTerminal();

		Serial.println(T_CORE.getDatosFotosensor());
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
		T_CORE.borrarPrimerElemento();
	}


	if (compararCadena(data, "t -dl")){
		nombreComando(data);
		T_CORE.borrarUltimoElemento();
	}

	if (compararCadena(data, "t -purge")){
		nombreComando(data);
		T_CORE.limpiarDatosTerminal();
	}


	if (compararCadena(data, "t -save")){
		nombreComando(data);
		guardarEstadoAlerta();
	}

	if (compararCadena(data, "t -load")){
		nombreComando(data);
		Serial.println("Comando incompleto prueba manualmente");
	}

	if (compararCadena(data, "iterrupcion -r")){
		nombreComando(data);

		guardarFlagEE("ERR_INTERRUPT", 0);
		guardarFlagEE("MENSAJE_EMERGEN", 0);
		guardarFlagEE("LLAMADA_EMERGEN", 0);
	}


	if(compararCadena(data, "at")){
		nombreComando(data);
		UART_GSM.println("AT");
		UART_GSM.println("AT+COPS?");

		esperarRespuestaUart2();
	}

	if(compararCadena(data, "echo ?")){
		nombreComando(data);
		UART_GSM.println("ATE?");

		esperarRespuestaUart2();
	}


	if(compararCadena(data, "speed ?")){
		nombreComando(data);
		UART_GSM.println("AT+IPR?");

		esperarRespuestaUart2();

	}


	if(compararCadena(data, "speed -c")){
		nombreComando(data);
		UART_GSM.println("AT+IPR=115200");
		UART_GSM.println("AT&W");
		UART_GSM.println("AT+CFUN=1,1");
	}




	if(compararCadena(data, "pila -t")){
		nombreComando(data);
		testTaskNodos();
	}

	if(compararCadena(data, "pila -t2")){
		nombreComando(data);
		testTaskNodos2();
	}

	if(compararCadena(data, "pila -d")){
		nombreComando(data);
		testTaskNodosDelete();
	}


	if(compararCadena(data, "pila -r")){
		nombreComando(data);
		testTaskNodosRecorrer();
	}


	if(compararCadena(data, "pila -m")){
		nombreComando(data);
		testTaskNodosMover();
	}

	if(compararCadena(data, "pila -p")){
		nombreComando(data);
		gestionarPilaDeTareas();
	}

	if(compararCadena(data, "pila -at")){
		nombreComando(data);
		testTaskNodosTimeout();
	}


	if(compararCadena(data, "pila -rp")){
		nombreComando(data);
		testTaskNodosRecuperarProcesable();
	}

	if(compararCadena(data, "pila -sleep")){
		nombreComando(data);
		rehabilitarEjecucionPila();
	}

	if(compararCadena(data, "power -f")){
		nombreComando(data);
		interrupcionFalloAlimentacion();
	}

	if (compararCadena(data, "gsm -r")){
		nombreComando(data);
		refrescarModuloGSM();
	}

	if (compararCadena(data, "t -m1")){
		nombreComando(data);
		Serial.println("Probando multiples detecciones");
		byte myArray[] = {1, 0, 0, 1, 0, 1, 0, 1};
		byte myArray2[] = {0, 0};

		for (int i = 0; i < 3; i++) {
			T_COCHERA.guardarDatosTerminal(myArray, myArray2);
		}
	}

	if (compararCadena(data, "t -m2")){
		nombreComando(data);
		Serial.println("Probando detecciones individuales");
		byte myArray[] = {1, 0, 0, 0, 0, 0, 0, 0};
		byte myArray2[] = {0, 0};
		for (int i = 0; i < 3; i++) {
			T_COCHERA.guardarDatosTerminal(myArray, myArray2);
		}
	}

	if (compararCadena(data, "t -m3")){
		nombreComando(data);
		Serial.println("Probando sabotaje");
		byte myArray[] = {1, 0, 0, 0, 0, 0, 0, 0};
		byte myArray2[] = {1, 0};

		T_COCHERA.guardarDatosTerminal(myArray, myArray2);

	}

	if (compararCadena(data, "t -m4")){
		nombreComando(data);
		Serial.println("Probando averia");
		byte myArray[] = {0, 0, 0, 0, 0, 0, 0, 0};
		byte myArray2[] = {1, 0};

		T_COCHERA.guardarDatosTerminal(myArray, myArray2);

	}


	if(compararCadena(data, "12")){
		nombreComando(data);

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











