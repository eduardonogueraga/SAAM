/*
 * Menu.cpp
 *
 *  Created on: 12 ago. 2021
 *      Author: isrev
 */

#include "Menu.h"
#include "Registro.h"

extern Registro registro;

Menu::Menu() {

	 estadoMenu = SELECT;
	 menuConfig = CONFIG_SELECT;
	 menuInfo= INFO_SELECT;
	 menuModulos = MOD_SELECT;
	 menuModos = MODOS_SELECT;
	 menuSensores = SENSORES_SELECT;
	 menuSensoresPir = 0;
	 menuInfoTime = TIME_MINUTOS;
	 menuInfoDatos = DATOS_ENTRADAS;
	 menuInfoRegistros = REGISTROS_DESCARGAR;
	 menuSaas = SAAS_SELECT;
	 menuSaasConf = SAAS_SELECT_CONF;

	 timeSelector = 0;
}

void Menu::procesoMenu(){

	switch(estadoMenu){

	case SELECT:

		pantalla.lcdLoadView(&pantalla, &Pantalla::lcdMenu);

			if(key == '1')
				estadoMenu = INFO;

			if(key == '2')
				estadoMenu = CONFIG;

			if(key == '3')
				estadoMenu = CHECK;

			if(key == '#')
				procesoCentral = ALARMA;

		break;

	case CONFIG:
		configMenu();
		break;

	case INFO:
		infoMenu();
		break;

	case CHECK:
		checkMenu();
		break;
	}
}


void Menu::configMenu(){

	switch(menuConfig){

	case CONFIG_SELECT:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfig);

		if(key == '1')
			menuConfig = CONFIG_MODOS;

		if(key == '2')
			menuConfig = CONFIG_SENSORES;

		if(key == '*')
			menuConfig = CONFIG_SUBMENU;

		if(key == '#')
			estadoMenu = SELECT;

		break;

	case CONFIG_MODOS:

		configModos();
		break;

	case CONFIG_SENSORES:

		configSensores();
		break;

	case CONFIG_MODULOS:

		//configComunicaciones();
		configModulos();
		break;

	case CONFIG_SAAS:

		configSaas();
		break;

	case CONFIG_SUBMENU:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSubMenu);
			if(key == '1')
				menuConfig = CONFIG_SAAS;

			if(key == '2')
				menuConfig = CONFIG_MODULOS;

			if(key == '#')
				menuConfig = CONFIG_SELECT;
	break;

	}
}

void Menu::configModos(){

	switch(menuModos){

	case MODOS_SELECT:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigModos);

		if(key == '1')
			menuModos = CH_MODO_TEST;

		if(key == '2')
			menuModos = CH_MODO_SENSIBLE;

		if(key == '#')
			menuConfig = CONFIG_SELECT;

		break;

	case CH_MODO_TEST:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &MODO_DEFAULT, (char*)"REAL");

		if(key == '1'){
			MODO_DEFAULT = !MODO_DEFAULT;
			char registroConjunto[50];
			snprintf(registroConjunto, sizeof(registroConjunto), "%s%d", "ALARMA ESTABLECIDA EN MODO: ",MODO_DEFAULT);
			registro.registrarLogSistema(registroConjunto);
			eventosJson.guardarLog((MODO_DEFAULT)?ALARMA_ESTABLECIDA_MODO_DEFAULT_LOG :ALARMA_ESTABLECIDA_MODO_PRUEBA_LOG);
		}

		if(key == '#')
			menuModos = MODOS_SELECT;

		break;

	case CH_MODO_SENSIBLE:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &configSystem.MODO_SENSIBLE, (char*)"SENS");

		if(key == '1'){
			configSystem.MODO_SENSIBLE = !configSystem.MODO_SENSIBLE;
			NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
		}

		if(key == '#')
			menuModos = MODOS_SELECT;
		break;

	}
}

void Menu::configSensores(){

	switch(menuSensores){

	case SENSORES_SELECT:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSensores);

		if(key == '1')
			menuSensores = CH_MG;

		if(key == '2')
			menuSensores = CH_PIR;

		if(key == '#')
			menuConfig = CONFIG_SELECT;

		break;

	case CH_MG:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &configSystem.SENSORES_HABLITADOS[0], (char*)"MG");

		if(key == '1'){
			sensorHabilitado[0] = !sensorHabilitado[0];
			arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS, 4);
			NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);

			if(!sensorHabilitado[0]){
				registro.registrarLogSistema("SENSOR PUERTA COCHERA DESHABILITADO");
				eventosJson.guardarLog(SENSOR_PUERTA_DESCONECTADO_LOG);
			}
		}

		if(key == '#')
			menuSensores = SENSORES_SELECT;

		break;

	case CH_PIR:

		switch(menuSensoresPir){

		case 0:
			pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSensoresPir);

			if(key == '1')
				menuSensoresPir = PIR_1;

			if(key == '2')
				menuSensoresPir = PIR_2;

			if(key == '3')
				menuSensoresPir = PIR_3;

			if(key == '#')
				menuSensores = SENSORES_SELECT;
			break;

		case PIR_1:
			modficarSensorPir(PIR_1);
			break;

		case PIR_2:
			modficarSensorPir(PIR_2);
			break;

		case PIR_3:
			modficarSensorPir(PIR_3);

			break;
		}

		break;
	}
}


void Menu::configModulos(){

	switch(menuModulos){

	case MOD_SELECT:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigModulos);

		if(key == '1')
			menuModulos = MOD_BT;

		if(key == '3')
			menuModulos = MOD_RTC;

		if(key == '2')
			menuModulos = MOD_SD;

		if(key == '#')
			menuConfig = CONFIG_SUBMENU;
		break;

	case MOD_BT:
		configComunicaciones();
		break;

	case MOD_SD:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &configSystem.MODULO_SD, (char*)"SD");

		if(key == '1'){
			configSystem.MODULO_SD = !configSystem.MODULO_SD;
			NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
		}
		if(key == '#')
			menuModulos = MOD_SELECT;

		break;

	case MOD_RTC:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &configSystem.MODULO_RTC, (char*)"RTC");

		if(key == '1'){
			configSystem.MODULO_RTC = !configSystem.MODULO_RTC;
			NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
		}
		if(key == '#')
			menuModulos = MOD_SELECT;
		break;
	}
}

void Menu::configSaas(){

	switch(menuSaas){

	case SAAS_SELECT:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSaas);

		if(key == '1')
			menuSaas = CH_ACTIVACION;

		if(key == '2')
			menuSaas = SAAS_CONF;


		if(key == '#')
			menuConfig = CONFIG_SUBMENU;

		break;

	case CH_ACTIVACION:
		configSaasActivacion();
		break;

	case SAAS_CONF:
		configSaasConf();
		break;

	}
}

void Menu::configSaasActivacion(){
	switch(menuSaasActivacion){

	case ACTIVAR_SELECT:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSaasActivacion);

		if(key == '1')
			menuSaasActivacion = ACTIVAR_MODELO;

		if(key == '2')
			menuSaasActivacion = ACTIVAR_NOTIFICACIONES;


		if(key == '#')
			menuSaas = SAAS_SELECT;

		break;

	case ACTIVAR_MODELO:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &configSystem.ENVIO_SAAS, (char*)"SAAS");

		if(key == '1'){
			configSystem.ENVIO_SAAS = !configSystem.ENVIO_SAAS;
			NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
		}
		if(key == '#')
			menuSaasActivacion = ACTIVAR_SELECT;

		break;

	case ACTIVAR_NOTIFICACIONES:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &configSystem.ENVIO_SAAS_NOTIFICACION, (char*)"NOTIF");

		if(key == '1'){
			configSystem.ENVIO_SAAS_NOTIFICACION = !configSystem.ENVIO_SAAS_NOTIFICACION;
			NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
		}
		if(key == '#')
			menuSaasActivacion = ACTIVAR_SELECT;

		break;

	}
}

void Menu::configSaasConf(){

	switch(menuSaasConf){

	case SAAS_SELECT_CONF:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSaasConf);

		if(key == '1')
			menuSaasConf = CH_CONF_TIEMPO;

		if(key == '2')
			menuSaasConf = SYNC_ID_PAQUETE;

		if(key == '3')
			menuSaasConf = SYNC_TOKEN_API;

		if(key == '#')
			menuSaas = SAAS_SELECT;

		break;


	case CH_CONF_TIEMPO:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSaasConfTiempo);

		if(key == '1'){
			configSystem.ESPERA_SAAS_MULTIPLICADOR++;
			if(configSystem.ESPERA_SAAS_MULTIPLICADOR>3){
				configSystem.ESPERA_SAAS_MULTIPLICADOR = 0;
			}

			NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
		}
		if(key == '#')
			menuSaasConf = SAAS_SELECT_CONF;

		break;

	case SYNC_ID_PAQUETE:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSaasConfSyncId);

		if(key == '1'){
			getIdPaqueteSaas();
			pantalla.limpiarPantalla();
		}
		if(key == '#')
			menuSaasConf = SAAS_SELECT_CONF;

		break;

	case SYNC_TOKEN_API:

		pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigSaasConfSyncToken);

		if(key == '1'){
			generarTokenSaas();
			pantalla.limpiarPantalla();
		}
		if(key == '#')
			menuSaasConf = SAAS_SELECT_CONF;

		break;

	}
}


void Menu::configComunicaciones(){

	byte bt;

	pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &bt, (char*)"CONN BT");

	if(key == '1'){
		/*
		if(sleepModeBT == 1){ //@PEND
			sleepModeBT = 0;
			alertsInfoLcd[INFO_BLUETOOH] = 0;
		}else if(sleepModeBT == 0){
			sleepModeBT = 1;
			alertsInfoLcd[INFO_BLUETOOH] = 1;
		}
		*/
	}

	if(key == '#')
		menuModulos = MOD_SELECT;
}

void Menu::modficarSensorPir(byte num){

  char sensor_name[] = "PIR ";
  char cadena_sensor[15];
  snprintf(cadena_sensor, sizeof(cadena_sensor), "%s%d", sensor_name, num);

	pantalla.lcdLoadView(&pantalla, &Pantalla::menuConfigChangeMode, &configSystem.SENSORES_HABLITADOS[num], cadena_sensor);

	if(key == '1'){
		sensorHabilitado[num] = !sensorHabilitado[num];
		arrCopy<byte>(sensorHabilitado, configSystem.SENSORES_HABLITADOS,4);
	    NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
	}


	if(key == '#')
		menuSensoresPir = 0;
}

void Menu::infoMenu(){

	switch(menuInfo){

	case INFO_SELECT:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfo);

		if(key == '1')
			menuInfo = INFO_TIEMPO;

		if(key == '3'){
			menuInfo = INFO_REGISTRO;
		}

		if(key == '2')
			menuInfo = INFO_DATOS;

		if(key == '#')
			estadoMenu = SELECT;

		break;

	case INFO_TIEMPO:
		infoTiempo();
		break;

	case INFO_DATOS:
		infoDatos();
		break;


	case INFO_REGISTRO:
		infoRegistro();
		break;

	}

}

void Menu::infoTiempo(){

	switch(menuInfoTime){

	case TIME_MINUTOS:
		timeSelector = TIME_MINUTOS;
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoTiempo, &timeSelector);

		break;

	case TIME_HORAS:
		timeSelector = TIME_HORAS;
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoTiempo, &timeSelector);
		break;

	case TIME_DIAS:
		timeSelector = TIME_DIAS;
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoTiempo, &timeSelector);
		break;


	case TIME_SEMANAS:
		timeSelector = TIME_SEMANAS;
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoTiempo, &timeSelector);
		break;

	}


	if(menuInfoTime >3){
		menuInfoTime =TIME_MINUTOS;
	}

	if(key == '1')
		menuInfoTime = static_cast<MENU_INFO_TIME>(static_cast<int>(menuInfoTime) + 1);

	if(key == '#'){
		menuInfo = INFO_SELECT;
		menuInfoTime = TIME_MINUTOS;
	}

}

void Menu::infoDatos(){

	switch(menuInfoDatos){

	case DATOS_ENTRADAS:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoSMSActual);
		break;

	case DATOS_SMS_HISTORICO:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoSMSHistorico);

		if(key == '3'){

			char temp[30];
			fecha.imprimeFechaSimple().toCharArray(temp, 30);

			if(configSystem.MODULO_RTC){

				configSystem.SMS_HISTORICO = 0;
				strcpy(configSystem.FECHA_SMS_HITORICO, temp);
				NVS_SaveData<configuracion_sistema_t>("CONF_SYSTEM", configSystem);
			}
			pantalla.limpiarPantalla();
		}
		break;

	case DATOS_ERR_HISTORICO:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoErrHistorico);
		if(key == '3'){
			guardarFlagEE("INTERUP_HIST", 0);
			pantalla.limpiarPantalla();
		}
		break;

	case DATOS_FECHA_RESET:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoResetAuto);
		break;

	case DATOS_SMS_DIARIO:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoSMSDiario);
		if(key == '3'){

			guardarFlagEE("N_SMS_ENVIADOS", 0);
			registro.registrarLogSistema("INTENTOS SMS DIARIOS RECUPERADOS");
			eventosJson.guardarLog(INTENTOS_SMS_DIARIOS_RECUPERADOS_LOG);
			pantalla.limpiarPantalla();
		}
		break;

	case DATOS_PROVEEDOR_RED:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoProveedorRed);
		break;
	case DATOS_VERSION:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoVersion);
		break;

	}

	if(menuInfoDatos >6){
		menuInfoDatos = DATOS_ENTRADAS;
	}

	if(key == '1')
		menuInfoDatos = static_cast<MENU_INFO_DATOS>(static_cast<int>(menuInfoDatos) + 1);

	if(key == '#'){
		menuInfo = INFO_SELECT;
		menuInfoDatos = DATOS_ENTRADAS;
	}
}

void Menu::infoRegistro(){

	switch(menuInfoRegistros){

	case REGISTROS_DESCARGAR:
		pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoRegistros);

		if(key == '1'){
			/*
			if(sleepModeBT == BT_ON){

				if(!registro.mostrarRegistro((char*)"SQL.txt", true)){
					pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoRegDescargandoError);
					delay(1000);
				}

			}else {
				pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoBluetoohAviso);
				delay(1000);
			}
			*/
		}

		if(key == '2'){
			menuInfoRegistros = REGISTROS_BORRAR;
			pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoRegistrosBorrar);
		}

		if(key == '#'){
			menuInfo = INFO_SELECT;
		}
		break;

	case REGISTROS_BORRAR:

		if(key == '1'){
			/*
			if(registro.truncateRegistro((char*)"SQL.txt")){
				pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoRegBorrado);
			}else{
				pantalla.lcdLoadView(&pantalla, &Pantalla::menuInfoRegBorradoError);
			}
			*/
		}

		if(key == '#'){
			menuInfoRegistros = REGISTROS_DESCARGAR;;
		}

		break;

	}

}

void Menu::checkMenu(){
	pantalla.lcdLoadView(&pantalla, &Pantalla::menuCheck);

	if(key == '1'){
		setMargenTiempo(tiempoBocina, 20000, 1.0);
	}

	sonarBocina();

	if(key == '#'){
		pararBocina();
		estadoMenu = SELECT;
	}

}
