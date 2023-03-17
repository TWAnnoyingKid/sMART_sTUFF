int trigPin = D6;
int echoPin = D7;

int trig1Pin = D1;
int echo1Pin = D2;

const int RightLight= D3;
const int LeftLight= D5;

int Rduration, Rdistance;
int Lduration, Ldistance;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(trig1Pin, OUTPUT);
  pinMode(echo1Pin, INPUT);

  pinMode(RightLight, OUTPUT);   digitalWrite(RightLight, HIGH);
  pinMode(LeftLight, OUTPUT);    digitalWrite(LeftLight, HIGH);

  Serial.begin(115200);
  Serial.println("done");
}

void loop(){
     RIGHT();
     LEFT();
  }

void RIGHT(){
    digitalWrite (trigPin, HIGH);
    delayMicroseconds (10);
    digitalWrite (trigPin, LOW);
    Rduration = pulseIn (echoPin, HIGH);
    Rdistance = (Rduration/2) / 29.1;
   
    if (Rdistance <5) { 
      Serial.println("RIGHT_LIGHT_SHINE");
      digitalWrite(RightLight, LOW);
      digitalWrite(LeftLight, HIGH);
    }
    else{
      digitalWrite(RightLight, HIGH);
    }
  }
    
void LEFT(){
    digitalWrite (trig1Pin, HIGH);
    delayMicroseconds (10);
    digitalWrite (trig1Pin, LOW);
    Lduration = pulseIn (echo1Pin, HIGH);
    Ldistance = (Lduration/2) / 29.1;
   
    if (Ldistance <5) { 
      Serial.println("LEFT_LIGHT_SHINE");
      digitalWrite(RightLight, HIGH);
      digitalWrite(LeftLight, LOW);
    }
    else{
      digitalWrite(LeftLight, HIGH);
    }
  }