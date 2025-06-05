/**************************************************************************************************
  Filename:       zcl_genericapp.h
  Revised:        $Date: 2014-06-19 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the ZigBee Cluster Library Home
                  Automation Sample Application.

***********************************/

#ifndef ZCL_GENERICAPP_H
#define ZCL_GENERICAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "bdb.h"
#include "zcl_power_profile.h"  

/*********************************************************************
 * CONSTANTS
 */
#define APP_ENDPOINT_1            1
#define APP_ENDPOINT_2            2   

#define MAX_BINDINGS              4

#define BDB_BIND_ADDED            0x00
#define BDB_BIND_REMOVED          0x01

typedef struct {
  uint16 dstAddr;        
  uint8 dstEndpoint;     
  uint8 clusterId;       
  uint8 srcEndpoint;     
} bindingEntry_t;

// Application Events
#define GENERICAPP_LEVEL_CTRL_EVT          0x0002
#define GENERICAPP_END_DEVICE_REJOIN_EVT   0x0008  
#define GENERICAPP_END_DEVICE_REJOIN_DELAY 10000

// Events for detecting long presses
#define GENERICAPP_LONG_PRESS_SW3_EVT      0x0100
#define GENERICAPP_LONG_PRESS_SW4_EVT      0x0200
#define GENERICAPP_LONG_PRESS_SW5_EVT      0x0400
#define GENERICAPP_LONG_PRESS_SW6_EVT      0x0800

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */

extern SimpleDescriptionFormat_t zclGenericApp_Desc;
extern SimpleDescriptionFormat_t zclGenericApp_Desc_2;

extern CONST zclCommandRec_t zclGenericApp_Cmds[];

extern CONST uint8 zclCmdsArraySize;

// attribute list
extern CONST zclAttrRec_t zclGenericApp_Attrs[];
extern CONST uint8 zclGenericApp_NumAttributes;

// Identify attributes
extern uint16 zclGenericApp_IdentifyTime;
extern uint8  zclGenericApp_IdentifyCommissionState;

// GENERICAPP_TODO: Declare application specific attributes here
extern uint8 zclGenericApp_OnOffSwitchType;
extern uint8 zclGenericApp_OnOffSwitchActions;

// Атрибуты батареи
extern uint8 zclGenericApp_BatteryVoltage;
extern uint8 zclGenericApp_BatteryPercentage;
extern uint16 zclGenericApp_BatteryAlarmState;

// Таблица биндингов
extern bindingEntry_t bindingTable[MAX_BINDINGS];
extern uint8 numBindings;

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Initialization for the task
  */
extern void zclGenericApp_Init( byte task_id );

/*
 *  Event Process for the task
 */
extern UINT16 zclGenericApp_event_loop( byte task_id, UINT16 events );

/*
 *  Reset all writable attributes to their default values.
 */
extern void zclGenericApp_ResetAttributesToDefaultValues(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_GENERICAPP_H */
