/*
 * Pantalla.h
 *
 *  Created on: 29 jul. 2021
 *      Author: isrev
 */

#ifndef SOURCE_PANTALLA_H_
#define SOURCE_PANTALLA_H_

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "Autenticacion.h"
#include "AlarmaUtil/Macros.h"
#include "Fecha.h"
#include "Terminal.h"




extern byte MODO_DEFAULT;
extern const char* version[];
extern unsigned long tiempoMargen;
extern Autenticacion auth;
extern Fecha fecha;
extern RespuestaTerminal respuestaTerminal;
extern const char* literalesZonas[2][MAX_DATOS_SUB_TRAMA];
extern byte desactivaciones, mensajesEnviados;
extern ConfigSystem configSystem;
extern ProveedorEstado coberturaRed();
extern String fixedLengthString(String& original, size_t fixedLength);
extern PilaTareaEstado estadoPila;

extern Terminal T_CORE;

class Pantalla {
private:
	String errorTexto;
public:
	Pantalla();
	void lcdLoadView(Pantalla *obj,void (Pantalla::*otherFunction)());
	void lcdLoadView(Pantalla *obj,void (Pantalla::*otherFunction)(byte*, char[]), byte* var, char parameter[15] = nullptr);
	void iniciar();
	void limpiarPantalla();
	void lcdInicio();
	void lcdError();
	void lcdFalloBateria();
	void lcdSensorPuertaDesconectado();
	void lcdAvisoResetAuto();
	void lcdAvisoBluetooh();
	void lcdClave();
	void lcdReposo();
	void lcdGuardia();
	void lcdAlerta();
	void lcdAvisoEnviado();
	void lcdInquieto();
	void lcdMenu();
	void menuConfig();
	void menuConfigSubMenu();
	void menuConfigModulos();
	void menuConfigModos();
	void menuConfigSensores();
	void menuConfigSensoresPir();
	void menuConfigChangeMode(byte* var, char name[15]);
	void menuInfo();
	void menuInfoTiempo(byte* var, char s[15] = nullptr);
	void menuInfoSMSActual();
	void menuInfoSMSHistorico();
	void menuInfoErrHistorico();
	void menuInfoVersion();
	void menuInfoResetAuto();
	void menuInfoSMSDiario();
	void menuInfoProveedorRed();
	void menuInfoRegistros();
	void menuInfoRegistrosBorrar();
	void menuInfoRegBorrado();
	void menuInfoRegBorradoError();
	void menuInfoRegDescargando();
	void menuInfoRegDescargandoError();
	void menuInfoBluetoohAviso();
	void menuConfigSaas();
	void menuConfigSaasConf();
	void menuConfigSaasActivacion();
	void menuConfigSaasConfTiempo();
	void menuConfigSaasConfSyncId();
	void menuConfigSaasConfSyncToken();
	void menuCheck();
	void errorEmergencia();
	void sysConexionGprs();
    void sysConexionGprsOk();
    void sysConexionGprsFail();
    void sysConexionGSM();
    void sysSincronizarFecha();
	String& getErrorTexto();
};

#endif /* SOURCE_PANTALLA_H_ */
