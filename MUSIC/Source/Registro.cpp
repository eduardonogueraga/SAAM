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
		  snprintf(nombreFicheroLog, sizeof(nombreFicheroLog), "%s_%08d_%s%s", "syslog", leerFlagEEInt("LOG_SEQ"), fecha.imprimeFechaFichero(),".txt");


		  SD_STATUS = 1;
		  Serial.println("ALMACENIAMENTO SD OK");

		  return 1;

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
	     root.print(fecha.imprimeFecha());
	     root.print("\n");
	     root.close();

}

void Registro::registrarLogHttpRequest(String* respuestaHttp){
	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return;

	//Nos movemos al diretorio de logs
	root = SD.open(directories[DIR_HTTP_LOG]);
	if (!root) {
		Serial.println("No se pudo abrir la carpeta http logs");
		return;
	}

 //Definimos el nombre del nuevo fichero http request
	snprintf(nombreFicheroHttpLog, sizeof(nombreFicheroHttpLog), "%s_%08d_%s%s", "httpLog", leerFlagEEInt("PACKAGE_ID"), fecha.imprimeFechaFichero(),".txt");
	snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", directories[DIR_HTTP_LOG], nombreFicheroHttpLog);

	root = SD.open(rutaAbosuluta, FILE_APPEND);
	if (!root) {
		Serial.println("Fallo al abrir el fichero de http logs");
		return;
	}
	root.print("Respuesta del servidor SAAS");
	root.print("\t");
	root.print(fecha.imprimeFecha());
	root.print("\n");
	root.print(*respuestaHttp);
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
		  if (!entry) {
			break;
		  }
		  String fileName = entry.name();
		  Serial.println(fileName);

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

