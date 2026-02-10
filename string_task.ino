void setup() { 
  Serial.begin(9600);
  Serial.println("Use format:");
  Serial.println("DATE:DD:MM:YYYY");
  Serial.println("TIME:HH:MM:SS");
  Serial.println("DATETIME:DD:MM:YY:HH:MM:SS");
}

void loop() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input.startsWith("DATE:") && !input.startsWith("DATETIME:")) {
    input.remove(0, input.indexOf(':') + 1);
    parseDate(input);
  } else if (input.startsWith("TIME:")) {
    input.remove(0, input.indexOf(':') + 1);
    parseTime(input);
  } else if (input.startsWith("DATETIME:")) {
    input.remove(0, input.indexOf(':') + 1);
    parseDateTime(input);
  } else {
    Serial.println("Invalid Command");
  }
}

// Generic split and remove logic for DATE
void parseDate(String s) {
  int arr[3];

  for (int i = 0; i < 3; i++) {
    int pos = s.indexOf(':');
    if (i < 2) {
      if (pos == -1) {
        Serial.println("Invalid DATE format");
        return;
      }
      arr[i] = s.substring(0, pos).toInt();
      s.remove(0, pos + 1);
    } else {
      arr[i] = s.toInt();
    }
  }

  if (arr[0] > 31 || arr[1] > 12 || arr[2] > 9999) {
    Serial.println("Invalid DATE digits");
    return;
  }
  if (arr[2] < 100) arr[2] += 2000;

  Serial.println("DATE");
  Serial.print("DAY=");
  Serial.println(arr[0] < 10 ? "0" + String(arr[0]) : String(arr[0]));
  Serial.print("MONTH=");
  Serial.println(arr[1] < 10 ? "0" + String(arr[1]) : String(arr[1]));
  Serial.print("YEAR=");
  Serial.println(arr[2]);
}

// Generic split and remove logic for TIME
void parseTime(String s) {
  int arr[3];

  for (int i = 0; i < 3; i++) {
    int pos = s.indexOf(':');
    if (i < 2) {
      if (pos == -1) {
        Serial.println("Invalid TIME format");
        return;
      }
      arr[i] = s.substring(0, pos).toInt();
      s.remove(0, pos + 1);
    } else {
      arr[i] = s.toInt();
    }
  }

  if (arr[0] > 23 || arr[1] > 59 || arr[2] > 59) {
    Serial.println("Invalid TIME digits");
    return;
  }

  Serial.println("TIME");
  Serial.print("HOUR=");
  Serial.println(arr[0] < 10 ? "0" + String(arr[0]) : String(arr[0]));
  Serial.print("MIN=");
  Serial.println(arr[1] < 10 ? "0" + String(arr[1]) : String(arr[1]));
  Serial.print("SEC=");
  Serial.println(arr[2] < 10 ? "0" + String(arr[2]) : String(arr[2]));
}

void parseDateTime(String s) {  // STRING DATEANDTIME
  int arr[6];

  for (int i = 0; i < 6; i++) {  //D.M.Y:H.M.S
    int pos = s.indexOf(':');    // location of index
    if (i < 5) {                 // colons
      if (pos == -1) {
        Serial.println("Invalid DATETIME format");
        return;
      }                                      // agat colons ni he to format galat he
      arr[i] = s.substring(0, pos).toInt();  // 12:08:25:14:30:45 //ar[o]=[0,2]="12".toint()12
      s.remove(0, pos + 1);                  // remove(0,2+1) start=0 end=3=12: ko remove -> 08:25:14:30:45 ye part bachega
    } else {
      arr[i] = s.toInt();  // last bala manually set
    }
  }

  if (arr[2] < 100) arr[2] += 2000;

  Serial.println("DATE & TIME");
  Serial.print("DAY=");
  Serial.println(arr[0] < 10 ? "0" + String(arr[0]) : String(arr[0]));
  Serial.print("MONTH=");
  Serial.println(arr[1] < 10 ? "0" + String(arr[1]) : String(arr[1]));
  Serial.print("YEAR=");
  Serial.println(arr[2]);
  Serial.print("HOUR=");
  Serial.println(arr[3] < 10 ? "0" + String(arr[3]) : String(arr[3]));
  Serial.print("MIN=");
  Serial.println(arr[4] < 10 ? "0" + String(arr[4]) : String(arr[4]));
  Serial.print("SEC=");
  Serial.println(arr[5] < 10 ? "0" + String(arr[5]) : String(arr[5]));
}
