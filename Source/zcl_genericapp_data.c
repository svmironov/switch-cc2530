/**************************************************************************************************
  Filename:       zcl_genericapp_data.c
  Revised:        $Date: 2014-05-12 13:14:02 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38502 $


  Description:    Zigbee Cluster Library - sample device application.

**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_genericapp.h"

/*********************************************************************
 * CONSTANTS
 */

#define GENERICAPP_DEVICE_VERSION     0
#define GENERICAPP_FLAGS              0

#define GENERICAPP_HWVERSION          1
#define GENERICAPP_ZCLVERSION         2
#define GENERICAPP_APPVERSION         1
#define GENERICAPP_STACKVERSION       1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Global attributes
const uint16 zclGenericApp_clusterRevision_all = 0x0001; 

// Basic Cluster
const uint8 zclGenericApp_ZCLVersion = GENERICAPP_ZCLVERSION;
const uint8 zclGenericApp_AppVersion = GENERICAPP_APPVERSION;
const uint8 zclGenericApp_StackVersion = GENERICAPP_STACKVERSION;
const uint8 zclGenericApp_HWRevision = GENERICAPP_HWVERSION;
const uint8 zclGenericApp_ManufacturerName[] = { 11, 'M','a','g','i','c','S','w','i','t','c','h'};
const uint8 zclGenericApp_ModelId[] = { 3, 'O','n','e' };
const uint8 zclGenericApp_DateCode[] = { 16, '2','0','2','5','0','5','2','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclGenericApp_PowerSource = POWER_SOURCE_BATTERY;
uint8 zclGenericApp_LocationDescription[] = { 16, 'H','O','M','E',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 zclGenericApp_PhysicalEnvironment = 0;
uint8 zclGenericApp_DeviceEnable = DEVICE_ENABLED;
uint8 zclGenericApp_SoftwareBuildID[] = { 4, '0','0','0','1' };

// Identify Cluster
uint16 zclGenericApp_IdentifyTime = 0;

// On/Off Cluster
uint8  zclGenericApp_OnOff = 0;

uint8 zclGenericApp_LevelCurrentLevel = 0x80;

#if ZCL_DISCOVER
CONST zclCommandRec_t zclGenericApp_Cmds[] =
{
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    COMMAND_BASIC_RESET_FACT_DEFAULT,
    CMD_DIR_SERVER_RECEIVED
  },

};

CONST uint8 zclCmdsArraySize = ( sizeof(zclGenericApp_Cmds) / sizeof(zclGenericApp_Cmds[0]) );
#endif // ZCL_DISCOVER

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
CONST zclAttrRec_t zclGenericApp_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_ZCL_VERSION, ZCL_DATATYPE_UINT8, ACCESS_CONTROL_READ, (void *)&zclGenericApp_ZCLVersion}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_APPL_VERSION, ZCL_DATATYPE_UINT8, ACCESS_CONTROL_READ, (void *)&zclGenericApp_AppVersion}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_STACK_VERSION, ZCL_DATATYPE_UINT8, ACCESS_CONTROL_READ, (void *)&zclGenericApp_StackVersion}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_HW_VERSION, ZCL_DATATYPE_UINT8, ACCESS_CONTROL_READ, (void *)&zclGenericApp_HWRevision}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_MANUFACTURER_NAME, ZCL_DATATYPE_CHAR_STR, ACCESS_CONTROL_READ, (void *)zclGenericApp_ManufacturerName}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_MODEL_ID, ZCL_DATATYPE_CHAR_STR, ACCESS_CONTROL_READ,(void *)zclGenericApp_ModelId}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_DATE_CODE, ZCL_DATATYPE_CHAR_STR, ACCESS_CONTROL_READ, (void *)zclGenericApp_DateCode}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_POWER_SOURCE, ZCL_DATATYPE_ENUM8, ACCESS_CONTROL_READ, (void *)&zclGenericApp_PowerSource}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_LOCATION_DESC, ZCL_DATATYPE_CHAR_STR, (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE), (void *)zclGenericApp_LocationDescription}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_PHYSICAL_ENV, ZCL_DATATYPE_ENUM8, (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE), (void *)&zclGenericApp_PhysicalEnvironment}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_DEVICE_ENABLED, ZCL_DATATYPE_BOOLEAN, (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE), (void *)&zclGenericApp_DeviceEnable}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_BASIC_SW_BUILD_ID, ZCL_DATATYPE_CHAR_STR, ACCESS_CONTROL_READ, (void *)&zclGenericApp_SoftwareBuildID}},
  {ZCL_CLUSTER_ID_GEN_BASIC, {ATTRID_CLUSTER_REVISION, ZCL_DATATYPE_UINT16, ACCESS_CONTROL_READ, (void *)&zclGenericApp_clusterRevision_all}},
  
#ifdef ZCL_IDENTIFY
  // *** Identify Cluster Attribute ***
  {ZCL_CLUSTER_ID_GEN_IDENTIFY, {ATTRID_IDENTIFY_TIME, ZCL_DATATYPE_UINT16, (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE), (void *)&zclGenericApp_IdentifyTime}},
  {ZCL_CLUSTER_ID_GEN_IDENTIFY, {ATTRID_CLUSTER_REVISION, ZCL_DATATYPE_UINT16, ACCESS_CONTROL_READ | ACCESS_CLIENT, (void *)&zclGenericApp_clusterRevision_all}},
#endif

  // *** On / Off Cluster *** //
  {ZCL_CLUSTER_ID_GEN_ON_OFF, {ATTRID_ON_OFF, ZCL_DATATYPE_BOOLEAN, ACCESS_CONTROL_READ, (void *)&zclGenericApp_OnOff}},
  {ZCL_CLUSTER_ID_GEN_ON_OFF, {ATTRID_CLUSTER_REVISION, ZCL_DATATYPE_UINT16, ACCESS_CONTROL_READ | ACCESS_CLIENT, (void *)&zclGenericApp_clusterRevision_all}},

  {ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL, {ATTRID_LEVEL_CURRENT_LEVEL, ZCL_DATATYPE_UINT8, ACCESS_CONTROL_READ, (void *)&zclGenericApp_LevelCurrentLevel}},
  {ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL, {ATTRID_CLUSTER_REVISION, ZCL_DATATYPE_UINT16, ACCESS_CONTROL_READ, (void *)&zclGenericApp_clusterRevision_all}},
 };

uint8 CONST zclGenericApp_NumAttributes = ( sizeof(zclGenericApp_Attrs) / sizeof(zclGenericApp_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */

const cId_t zclGenericApp_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
};

const cId_t zclGenericApp2_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
};

#define ZCLGENERICAPP_MAX_INCLUSTERS   (sizeof(zclGenericApp_InClusterList) / sizeof(zclGenericApp_InClusterList[0]))
#define ZCLGENERICAPP2_MAX_INCLUSTERS   (sizeof(zclGenericApp2_InClusterList) / sizeof(zclGenericApp2_InClusterList[0]))

const cId_t zclGenericApp_OutClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL
};

const cId_t zclGenericApp2_OutClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY,
  ZCL_CLUSTER_ID_GEN_ON_OFF,
  ZCL_CLUSTER_ID_GEN_LEVEL_CONTROL
};

#define ZCLGENERICAPP_MAX_OUTCLUSTERS  (sizeof(zclGenericApp_OutClusterList) / sizeof(zclGenericApp_OutClusterList[0]))
#define ZCLGENERICAPP2_MAX_OUTCLUSTERS  (sizeof(zclGenericApp2_OutClusterList) / sizeof(zclGenericApp2_OutClusterList[0]))

SimpleDescriptionFormat_t zclGenericApp_Desc =
{
  APP_ENDPOINT_1,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16 AppProfId;
  // GENERICAPP_TODO: Replace ZCL_HA_DEVICEID_ON_OFF_LIGHT with application specific device ID
  ZCL_HA_DEVICEID_ON_OFF_SWITCH,  //  uint16 AppDeviceId; 
  GENERICAPP_DEVICE_VERSION,            //  int   AppDevVer:4;
  GENERICAPP_FLAGS,                     //  int   AppFlags:4;
  ZCLGENERICAPP_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclGenericApp_InClusterList, //  byte *pAppInClusterList;
  ZCLGENERICAPP_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)zclGenericApp_OutClusterList //  byte *pAppInClusterList;
};


SimpleDescriptionFormat_t zclGenericApp_Desc_2 =
{
  APP_ENDPOINT_2,
  ZCL_HA_PROFILE_ID,
  ZCL_HA_DEVICEID_ON_OFF_SWITCH,
  GENERICAPP_DEVICE_VERSION,
  GENERICAPP_FLAGS,
  ZCLGENERICAPP2_MAX_INCLUSTERS,
  (cId_t *)zclGenericApp2_InClusterList,
  ZCLGENERICAPP2_MAX_OUTCLUSTERS,
  (cId_t *)zclGenericApp2_OutClusterList
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
  
void zclGenericApp_ResetAttributesToDefaultValues(void)
{
  int i;
  
  zclGenericApp_LocationDescription[0] = 16;
  for (i = 1; i <= 16; i++)
  {
    zclGenericApp_LocationDescription[i] = ' ';
  }
  
  zclGenericApp_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;
  zclGenericApp_DeviceEnable = DEVICE_ENABLED;
  
#ifdef ZCL_IDENTIFY
  zclGenericApp_IdentifyTime = 0;
#endif
  
}

/****************************************************************************
****************************************************************************/


