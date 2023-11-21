/*
 * Registro.cpp
 *
 *  Created on: 3 abr. 2023
 *      Author: Eduardo
 */

#include "Registro.h"

#define sck 14
#define miso 2
#define mosi 15
#define ss 13

File root;  // @suppress("Abstract class cannot be instantiated")
SPIClass SDSPI(HSPI);

Registro::Registro() {
//
}

byte Registro::iniciar(){

	  if(!configSystem.MODULO_SD)
		return 0;

		Serial.print("\nINICIO SD CARD\n");

		SDSPI.begin(sck, miso, mosi, -1);
		SD.begin(ss, SDSPI);

		  if (!SD.begin(ss, SDSPI)) {
			  return 0;
		  }

		  int numDirectories = sizeof(directories) / sizeof(directories[0]);

		  // Crear los directorios uno por uno
		  for (int i = 0; i < numDirectories; i++) {
		    const char* directory = directories[i];

		    if (!SD.exists(directory)) {
		      // Intentar crear el directorio
		      if (!SD.mkdir(directory)) {
		        Serial.print("Error al crear el directorio ");
		        Serial.println(directory);
		        break;

		      } else {
		        Serial.print("Directorio ");
		        Serial.print(directory);
		        Serial.println(" creado exitosamente.");
		      }
		    }
		  }


		  //Definimos el nombre del nuevo fichero de syslog
		  crearNuevoNombreLog();

		  SD_STATUS = 1;
		  Serial.println("ALMACENIAMENTO SD OK");

		  return 1;

}

void Registro::crearNuevoNombreLog(){
	snprintf(nombreFicheroLog, sizeof(nombreFicheroLog), "%s_%08d_%s%s", "syslog", leerFlagEEInt("LOG_SEQ"), fecha.imprimeFechaFichero(),".txt");
	//Acutalizamos el secuencial de los logs para esta ejecucion
	guardarFlagEE("LOG_SEQ", (leerFlagEEInt("LOG_SEQ")+1));
}

void Registro::registrarLogSistema(char descripcion[190]){

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return;

	 //Nos movemos al diretorio de logs
	   root = SD.open(directories[DIR_LOGS]);
	   if (!root) {
		 Serial.println("No se pudo abrir la carpeta logs");
		 return;
	   }

	   snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[DIR_LOGS], nombreFicheroLog);

	     root = SD.open(rutaAbosuluta, FILE_APPEND);
			 if (!root) {
			   Serial.println("Fallo al abrir el fichero de logs");
			   return;
			 }
	     root.print(descripcion);
	     root.print("\t");
	     root.print(fecha.imprimeFecha(1));
	     root.print("\n");
	     root.close();

}

void Registro::registrarLogHttpRequest(char respuestaHttp[300]){
	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return;

	//Nos movemos al diretorio de logs
	root = SD.open(directories[DIR_HTTP_LOG]);
	if (!root) {
		Serial.println("No se pudo abrir la carpeta http logs");
		return;
	}

 //Definimos el nombre del nuevo fichero http request
	snprintf(nombreFicheroHttpLog, sizeof(nombreFicheroHttpLog), "%s_%08d_%s%s", "httpLog", leerFlagEEInt("PACKAGE_ID"), fecha.imprimeFechaFichero(1),".txt");
	snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[DIR_HTTP_LOG], nombreFicheroHttpLog);

	root = SD.open(rutaAbosuluta, FILE_APPEND);
	if (!root) {
		Serial.println("Fallo al abrir el fichero de http logs");
		return;
	}
	root.print("Respuesta del servidor SAAS");
	root.print("\t");
	root.print(fecha.imprimeFecha(1));
	root.print("\n");
	root.print(respuestaHttp);
	root.close();

}

void Registro::mostrarRegistro(RegistroDirectorios dir){


	if(!configSystem.MODULO_SD || SD_STATUS == 0)
	return;

	const char* nombreDir = directories[dir];

	//Nos movemos al diretorio de logs
	   root = SD.open(directories[dir]);
	   if (!root) {
		 Serial.print("No se pudo abrir la carpeta ");
		 Serial.println(nombreDir);

		 return;
	   }

	   File file;

	   if(dir == DIR_LOGS){
		   snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[dir], nombreFicheroLog);
		   file = SD.open(rutaAbosuluta);
	   }else {
		   file = root.openNextFile(); //Solo para directorios con unico fichero
	   }

		   if (file) {
		     // Lee línea por línea hasta el final del archivo
		     while (file.available()) {
		       String line = file.readStringUntil('\n');
		       Serial.println(line);
		     }
		     // Cierra el archivo cuando termina de leer
		     file.close();
		   } else {
		     Serial.println("Error al abrir el archivo.");
		   }

		   root.close();
}

void Registro::listarRegistros(RegistroDirectorios dir){

	  root = SD.open(directories[dir]);

		while (true) {
		  File entry = root.openNextFile();
		  if (!entry /*|| entry.isDirectory()*/) {
			break;
		  }
		  String fileName = entry.name();
		  Serial.print(fileName);
		  Serial.print("\t");
		  Serial.println(entry.size(), DEC);

		  entry.close();
		}

		root.close();

}

void Registro::borrarRegistros(RegistroDirectorios dir){

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
	return;

	  root = SD.open(directories[dir]);

	    while (true) {
	      File entry = root.openNextFile();
	      if (!entry) {
	        break;
	      }
	      String fileName = entry.name();

	      	  // Borra el archivo
			  if (SD.remove(fileName)) {
				Serial.print("Archivo borrado: ");
				Serial.println(fileName);
			  } else {
				Serial.print("Error al borrar el archivo: ");
				Serial.println(fileName);
			  }

	      entry.close();
	    }

	    root.close();

}


byte Registro::exportarEventosJson(StaticJsonDocument<MAX_SIZE_JSON>* json){

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return 0;

	//Nos movemos al diretorio de logs
	root = SD.open(directories[DIR_JSON_REQUEST]);
	if (!root) {
		Serial.println("No se pudo abrir la carpeta json");
		return 0;
	}


	snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[DIR_JSON_REQUEST], nombreFicheroJsonRequest);

	root = SD.open(rutaAbosuluta, FILE_APPEND);
	if (!root) {
		Serial.println("Fallo al abrir el fichero de json");
		return 0;
	}

	String SALIDA_JSON = "";
	serializeJson(*json, SALIDA_JSON);

	//Se imprime el resultado en el monitor serie
	Serial.println(SALIDA_JSON);

	root.print(SALIDA_JSON);
	root.print("\n");
	root.close();


	return 1;

}

String Registro::extraerPrimerElemento(RegistroDirectorios dir){

	String line = "";

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return line;


	snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[dir], nombreFicheroJsonRequest);
	snprintf(rutaAbosulutaTemporal, sizeof(rutaAbosulutaTemporal), "%s/%s", directories[dir], "temp.txt");

	const char* nombreDir = directories[dir];

	File file;
	File tempFile = SD.open(rutaAbosulutaTemporal, FILE_APPEND);

	//Nos movemos al diretorio de logs
	root = SD.open(directories[dir]);
	if (!root) {
		Serial.print("No se pudo abrir la carpeta ");
		Serial.println(nombreDir);

		return line;
	}

	file = root.openNextFile();

	if (file) {

		byte flag_primera_linea = 1;

		while (file.available()) {
			if(flag_primera_linea == 1){
				line = file.readStringUntil('\n');
				flag_primera_linea = 0;
			}else {
				tempFile.print(file.readStringUntil('\n'));
				tempFile.print("\n");
			}
		}

		file.close();
		tempFile.close();

		//Borrar original
		SD.remove(rutaAbosuluta);
		//Renombrar fichero
		SD.rename(rutaAbosulutaTemporal, rutaAbosuluta);

	} else {
		Serial.println("Error al abrir el archivo.");
	}

	root.close();

	return line;
}

String Registro::leerPrimerElemento(RegistroDirectorios dir){

	String line = "";

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return line;

	snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[dir], nombreFicheroJsonRequest);

	const char* nombreDir = directories[dir];

	File file;

	//Nos movemos al diretorio de logs
	root = SD.open(directories[dir]);
	if (!root) {
		Serial.print("No se pudo abrir la carpeta ");
		Serial.println(nombreDir);

		return line;
	}

	file = root.openNextFile();

	if (file) {

		byte flag_primera_linea = 1;

		while (file.available() && flag_primera_linea == 1) {
			if(flag_primera_linea == 1){
				line = file.readStringUntil('\n');
				flag_primera_linea = 0;
			}
		}

		file.close();


	} else {
		Serial.println("Error al abrir el archivo.");
	}

	root.close();

	return line;
}




void Registro::actualizarUltimoElemento(const char* campoJson, int nuevoValor, RegistroDirectorios dir){
	/*Si no se expecifica el valor este unicamente se incrementara*/
	String line = "";

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return;

	snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[dir], nombreFicheroJsonRequest);
	snprintf(rutaAbosulutaTemporal, sizeof(rutaAbosulutaTemporal), "%s/%s", directories[dir], "temp.txt");

	const char* nombreDir = directories[dir];

	File file;
	File tempFile = SD.open(rutaAbosulutaTemporal, FILE_APPEND);

	//Nos movemos al diretorio de logs
	root = SD.open(directories[dir]);
	if (!root) {
		Serial.print("No se pudo abrir la carpeta ");
		Serial.println(nombreDir);
	}

	file = root.openNextFile();

	if (file) {

		byte flag_primera_linea = 1;

		while (file.available()) {
			if(flag_primera_linea == 1){
				line = file.readStringUntil('\n');

				if(nuevoValor == -1){
					nuevoValor = this->obtenerValorCampo(line, campoJson).toInt();
					nuevoValor++;
				}

				line = this->modificarCampo(line, campoJson, String(nuevoValor));

				tempFile.print(line);
				tempFile.print("\n");

				flag_primera_linea = 0;
			}else {
				tempFile.print(file.readStringUntil('\n'));
				tempFile.print("\n");
			}
		}

		file.close();
		tempFile.close();

		//Borrar original
		SD.remove(rutaAbosuluta);
		//Renombrar fichero
		SD.rename(rutaAbosulutaTemporal, rutaAbosuluta);

	} else {
		Serial.println("Error al abrir el archivo.");
	}

	root.close();
}

int Registro::leerReintentosModelo(const String* modelo){
	const String m = *modelo;
	return this->obtenerValorCampo(m, "retry").toInt();
}

String Registro::actualizarIdModelo(String* modelo, int id){
	 String m = *modelo;
	 return this->modificarCampo(m, "id", String(id));
}

bool Registro::enviarFicheroPorFTP(int bytes, const char* rutaFichero, const char* nombreFichero){

	File file;
	file = SD.open(rutaFichero);

	if(!crearFicheroFtp(nombreFichero,bytes)){
		Serial.printf("Fallo al crear el fichero %s en el servidor", rutaFichero);
		Serial.print("\n");
		return false;
	}else {
		Serial.printf("Fichero %s creado en el servidor", rutaFichero);
		Serial.print("\n");
	}

	if (file) {
		// Lee linea por linea hasta el final del archivo
		while (file.available()) {
			String linea = file.readStringUntil('\n');
			const char* l = linea.c_str();
			enviarBufferFtp(l);
		}
		// Cierra el archivo cuando termina de leer
		file.close();

		//Cerramos el fichero en el servidor ftp
		if(!cerrarFicheroFtp()){
			Serial.printf("Fallo al cerrar el fichero %s en el servidor", rutaFichero);
			Serial.print("\n");
			return false;
		}else {
			Serial.printf("Fichero %s cerrado transferencia OK", rutaFichero);
			Serial.print("\n");
		}

	} else {
		Serial.printf("Fallo al abrir el fichero %s", rutaFichero);
		return false;
	}

	return true;
}

RespuestaFtp Registro::envioRegistrosFTP(){
	//Los ficheros se envian por ftp y son movidos a backup. Al finalizar se actualiza un nuevo registro de log
	RespuestaFtp respuesta;

	if(!configSystem.MODULO_SD || SD_STATUS == 0){
		respuesta.error = 1;
		return respuesta;
	}

	File tempFile; //Guarda la copia en la ruta de backup
	File entry;
	byte error = 0;

	root = SD.open(directories[DIR_LOGS]);
	if (!root) {
		Serial.println("No se pudo abrir la carpeta de logs ");
		respuesta.error = 1;
		respuesta.msg = "No se pudo abrir la carpeta de logs";
		return respuesta;
	}

	if (!abrirConexionFtp()) {
		Serial.println("Err no se pudo abrir la conexion FTP");
		respuesta.error = 1;
		respuesta.msg = "Err no se pudo abrir la conexion FTP";
		return respuesta;
	}

	//Itero los ficheros de la ruta
	while (true) {
		entry = root.openNextFile();
		if (!entry) {break;}
		if(entry.isDirectory()) {continue;}

		const char* nombreLog = entry.name();
		// Nombre del archivo solamente
		const char* fichero = &nombreLog[13];
		const int bytesFichero = entry.size();

		//Enviamos el fichero por ftp
		if(!enviarFicheroPorFTP(bytesFichero, nombreLog, fichero)){
			Serial.println("Error durante la tranferencia del fichero");
			error = 1;
			break;
		}

		//Si lo envia OK lo muevo a la ruta de backup -> (funcion que copie el fichero)
		snprintf(rutaAbosulutaBackup, sizeof(rutaAbosulutaBackup), "%s/%s", directories[DIR_LOGS_BACKUP], fichero);
		//Serial.println(rutaAbosulutaBackup);

		tempFile = SD.open(rutaAbosulutaBackup, FILE_APPEND);

		if (!tempFile) {
			Serial.println("Fallo al abrir el fichero de backup");
		}

		if (entry) {
			while (entry.available()) {
				tempFile.print(entry.readStringUntil('\n'));
				tempFile.print("\n");
			}
			entry.close();
			tempFile.close();

			//Borrar original
			if (SD.remove(nombreLog)) {
				Serial.print("Archivo borrado: ");
				Serial.println(nombreLog);
			} else {
				Serial.print("Error al borrar el archivo: ");
				Serial.println(nombreLog);
			}

		} else {
			Serial.println("Error al abrir el archivo.");
		}
	}

	root.close();

	//Si todo va bien actualizo el nuevo registro para proximos logs
	crearNuevoNombreLog();

	if (!cerrarConexionFtp() || error == 1) {
		Serial.println("Err no se pudo cerrar la conexion FTP");
		respuesta.error = 1;
		respuesta.msg = (error == 1) ? "Error durante la transferencia del fichero" : "Err no se pudo cerrar la conexión FTP";
		return respuesta;
	}else {
		Serial.println("Conexion FTP cerrada bye!");
	}

	respuesta.error = 0;
	return respuesta;
}

String Registro::modificarCampo(String cadena, const String& nombre_campo, const String& nuevo_valor) {
    String campo_a_buscar = "\"" + nombre_campo + "\":\"";
    int inicio = cadena.indexOf(campo_a_buscar);
    if (inicio != -1) {
        int fin = cadena.indexOf("\"", inicio + campo_a_buscar.length());
        if (fin != -1) {
            cadena = cadena.substring(0, inicio + campo_a_buscar.length()) + nuevo_valor + cadena.substring(fin);
        }
    }
    return cadena;
}

// Función para obtener el valor actual de un campo en la cadena
String Registro::obtenerValorCampo(const String& cadena, const String& nombre_campo) {
    String campo_a_buscar = "\"" + nombre_campo + "\":\"";
    int inicio = cadena.indexOf(campo_a_buscar);
    if (inicio != -1) {
        int fin = cadena.indexOf("\"", inicio + campo_a_buscar.length());
        if (fin != -1) {
            return cadena.substring(inicio + campo_a_buscar.length(), fin);
        }
    }
    return ""; // Si no se encuentra el campo, devolvemos una cadena vacía
}
