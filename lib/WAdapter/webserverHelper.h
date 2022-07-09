#ifndef WEBSERVERHELPER_H
#define	WEBSERVERHELPER_H


#include <ESPAsyncWebServer.h>

String getValueOrEmpty(AsyncWebServerRequest* request, const char * p){
    if (!request->hasParam(p)) return String("");
    else return request->getParam(p)->value();
}


#endif