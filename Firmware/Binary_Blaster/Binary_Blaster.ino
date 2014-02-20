/******************************************************************************
Binary_Blaster.ino
Firmware for the Binary Blaster PTH Soldering Kit
Pete Lewis @ SparkFun Electronics
February 13th 2014
https://github.com/sparkfun/Binary_Blaster

This code is meant to be used with SFE's Binary Blaster thru-hole soldering kit. 
It can be found here: https://www.sparkfun.com/products/12037
It is an educational kit that is meant to teach thru-hole soldering, 
and also challenge the player to practice binary conversion. The default playing
mode is to display the values in decimal. If you'd like to play hexadecimal 
mode, then you can do so by holding down the "bit0" button while powering up.

Resources:
No additional library requirements

Development environment specifics:
Arduino 1.0.5
Hardware: v10*
*Note, PB0 (aka arduino pin "D8") has been removed from the IC, to make it a
keyed part. This helps beginners avoid incorrect placement of the IC.

This code is beerware; if you see me (or any other SparkFun employee) at the
local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

//-----------------------------------------------------------------------------
// HARDWARE PIN DECLARATIONS

// Define the anode (aka "+" or "high side") pins on the 7-segment displays
  // There are two 7-segment displays on this game.
  // We control each display by both writing the anode digit pin HIGH,
  // and writing the desired single segment cathode pin LOW. Only one segment 
  // may be actually lit at one instance. We blink segments in a fast 
  // sequence, and then it appears to illuminate multiple segments 
  // simultaneously. 
  // ***Note, the cathodes for the 7 segments are all connected
  // to the pins on PORTB. These are controlled by writing directly to the
  // entire port (e. g. "PORTB = ~0b00001100;") in function set_7segment_pins().
const int digit_1_anode = A5;
const int digit_2_anode = A4;

// Define pins connected to the switches inside the tactile LED buttons
  // Note, the tactile buttons have switches and LEDs inside them.
  // These pin definitions are for the switches. They will be set as inputs,
  // and used to know if the player is pressing the button down.
  // Each button corresponds to a bit, and therefore has the "bit0" or
  // "bit1" in it's name. This helps keep the player's input more
  // understandable. This pins are read in the function, read_buttons().
const int button_input_pin_bit0 = A3;
const int button_input_pin_bit1 = A2;
const int button_input_pin_bit2 = A1;
const int button_input_pin_bit3 = A0;

// Define pins connected to the LEDs inside the tactile LED buttons
  // Note, These pin definitions are for the LEDs, not the switches.
  // These are used to control the LEDs. If the user presses the correct
  // binary equivalent, then they will light up to show which bits have been
  // set. Turning an LED on means that bit is set. Turning an LED off means
  // that bit is cleared. When the game is waiting for the player to "blast
  // in" the correct bits, all bits are set as cleared.
  // Each LED corresponds to a bit, and therefore has the "bit0" or
  // "bit1" in it's name.
  // These pins are controlled in functions, display_leds() and
  // display_zoom().
const int led_pin_bit0 = 5;
const int led_pin_bit1 = 4;
const int led_pin_bit2 = 3;
const int led_pin_bit3 = 2;

//-----------------------------------------------------------------------------
// GAMEPLAY VARIABLE DECLARATIONS

// Define an array to store the sequence of values the player must repeat
  // This will include 16 possibilities, although we will not use "0",
  // because "0" is hard for the player to input. All other values will 
  // require the player to set bits (aka press the buttons) to create
  // the correct binary 4 bit equivalent.
  // This array will be filled in each time we call shuffle_sequence()
int sequence[16];

// Define a variable to store a time stamp of when we start each game
  // This will be used to calculate the total time it takes the player
  // to complete all 15 possibilities (this is how they win the game). 
  // Note, there is a timeout feature that will cause the player to loose
  // if they do not input the correct buttons within the timeout time.
  // This also resets the game and will require the player to do all 15 again
  // before they can win.
  // At the end of the game, it will display the total time that it took the
  // player to complete all 15. This is their "score" for that completion.
  // This way, the player can know how fast they were able to convert, 
  // and practice to decrease their total time.
long start_timestamp;

// Define the amount of time it takes to timeout during game-play
  // Each time a new value is displayed on the 7-segment displays, the player
  // only has a set amount of time to press the correct buttons. If they think
  // or wait too long, then the game will timeout and reset. This makes the 
  // game a bit more challenging and requires that the player really know
  // their conversions. By default is is set to 1000 milliseconds 
  // (aka 1 second). You can decrease this number to make it more difficult,
  // or make it longer if you'd like to have more time to practice.
  // This variable is used in function, listen_for_value().
#define TIMEOUT_MILISECONDS 1000

// Define a variable to display in hexadecimal or decimal
  // If this is set to true, then it will display in hex.
  // If false, then it will display in decimal.
  // This is checked in the function, display_7seg(), which ultimately changes
  // the display segments in set_7segment_pins().
  // Default is false, if you hold down bit "0" right after powering on, it 
  // will change to hex mode.
boolean Hex_mode = false;

void setup()
{
  // Begin serial for debug readout. Also used to see more precise timestamps
    // aka the players "score" when they have completed the game.
  Serial.begin(9600);
  
  // Button switches (INPUTS with pullups)
  pinMode(button_input_pin_bit0, INPUT_PULLUP);
  pinMode(button_input_pin_bit1, INPUT_PULLUP);
  pinMode(button_input_pin_bit2, INPUT_PULLUP);
  pinMode(button_input_pin_bit3, INPUT_PULLUP);
  
  // Button LEDs (OUTPUTS and also write them LOW aka "off")
  pinMode(led_pin_bit0, OUTPUT);
  pinMode(led_pin_bit1, OUTPUT);
  pinMode(led_pin_bit2, OUTPUT);
  pinMode(led_pin_bit3, OUTPUT);
  digitalWrite(led_pin_bit0, LOW);
  digitalWrite(led_pin_bit1, LOW);  
  digitalWrite(led_pin_bit2, LOW);
  digitalWrite(led_pin_bit3, LOW);

  // Set the anode pins of each 7 segment display to OUTPUTs
    // This will be used to turn on each display individually.
    // Each of these pins is connected to the anode of all 7 segments
    // for its corresponding display. Right them HIGH to turn them on,
    // Right them LOW to turn them off.
  pinMode(digit_1_anode, OUTPUT); // connected to "+" of digit 1 (left)
  pinMode(digit_2_anode, OUTPUT); // connected to "+" of digit 2 (right)
  
  // Set all of the segments control pins to OUTPUT (set to "1")
    // Pins 1-7 on PORT B are used to control the segments on both displays.
    // I can set all of them to outputs with the following single line of code,
    // by setting the register "DDRB" to the desired binary value.
    // *Note, PB0 (aka arduino pin "D8" has been removed from the IC, to make it a
    // keyed part. This helps beginners avoid incorrect placement of the IC.
  DDRB = 0b11111111;
  
  // Set all of the segments control pins to LOW (set to "0")
    // Pins 1-7 on PORT B are used to control the segments on both displays.
    // I can set all of them to outputs with the following single line of code:
  PORTB = 0b00000000; // all low (turn them all off)
  
  // Check for hex or decimal mode
    // If you power up with no buttons pressed, it will default to "Decimal mode"
    // In decimal, the sequence[] array values will appear on the display as 
    // decimal values like so: 
    // "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15".
    // If you power up with the "bit0" button pressed, then it will play in 
    // "Hex mode". In this mode, the numbers in the sequence[] array will 
    // appear on the display as hex values like so:
    // "1","2","3","4","5","6","7","8","9","A","b","C","d","E","F".
    // It will also display all of the possible hex values in hex_mode_entered()
    // to show that mode has been entered.
  if(digitalRead(button_input_pin_bit0) == LOW) 
  {
    Hex_mode = true;
    hex_mode_entered();
  }
  else Hex_mode = false;
  
  // Test out the button LEDs, displays and buzzer
    // This is helpful when you first power up. It can help you verify that the
    // parts have been placed and soldered properly.
  display_roundabout();  // Blinks the segments of each display a bit.
  display_zoom(); // Blinks the button LEDs 
  buzz_blast(300); // Buzzes the buzzer in a "blast off" sort of sound.
}

// the loop routine runs over and over again forever:
void loop()
{
  // Wait for the player to press any button in order to start a new game
    // Enter a while loop that will remain true until they press a button
    // display_roundabout_waiting() will return true, until a button is pressed.
    // When a button is pressed, it will return false. This will cause the 
    // while loop to no longer be true, and thus allow us to continue on to 
    // the start of the game (the rest of the main loop)
  while(display_roundabout_waiting());
  
  // Shuffle the order of the values in the sequence[] array.
    // This makes the game more challenging, because each time the player starts
    // a new game, they will get the 15 possibilities in a different order.
  shuffle_sequence();
  
  // Display a blinks on the button LEDs to indicate the new game start.
  display_zoom();
  
  // Take a time stamp to remember when they started the active game.
    // This will be used later to calculate their total time (aka "score")
  start_timestamp = micros();
  
  // Show the time stamp in a serial terminal window. Mostly for debugging.
  Serial.print("start_timestamp:");
  Serial.println(start_timestamp);
  
  // Show the player each value in the sequence[] array (one at a time).
    // Use a "for" loop to show each value, and listen to player input
    // As they correctly input each value, increment to the next spot in 
    // the sequence[] array.
  for(int i = 0 ; i < 16 ; i++)
  {
    // Check to see if the player has made it to the last spot in the sequence[]
      // array - which is 15. If they have, then call the function,
      // display_winner() and break out of this for loop.
    if(i == 15)
    {
      display_winner(); // player made it through all 15 - they win!
      break;
    }
    
    // Display a roundabout blink on the display, to give the player a small
      // amount of time to get ready for the next value to appear.
    display_roundabout();
    
    // Display the current value that the player must input.
      // We use the index in this for loop, "i", to call from the 
      // sequence[] array
    display_7seg(sequence[i]);
    
    // Wait for the player to input the correct value.
      // Note, most of this is done in the function, listen_for_value().
      // Check that out in the lower portion of this code for more info.
      // We are waiting for the correct value. If the player inputs the correct
      // value, then listen_for_value() will return true, and the following if
      // statement contents will execute. If they timeout, then 
      // listen_for_value() will return false, and we will continue on to 
      // show loser and break out of this for loop.
    if(listen_for_value(sequence[i])) 
    {
      // If we get here, it means the player has pressed the correct input
      // Now, let's show the correct value on button LEDs.
      display_leds(read_buttons());
      
      // Buzz a "blast" tone on the buzzer. Use the index "i" to increment
        // the start note higher each time we climb up to 15. This causes the 
        // tone to get higher and higher in pitch each time, adding an element 
        // of excitement and urgency as they get closer to winning.
      int start_note = 300;
      start_note += i*40;
      buzz_blast(start_note);
      
      // Clear all of the button LEDs
      display_leds(0);
    }
    else // If we get here, the player has timed out, and thus lost the game.
    {
      display_loser();
      
      // Show the user what they should have pressed.
      // This is designed to help a beginner learn the conversions, and 
      // hopefully enables a true binary newbie to learn it by simply watching 
      // and repeating.
      display_correct_conversion(sequence[i]);
      
      break; 
    }  
  }  
}

// Define a function to listen for a value to be pressed on the buttons
  // Send this function a value, and it will return "true" as soon as the 
  // player presses the correct value. If the player does not press the
  // desired value on the buttons, then this function will eventually
  // timeout and return a "false". Also note, the player can actually input
  // any wrong combination of buttons, and it will not do anything.
  // The only way a player can lose, is by timing out.
boolean listen_for_value(int correct_value)
{
  int timeout = TIMEOUT_MILISECONDS;
  while(timeout)
  {
    display_7seg(correct_value);
    if(read_buttons() == correct_value)
    {
      return true;
    } 
    timeout -=1;
  }
  return false; // if we get here, the player has timed out
}

// Define a function to set the 7 individual segment pins
  // Send this function a decimal value, and it will set the 7 pins
  // connected to the segments to look like a "1", "2", and so on.
  // *Note, these segment control pins are bussed in the hardware to
  // both displays (aka digits). In order to properly control each display,
  // you must also set the desired anode pin control (digit_1_anode or
  // digit_2_anode).
  // **Note, you can send this function some special decimal values
  // above 100 (see below) to have more precise control. This method is used
  // in the function blink_segment() and display_roundabout() for fun blinking
  // during game-play and while waiting to start a new game.
  // ***Note, if you send it 10-15, it will display in HEX
void set_7segment_pins(int decimal_val)
{
  // The following if statements set the entire port to control all 
    // 7 control pins in one line of code. The letter above each binary
    // value represent each segment, and correspond to the letters defined
    // in the data sheet for the display. See the entire data sheet here:
    // https://www.sparkfun.com/datasheets/Components/YSD-160AR4B-8.pdf
                                //GFEDCBA-
  if(decimal_val == 1) PORTB = ~0b00001100;
  if(decimal_val == 2) PORTB = ~0b10110110;
  if(decimal_val == 3) PORTB = ~0b10011110;
  if(decimal_val == 4) PORTB = ~0b11001100;
  if(decimal_val == 5) PORTB = ~0b11011010;
  if(decimal_val == 6) PORTB = ~0b11111010;
  if(decimal_val == 7) PORTB = ~0b00001110;
  if(decimal_val == 8) PORTB = ~0b11111110;
  if(decimal_val == 9) PORTB = ~0b11011110;
  if(decimal_val == 0) PORTB = ~0b01111110;
  // The next if statements are for displaying HEX values
  if(decimal_val == 10) PORTB = ~0b11101110; // "A"
  if(decimal_val == 11) PORTB = ~0b11111000; // "b"
  if(decimal_val == 12) PORTB = ~0b01110010; // "C"
  if(decimal_val == 13) PORTB = ~0b10111100; // "d"
  if(decimal_val == 14) PORTB = ~0b11110010; // "E"
  if(decimal_val == 15) PORTB = ~0b11100010; // "F"
  // Special decimal values used to light up individual segments
                                  //GFEDCBA-
                                  //7654321-
  if(decimal_val == 101) PORTB = ~0b00000010;
  if(decimal_val == 102) PORTB = ~0b00000100;
  if(decimal_val == 103) PORTB = ~0b00001000; 
  if(decimal_val == 104) PORTB = ~0b00010000; 
  if(decimal_val == 105) PORTB = ~0b00100000; 
  if(decimal_val == 106) PORTB = ~0b01000000; 
  if(decimal_val == 107) PORTB = ~0b10000000;  
}

// Define a function to control both displays (both the left and right digits)
  // Send this function two integers. They will be blinked either 6 or 3 ms.
  // If you don't send a digit 2 value, then it will default to "-1" and this
  // will cause the function to only display a value on digit 1.
  // d1 is the right digit. It is controlled by digit_1_anode.
  // d2 is the left digit. It is controlled by digit_2_anode.
void display_dig1_2(int d1, int d2=-1)
{
  // Define a delay time variable. this helps keep the timing the same, whether
  // you want to display one or two digits.
  // Default to 6 (milliseconds)
  int delay_time = 6; 
  
  // Cut it in half (to 3 milliseconds) if you're displaying 2 digits.
  if(d2 != -1) delay_time = 3;
  
  // Always at least display digit 1
  set_7segment_pins(d1);
  digitalWrite(digit_1_anode, HIGH);
  digitalWrite(digit_2_anode, LOW);
  delay(delay_time);
  
  // Only if there was a non-default value sent, display digit 2
  if(d2 != -1)
  {
    set_7segment_pins(d2);
    digitalWrite(digit_1_anode, LOW);   
    digitalWrite(digit_2_anode, HIGH);  
    delay(delay_time);               
  }
  
  // Turn both digits off
  digitalWrite(digit_1_anode, LOW);  
  digitalWrite(digit_2_anode, LOW);  
}

// Define a function to display any value 1-99 on the two displays
  // Send this function a value from 1- 99, and it will blink (very briefly)
  // the value on the displays. It will use one or two displays as necessary.
  // Note, this is a very quick blink of a value, in order to actually see the 
  // segments light up for a substantial time, you must loop this function.
  // Note, because we can only control each display one-at-a-time, we must
  // separate digit 1 and 2 from any values greater than 9.
  // This function also checks for Hex_mode. To display in hexadecimal, this
  // function can simply send values greater than 9 as a single digit value to
  // display_dig_2(). set_7segment_pins() will take care of the rest.
void display_7seg(int value)
{
  if((value < 10) | Hex_mode) display_dig1_2(value);
  else 
  {
    // Define two separate variables to store digit 1 and digit 2.
    int dig1 = 0;
    int dig2 = value;
    
    // Continue to subtract 10 from the original value, until you reach below 10.
    // Each time we subtract 10, increment digit 1.
    while(dig2 >= 10) 
    {
      dig2 -= 10;
      dig1 += 1;
    }
    
    // Send our refined digit values to display_dig1_2() function.
    display_dig1_2(dig2, dig1); 
  }
}

// Define a function to randomly shuffle the 15 options in the sequence[] array
  // This function will take the sequence[] array and then randomize the order.
  // It's a bit more tricky then I first thought, so I'll try to break it down
  // as best I can. The key to this function working properly is the scooting
  // that happens at the second for loop. It enables us to keep track of the 
  // remaining available options, and ultimately avoids any doubling of options
  // in the final shuffled array.
void shuffle_sequence()
{
  // Start with all 15 options available
  int sequence_options[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  
  // Using a for loop, choose a random position (aka value) from 
  // sequence_options[] array
  for(int i = 0 ; i < 15 ; i++)
  {
    // Create a random number between 0 and "15-i".
    // The "15-i" is the tricky part of this shuffle function. It is necessary
    // because we are going to scoot the remaining options to the left each time
    // we pull a new random value from sequence_options[] array.
    int random_choice = random(0,15-i);
    
    // Set the current sequence spot (call with our index, "i", to a randomly 
    // chosen option
    sequence[i] = sequence_options[random_choice]; 
  
    // Scoot the remaining options over to the left
    // This is so we can pick from the beginning of the array next time we loop
    // around in this for loop.
    // Starting at "random_choice" position, move everything else over one spot.
    for(int j = random_choice ; j < 15 ; j++)
    {          
    // Grab the option next door (j+1), and scoot it over one spot (j).
    // Do this until you've reached the end of the array (j will be 14)    
    sequence_options[j] = sequence_options[j+1];   
    }                                                
  }
}

// Define a function to read the four buttons for player input
  // This function will return an integer, "input_var", that the player is
  // currently pressing the binary equivalent on the buttons.
  // It reads each button, and sets the corresponding bit within "input_var".
int read_buttons(){
  int input_var = 0;
  if(!digitalRead(button_input_pin_bit0)) input_var |= (1<<0);
  if(!digitalRead(button_input_pin_bit1)) input_var |= (1<<1);
  if(!digitalRead(button_input_pin_bit2)) input_var |= (1<<2);
  if(!digitalRead(button_input_pin_bit3)) input_var |= (1<<3);
  return input_var;
}

// Define a function to flash a single button LED
  // This is used in the blink_zoom() function.
  // Send this an integer of the LED you'd like to blink.
  // It will send that LED high for 50 milliseconds, and then turn it off.
  // Note, this will only allow you to turn on one LED at a time. If you'd like
  // to turn on multiple LEDs at the same time, then use display_leds()
void flash_button_led(int led)
{
  // Check "led" variable against the bit number,
  // Then write the corresponding led_pin_bit HIGH.
  if(led == 0) digitalWrite(led_pin_bit0, HIGH);
  if(led == 1) digitalWrite(led_pin_bit1, HIGH);  
  if(led == 2) digitalWrite(led_pin_bit2, HIGH);
  if(led == 3) digitalWrite(led_pin_bit3, HIGH);
  
  delay(50);
  
  // Turn off all 4 LEDs
  digitalWrite(led_pin_bit0, LOW);
  digitalWrite(led_pin_bit1, LOW);  
  digitalWrite(led_pin_bit2, LOW);
  digitalWrite(led_pin_bit3, LOW);
}

// Define a function to blink one single display
  // Send this the specific segment combo and the desired display (either 
  // digit 1 or 1), and this function will blink it for 50 milliseconds. 
  // This is used in display_roundabout() and display_roundabout_waiting().
void blink_segment(int seg, int digit)
{
  set_7segment_pins(seg);
  if(digit == 1) 
  {
    digitalWrite(digit_1_anode, HIGH);
    digitalWrite(digit_2_anode, LOW);
  }
  if(digit == 2) 
  {
    digitalWrite(digit_1_anode, LOW);
    digitalWrite(digit_2_anode, HIGH);
  }
  delay(50);
  digitalWrite(digit_1_anode, LOW);
  digitalWrite(digit_2_anode, LOW);
}

// Define a function to blink segments in a sequence that sends the "blink"
  // around the outer perimeter segments of both digits. This looks like the 
  // "blink" is going around in circles. It uses the special values (above 100)
  // feature of blink_segment(). It's used after the player inputs a correct
  // combo of buttons during game-play.
void display_roundabout()
{
  blink_segment(101,1);
  blink_segment(102,1);
  blink_segment(103,1);
  blink_segment(104,1);
  blink_segment(104,2);
  blink_segment(105,2);
  blink_segment(106,2);
  blink_segment(101,2);
}

// Define a function that blinks the button LEDs from left to right and back
  // This kind of looks like the "blink" is zooming along the buttons.
  // It is used at various places during game-play to add some fun blinks.
void display_zoom()
{
  flash_button_led(0);
  flash_button_led(1);
  flash_button_led(2);
  flash_button_led(3);
  flash_button_led(2);
  flash_button_led(1);
  flash_button_led(0); 
}

// Define a function to control the 4 button LEDs
  // Send this function an integer value, and it will display the binary 
  // equivalent on the button LEDs. In order to do this, it can turn on
  // multiple LEDs at the same time. Also note, send this a "0" to turn off
  // all LEDs. This function is called after the player inputs the correct
  // button combo, to indicate they are correct, and to allow the player to 
  // see the correct binary equivalent for a moment before the next value is
  // displayed.
void display_leds(int value)
{
  if(value == 0)
  {
    digitalWrite(led_pin_bit0, LOW);
    digitalWrite(led_pin_bit1, LOW);  
    digitalWrite(led_pin_bit2, LOW);
    digitalWrite(led_pin_bit3, LOW);
  } 
  else 
  {
    // Check to see if we need to set each bit separately, and then write that
    // specific bit pin HIGH. Use bit masking to find out if the value
    // does have that specific bit set.
    if(value & 0b00000001) digitalWrite(led_pin_bit0, HIGH);
    if(value & 0b00000010) digitalWrite(led_pin_bit1, HIGH);
    if(value & 0b00000100) digitalWrite(led_pin_bit2, HIGH);
    if(value & 0b00001000) digitalWrite(led_pin_bit3, HIGH);
  }
}

// Define a function to use when the player has won!
  // This is called when the user has correctly pressed all 15 value 
  // possibilities in sequence[] array. It take a time stamp and calculates
  // how long it took. This is the players "score". It displays this total time
  // in decimal (and so it needs to temporarily turn off Hex_mode, if 
  // necessary. It also prints the time in microseconds to the terminal,
  // for a more precise score, if the player would like to know it.
void display_winner()
{
  // Take a time stamp to know the players completion time.
  long end_timestamp = micros();
  
  // subtract start from end, to calculate total.
  long total_time_microseconds = end_timestamp - start_timestamp;
  
  // Show the total time in microseconds on the serial terminal window.
  Serial.print("total_time_microseconds:");
  Serial.println(total_time_microseconds);
 
  // Convert microseconds to seconds. This will be used to display the time
  // in seconds on the 7-segment displays.
  int total_time = total_time_microseconds/1000000;
 
  display_zoom();
  display_zoom();
  display_zoom(); 
  
  // Check to see if we are in Hex mode. If so, we will need to temporarily
  // turn that off (and go to default decimal mode) to display the total time
  // to the player on the two 7 segment displays. It will most likely be some
  // value between 20 and 60, so this is much easier to understand in
  // decimal.
  if(Hex_mode) 
  {
  Hex_mode = false;
  for(int i = 0 ; i < 300 ; i++)
  {
    display_7seg(total_time); // show time (score) to user
  }
  Hex_mode = true;
  }
  else
  {
    for(int i = 0 ; i < 300 ; i++)
    {
      display_7seg(total_time); // show time (score) to user
    }
  }
}

// Define a function to buzz the buzzer in a series of tones that sound like a 
  // blast. Send this function a start note, and it will buzz a series of tones
  // that start at the start_note, but then increment down in steps of 10.
  // If you send it a start_note of 860, then it will continue to increment all
  // the way back down to 50. This only happens when you win the game.
void buzz_blast(int start_note)
{
  int end_note = start_note - 150; // Just go down 150 for most blasts.
  if(start_note == 860) end_note = 50; // Take it all the way down at a win!
  for(int i = start_note ; i > end_note ; i -= 10)
  {
    tone(6, i, 25);
    delay(25);
  }
}

// Define a function to use during waiting mode (aka "attract mode")
  // This does the same blinking on the 7 segment displays as 
  // display_roundabout(), but it includes a check to see if a button is 
  // being pressed in between each blink. This makes for a quick response,
  // when the player wants to begin a new game.
  // It will return true while all buttons remain HIGH.
  // The instance the player hits a button, it returns false.
boolean display_roundabout_waiting()
{
  if(!read_buttons()) blink_segment(101,1);
  if(!read_buttons()) blink_segment(102,1);
  if(!read_buttons()) blink_segment(103,1);
  if(!read_buttons()) blink_segment(104,1);
  if(!read_buttons()) blink_segment(104,2);
  if(!read_buttons()) blink_segment(105,2);
  if(!read_buttons()) blink_segment(106,2);
  if(!read_buttons()) blink_segment(101,2);
  if(!read_buttons()) return true;
  else return false;
}

// Define a function to show that the player has timed out and the game is 
  // resetting. This blinks the left two button LEDs, then the right two button
  // LEDs (back and forth), while also making some lower tones on the buzzer.
  // This is similar to the loser sounds and LED pattern on the original Simon
  // Says PTH kit. This function is only called when the player times out,
  // which is the only way you can lose the game.
void display_loser()
{
  display_leds(12);
  buzz_blast(600);
  display_leds(0);
    
  display_leds(3);
  buzz_blast(300);
  display_leds(0);
    
  display_leds(12);
  buzz_blast(600);
  display_leds(0);
    
  display_leds(3);
  buzz_blast(300);
  display_leds(0);
}

// Define a function to display a value on the 7-segment, and show the
  // correct binary equivalent on the buttons for 3 seconds.
  // Send this function a value from 1-15, and it will show the correct
  // buttons for 3 seconds. When the player times out they lose that 
  // round, but this function is used to hopefully teach them what
  // they should have pressed.
void display_correct_conversion(int i)
{
    for(int j = 0 ; j < 500 ; j++) 
    {
      display_7seg(i);
      display_leds(i);
    }
    display_leds(0);
}
 
// Define a function to print the entire sequence to a serial terminal window
  // This was mostly used while debugging the function, shuffle_sequence()
void print_sequence()
{
  for(int i = 0 ; i < 15 ; i++)
  {
    Serial.print(sequence[i]);
    Serial.print(", ");
  } 
  Serial.println();
}

// Define a function to show that hex mode has been entered
  // This is called during setup if bit0 is held down while powering up
  // It increments through A-F to show hex mode is entered.
void hex_mode_entered()
{
  for(int i = 10 ; i < 16 ; i++)
  {
    for(int j = 0 ; j < 50 ; j++) display_7seg(i);
    delay(50);
  }
}

// Define a function to test the button LED control functions
  // Runs a routine where the 7 segment display AND the button LEDs display
  // an incrementing value from 0 to 15. Note, this is not used during game-play 
  // and is not actually called anywhere in the above code. It was used during 
  // testing, and may be useful for future development on this code.
void TESTING_button_LED_functions()
{
  for(int i = 1 ; i < 16 ; i++)
  {
    for(int j = 0 ; j < 200 ; j++) 
    {
      display_7seg(i);
      display_leds(i);
    }
    delay(10);
    display_leds(0);
  }
}
