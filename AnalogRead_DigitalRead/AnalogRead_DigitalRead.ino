#include <Arduino_FreeRTOS.h>
#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).
#include <LiquidCrystal.h>
const int rs = 10, en = 9, d4 = 7, d5 = 6, d6 = 5, d7 = 4;

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only only one Task is accessing this resource at any time.
SemaphoreHandle_t xSerialSemaphore;

// define two Tasks for DigitalRead & AnalogRead
void TaskAnalogRead_0( void *pvParameters );
void TaskAnalogRead_1( void *pvParameters );
//void lcd_display( void *pvParameters );


// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 9600 bits per second:
  pinMode(8, OUTPUT);
  
  analogReference(INTERNAL1V1); //in the setup() function
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Semaphores are useful to stop a Task proceeding, where it should be paused to wait,
  // because it is sharing a resource, such as the Serial port.
  // Semaphores should only be used whilst the scheduler is running, but we can set it up here.
  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  // Now set up two Tasks to run independently.
  xTaskCreate(
    TaskAnalogRead_0
    ,  "AnalogRead_0"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
  
  xTaskCreate(
    TaskAnalogRead_1
    ,  "AnalogRead_1"
    ,  512  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the Task scheduler, which takes over control of scheduling individual Tasks, is automatically started.
}

void loop()
{

}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/


void TaskAnalogRead_0( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  for (;;)
  {
    // read the input on analog pin 0:
    //int sensorValue = analogRead(A0);
    // See if we can obtain or "Take" the Serial Semaphore.
    
   // digitalWrite(8, LOW);  
    //delay(100);
    digitalWrite(8, LOW);
    int val = analogRead(A0);
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the value you read:
      float tmp = 100*( val/1024.0)*1.1;
      Serial.print("TEMPRATURE = ");
      Serial.print(tmp);  
      Serial.print("*C");
      Serial.println();
     // digitalWrite(9, HIGH);
      LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
      lcd.begin(16,2);
      lcd.print("temp=");
      lcd.print(tmp);
 
    
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }

    vTaskDelay(20);  // one tick delay (15ms) in between reads for stability
  }
}


void TaskAnalogRead_1( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  for (;;)
  {
    // read the input on analog pin 0:
    //int sensorValue = analogRead(A0);
    
  //  digitalWrite(8, HIGH);
    //delay(100);
    digitalWrite(8, HIGH);
    int val2 = analogRead(A0);
    // See if we can obtain or "Take" the Serial Semaphore.
    // If the semaphore is not available, wait 5 ticks of the Scheduler to see if it becomes free.
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource.
      // We want to have the Serial Port for us alone, as it takes some time to print,
      // so we don't want it getting stolen during the middle of a conversion.
      // print out the value you read:
      float lum = ( (float)(val2)/1024.0)*1.1;
     
      
      Serial.print("LUM = ");
      Serial.print(lum);  
      Serial.print("*V");
      Serial.println();
      //digitalWrite(9, HIGH);
      LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
      lcd.begin(16,2);
      lcd.print("lum=");
      lcd.print(lum);
      
     
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }

    vTaskDelay(20);  // one tick delay (15ms) in between reads for stability
  }
}
