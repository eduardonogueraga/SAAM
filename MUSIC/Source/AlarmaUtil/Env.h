/*
 * Env.h
 *
 *  Created on: 30 jul. 2021
 *      Author: isrev
 */

#ifndef ENV_H_
#define ENV_H_

#include "Arduino.h"
#include <Keypad.h>

const char password [] = "8899";
const char telefonoPrincipal[] = "123456789";
const char telefonoLlamada_1[] = "123456789";
const char telefonoLlamada_2[] = "123456789";


// Configura los datos de tu red GSM (APN, usuario y pass)
const char apn[] = "internet";
const char gsmUser[] = "";
const char gsmPass[] = "";

// Server details
/*
 * Test GET -> vsh.pp.ua /TinyGSM/logo.txt
 *
 * Test POST -> httpbin.org /post
 *
 */

const char serverUrl[]   = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";

const int  portHttp  = 80;
//const int  portHttps = 443;

//SAAS
const char getUltimoPaquete[] = "/api/package/get/";
const char postEventosJson[] = "/api/package/new/";
const char postNotificacionJson[] = "/api/notice/new/";
const char postTokenSanctum[] = "/api/sanctum/token";

#endif /* ENV_H_ */
