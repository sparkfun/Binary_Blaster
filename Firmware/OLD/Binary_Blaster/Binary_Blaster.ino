/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */

int dig1 = A4;
int dig2 = A5;
int sequence_value[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
 int sequence_order[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14}; 


int button_bit0 = A3;
int button_bit1 = A2;
int button_bit2 = A1;
int button_bit3 = A0;

// dig1 HIGH is D7
// dig2 HIGH side is PB7
// write other pins low to turn on segments

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(9600);
  
  DDRB = 0b11111111; // all outputs
  PORTB = 0b00000000; // all low
  
  pinMode(button_bit0, INPUT);
  pinMode(button_bit1, INPUT);
  pinMode(button_bit2, INPUT);
  pinMode(button_bit3, INPUT);
  
  digitalWrite(button_bit0, HIGH);
  digitalWrite(button_bit1, HIGH);
  digitalWrite(button_bit2, HIGH);
  digitalWrite(button_bit3, HIGH);  
  
  pinMode(dig1, OUTPUT);
  pinMode(dig2, OUTPUT);  
    
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
  
//  Serial.println(read_buttons());
//  delay(250);
  
  randomize_sequence();
  print_sequence();
  delay(2000);
    
    
  
  
//  display_dig1_2(1,2);
//  delay(500);               // wait for a second
//  display_dig1_2(3,4);
//  delay(500);               // wait for a second
//    display_dig1_2(5,6);
//  delay(500);               // wait for a second
//    display_dig1_2(7,8);
//  delay(500);               // wait for a second
//   display_dig1_2(0,9);
//  delay(500);               // wait for a second
  
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
}

void display_dig1_2(int val1, int val2){
  set_digit(val1);
  digitalWrite(dig1, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(dig2, LOW);    // turn the LED off by making the voltage LOW
  delay(50);               // wait for a second
  set_digit(val2);
  digitalWrite(dig1, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(dig2, HIGH);    // turn the LED off by making the voltage LOW
  delay(50);               // wait for a second
}

void randomize_sequence(){
 for(int i=0;i<15;i++){
//   Serial.print("randomizing sequence array pos:");
//   Serial.println(i);
    sequence_order[i] = random(0,15);
//    print_sequence();
    while(check_against_previous(i)) sequence_order[i] = random(0,15);
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
  Serial.print(sequence_order[i]);
  Serial.print(", ");
 } 
 Serial.println();
}

boolean check_against_previous(int i){
  boolean result = false;
  int j=i;
 while(j){
//   Serial.print("comparing:");
//   Serial.print(sequence_order[i]);
//   Serial.print(" and ");
//   Serial.println(sequence_order[i-j]); 
   
  if(sequence_order[i] == sequence_order[i-j]) {
//   Serial.println("match found"); 
    result = true;
    break;
  }
//  Serial.println("clear"); 
  j-=1; 
 }
 
 return result;
}
