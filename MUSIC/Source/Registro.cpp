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

		  //Comprueba los directorios

		  root = SD.open("/");

		  if (!SD.exists(saaLogs)) {
		    Serial.println("Creando directorio de logs");

		    if (SD.mkdir(saaLogs)) {
		      Serial.println("Directorio creado exitosamente.");
		    } else {
		      Serial.println("Error al crear el directorio.");
		      return 0;
		    }
		  }

		  SD_STATUS = 1;
		  Serial.println("ALMACENIAMENTO SD OK");

		   // Recorre la lista de archivos y los borra si empiezan por "log"
		  root = SD.open(saaLogs);

		    while (true) {
		      File entry = root.openNextFile();
		      if (!entry) {
		        break;
		      }
		      String fileName = entry.name();
		      Serial.println(fileName);

		      if (fileName.startsWith("sys")) {
		        //SD.remove("/logs/" + fileName);
		        Serial.println(fileName);
		      }
		      entry.close();
		    }

		    // Cierra la carpeta "logs"
		    root.close();

		    return 1;

}

void Registro::registrarLogSistema(char descripcion[190]){

	if(!configSystem.MODULO_SD || SD_STATUS == 0)
		return;

	 //Nos movemos al diretorio de logs
	   root = SD.open(saaLogs);
	   if (!root) {
		 Serial.println("No se pudo abrir la carpeta logs");
		 return;
	   }

	   int secuencialLog = leerFlagEEInt("LOG_SEQ");

	   char nombreFichero[25];
	   snprintf(nombreFichero, sizeof(nombreFichero), "%s%08d%s", "systemlog_", secuencialLog, ".txt");
	   //PEND ANADIR FECHA AL LOG SI RTC IS ON

	   snprintf(rutaAbosuluta, sizeof(rutaAbosuluta), "%s/%s", saaLogs, nombreFichero);

	     root = SD.open(rutaAbosuluta, FILE_APPEND);
			 if (!root) {
			   Serial.println("Fallo al abrir el fichero de logs");
			   return;
			 }
	     root.println(descripcion);
	     root.close();

}



