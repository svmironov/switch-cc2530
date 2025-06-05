/**************************************************************************************************
  Filename:       hal_key.c
  Revised:        $Date: 2010-09-15 19:02:45 -0700 (Wed, 15 Sep 2010) $
  Revision:       $Revision: 23815 $

**************************************************************************************************/

/*********************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_board.h"
#include "hal_drivers.h"
#include "hal_adc.h"
#include "hal_key.h"
#include "osal.h"

#if (defined HAL_KEY) && (HAL_KEY == TRUE)

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
   
#define HAL_KEY_RISING_EDGE   0
#define HAL_KEY_FALLING_EDGE  1

#define HAL_KEY_PORT_0_EDGEBIT   BV(0)
#define HAL_KEY_PORT_1_EDGEBIT   BV(1)
#define HAL_KEY_PORT_2_EDGEBIT   BV(3)

#define HAL_KEY_DEBOUNCE_VALUE  10

/* CPU port interrupt */
#define HAL_KEY_CPU_PORT_0_IF P0IF
#define HAL_KEY_CPU_PORT_1_IF P1IF
#define HAL_KEY_CPU_PORT_2_IF P2IF

/* SW_6 is at P0.3 */
#define HAL_KEY_SW_6_PORT   P0
#define HAL_KEY_SW_6_BIT    BV(3)
#define HAL_KEY_SW_6_SEL    P0SEL
#define HAL_KEY_SW_6_DIR    P0DIR
/* edge interrupt */
#define HAL_KEY_SW_6_EDGEBIT  BV(0)
#define HAL_KEY_SW_6_EDGE     HAL_KEY_FALLING_EDGE
/* SW_6 interrupts */
#define HAL_KEY_SW_6_IEN      IEN1  
#define HAL_KEY_SW_6_IENBIT   BV(5) 
#define HAL_KEY_SW_6_ICTL     P0IEN 
#define HAL_KEY_SW_6_ICTLBIT  BV(3) 
#define HAL_KEY_SW_6_PXIFG    P0IFG 

/* SW_5 is at P0.2 */
#define HAL_KEY_SW_5_PORT   P0
#define HAL_KEY_SW_5_BIT    BV(2)
#define HAL_KEY_SW_5_SEL    P0SEL
#define HAL_KEY_SW_5_DIR    P0DIR
/* edge interrupt */
#define HAL_KEY_SW_5_EDGEBIT  BV(0)
#define HAL_KEY_SW_5_EDGE     HAL_KEY_FALLING_EDGE
/* SW_5 interrupts */
#define HAL_KEY_SW_5_IEN      IEN1  
#define HAL_KEY_SW_5_IENBIT   BV(5) 
#define HAL_KEY_SW_5_ICTL     P0IEN 
#define HAL_KEY_SW_5_ICTLBIT  BV(2) 
#define HAL_KEY_SW_5_PXIFG    P0IFG 

/* SW_1 is at P2.0 */
#define HAL_KEY_SW_1_PORT   P2
#define HAL_KEY_SW_1_BIT    BV(0)
#define HAL_KEY_SW_1_SEL    P2SEL
#define HAL_KEY_SW_1_DIR    P2DIR
/* edge interrupt */
#define HAL_KEY_SW_1_EDGEBIT  BV(3)
#define HAL_KEY_SW_1_EDGE     HAL_KEY_FALLING_EDGE
/* Joy move interrupts */
#define HAL_KEY_SW_1_IEN      IEN2  
#define HAL_KEY_SW_1_IENBIT   BV(1) 
#define HAL_KEY_SW_1_ICTL     P2IEN 
#define HAL_KEY_SW_1_ICTLBIT  BV(0) 
#define HAL_KEY_SW_1_PXIFG    P2IFG 

/* SW_4 is at P0.1 */
#define HAL_KEY_SW_4_PORT   P0
#define HAL_KEY_SW_4_BIT    BV(1)
#define HAL_KEY_SW_4_SEL    P0SEL
#define HAL_KEY_SW_4_DIR    P0DIR
/* edge interrupt */
#define HAL_KEY_SW_4_EDGEBIT  BV(0)
#define HAL_KEY_SW_4_EDGE     HAL_KEY_FALLING_EDGE
/* SW_4 interrupts */
#define HAL_KEY_SW_4_IEN      IEN1  
#define HAL_KEY_SW_4_IENBIT   BV(5) 
#define HAL_KEY_SW_4_ICTL     P0IEN 
#define HAL_KEY_SW_4_ICTLBIT  BV(1) 
#define HAL_KEY_SW_4_PXIFG    P0IFG 

/* SW_3 is at P0.4 */
#define HAL_KEY_SW_3_PORT   P0
#define HAL_KEY_SW_3_BIT    BV(4)
#define HAL_KEY_SW_3_SEL    P0SEL
#define HAL_KEY_SW_3_DIR    P0DIR
/* edge interrupt */
#define HAL_KEY_SW_3_EDGEBIT  BV(0)
#define HAL_KEY_SW_3_EDGE     HAL_KEY_FALLING_EDGE
/* SW_3 interrupts */
#define HAL_KEY_SW_3_IEN      IEN1  
#define HAL_KEY_SW_3_IENBIT   BV(5) 
#define HAL_KEY_SW_3_ICTL     P0IEN 
#define HAL_KEY_SW_3_ICTLBIT  BV(4) 
#define HAL_KEY_SW_3_PXIFG    P0IFG 

static bool port0Edge = false;
static bool port1Edge = false;
volatile bool HAL_KEY_SW_6_ACTIVE = false;
volatile bool HAL_KEY_SW_5_ACTIVE = false;
volatile bool HAL_KEY_SW_4_ACTIVE = false;
volatile bool HAL_KEY_SW_3_ACTIVE = false;
volatile bool HAL_KEY_SW_1_ACTIVE = false;

/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/


/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
static halKeyCBack_t pHalKeyProcessFunction;
static uint8 HalKeyConfigured;
bool Hal_KeyIntEnable;            /* interrupt enable/disable flag */

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
void halProcessKeyInterrupt(void);
uint8 halGetJoyKeyInput(void);



/**************************************************************************************************
 *                                        FUNCTIONS - API
 **************************************************************************************************/


/**************************************************************************************************
 * @fn      HalKeyInit
 *
 * @brief   Initilize Key Service
 *
 * @param   none
 *
 * @return  None
 **************************************************************************************************/
void HalKeyInit( void )
{
  P0SEL &= ~(HAL_KEY_SW_1_BIT | HAL_KEY_SW_3_BIT | HAL_KEY_SW_4_BIT | HAL_KEY_SW_5_BIT | HAL_KEY_SW_6_BIT);
  P0DIR &= ~(HAL_KEY_SW_1_BIT | HAL_KEY_SW_3_BIT | HAL_KEY_SW_4_BIT | HAL_KEY_SW_5_BIT | HAL_KEY_SW_6_BIT);
  P0INP &= ~(HAL_KEY_SW_1_BIT | HAL_KEY_SW_3_BIT | HAL_KEY_SW_4_BIT | HAL_KEY_SW_5_BIT | HAL_KEY_SW_6_BIT); // pull-up

  /* Initialize callback function */
  pHalKeyProcessFunction  = NULL;

  /* Start with key is not configured */
  HalKeyConfigured = FALSE;
}


/**************************************************************************************************
 * @fn      HalKeyConfig
 *
 * @brief   Configure the Key serivce
 *
 * @param   interruptEnable - TRUE/FALSE, enable/disable interrupt
 *          cback - pointer to the CallBack function
 *
 * @return  None
 **************************************************************************************************/
void HalKeyConfig (bool interruptEnable, halKeyCBack_t cback)
{
  /* Enable/Disable Interrupt or */
  Hal_KeyIntEnable = interruptEnable;

  /* Register the callback fucntion */
  pHalKeyProcessFunction = cback;

  /* Determine if interrupt is enable or not */
  if (Hal_KeyIntEnable)
  {
    
    /* Rising/Falling edge configuratinn */
    PICTL |= HAL_KEY_PORT_0_EDGEBIT; 
    PICTL |= HAL_KEY_PORT_1_EDGEBIT; 
    PICTL |= HAL_KEY_PORT_2_EDGEBIT; 
    
    HAL_KEY_SW_6_ICTL |= HAL_KEY_SW_6_ICTLBIT;
    HAL_KEY_SW_6_IEN |= HAL_KEY_SW_6_IENBIT;
    HAL_KEY_SW_6_PXIFG = ~(HAL_KEY_SW_6_BIT);

    HAL_KEY_SW_5_ICTL |= HAL_KEY_SW_5_ICTLBIT;
    HAL_KEY_SW_5_IEN |= HAL_KEY_SW_5_IENBIT;
    HAL_KEY_SW_5_PXIFG = ~(HAL_KEY_SW_5_BIT);  
    
    HAL_KEY_SW_4_ICTL |= HAL_KEY_SW_4_ICTLBIT;
    HAL_KEY_SW_4_IEN |= HAL_KEY_SW_4_IENBIT;
    HAL_KEY_SW_4_PXIFG = ~(HAL_KEY_SW_4_BIT);

    HAL_KEY_SW_3_ICTL |= HAL_KEY_SW_3_ICTLBIT;
    HAL_KEY_SW_3_IEN |= HAL_KEY_SW_3_IENBIT;
    HAL_KEY_SW_3_PXIFG = ~(HAL_KEY_SW_3_BIT);
    
    HAL_KEY_SW_1_ICTL |= HAL_KEY_SW_1_ICTLBIT;
    HAL_KEY_SW_1_IEN |= HAL_KEY_SW_1_IENBIT;
    HAL_KEY_SW_1_PXIFG = ~(HAL_KEY_SW_1_BIT);

    /* Do this only after the hal_key is configured - to work with sleep stuff */
    if (HalKeyConfigured == TRUE)
    {
      osal_stop_timerEx(Hal_TaskID, HAL_KEY_EVENT);  /* Cancel polling if active */
    }
  }
  else    /* Interrupts NOT enabled */
  {
    HAL_KEY_SW_6_ICTL &= ~(HAL_KEY_SW_6_ICTLBIT); /* don't generate interrupt */
    HAL_KEY_SW_6_IEN &= ~(HAL_KEY_SW_6_IENBIT);   /* Clear interrupt enable bit */

    HAL_KEY_SW_5_ICTL &= ~(HAL_KEY_SW_5_ICTLBIT); 
    HAL_KEY_SW_5_IEN &= ~(HAL_KEY_SW_5_IENBIT);  
    
    HAL_KEY_SW_4_ICTL &= ~(HAL_KEY_SW_4_ICTLBIT); 
    HAL_KEY_SW_4_IEN &= ~(HAL_KEY_SW_4_IENBIT);   

    HAL_KEY_SW_3_ICTL &= ~(HAL_KEY_SW_3_ICTLBIT);
    HAL_KEY_SW_3_IEN &= ~(HAL_KEY_SW_3_IENBIT);   
    
    HAL_KEY_SW_1_ICTL &= ~(HAL_KEY_SW_1_ICTLBIT); 
    HAL_KEY_SW_1_IEN &= ~(HAL_KEY_SW_1_IENBIT);     
    
    osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
  }

  /* Key now is configured */
  HalKeyConfigured = TRUE;
}


/**************************************************************************************************
 * @fn      HalKeyRead
 *
 * @brief   Read the current value of a key
 *
 * @param   None
 *
 * @return  keys - current keys status
 **************************************************************************************************/
uint8 HalKeyRead ( void )
{
  uint8 keys = 0;

  if ((HAL_KEY_SW_6_PORT & HAL_KEY_SW_6_BIT) == 0)
  {
    keys |= HAL_KEY_SW_6;
  }

  if ((HAL_KEY_SW_5_PORT & HAL_KEY_SW_5_BIT) == 0)  /* Key is active low */
  {
    keys |= HAL_KEY_SW_5;
  }

  if ((HAL_KEY_SW_4_PORT & HAL_KEY_SW_4_BIT) == 0)  
  {
    keys |= HAL_KEY_SW_4;
  }

  if ((HAL_KEY_SW_3_PORT & HAL_KEY_SW_3_BIT) == 0)  
  {
    keys |= HAL_KEY_SW_3;
  }

  if ((HAL_KEY_SW_1_PORT & HAL_KEY_SW_1_BIT) == 0) 
  {
    keys |= HAL_KEY_SW_1;
  }

  return keys;
}




/**************************************************************************************************
 * @fn      HalKeyPoll
 *
 * @brief   Called by hal_driver to poll the keys
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalKeyPoll (void)
{
  uint8 keys = 0;

  if (HAL_KEY_SW_6_ACTIVE == true)  /* Interrupt Flag has been set */
  {
    keys |= HAL_KEY_SW_6;
  }
  HAL_KEY_SW_6_ACTIVE = false;


  if (HAL_KEY_SW_5_ACTIVE == true)  
  {
    keys |= HAL_KEY_SW_5;
  }  
  HAL_KEY_SW_5_ACTIVE = false;  

  
  if (HAL_KEY_SW_4_ACTIVE == true)  
  {
    keys |= HAL_KEY_SW_4;
  }  
  HAL_KEY_SW_4_ACTIVE = false;  

  if (HAL_KEY_SW_3_ACTIVE == true)  
  {
    keys |= HAL_KEY_SW_3;
  }  
  HAL_KEY_SW_3_ACTIVE = false;  

  
  if (HAL_KEY_SW_1_ACTIVE == true)  
  {
    keys |= HAL_KEY_SW_1;
  }  
  HAL_KEY_SW_1_ACTIVE = false;  

  /* Invoke Callback if new keys were depressed */
  if (pHalKeyProcessFunction
#ifdef HAL_LEGACY_KEYS
    && keys //in legacy modes, only report key presses and do not report when a key is released
#endif
    )
  {
    (pHalKeyProcessFunction) (keys, HAL_KEY_STATE_NORMAL);
  }
}


/**************************************************************************************************
 * @fn      halProcessKeyInterrupt
 *
 * @brief   Checks to see if it's a valid key interrupt, saves interrupt driven key states for
 *          processing by HalKeyRead(), and debounces keys by scheduling HalKeyRead() 25ms later.
 *
 * @param
 *
 * @return
 **************************************************************************************************/
void halProcessKeyInterrupt (void)
{
    osal_start_timerEx (Hal_TaskID, HAL_KEY_EVENT, HAL_KEY_DEBOUNCE_VALUE);
}


/**************************************************************************************************
 * @fn      HalKeyEnterSleep
 *
 * @brief  - Get called to enter sleep mode
 *
 * @param
 *
 * @return
 **************************************************************************************************/
void HalKeyEnterSleep ( void )
{
}

/**************************************************************************************************
 * @fn      HalKeyExitSleep
 *
 * @brief   - Get called when sleep is over
 *
 * @param
 *
 * @return  - return saved keys
 **************************************************************************************************/
uint8 HalKeyExitSleep ( void )
{
  /* Wake up and read keys */
  return ( HalKeyRead () );
}

/***************************************************************************************************
 *                                    INTERRUPT SERVICE ROUTINE
 ***************************************************************************************************/

/**************************************************************************************************
 * @fn      halKeyPort0Isr
 *
 * @brief   Port0 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort0Isr, P0INT_VECTOR )
{
  HAL_ENTER_ISR();

  if (HAL_KEY_SW_6_PXIFG & HAL_KEY_SW_6_BIT) /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_6_ACTIVE = true;
  }

   if (HAL_KEY_SW_5_PXIFG & HAL_KEY_SW_5_BIT) 
  {
    HAL_KEY_SW_5_ACTIVE = true;
  }

  if (HAL_KEY_SW_4_PXIFG & HAL_KEY_SW_4_BIT)
  {
    HAL_KEY_SW_4_ACTIVE = true;
  }

  if (HAL_KEY_SW_3_PXIFG & HAL_KEY_SW_3_BIT)
  {
    HAL_KEY_SW_3_ACTIVE = true;
  }
  
    /* Toggle edge detection for all pins on this port */
  if (port0Edge)
    PICTL |= HAL_KEY_PORT_0_EDGEBIT;
  else
    PICTL &= ~(HAL_KEY_PORT_0_EDGEBIT);
  port0Edge = !port0Edge;
   
  halProcessKeyInterrupt();

  //Clear the CPU interrupt flag for Port_0
  //PxIFG has to be cleared before PxIF
  HAL_KEY_SW_6_PXIFG = 0;
  HAL_KEY_SW_5_PXIFG = 0;
  HAL_KEY_SW_4_PXIFG = 0;
  HAL_KEY_SW_3_PXIFG = 0;
  HAL_KEY_CPU_PORT_0_IF = 0;
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

/**************************************************************************************************
 * @fn      halKeyPort1Isr
 *
 * @brief   Port1 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort1Isr, P1INT_VECTOR )
{
  HAL_ENTER_ISR();

   if (HAL_KEY_SW_5_PXIFG & HAL_KEY_SW_5_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_5_ACTIVE = true;
  }
   
    if (port1Edge)
    PICTL |= HAL_KEY_PORT_1_EDGEBIT;
  else
    PICTL &= ~(HAL_KEY_PORT_1_EDGEBIT);
  port1Edge = !port1Edge;

  
  halProcessKeyInterrupt();

  //Clear the CPU interrupt flag for Port_1
  //PxIFG has to be cleared before PxIF
  HAL_KEY_SW_5_PXIFG = 0;
  HAL_KEY_CPU_PORT_1_IF = 0;
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

/**************************************************************************************************
 * @fn      halKeyPort2Isr
 *
 * @brief   Port2 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort2Isr, P2INT_VECTOR )
{
  HAL_ENTER_ISR();

  if (HAL_KEY_SW_1_PXIFG & HAL_KEY_SW_1_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_1_ACTIVE = true;
  }  
  
  halProcessKeyInterrupt();

  //Clear the CPU interrupt flag for Port_0
  //PxIFG has to be cleared before PxIF
  HAL_KEY_SW_1_PXIFG = 0;
  HAL_KEY_CPU_PORT_2_IF = 0;  
   
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

#else


void HalKeyInit(void){}
void HalKeyConfig(bool interruptEnable, halKeyCBack_t cback){}
uint8 HalKeyRead(void){ return 0;}
void HalKeyPoll(void){}

#endif /* HAL_KEY */





/**************************************************************************************************
**************************************************************************************************/