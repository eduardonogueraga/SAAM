/*
 * Macros.h
 *
 *  Created on: 18 mar. 2023
 *      Author: isrev
 */

#ifndef SOURCE_MACROS_H_
#define SOURCE_MACROS_H_


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
#define GSM_TX 19
#define GSM_RX 23

#define RS_TX 18
#define RS_RX 5


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
#define MAX_SIZE_JSON 3584 //Bytes maximos del modelo JSON 3.5K

//COMUNICACION LINEA
enum infoTrazas{DESTINATARIO, AUTOR, METODO, METODO_DESC, ESTADO_SERVICIO, FOTOSENSOR};
enum metodosTrazas{MTH_DATA, MTH_RETRY, MTH_RESET};

#define N_TERMINALES_LINEA 2

#define MAX_DATOS_TRAMA 6
#define MAX_DATOS_SUB_TRAMA 8
#define MAX_DATOS_CTL_LINEA 2
#define MAX_REINTENTOS_MASTER 2

enum infoMapeoSensores {
	L101_SENSOR,
	L102_SENSOR,
	L103_SENSOR,
	L104_SENSOR,
	L201_SENSOR,
	L202_SENSOR,
	L203_SENSOR,
	L204_SENSOR,
	C01_LINEA,
	C02_LINEA,
};

typedef enum {
	SILENCIO,
	TRAMA_KO,
	TRAMA_OK
} LecturasLinea;

typedef enum {
	LLAMAR_TERMINAL,
	ESCUCHAR_LINEA,
	SOLICITAR_REINTENTO,
	REINTENTAR
} EstadosTerminal;

//DEFINICIONES DE FUNCIONES
void printSystemInfo();

void guardarFlagEE(const char* key, int value);
void guardarFlagEE(const char* key, uint8_t value);
uint8_t leerFlagEE(const char* key);
int leerFlagEEInt(const char* key);

template <typename T> void NVS_SaveData(const char* key, T value);
template <typename T> T NVS_RestoreData(const char* key);


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


typedef enum {
	DIR_RAIZ,
	DIR_SISTEMA,
	DIR_HTTP,
	DIR_LOGS,
	DIR_JSON_REQUEST,
	DIR_HTTP_LOG,
} RegistroDirectorios;

//LITERALES SAAS

typedef enum  {
  AVISO_ALARMA_MOVIMIENTO_PORCHE = 113,
  AVISO_ALARMA_MOVIMIENTO_COCHERA = 114,
  AVISO_ALARMA_MOVIMIENTO_ALMACEN = 115,
  AVISO_ALARMA_PUERTA_COCHERA_ABIERTA = 116,
  ALARMA_REACTIVADA_EXITO = 117,
  MOVIMIENTO_DETEC_SABOTAJE_ALIMENTACION = 118,
  FALLO_ALIMENTACION_PRINCIPAL = 119,
  BATERIA_EMERGENCIA_DESACTIVADA= 120
}	SAAS_LITERAL_NOTIFICACIONES;


typedef enum  {
	TLF_NUM_1 = 1,
	TLF_NUM_2 = 0,
	TLF_NUM_3= 2
}	SAAS_LITERAL_NOTIFICACIONES_TLF;


typedef enum  {
	ALARMA_INICIADA_LOG = 121,
	INTENTOS_SMS_DIARIOS_RECUPERADOS_LOG = 122,
	SENSOR_PUERTA_DESCONECTADO_LOG = 123,
	BATERIA_EMERGENCIA_ACTIVADA_LOG = 124,
	BATERIA_EMERGENCIA_DESACTIVADA_LOG = 125,
	INTENTOS_SMS_REALIZADOS_LOG = 126,
	INTENTOS_SMS_DIARIOS_ACABADOS_LOG = 127,
	ALARMA_ESTABLECIDA_MODO_DEFAULT_LOG = 128,
	ALARMA_ESTABLECIDA_MODO_PRUEBA_LOG = 129,
	LLAMANDO_A_MOVIL_LOG = 130,
	RESET_AUTOMATICO_LOG = 131,
	RESET_MANUAL_LOG = 132,
	FALLO_ALIMENTACION_LOG = 133,
	FALLO_SENSOR_LOG = 134
} SAAS_LITERAL_LOGS;

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
