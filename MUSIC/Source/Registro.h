/*
 * Registro.h
 *
 *  Created on: 3 abr. 2023
 *      Author: Eduardo
 */

#ifndef SOURCE_REGISTRO_H_
#define SOURCE_REGISTRO_H_

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include <fcntl.h>
#include "AlarmaUtil/Macros.h"
#include "Fecha.h"
#include <ArduinoJson.h>


extern ConfigSystem configSystem;
extern byte SD_STATUS;
extern byte accesoAlmacenamientoSD;
extern QueueHandle_t colaRegistros;

extern bool crearFicheroFtp(const char* nombreArchivo, int bytes);
extern void enviarBufferFtp(const char* buffer);
extern bool cerrarFicheroFtp();
extern bool abrirConexionFtp();
extern bool cerrarConexionFtp();
extern bool comprobarFicheroFtp(const char* nombreArchivo);


extern Fecha fecha;

class Registro {
private:
	File root;
	const char* directories[7] = {"/saa",
									"/saa/sys",
									"/saa/http",
									"/saa/sys/log",
									"/saa/sys/json",
									"/saa/http/log",
									"/saa/sys/log/backup"};
	char rutaAbosuluta[60];
	char rutaAbosulutaTemporal[60];
	char rutaAbosulutaBackup[100];
	char nombreFicheroLog[40];
	char nombreFicheroJsonRequest[25] = "TEMP_JSON_RESQUEST.txt";
	char nombreFicheroHttpLog[40];
	RegistroDirectorios registroDirectorios;
	String modificarCampo(String cadena, const String& nombre_campo, const String& nuevo_valor);
	String obtenerValorCampo(const String& cadena, const String& nombre_campo);
	void crearNuevoNombreLog();
	int contarLineas(RegistroDirectorios dir);
public:
	Registro();
	byte iniciar();
	void registrarLogSistema(char descripcion[190]);
	void registrarLogHttpRequest(char respuestaHttp[300]);
	void mostrarRegistro(RegistroDirectorios dir = DIR_LOGS);
	void listarRegistros(RegistroDirectorios dir = DIR_LOGS);
	void borrarRegistros(RegistroDirectorios dir = DIR_LOGS);
	byte exportarEventosJson(StaticJsonDocument<MAX_SIZE_JSON>* json);
	String extraerPrimerElemento(RegistroDirectorios dir = DIR_JSON_REQUEST);
	String leerPrimerElemento(RegistroDirectorios dir = DIR_JSON_REQUEST);
	void actualizarUltimoElemento(const char* campoJson,int nuevoValor= -1, RegistroDirectorios dir = DIR_JSON_REQUEST);
	int leerReintentosModelo(const String* modelo);
	String actualizarIdModelo(String* modelo, int id);
	bool enviarFicheroPorFTP(int bytes, const char* rutaFichero, const char* nombreFichero);
	RespuestaFtp envioRegistrosFTP();


};

#endif /* SOURCE_REGISTRO_H_ */
