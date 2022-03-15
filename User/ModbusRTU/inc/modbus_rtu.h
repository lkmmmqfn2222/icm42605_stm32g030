#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#include "modbus_cfg.h"  

#if (MODBUS_CFG_SLAVE_EN == 1)
#include "modbus_slave.h"
#endif 
#if (MODBUS_CFG_HOST_EN == 1)
#include "modbus_host.h"
#endif


#endif