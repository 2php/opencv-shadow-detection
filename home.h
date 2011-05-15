#include "frameObject.h"
#include "shadow.h"
#include <process.h>
#include "ThreadPool.h"
#include <iostream>
#include <string>

#define TL 15

static initializationParams initPar;
void Start(initializationParams par,string path);