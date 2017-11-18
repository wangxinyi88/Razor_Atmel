/**********************************************************************************************************************
File: user_app1.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app1.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern u32 G_u32AntApiCurrentDataTimeStamp;                  
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;   
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];//收到从机发过的值
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;  

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */
static u32 UserApp_u32Timeout;
static u32 UserApp_u32DataMsgCount = 0;   /* ANT_DATA packets received */
static u32 UserApp_u32TickMsgCount = 0;   /* ANT_TICK packets received */
static bool bStateflag=TRUE    ;           /*true->master     flase->slave*/
static u8 u8MAorSL;
static bool bShow=TRUE;
static bool bLcdSlaveShow=FALSE;
static bool bLedShow=FALSE;
/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
/* Wait for ANT channel assignment */
static void UserApp1SM_WaitChannelAssign()
{
 
  if( AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CONFIGURED)
  {
    /* Channel assignment is successful, so open channel and
    proceed to Idle state */
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  

  /* Watch for time out */
  if(IsTimeUp(&UserApp_u32Timeout, 3000))
  {
    UserApp1_StateMachine = UserApp1SM_Error;    
  }
    
} /* end UserApp1SM_AntChannelAssign */
/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/

 void UserApp1Initialize(void)
{
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(BLUE);
  LedOff(RED); 
  LedOff(WHITE); 
  LedOff(PURPLE); 
  LedOff(CYAN); 
  LedOff(ORANGE); 
  static u8 au8WelcomeMessage[] = "Hide and go Seek";
  static  u8 au8Instructions[] = "Start Press Button 0 ";
  AntAssignChannelInfoType sAntSetupData;
   bLcdSlaveShow=FALSE;
   bShow=TRUE;
   bStateflag=TRUE; 
   bLedShow=TRUE;
  /* Clear screen and place start messages */
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, au8WelcomeMessage); 
  LCDMessage(LINE2_START_ADDR, au8Instructions); 

  /* Start with LED0 in RED state = channel is not configured */

 /* Configure ANT for this application */
  sAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  sAntSetupData.AntChannelType      = ANT_CHANNEL_TYPE_MASTER;
  sAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  sAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  
  sAntSetupData.AntDeviceIdLo       = ANT_DerEVICEID_LO_USERAPP;
  sAntSetupData.AntDeviceIdHi       = ANT_DEVICEID_HI_USERAPP;
  sAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  sAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  sAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  sAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;

  sAntSetupData.AntNetwork = ANT_NETWORK_DEFAULT;
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    sAntSetupData.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
    
  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&sAntSetupData) )
  {
    /* Channel is configured, so change LED to yellow */
    UserApp1_StateMachine = UserApp1SM_WaitChannelAssign;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    LedBlink(RED, LED_4HZ);
    UserApp1_StateMachine = UserApp1SM_Error;
  }
} /* end UserApp1Initialize() */

 static void UserApp1SlaveInitialize(void)
{
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(BLUE);
  LedOff(RED); 
  LedOff(WHITE); 
  LedOff(PURPLE); 
  LedOff(CYAN); 
  LedOff(ORANGE); 
  PWMAudioOn(BUZZER1);

  static u8 au8WelcomeMessage[] = "Hide and go Seek";
  static u8 au8Instructions[] = "Start Press Button 0 ";
  AntAssignChannelInfoType sAntSetupData;
  
   bStateflag=FALSE;  
  /* Clear screen and place start messages */
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, au8WelcomeMessage); 
  LCDMessage(LINE2_START_ADDR, au8Instructions); 

  /* Start with LED0 in RED state = channel is not configured */

  
 /* Configure ANT for this application */
  sAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  sAntSetupData.AntChannelType      = ANT_CHANNEL_TYPE_SLAVE;
  sAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  sAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  
  sAntSetupData.AntDeviceIdLo       = ANT_DerEVICEID_LO_USERAPP;
  sAntSetupData.AntDeviceIdHi       = ANT_DEVICEID_HI_USERAPP;
  sAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  sAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  sAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  sAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;

  sAntSetupData.AntNetwork = ANT_NETWORK_DEFAULT;
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    sAntSetupData.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
    
  /* If good initialization, set state to Idle */
  if( AntAssignChannel(&sAntSetupData) )
  {
    /* Channel is configured, so change LED to yellow */
    UserApp1_StateMachine = UserApp1SM_WaitChannelAssign;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    LedBlink(RED, LED_4HZ);
    UserApp1_StateMachine = UserApp1SM_Error;
  }
} /* end UserApp1Initialize() */
  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
#if 0
static void AntUnassignChannelNumber(ANT_CHANNEL_USERAPP)
{
 if( AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_UNCONFIGURED)
 {
     UserApp1_StateMachine = UserApp1SlaveInitialize;
 }
}
#endif
static void UserAppSM_WaitChannelClose(void)
{
  /* Monitor the channel status to check if channel is closed */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CLOSED)
  {
     if(WasButtonPressed(BUTTON1))
     {
      ButtonAcknowledge(BUTTON1);
      UserApp1_StateMachine = UserApp1SlaveInitialize;
     }
  }
  
 
} /* end UserAppSM_WaitChannelClose() */

static void UserAppSM_ChannelOpen(void)
{

  static u8 u8LastState = 0xff;
  static u8 au8SwitchSlaveTip[]="switch slave ?";
  static u8 au8SwitchSlaveButton[]="Please press Button1";
  static u8 au8TickMessage[] = "EVENT x\n\r";  /* "x" at index [6] will be replaced by the current code */
  static u8 au8TestMessage[] = {1, 2, 3, 4, 0xA5, 6, 7, 8};
   static u8 au8MasterTestMessage[] = {8, 7, 6, 5, 0xA5, 4, 3, 2, 1};
  static u8 u810Counter=0;
  static u8 u8BuzzerCounter=0;
  static bool bCompleted=TRUE;
  static s8 s8RssiValue;
  static s8 s8AbsRssiValue;
  LedNumberType aeLedDisplayLevels[LED_NUMBER] = {RED,ORANGE,YELLOW,GREEN,CYAN,BLUE,PURPLE,WHITE};
  static u8 au8LcdHiderShow[20]="Hide               ";
  static u8 au8LcdMasterShow[20]= "MASTER    -XX       ";
  u8* pau8LcdMasterShow=(&au8LcdMasterShow[0])+11;
  static u8 au8LcdSeekerShow[20]="Seeker      9      ";
  static u8 *pau8LcdSeekerShow=(&au8LcdSeekerShow[0])+12;
  static u8 au8LcdSlaveShow[20]= "SLAVE    -XX       ";
   u8 *pau8LcdSlaveShow=(&au8LcdSlaveShow[0])+11;
  static u8 au8LcdTipShow[20]="Ready? Yes.press B2";
  static s8 as8Levels[LED_NUMBER] = {-99,-84,-76,-69,-63,-58,-54,-51};
  static u8 u8TenCounter;
  /* A slave channel can close on its own, so explicitly check channel status */
  
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) != ANT_OPEN)
  {
     u8LastState = 0xff;
    UserApp_u32Timeout = G_u32SystemTime1ms;
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE2_START_ADDR, au8SwitchSlaveButton); 
    LCDMessage(LINE1_START_ADDR, au8SwitchSlaveTip); 
    UserApp1_StateMachine = UserAppSM_WaitChannelClose;
  } /* if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) != ANT_OPEN) */
   
  /* Always check for ANT messages */
  if( AntReadAppMessageBuffer() )
  {

     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      s8RssiValue=G_sAntApiCurrentMessageExtData.s8RSSI;
       
       if(s8RssiValue < s8WeakRssi)
       {
         s8RssiValue=-99;
       }
       
       if(s8RssiValue > 0)
       {
          s8AbsRssiValue = (u8)s8RssiValue;
       }
       else if(s8RssiValue < 0)
       {
         s8AbsRssiValue = (u8)(~s8RssiValue + 1);
        
       }
       else    
       {
         s8AbsRssiValue=0;
       }
       if(bStateflag)
       {
        *pau8LcdMasterShow = (s8AbsRssiValue / 10) + NUMBER_ASCII_TO_DEC;
        pau8LcdMasterShow++;
        *pau8LcdMasterShow = (s8AbsRssiValue % 10) + NUMBER_ASCII_TO_DEC;
       }
       else
       { 
        *pau8LcdSlaveShow = (s8AbsRssiValue / 10) + NUMBER_ASCII_TO_DEC;
        pau8LcdSlaveShow++;
        *pau8LcdSlaveShow = (s8AbsRssiValue % 10) + NUMBER_ASCII_TO_DEC;
       }
       if(bLedShow)
       {
           /*led show*/
          for(u8 i = 0; i < LED_NUMBER; i++)
          {
            if(s8RssiValue >= as8Levels[i])
            {
              LedOn(aeLedDisplayLevels[i]);
            }
            else
            {
              LedOff(aeLedDisplayLevels[i]);
            }
          }
       }
     
      if(s8AbsRssiValue<46)
      {
        if(bStateflag)
        {
         LCDMessage(LINE1_START_ADDR, "Hide  you found me > - <"); 
        }
        else
        {
         LCDMessage(LINE1_START_ADDR, "Seeker    found you *0*"); 
        }
      }
      else
      {
        bCompleted=TRUE;
        if(!bStateflag)
        {
          LCDMessage(LINE1_START_ADDR, "Seeker                  "); 
        }
      }
    }/* end if(G_eAntApiCurrentMessageClass == ANT_DATA) */
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {    
     UserApp_u32TickMsgCount++;
     if(bStateflag)
     {
        AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8MasterTestMessage);

        if(bCompleted)
        {
         
          LCDCommand(LCD_CLEAR_CMD);
          LCDMessage(LINE1_START_ADDR,au8LcdHiderShow); 
          LCDMessage(LINE2_START_ADDR,au8LcdMasterShow);
          u810Counter=0;     
       }
     } 
      else
      {
       if(bShow)
       {
        AntQueueBroadcastMessage(ANT_CHANNEL_USERAPP, au8TestMessage);
        LCDCommand(LCD_CLEAR_CMD);
        LCDMessage(LINE1_START_ADDR,au8LcdSeekerShow); 
        u8TenCounter=*pau8LcdSeekerShow-0x30;
        u8TenCounter--;
        *pau8LcdSeekerShow=u8TenCounter+0x30;
        
          if(u8TenCounter == 0)
          {
            bShow=FALSE;
            LCDCommand(LCD_CLEAR_CMD);
            LCDMessage(LINE1_START_ADDR,au8LcdSeekerShow);
            LCDMessage(LINE2_START_ADDR,au8LcdTipShow );
          }
        }
        if(WasButtonPressed(BUTTON2))
        {
          ButtonAcknowledge(BUTTON2);
          
          PWMAudioSetFrequency(BUZZER1, 500) ;
          bLedShow=TRUE;
          bLcdSlaveShow=TRUE;
           
          u8BuzzerCounter++;
          if(u8BuzzerCounter == 1000)
          {
            u8BuzzerCounter=0;
            PWMAudioOff(BUZZER1);
          }
        }
       if(bLcdSlaveShow)
       {
         u8BuzzerCounter++;
          if(u8BuzzerCounter == 10)
          {
            u8BuzzerCounter=0;
            PWMAudioOff(BUZZER1);
          }
        
          LCDMessage(LINE2_START_ADDR,au8LcdSlaveShow );
       }
        
       
        
      }
                    

      

    } /* end else if(G_eAntApiCurrentMessageClass == ANT_TICK) */
    

  } /* end AntReadAppMessageBuffer() */
  
    
  if(WasButtonPressed(BUTTON3))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON3);
    bCompleted=FALSE;
    /* Queue close channel, initialize the u8LastState variable and change LED to blinking green to indicate channel is closing */
    AntCloseChannelNumber(ANT_CHANNEL_USERAPP);
   // u8LastState = 0xff;
          
    /* Set timer and advance states */
    UserApp_u32Timeout = G_u32SystemTime1ms;
    for(u8 u8k=0;u8k < LEVE_NUBER;u8k++)
    {
      LedOff(aeLedDisplayLevels[u8k]);
    }
 
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE2_START_ADDR, au8SwitchSlaveButton); 
    LCDMessage(LINE1_START_ADDR, au8SwitchSlaveTip); 
    UserApp1_StateMachine = UserAppSM_WaitChannelClose  ;
  } /* end if(WasButtonPressed(BUTTON3)) */
}

static void UserAppSM_WaitChannelOpen(void)
{
  /* Monitor the channel status to check if channel is opened */
  if(AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_OPEN)
  {
    
    UserApp1_StateMachine = UserAppSM_ChannelOpen;
  }

  /* Check for timeout */
  if( IsTimeUp(&UserApp_u32Timeout, TIMEOUT_VALUE) )
  {
    AntCloseChannelNumber(ANT_CHANNEL_USERAPP);
    LedOff(GREEN);
    LedOn(YELLOW);
    UserApp1_StateMachine = UserApp1SM_Idle;
  }

} /* end UserAppSM_WaitChannelOpen() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ??? */
static void UserApp1SM_Idle(void)
{
                   
  /* Look for BUTTON 0 to open channel */
  if(WasButtonPressed(BUTTON0))
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);
    //LCDCommand(LCD_CLEAR_CMD);
    /* Queue open channel and change LED0 from yellow to blinking green to indicate channel is opening */
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP);

    /* Set timer and advance states */
    UserApp_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserAppSM_WaitChannelOpen;
  }
} /* end UserApp1SM_Idle() */
    

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/