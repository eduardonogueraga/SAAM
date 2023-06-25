/*
 * Macros.h
 *
 *  Created on: 18 mar. 2023
 *      Author: isrev
 */

#ifndef SOURCE_MACROS_H_
#define SOURCE_MACROS_H_

#include <EEPROM.h>


//MACROS
#define TIEMPO_OFF_TEST 0.1666
#define TIEMPO_ON_TEST 0.01
#define TIEMPO_REACTIVACION_TEST 0.1
#define TIEMPO_MODO_SENSIBLE_TEST 0.0166
#define TIEMPO_OFF_MODO_SENSIBLE 0.1666
#define TIEMPO_PRORROGA_GSM_TEST 0.05
#define TIEMPO_BOCINA_TEST 0.0333
#define TIEMPO_BOCINA_REACTIVACION_TEST 0.0666

//TECLADO
#define ROW_1 36
#define ROW_2 34
#define ROW_3 32
#define ROW_4 30
#define COL_1 38
#define COL_2 40
#define COL_3 42

//PANTALLA
#define CODEC_LCD 0x25 //  0x3F 0x27

//SENSORES
#define PIR_SENSOR_1 5//P0
#define PIR_SENSOR_2 7//P1
#define PIR_SENSOR_3 6//P2
#define MG_SENSOR 	 4//P3

//BOCINA
#define BOCINA_PIN 0//P7

//WATCHDOG
#define WATCHDOG 3//P4 // @develop ('Pines adaptados')

//RS485
#define RS_CTL 1//P6

//CONTROL RELES 12V
#define RELE_AUXILIAR 10

//SLEEPMODE
#define GSM_PIN 2//P5
#define BT_PIN 12

//COMUNICACIONES
#define GSM_TX 8
#define GSM_RX 9
#define BT_TX 10
#define BT_RX 11

//SD
#define REGISTRO_SS_PIN 53

//RESET
#define RESETEAR 9//P9

//LED PUERTA
#define LED_COCHERA 8//P8

//DIR I2C
#define MCP_ADDR 0x27  // mcp23017 address

//BATERIA E INTERRUPCIONES
#define SENSOR_BATERIA_RESPALDO 15 //Monitoriza el flujo de tension en el booster
#define FALLO_BATERIA_PRINCIPAL 19 //Interrupcion por fallo en la bateria

//SISTEMA
#define TIEMPO_ALERT_LCD 6000
#define	NUMERO_ALERTAS 4
#define TOTAL_SENSORES 4 //EL numero de sensores activos

//MACROS EEPROM
#define MENSAJES_ENVIADOS 0
#define EE_ESTADO_GUARDIA 1
#define EE_ESTADO_ALERTA 2

#define EE_ERROR_INTERRUPCION 3
#define EE_MENSAJE_EMERGENCIA 4
#define EE_INTERRUPCIONES_HISTORICO 5
#define EE_CODIGO_ERROR 6

#define EE_LLAMADA_EMERGENCIA 7
#define EE_FLAG_PUERTA_ABIERTA 9

#define EE_CONFIG_STRUCT 35
#define EE_DATOS_SALTOS 100

//DEFINICIONES DE FUNCIONES
void printSystemInfo();

void guardarFlagEE(const char* key, int value);
void guardarFlagEE(const char* key, uint8_t value);
uint8_t leerFlagEE(const char* key);
int leerFlagEEInt(const char* key);

template <typename T> void NVS_SaveData(const char* key, T value);
template <typename T> T NVS_RestoreData(const char* key);

void insertQuery(void (*otherFunction)(String*));
void insertQuery(void (*otherFunction)(String*, String, String), String param1, String param2);

void sqlMensajes(String *p);
void sqlSmsIntentosRealizados(String *p);
void sqlSmsIntentosAcabados(String *p);
void sqlModoAlarma(String *p);
void sqlSensorEstandar(String *p, String tipo, String estado);
void sqlSensorPhantom(String *p, String tipo, String estado);
void sqlSalto(String *p);
void sqlUpdateSalto(String *p);
void sqlUpdateEntrada(String *p);
void sqlUpdateErrores(String *p);
void sqlSensorPuertaDeshabilitado(String *p);
void sqlIntentosRecuperados(String *p);

void pantallaDeErrorInicial(String mensaje);
//STRUCTS

struct datos_saltos_t{
	byte ZONA;
	byte INTENTOS_REACTIVACION = 0;
	int DATOS_SENSOR[4];
};

typedef struct datos_saltos_t EE_DatosSalto;

struct configuracion_sistema_t {

	byte MODO_SENSIBLE = 1;
	byte MODULO_SD = 1;
	byte MODULO_RTC = 1;
	byte SENSORES_HABLITADOS[4] = {1,1,1,1};
	byte SMS_HISTORICO;
	char FECHA_SMS_HITORICO[30];

};

 typedef struct configuracion_sistema_t ConfigSystem;

//ENUM Y TYPEDEFS
enum aliasConfigSistema{MD_SENS, MDL_SD, MDL_RTC};
enum ZonasSensor{MG,PIR_1,PIR_2,PIR_3};

typedef enum {
	ALARMA,
	MENU,
	ERROR
} ProcesoCentral;

typedef enum {
	ESTADO_REPOSO,
	ESTADO_GUARDIA,
	ESTADO_ALERTA,
	ESTADO_ENVIO
} EstadosAlarma;

typedef enum {
	INFO_FALLO_BATERIA,
	INFO_SENSOR_PUERTA_OFF,
	INFO_RESET_AUTO,
	INFO_BLUETOOH
} Infos;

typedef enum {
	GSM_ON,
	GSM_OFF,
	GSM_TEMPORAL
} SLEEPMODE_GSM;

typedef enum {
	BT_OFF,
	BT_ON
} SLEEPMODE_BT;


typedef enum {
	TLF_1,
	TLF_2,
	COLGAR
} LLAMADAS_GSM;

typedef enum {
	ERR_FALLO_ALIMENTACION,
	ERR_FALLO_SENSOR
} CODIGO_ERROR;

typedef enum {
	GUARDAR_DATOS,
	COMPROBAR_DATOS,
	ENVIAR_AVISO,
	REALIZAR_LLAMADAS,
	ESPERAR_AYUDA
} EstadosError;


typedef enum {
	SMS_TIPO_SALTO,
	SMS_TIPO_INFO,
	SMS_TIPO_ERROR,
} TiposMensaje;

//MENU

typedef enum {
	SELECT,
	CONFIG,
	INFO,
	CHECK
} EstadosMenu;

typedef enum {
	CONFIG_SELECT,
	CONFIG_MODOS,
	CONFIG_MODULOS,
	CONFIG_SENSORES
} MENU_CONFIG;


typedef enum {
	MOD_SELECT,
	MOD_BT,
	MOD_SD,
	MOD_RTC
} MENU_CONFIG_MODULOS;

typedef enum {
	MODOS_SELECT,
	CH_MODO_TEST,
	CH_MODO_SENSIBLE
} MENU_CONFIG_MODOS;

typedef enum {
	SENSORES_SELECT,
	CH_MG,
	CH_PIR
} MENU_CONFIG_SENSORES;

typedef enum {
	INFO_SELECT,
	INFO_TIEMPO,
	INFO_DATOS,
	INFO_REGISTRO
} MENU_INFO;

typedef enum {
	TIME_MINUTOS,
	TIME_HORAS,
	TIME_DIAS,
	TIME_SEMANAS,
} MENU_INFO_TIME;

typedef enum {
	DATOS_ENTRADAS,
	DATOS_SMS_HISTORICO,
	DATOS_ERR_HISTORICO,
	DATOS_FECHA_RESET,
	DATOS_SMS_DIARIO,
	DATOS_VERSION,
} MENU_INFO_DATOS;

typedef enum {
	REGISTROS_DESCARGAR,
	REGISTROS_BORRAR,
} MENU_INFO_REGISTROS;

//TEMPLATES GLOBALES
template <class T> uint8_t arrSum(byte arr[], byte tam){
	byte r = 0;

	for (int i = 0; i <tam; i++) {
		r += arr[i];
	}
	return r;
}

template <class T> void arrCopy(byte origen[], byte destino[], byte tam) {
	byte *p = origen;
	byte *q = destino;
	while (p != origen + tam)
		*q++ = *p++;
}

template <class T> void arrCopy(int origen[], int destino[], byte tam) {
	int *p = origen;
	int *q = destino;
	while (p != origen + tam)
		*q++ = *p++;
}


#endif /* SOURCE_MACROS_H_ */
