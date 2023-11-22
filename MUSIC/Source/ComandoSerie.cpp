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

	if (compararCadena(data, "help")) {
		nombreComando(data);
		mostrarAyuda();
	}

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

	if (compararCadena(data, "ls backup")){
		nombreComando(data);
		registro.listarRegistros(DIR_LOGS_BACKUP);
	}

	if (compararCadena(data, "purge backup")){
		nombreComando(data);
		registro.borrarRegistros(DIR_LOGS_BACKUP);
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


	if(compararCadena(data, "http -pack c")){
		nombreComando(data);

		String json = R"({"version":"MUSIC VE21R0","retry":"0","id":"204","date":"2023-10-08T00:07:31","System":[{"action":"1","msen":"1","alive":"816585","traffic":"0|0|0|0","modules":"1|1|0","sensors":"102;1|103;1|104;1|105;1","reset":"2023-10-17T23:54:00"}],"Entry":[{"isnew":true,"reg":"1|0|0|0","date":"2023-10-08T00:06:23","Log":[{"reg":"137","date":"2023-10-08T00:06:23"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:32","Log":[{"reg":"139","date":"2023-10-08T00:06:32"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:34","Log":[{"reg":"139","date":"2023-10-08T00:06:34"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:36","Log":[{"reg":"139","date":"2023-10-08T00:06:36"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:38","Log":[{"reg":"139","date":"2023-10-08T00:06:38"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:40","Log":[{"reg":"139","date":"2023-10-08T00:06:40"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:44","Log":[{"reg":"139","date":"2023-10-08T00:06:44"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:46","Log":[{"reg":"139","date":"2023-10-08T00:06:46"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:51","Log":[{"reg":"139","date":"2023-10-08T00:06:51"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:53","Log":[{"reg":"139","date":"2023-10-08T00:06:53"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:54","Log":[{"reg":"139","date":"2023-10-08T00:06:54"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:56","Log":[{"reg":"139","date":"2023-10-08T00:06:56"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:06:58","Log":[{"reg":"139","date":"2023-10-08T00:06:58"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:07:03","Log":[{"reg":"139","date":"2023-10-08T00:07:03"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:07:05","Log":[{"reg":"139","date":"2023-10-08T00:07:05"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:07:10","Log":[{"reg":"139","date":"2023-10-08T00:07:10"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:07:13","Log":[{"reg":"139","date":"2023-10-08T00:07:13"}]},{"isnew":true,"reg":"0|0|0|0","date":"2023-10-08T00:07:15","Log":[{"reg":"139","date":"2023-10-08T00:07:15"}]}]})";
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


	if (compararCadena(data, "json -feed")){
		nombreComando(data);
		for (int i = 0; i < 25; i++) {
			eventosJson.guardarLog(ALARMA_DESACTIVADA_MANUALMENTE_LOG);
		}
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

	if (compararCadena(data, "interrupcion -r")){
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

	//GSM FileSystem

	if(compararCadena(data, "gsm dir")){
		nombreComando(data);
		UART_GSM.println("AT+FSCD?");

		esperarRespuestaUart2();
	}

	if(compararCadena(data, "gsm cd F")){
		nombreComando(data);
		UART_GSM.println("AT+FSCD=F:/");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "gsm cd C")){
		nombreComando(data);
		UART_GSM.println("AT+FSCD=C:/");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "gsm cd E")){
		nombreComando(data);
		UART_GSM.println("AT+FSCD=E:/");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "gsm ls")){
		nombreComando(data);
		UART_GSM.println("AT+FSLS");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "gsm load")){
		nombreComando(data);
		UART_GSM.println("AT+CFTRANRX=\"e:/toftp.txt\",50");
		esperarRespuestaUart2();
	}


	if(compararCadena(data, "gsm content")){
		nombreComando(data);
		UART_GSM.println("Hola este el el contenido del load un saludo");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "gsm clear")){
		nombreComando(data);
		UART_GSM.println("AT+FSDEL=toftp.txt");
		esperarRespuestaUart2();
	}

	//FTP
	if(compararCadena(data, "ftp")){
		nombreComando(data);
		testEnvioFtp();
	}

	//Inicio rapido
	if(compararCadena(data, "ftp run")){
		nombreComando(data);
		if( iniciarServicioFtp() &&
				abrirSesionFtp() &&
				cambiarDirectorioTrabajoFtp()){
			Serial.println("Listo para recibir ficheros");
		}else {
			Serial.println("Err iniciando ftp");
		}

	}
	//Detencion rapida
	if(compararCadena(data, "ftp halt")){
		nombreComando(data);
		if(cerrarConexionFtp()){
			Serial.println("Ftp detenido");
		}else {
			Serial.println("Err deteniendo ftp");
		}
	}

	if(compararCadena(data, "ftp gprs on")){
		nombreComando(data);
		establecerConexionGPRS();
	}


	if(compararCadena(data, "ftp start")){
		nombreComando(data);
		UART_GSM.println("AT+CFTPSSTART");
		esperarRespuestaUart2();

	}

	if(compararCadena(data, "ftp ip")){
		nombreComando(data);
		UART_GSM.println("AT+CFTPSSINGLEIP=1");
		esperarRespuestaUart2();
	}


	if(compararCadena(data, "ftp login")){
		nombreComando(data);

		char FTPConnectionCommand[150];
		sprintf(FTPConnectionCommand, "AT+CFTPSLOGIN=\"%s\",%i,\"%s\", \"%s\",0", serverUrl, 21, ftpUser, ftpPassword);

		UART_GSM.println(FTPConnectionCommand);
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp pwd")){
		nombreComando(data);

		Serial.println(pwdFtp());
		//UART_GSM.println("AT+CFTPSPWD");
		//esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp cd")){
		nombreComando(data);

		char FTPPwdCommand[150];
		sprintf(FTPPwdCommand, "AT+CFTPSCWD=\"%s\"", ftpWorkingDir);
		UART_GSM.println(FTPPwdCommand);
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp put")){
		nombreComando(data);
		UART_GSM.println("AT+CFTPSPUTFILE=\"toftp.txt\",3");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp put -s")){
		nombreComando(data);

		const char* nombreArchivo = "datos.txt";
		int tamanoArchivo = 50;
		const char* contenidoMensaje = "Este es el contenido del archivo de datos bytes123";

		if(enviarFicheroFtpBuffer(nombreArchivo, tamanoArchivo, contenidoMensaje)){
			Serial.println("Fichero enviado Ok");
		}else {
			Serial.println("KO enviando");
		}

		//UART_GSM.println("AT+CFTPSPUT=\"serialFtp.txt\",20");
		//esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp load txt")){
		nombreComando(data);
		UART_GSM.println("Hola este es un mensaje de 20 bytes");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp logout")){
		nombreComando(data);
		UART_GSM.println("AT+CFTPSLOGOUT");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp stop")){
		nombreComando(data);
		UART_GSM.println("AT+CFTPSSTOP");
		esperarRespuestaUart2();
	}

	if(compararCadena(data, "ftp send")){
		nombreComando(data);
		registro.envioRegistrosFTP();
	}

	if(compararCadena(data, "ftp gprs off")){
		nombreComando(data);
		cerrarConexionGPRS();
	}

	if(compararCadena(data, "pila -t")){
		nombreComando(data);
		testTaskNodos();
	}

	if(compararCadena(data, "pila -t2")){
		nombreComando(data);
		testTaskNodos2();
	}

	if(compararCadena(data, "pila -t3")){
		nombreComando(data);
		encolarEnvioFtpSaas();
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

	if (compararCadena(data, "http -4")){
		nombreComando(data);
		guardarFlagEE("ERR_HTTP_4", 1);
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

	if(compararCadena(data, "aes")){
		nombreComando(data);
		pruebaCifrado();
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

void ComandoSerie::mostrarAyuda() {
  Serial.println("Comandos disponibles:");
  Serial.println("set on - Encender alarma");
  Serial.println("set off - Apagar alarma");
  Serial.println("set in - Activar modo inquieto");
  Serial.println("set mode - Cambiar modo de alarma");
  Serial.println("menu - Cambiar al modo de menu");
  Serial.println("pir1 - Simular PIR 1");
  Serial.println("pir2 - Simular PIR 2");
  Serial.println("pir3 - Simular PIR 3");
  Serial.println("mg - Simular Puerta");
  Serial.println("cp - Generar informe de terminal core");
  Serial.println("ch puerta - Habilitar/deshabilitar sensor de puerta");
  Serial.println("mail - Test para Estado Envio");
  Serial.println("ls - Listar registros directorio Logs");
  Serial.println("log - Mostrar contenido ultimo log");
  Serial.println("clear - Borrar todos los ficheros del directorio log");
  Serial.println("clear json - Borrar todos los ficheros del directorio json request");
  Serial.println("json - Mostrar modelo JSON");
  Serial.println("json -p - Purgar modelo JSON");
  Serial.println("json -make - Componer JSON");
  Serial.println("cat json - Mostrar registros JSON");
  Serial.println("ls json - Listar registros JSON");
  Serial.println("json -e - Exportar fichero JSON");
  Serial.println("json -save - Guardar JSON en NVS");
  Serial.println("json -load - Cargar JSON desde NVS");
  Serial.println("json -t - Refrescar Modelo en (NVS purga compone guarda y carga)");
  Serial.println("json -send - Enviar informe a Saas");
  Serial.println("json -send not - Enviar notificacion a Saas");
  Serial.println("json -feed - Rellena el modelo con datos de prueba");
  Serial.println("note -t - Prueba envio notificación por metodo directo");
  Serial.println("note -send - Prueba envio notificacion por tarea RTOS");
  Serial.println("json -ch - Prueba cambiar datos JSON fichero");
  Serial.println("http -id - Obtener ID del paquete Saas");
  Serial.println("http -token - Generar token Saas");
  Serial.println("http -pack - Prueba envio paquete hardcoded a Saas");
  Serial.println("http -pack c - Prueba envio paquete hardcoded a Saas Compacto");
  Serial.println("http -not - Prueba envio notificacion  hardcoded a Saas");
  Serial.println("token - Mostrar token Saas almacenado en NVS");
  Serial.println("ls http - Listar registros directorio HTTP");
  Serial.println("cat http - Mostrar registro actual HTTP");
  Serial.println("purge http - Borrar registros en directorio HTTP");
  Serial.println("make http - Enviar una peticion HTTP de prueba (No Saas)");
  Serial.println("encolar reg - Enviar registro a cola de registros de sub tareas");
  Serial.println("go saas - Activar/desactivar envio a Saas");
  Serial.println("go ter - Activar/desactivar escucha de la linea RS");
  Serial.println("rs -t - Simula contenido entrante por RS");
  Serial.println("t -print - Prueba mostrar datos de terminal cochera");
  Serial.println("t -add - Prueba agregar datos de terminal");
  Serial.println("t -addr - Prueba agregar datos aleatorios de terminal");
  Serial.println("t -show - Prueba mostrar datos de todos los terminales");
  Serial.println("t -df - Prueba borrar primer elemento de terminal");
  Serial.println("t -dl - Prueba borrar ultimo elemento de terminal");
  Serial.println("t -purge - Prueba limpiar datos de terminal");
  Serial.println("t -save - Prueba guardar estado del terminal en NVS");
  Serial.println("t -load - Prueba carga estado del terminal desde NVS (incompleto)");
  Serial.println("interrupcion -r - Reiniciar los flags del estadp interrupcion");
  Serial.println("at - Enviar comando AT a GSM");
  Serial.println("echo ? - Consultar configuracion de eco GSM");
  Serial.println("speed ? - Consultar velocidad de GSM");
  Serial.println("speed -c - Cambiar velocidad de GSM");
  Serial.println("gsm dir - Obtener directorio GSM Filesystem");
  Serial.println("gsm cd F - Cambiar directorio GSM a F Filesystem");
  Serial.println("gsm cd C - Cambiar directorio GSM a C Filesystem");
  Serial.println("gsm cd E - Cambiar directorio GSM a E Filesystem");
  Serial.println("gsm ls - Listar contenido GSM Filesystem");
  Serial.println("gsm load - Cargar Fichero a GSM Filesystem");
  Serial.println("gsm content - Contenido GSM para el fichero desde serie");
  Serial.println("gsm clear - Limpiar GSM Filesystem");
  Serial.println("ftp - Envio Test FTP");
  Serial.println("ftp run - Encender Inicio Rapido FTP");
  Serial.println("ftp halt - Encender Detencion Rapida FTP");
  Serial.println("ftp gprs on - Encender GPRS para FTP");
  Serial.println("ftp start - Iniciar sesión FTP");
  Serial.println("ftp ip - Dirección IP FTP");
  Serial.println("ftp login - Iniciar sesión FTP");
  Serial.println("ftp pwd - Directorio de trabajo FTP");
  Serial.println("ftp cd - Cambiar directorio FTP");
  Serial.println("ftp put - Subir archivo FTP");
  Serial.println("ftp put -s - Subir archivo FTP desde serial");
  Serial.println("ftp load txt - Cargar archivo de texto FTP");
  Serial.println("ftp logout - Cerrar sesión FTP");
  Serial.println("ftp stop - Detener conexión FTP");
  Serial.println("ftp gprs off - Apagar GPRS para FTP");
  Serial.println("pila -t - Probar encolado Notificacion en pila de tareas");
  Serial.println("pila -t2 - Probar encolado Paquete en pila de tareas");
  Serial.println("pila -t3 - Probar encolado FTP en pila de tareas");
  Serial.println("pila -d - Eliminar elementos de pila");
  Serial.println("pila -r - Recorrer pila de tareas");
  Serial.println("pila -m - Mover primero al final de la pila");
  Serial.println("pila -p - Probar iteracion de gestion de pila");
  Serial.println("pila -at - Probar timeout en pila de tareas para el primer elemento");
  Serial.println("pila -rp - Recuperar procesable en pila de tareas");
  Serial.println("pila -sleep - Rehabilitar ejecucion de pila de tareas con tiempo de espera");
  Serial.println("power -f - Simular fallo de alimentacion");
  Serial.println("gsm -r - Refrescar modulo GSM");
  Serial.println("http -4 - Forzar error HTTP -4");
  Serial.println("t -m1 - Prueba  de terminal múltiples detecciones");
  Serial.println("t -m2 - Prueba de terminal detecciones individuales");
  Serial.println("t -m3 - Prueba de terminal sabotaje");
  Serial.println("t -m4 - Prueba de terminal averia");
  Serial.println("12 - Recuperar intentos diarios de SMS y notificaciones");
  Serial.println("info - Mostrar informacion del sistema");
  Serial.println("clock - Cambiar estado del modulo RTC");
  Serial.println("bye - Reiniciar el sistema");
}










