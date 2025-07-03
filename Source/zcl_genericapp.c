/**************************************************************************************************
  Filename:       zcl_genericapp.c
  Revised:        $Date: 2014-10-24 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $

  Description:    This file contains the ZigBee Cluster Library Home
                  Automation Sample Application.

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_genericapp.h"

#include "BindingTable.h"
#include "ZComDef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "MT_SYS.h"

#include "nwk_util.h"

#include "bdb.h"
#include "bdb_interface.h"
#include "gp_interface.h"

#include "onboard.h"

/* HAL */
#include "hal_key.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 zclGenericApp_TaskID;
devStates_t zclGenericApp_NwkState = DEV_INIT;

//endpoints
afAddrType_t zclGenericApp_DstAddr;
afAddrType_t zclGenericApp_DstAddr_2;

// threshold for long press in milliseconds
#define LONG_PRESS_TIME 1500

static uint32 keyPressTime[6] = {0};
static bool   keyPressed[6] = {false};
static bool   longPressStarted[6] = {false};

static endPointDesc_t sw_ep_1 =
{
  APP_ENDPOINT_1,                  
  0,
  &zclGenericApp_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  
  (afNetworkLatencyReq_t)0            
};

static endPointDesc_t sw_ep_2 =
{
  APP_ENDPOINT_2,                  
  0,
  &zclGenericApp_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  
  (afNetworkLatencyReq_t)0           
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void zclGenericApp_HandleKeys( byte shift, byte keys );
static void zclGenericApp_BasicResetCB( void );
static void zclGenericApp_ProcessIdentifyTimeChange( uint8 endpoint );
static void zclGenericApp_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);
static void sendToAllBindings(bool level, uint8 cmd, byte endpoint);

/*********************************************************************
 * STATUS STRINGS
 */

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclGenericApp_CmdCallbacks =
{
  zclGenericApp_BasicResetCB,             // Basic Cluster Reset command
  NULL,                                   // Identify Trigger Effect command
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
#ifdef ZCL_GROUPS
  NULL,                                   // Group Response commands
#endif
  NULL,                                  // RSSI Location command
  NULL                                   // RSSI Location Response command
};

void zclGenericApp_Init( byte task_id )
{
  zclGenericApp_TaskID = task_id;
  osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
  
  zclGenericApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  zclGenericApp_DstAddr.endPoint = APP_ENDPOINT_1;

  bdb_RegisterSimpleDescriptor( &zclGenericApp_Desc );
  zclGeneral_RegisterCmdCallbacks( APP_ENDPOINT_1, &zclGenericApp_CmdCallbacks );
  zcl_registerAttrList( APP_ENDPOINT_1, zclGenericApp_NumAttributes, zclGenericApp_Attrs );
  
  zclGenericApp_DstAddr_2.addrMode = (afAddrMode_t)Addr16Bit;
  zclGenericApp_DstAddr_2.endPoint = APP_ENDPOINT_2;
  
  bdb_RegisterSimpleDescriptor(&zclGenericApp_Desc_2);
  zclGeneral_RegisterCmdCallbacks( APP_ENDPOINT_2, &zclGenericApp_CmdCallbacks );
  zcl_registerAttrList( APP_ENDPOINT_2, zclGenericApp_NumAttributes, zclGenericApp_Attrs );

  RegisterForKeys( zclGenericApp_TaskID );
  
  afRegister( &sw_ep_1 );
  afRegister( &sw_ep_2 ); 

  bdb_RegisterCommissioningStatusCB( zclGenericApp_ProcessCommissioningStatus );
  bdb_RegisterIdentifyTimeChangeCB( zclGenericApp_ProcessIdentifyTimeChange );
  
  bdb_StartCommissioning(BDB_COMMISSIONING_REJOIN_EXISTING_NETWORK_ON_STARTUP);
}

/*********************************************************************
 * @fn          zclSample_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zclGenericApp_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;

  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclGenericApp_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          break;

        case KEY_CHANGE:
          zclGenericApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case ZDO_STATE_CHANGE:
          zclGenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
          break;

        default:
          break;
      }
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & GENERICAPP_END_DEVICE_REJOIN_EVT )
  {
  #if ZG_BUILD_ENDDEVICE_TYPE
    bdb_ZedAttemptRecoverNwk();
  #endif
    return ( events ^ GENERICAPP_END_DEVICE_REJOIN_EVT );
  }

  if ( events & GENERICAPP_LONG_PRESS_SW3_EVT )
  {
    if ( keyPressed[2] && !longPressStarted[2] )
    {
      sendToAllBindings(TRUE, LEVEL_MOVE_UP, APP_ENDPOINT_1);
      longPressStarted[2] = true;
    }
    return ( events ^ GENERICAPP_LONG_PRESS_SW3_EVT );
  }

  if ( events & GENERICAPP_LONG_PRESS_SW4_EVT )
  {
    if ( keyPressed[3] && !longPressStarted[3] )
    {
      sendToAllBindings(TRUE, LEVEL_MOVE_DOWN, APP_ENDPOINT_1);
      longPressStarted[3] = true;
    }
    return ( events ^ GENERICAPP_LONG_PRESS_SW4_EVT );
  }

  if ( events & GENERICAPP_LONG_PRESS_SW5_EVT )
  {
    if ( keyPressed[4] && !longPressStarted[4] )
    {
      sendToAllBindings(TRUE, LEVEL_MOVE_UP, APP_ENDPOINT_2);
      longPressStarted[4] = true;
    }
    return ( events ^ GENERICAPP_LONG_PRESS_SW5_EVT );
  }

  if ( events & GENERICAPP_LONG_PRESS_SW6_EVT )
  {
    if ( keyPressed[5] && !longPressStarted[5] )
    {
      sendToAllBindings(TRUE, LEVEL_MOVE_DOWN, APP_ENDPOINT_2);
      longPressStarted[5] = true;
    }
    return ( events ^ GENERICAPP_LONG_PRESS_SW6_EVT );
  }
  
  return 0;
}


/*********************************************************************
 * @fn      zclGenericApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_5
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void zclGenericApp_HandleKeys(byte shift, byte keys)
{
  uint8 currentState = HalKeyRead();
  uint32 currentTime = osal_GetSystemClock();

  if (keys & HAL_KEY_SW_1)
  {
    bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING);
  }

  /* SW3 control */
  if (keys & HAL_KEY_SW_3)
  {
    if (currentState & HAL_KEY_SW_3)
    {
      keyPressed[2] = true;
      keyPressTime[2] = currentTime;
      longPressStarted[2] = false;
      osal_start_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW3_EVT, LONG_PRESS_TIME);
    }
    else if (keyPressed[2])
    {
      osal_stop_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW3_EVT);
      if (longPressStarted[2])
      {
        sendToAllBindings(TRUE, COMMAND_LEVEL_STOP, APP_ENDPOINT_1);
      }
      else
      {
        sendToAllBindings(FALSE, COMMAND_ON, APP_ENDPOINT_1);
      }
      longPressStarted[2] = false;
      keyPressed[2] = false;
    }
  }

  /* SW4 control */
  if (keys & HAL_KEY_SW_4)
  {
    if (currentState & HAL_KEY_SW_4)
    {
      keyPressed[3] = true;
      keyPressTime[3] = currentTime;
      longPressStarted[3] = false;
      osal_start_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW4_EVT, LONG_PRESS_TIME);
    }
    else if (keyPressed[3])
    {
      osal_stop_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW4_EVT);
      if (longPressStarted[3])
      {
        sendToAllBindings(TRUE, COMMAND_LEVEL_STOP, APP_ENDPOINT_1);
      }
      else
      {
        sendToAllBindings(FALSE, COMMAND_OFF, APP_ENDPOINT_1);
      }
      longPressStarted[3] = false;
      keyPressed[3] = false;
    }
  }

  /* SW5 control */
  if (keys & HAL_KEY_SW_5)
  {
    if (currentState & HAL_KEY_SW_5)
    {
      keyPressed[4] = true;
      keyPressTime[4] = currentTime;
      longPressStarted[4] = false;
      osal_start_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW5_EVT, LONG_PRESS_TIME);
    }
    else if (keyPressed[4])
    {
      osal_stop_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW5_EVT);
      if (longPressStarted[4])
      {
        sendToAllBindings(TRUE, COMMAND_LEVEL_STOP, APP_ENDPOINT_2);
      }
      else
      {
        sendToAllBindings(FALSE, COMMAND_ON, APP_ENDPOINT_2);
      }
      longPressStarted[4] = false;
      keyPressed[4] = false;
    }
  }

  /* SW6 control */
  if (keys & HAL_KEY_SW_6)
  {
    if (currentState & HAL_KEY_SW_6)
    {
      keyPressed[5] = true;
      keyPressTime[5] = currentTime;
      longPressStarted[5] = false;
      osal_start_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW6_EVT, LONG_PRESS_TIME);
    }
    else if (keyPressed[5])
    {
      osal_stop_timerEx(zclGenericApp_TaskID, GENERICAPP_LONG_PRESS_SW6_EVT);
      if (longPressStarted[5])
      {
        sendToAllBindings(TRUE, COMMAND_LEVEL_STOP, APP_ENDPOINT_2);
      }
      else
      {
        sendToAllBindings(FALSE, COMMAND_OFF, APP_ENDPOINT_2);
      }
      longPressStarted[5] = false;
      keyPressed[5] = false;
    }
  }
}


static void sendToAllBindings(bool level ,uint8 cmd, byte endpoint)
{
  uint8 bindIndex = 0;
  BindingEntry_t *bind;

  while ((bind = bindFind(endpoint, ZCL_CLUSTER_ID_GEN_ON_OFF, bindIndex)) != NULL)
  {
    afAddrType_t dstAddr;

    if (bind->dstGroupMode == DSTGROUPMODE_ADDR)
    {
      dstAddr.addrMode = afAddr16Bit;
      dstAddr.endPoint = bind->dstEP;
      dstAddr.addr.shortAddr = bindingAddrMgsHelperConvertShort(bind->dstIdx);
    }
    else if (bind->dstGroupMode == DSTGROUPMODE_GROUP)
    {
      dstAddr.addrMode = afAddrGroup;
      dstAddr.endPoint = 0xFF; 
      dstAddr.addr.shortAddr = bind->dstIdx;
    }
    else
    {
      bindIndex++;
      continue;
    }

    if (!level && cmd == COMMAND_OFF)
    {
      zclGeneral_SendOnOff_CmdOff(endpoint, &dstAddr, FALSE, bdb_getZCLFrameCounter());
    }
    else if (!level && cmd == COMMAND_ON)
    {
      zclGeneral_SendOnOff_CmdOn(endpoint, &dstAddr, FALSE, bdb_getZCLFrameCounter());
    }
    else if (level && (cmd == LEVEL_MOVE_UP || cmd == LEVEL_MOVE_DOWN)){
       zclGeneral_SendLevelControlMoveRequest(endpoint, &dstAddr, COMMAND_LEVEL_MOVE, cmd, 50, TRUE, bdb_getZCLFrameCounter());
    }
    else if(level && cmd == COMMAND_LEVEL_STOP){
       zclGeneral_SendLevelControlStopRequest(APP_ENDPOINT_1, &dstAddr, COMMAND_LEVEL_STOP, TRUE, bdb_getZCLFrameCounter());
    }

    bindIndex++;
  }
}


/*********************************************************************
 * @fn      zclGenericApp_ProcessCommissioningStatus
 *
 * @brief   Callback in which the status of the commissioning process are reported
 *
 * @param   bdbCommissioningModeMsg - Context message of the status of a commissioning process
 *
 * @return  none
 */
static void zclGenericApp_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
  switch(bdbCommissioningModeMsg->bdbCommissioningMode)
  {
    case BDB_COMMISSIONING_FORMATION:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING | bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
      }
      break;
    case BDB_COMMISSIONING_NWK_STEERING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        // Network steering successful
      }
      break;
    case BDB_COMMISSIONING_FINDING_BINDING:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_SUCCESS)
      {
        // Finding binding successful
      }
      break;
    case BDB_COMMISSIONING_INITIALIZATION:
      // Initialization notification
      break;
#if ZG_BUILD_ENDDEVICE_TYPE    
    case BDB_COMMISSIONING_PARENT_LOST:
      if(bdbCommissioningModeMsg->bdbCommissioningStatus == BDB_COMMISSIONING_NETWORK_RESTORED)
      {
        // Network restored
      }
      else
      {
        osal_start_timerEx(zclGenericApp_TaskID, GENERICAPP_END_DEVICE_REJOIN_EVT, GENERICAPP_END_DEVICE_REJOIN_DELAY);
      }
      break;
#endif 
  }
}

/*********************************************************************
 * @fn      zclGenericApp_ProcessIdentifyTimeChange
 *
 * @brief   Called to process any change to the IdentifyTime attribute.
 *
 * @param   endpoint - in which the identify has change
 *
 * @return  none
 */
static void zclGenericApp_ProcessIdentifyTimeChange( uint8 endpoint )
{
  (void) endpoint;
}

/*********************************************************************
 * @fn      zclGenericApp_BasicResetCB
 *
 * @brief   Callback from the ZCL General Cluster Library
 *          to set all the Basic Cluster attributes to default values.
 *
 * @param   none
 *
 * @return  none
 */
static void zclGenericApp_BasicResetCB( void )
{
  zclGenericApp_ResetAttributesToDefaultValues();
}




