/*
  Binary Blaster
  Originally written by Pete Lewis October 2013
  SparkFun Electronics
  Beerware Liscence. Feel free to use/tweak this, but if you meet me at a bar, you can buy me a beer.
  
  This code is meant to be used with SFE's Binary Blaster thru-hole soldering kit.
  It is an educational kit that is not only meant to teach thru-hole soldering,
  but it also challanges the player to practice binary conversion.
  
  
 */

int dig1 = A5;
int dig2 = A4;

int sequence[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // start with "0"s, but it will be filled in when we call randomize_sequence()

int button_bit0 = A3;
int button_bit1 = A2;
int button_bit2 = A1;
int button_bit3 = A0;

int button_led_bit0 = 5;
int button_led_bit1 = 4;
int button_led_bit2 = 3;
int button_led_bit3 = 2;

long start = 0; // used to store start time for each new game.

// dig1 HIGH is D7
// dig2 HIGH side is PB7
// write other pins low to turn on segments

void setup() {                
  Serial.begin(9600);
  
  DDRB = 0b11111111; // all outputs (all of the pins on Port B are connected to the 7 segment displays)
  PORTB = 0b00000000; // all low (turn them all off)
  
  // BUTTON SWITCHES (INPUTS)
  pinMode(button_bit0, INPUT);
  pinMode(button_bit1, INPUT);
  pinMode(button_bit2, INPUT);
  pinMode(button_bit3, INPUT);
  
  digitalWrite(button_bit0, HIGH); // internal pullup
  digitalWrite(button_bit1, HIGH);
  digitalWrite(button_bit2, HIGH);
  digitalWrite(button_bit3, HIGH);  
  
  // BUTTON LEDS (OUTPUTS)
  pinMode(button_led_bit0, OUTPUT);
  pinMode(button_led_bit1, OUTPUT);
  pinMode(button_led_bit2, OUTPUT);
  pinMode(button_led_bit3, OUTPUT);
  
  digitalWrite(button_led_bit0, LOW); // turn off
  digitalWrite(button_led_bit1, LOW);  
  digitalWrite(button_led_bit2, LOW);
  digitalWrite(button_led_bit3, LOW);

  pinMode(dig1, OUTPUT); // connected to GND of digit 1 (left)
  pinMode(dig2, OUTPUT); // connected to GND of digit 2 (right)
  
  display_roundabout();  // blinky blinky
  display_zoom(); // zoomy blinky
  buzz_blast(300);
  
//  for testing the segment display functions...  
//  for(int i=0;i<22;i++){
//    for(int j = 0;j<50;j++) display_7seg(i);
//    delay(50);
//  }

//  for testing the button led display functions...  
//  for(int i=1;i<16;i++){
//    for(int j = 0;j<200;j++) {
//      display_7seg(i);
//      display_leds(i);
//    }
//    delay(10);
//    display_leds(0);
//  }


}

// the loop routine runs over and over again forever:
void loop() {
  //wait for user to press a button
  
  //begin game
  
  //choose random sequence of possible decimal values
  
  //display first decimal value
  
  //wait for user to press the correct binary represantation of value with the 4 buttons.
  
  //timeout if they don't press anything
  
  //play loser if they press the inccorect button combo
  
  //continue through all possible decimal values if they keep pressing the correct buttons.
  
  //display total game time
  
  // randomize_sequence();
  
//  
//  delay(250);
  
  while(!read_buttons()) display_roundabout(); // wait for the user to press a button to begin game
  
  randomize_sequence();
  print_sequence();
  delay(1000);
  
  start = micros(); // used to track total game time - the plyaers "score"
  Serial.print("start:");
  Serial.println(start);
  
  for(int i=0;i<16;i++){
  if(i == 15) {
    display_winner(); // player made it through all 15 - they win!
    break;
  }
  display_roundabout();
  display_7seg(sequence[i]);
  if(listen_for_button(i)) {
    display_leds(read_buttons()); // show buttons pressed, just so you can see correct bits illuminated
    int start_note = 300;
    start_note += i*40;
    buzz_blast(start_note);
    delay(250);
    display_leds(0); // clear leds
  }
  else break; // listen_for_button() will return false if the user doesn't press the correct button within timeout
  
  }  
}

boolean listen_for_button(int i){
  int timeout = 1000;
  while(timeout){
    display_7seg(sequence[i]);
    Serial.print(sequence[i]);
    Serial.print(":");
   Serial.println(read_buttons());
   if(sequence[i] == read_buttons()){
    Serial.println("CORRECT!");
    return true;
   } 
   timeout -=1;
//   Serial.println(timeout);
//   delay(100);
  }
  return false;
}

void set_digit(int decimal_val){
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
 
 // special *values* used to light up individual segments
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


void display_dig1_2(int d1, int d2=-1){
  int delay_time = 6;
  if(d2 != -1) delay_time = 3;
  
  set_digit(d1);
  digitalWrite(dig1, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(dig2, LOW);    // turn the LED off by making the voltage LOW
  delay(delay_time);               // wait for a second
  
  if(d2 != -1){
  set_digit(d2);
  digitalWrite(dig1, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(dig2, HIGH);    // turn the LED off by making the voltage LOW
  delay(delay_time);               // wait for a second
  }
  
  digitalWrite(dig1, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(dig2, LOW);    // turn the LED off by making the voltage LOW
}

void display_7seg(int value){
  if(value<10) display_dig1_2(value);
  else 
  {
    int dig1 = 0;
    int dig2 = value;
    while(dig2 >= 10) 
    {
      dig2 -= 10;
      dig1 += 1;
    }
    display_dig1_2(dig2, dig1); 
  }
}

void randomize_sequence(){
  
 int sequence_options[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; // start with all options avaialable (left aligned) 
  
 for(int i=0;i<15;i++){
  int random_choice = random(0,15-i); // this choice will be used as an index to pull from sequence_options[] array.
                                      // it will always be pulling from 0 (left aligned) to some remaining amound (15-i)
//  Serial.print("i=");
//  Serial.println(i);
//  Serial.print("random_choice=");
//  Serial.println(random_choice);  
  
  sequence[i] = sequence_options[random_choice]; // set the current sequence spot to a randomly chosen option
  
// scoot the remaining options over to the left
// (so we can pick from the beginning of the array next time around)
  for(int j=random_choice;j<15;j++){               // starting at "random_choice" position, move everything else over one spot.
    sequence_options[j] = sequence_options[j+1];   // grab the option next door (j+1), and scoot it over one.
  }                                                // Do this until you've reached the end of the array (j will be 14)

//  Serial.println("options:");
//  for(int i = 0 ; i < 15 ; i++){
//  Serial.print(sequence_options[i]);
//  Serial.print(", ");
//  } 
//  Serial.println();
//  delay(250);
//  
//  Serial.println("sequence:");
//  print_sequence();
//  delay(250);
 }
}

int read_buttons(){
  int button_input = 0;
  if(!digitalRead(button_bit0)) button_input |= (1<<0);
  if(!digitalRead(button_bit1)) button_input |= (1<<1);
  if(!digitalRead(button_bit2)) button_input |= (1<<2);
  if(!digitalRead(button_bit3)) button_input |= (1<<3);
  return button_input;
}

void print_sequence(){
 for(int i = 0 ; i < 15 ; i++){
  Serial.print(sequence[i]);
  Serial.print(", ");
 } 
 Serial.println();
}

void flash_button_led(int led){
  if(led == 0) digitalWrite(button_led_bit0, HIGH); // turn on
  if(led == 1) digitalWrite(button_led_bit1, HIGH);  
  if(led == 2) digitalWrite(button_led_bit2, HIGH);
  if(led == 3) digitalWrite(button_led_bit3, HIGH);
  delay(50);
  digitalWrite(button_led_bit0, LOW); // turn off
  digitalWrite(button_led_bit1, LOW);  
  digitalWrite(button_led_bit2, LOW);
  digitalWrite(button_led_bit3, LOW);
  //delay(50);
}

void blink_segment(int seg, int digit){
  set_digit(seg);
  if(digit == 1) {
    digitalWrite(dig1, HIGH);
    digitalWrite(dig2, LOW);
  }
  if(digit == 2) {
    digitalWrite(dig1, LOW);
    digitalWrite(dig2, HIGH);
  }
  delay(50);
  digitalWrite(dig1, LOW);
  digitalWrite(dig2, LOW);
  
}

void display_roundabout(){
  blink_segment(101,1);
  blink_segment(102,1);
  blink_segment(103,1);
  blink_segment(104,1);
  blink_segment(104,2);
  blink_segment(105,2);
  blink_segment(106,2);
  blink_segment(101,2);
}

void display_zoom(){
 flash_button_led(0);
 flash_button_led(1);
 flash_button_led(2);
 flash_button_led(3);

 flash_button_led(2);
 flash_button_led(1);
 flash_button_led(0); 
 
}

void display_leds(int value){
 if(value == 0){
  digitalWrite(button_led_bit0, LOW); // turn off
  digitalWrite(button_led_bit1, LOW);  
  digitalWrite(button_led_bit2, LOW);
  digitalWrite(button_led_bit3, LOW);
 } 
 else {   
   if(value & 0b00000001) digitalWrite(button_led_bit0, HIGH);
   if(value & 0b00000010) digitalWrite(button_led_bit1, HIGH);
   if(value & 0b00000100) digitalWrite(button_led_bit2, HIGH);
   if(value & 0b00001000) digitalWrite(button_led_bit3, HIGH);
 }
}

void display_winner(){
 long end_time = micros();
 
 long total_time_microseconds = end_time - start;
 int total_time = total_time_microseconds/1000000; // convert microseconds to seconds
 Serial.print("total_time_microseconds:");
 Serial.println(total_time_microseconds);
 display_zoom();
 display_zoom();
 display_zoom(); 
 for(int i=0;i<300;i++){
 display_7seg(total_time); // show time (score) to user
 }
 
}

void buzz_blast(int start_note){
  int end_note = start_note-150;
  if(start_note == 860) end_note = 50; // winner sound
  for(int i=start_note;i>end_note;i-=10){
  tone(6, i, 25);
  delay(25);
  //noTone(6);
  }
}
