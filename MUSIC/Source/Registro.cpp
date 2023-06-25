/*
 * Registro.cpp
 *
 *  Created on: 3 abr. 2023
 *      Author: Eduardo
 */

#include "Registro.h"

File root; // @suppress("Type cannot be resolved")
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
		  snprintf(nombreFichero, sizeof(nombreFichero), "%s_%08d_%s%s", "syslog", leerFlagEEInt("LOG_SEQ"), fecha.imprimeFechaFichero(),".txt");

		  SD_STATUS = 1;
		  Serial.println("ALMACENIAMENTO SD OK");

		  return 1;

}

void Registro::registrarLogSistema(char descripcion[190]){

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return;

	 //Nos movemos al diretorio de logs
	   root = SD.open(sysLog);
	   if (!root) {
		 Serial.println("No se pudo abrir la carpeta logs");
		 return;
	   }

	   snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", sysLog, nombreFichero);

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

void Registro::mostrarRegistro(){


	if(!configSystem.MODULO_SD || SD_STATUS == 0)
	return;

	//Nos movemos al diretorio de logs
	   root = SD.open(sysLog);
	   if (!root) {
		 Serial.println("No se pudo abrir la carpeta logs");
		 return;
	   }

	   snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", sysLog, nombreFichero);


		 File file = SD.open(rutaAbosuluta);
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

void Registro::listarRegistros(){

	  root = SD.open(sysLog);

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

void Registro::borrarRegistros(){

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
	return;

	  root = SD.open(sysLog);

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

