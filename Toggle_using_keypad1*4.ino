int ledPins[] = {7, 8, 9};        // Led
bool ledState[] = {0, 0, 0};      // Initial OFF kiya 

int buttonPins[] = {2, 3, 4};     // Keypad buttons
bool lastButtonState[] = {HIGH, HIGH, HIGH};  // Previous state condition 

void setup() {
  for(int i=0;i<3;i++){
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); //all off
    
    pinMode(buttonPins[i], INPUT_PULLUP); //setting button to inout mode
  }
}

void loop() {........................................................................................................................................................................................................................................................
  for(int i=0;i<3;i++){
    bool buttonState = digitalRead(buttonPins[i]);

    // Detect button press
    if(buttonState == LOW && lastButtonState[i] == HIGH){
      ledState[i] = !ledState[i];           // toggle LED state
      digitalWrite(ledPins[i], ledState[i]);
    }

    lastButtonState[i] = buttonState;       // update last state
  }
}
 
