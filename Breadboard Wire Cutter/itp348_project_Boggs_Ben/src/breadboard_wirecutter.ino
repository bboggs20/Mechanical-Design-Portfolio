#include <queue>


/******** DECLARATIONS *********/

// publishes queue contents to app, and to terminal if sout = true
void printQueue(bool sout = false);

// check wire stock
bool stocked(); 

// cut a wire length l
void cut (double l);    

// add job to queue. 0 if success, -1 if failure
int queueJob(String job);    

// find # of spaces in a string
int findSpaces(String str);     

// helper class for cutting jobs
struct bbWire 
{
    double length; // length of wire in mm
    bool end; // end of job or not
};

// Declare servo object
Servo s;

/************ PINS *************/

// Sensor pins
#define SWITCH D2
#define REFLECTANCE D8

// Servo signal pin
#define SERVO D6

// Motor pins (A is sandpapered)
#define PWMA D5
#define AIN2 D4
#define AIN1 D3

#define PWMB A0
#define BIN2 A1
#define BIN1 A2

#define STBY A3

/********** CONSTANTS ***********/

// mm per breadboard unit
const double _mm_per_bbu = 2.5;

// wheel diameter in mm
const double _wheel_d = 61.0;

// steady-state motor speed in mm/ms
const double _speed = 0.23;

// corresponding motor pwm speed
const int _pwmspeed = 255;

// breadboard hole depth in mm
const double _bb_depth = 6.5;

// servo 'open' angle in degrees (wire can pass)
const int _open = 75;

// servo 'closed' angle in degrees (wire is cut)
const int _close = 180;

// states
#define waiting_for_job 1
#define cutting 2
#define waiting_for_pickup 3
#define waiting_for_table_return 4
#define low_wire_stock 5

/********** VARIABLES ***********/

String State = "Waiting for Job";
int _state = 0;
int _prev = 0;
unsigned long _t0, _publishTime;
std::queue<bbWire*> wires;

void setup() 
{
    Particle.function("Queue_Job", queueJob);
    
    pinMode(STBY, INPUT_PULLUP);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWMA, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(SWITCH, INPUT);
    pinMode(REFLECTANCE, INPUT);
    s.attach(SERVO);
    _state = 1;

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);

    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    
    s.write(_open);
    delay(200);
    
    // analogWrite(PWMA, _pwmspeed);
    // analogWrite(PWMB, _pwmspeed);
    // delay(3000);
    // analogWrite(PWMA, 0);
    // analogWrite(PWMB, 0);

    Serial.begin();
    _publishTime = 4000;
    printQueue();
}


void loop() 
{
    /**
    if (!stocked())
    {
        while (!stocked()) t = t;
        Serial.println("t = " + String(t));
        analogWrite(PWMA, _pwmspeed);
        analogWrite(PWMB, _pwmspeed);
        delay(t);
        analogWrite(PWMA, 0);
        analogWrite(PWMB, 0);
    }
    if (digitalRead(SWITCH) == HIGH)
    {
        t += 50;
        while (digitalRead(SWITCH) == HIGH) t = t;
    }
    **/
    if (millis() - _t0 > _publishTime)
    {
        Particle.publish("State", State, PUBLIC);
        printQueue();
    }
    
    switch (_state)
    {
        case waiting_for_job:
            if (_state != _prev)
            {
                Particle.publish("State", "Waiting for Job", PUBLIC);
                _t0 = millis();
                Serial.println("Waiting for Job");
            }
            if (stocked())
            {
                if (!wires.empty())
                {
                    _state = cutting;
                    State = "Cutting";
                }
                    
            }
            else
            {
                _state = low_wire_stock;
                State = "Low Wire Stock";
            }
                
            _prev = waiting_for_job;
            break;

        case cutting:
            if (_state != _prev)
            {
                Particle.publish("State", "Cutting", PUBLIC);
                _t0 = millis();
                Serial.println("Cutting");
            }
            if (!stocked())
            {
                _state = low_wire_stock;
                State = "Low Wire Stock";
            }
            else
            {
                bool end = false;
                while (!end) 
                {
                    bbWire* curr = wires.front();
                    Serial.println("l = " + String(curr->length));
                    cut(curr->length);
                    end = curr->end;
                    wires.pop();
                    delete curr;
                }
                    
                if (stocked())
                {
                    _state = waiting_for_pickup;
                    State = "Waiting for Pickup";
                }
                else
                {
                    _state = low_wire_stock;
                    State = "Low Wire Stock";
                }
                    
            }
            _prev = cutting;
            break;

        case waiting_for_pickup:
            if (_state != _prev)
            {
                Particle.publish("State", "Waiting for Pickup", PUBLIC);
                _t0 = millis();
                Serial.println("Waiting for Pickup"); 
            }
            if (digitalRead(SWITCH) == HIGH) // table moved
            {
                _state = waiting_for_table_return;
                State = "Waiting for Table Return";
            }
                
            _prev = waiting_for_pickup;
            break;

        case waiting_for_table_return:
            if (_state != _prev)
            {
                Particle.publish("State", "Waiting for Table Return", PUBLIC);
                _t0 = millis();
                Serial.println("Waiting for Table Return");
            }
            if (digitalRead(SWITCH) == LOW) // table returned
            {
                if (wires.empty())
                {
                    _state = waiting_for_job;
                    State = "Waiting for Job";
                }
                else
                {
                    _state = cutting;
                    State = "Cutting";
                    delay(500);
                }
            }
            _prev = waiting_for_table_return;
            break;
        
        case low_wire_stock:
            if (_state != _prev)
            {
                Particle.publish("State", "Low Wire Stock", PUBLIC);
                _t0 = millis();
                Serial.println("Low Wire Stock");
            }
            if (stocked())
            {
                if (wires.empty())
                {
                    _state = waiting_for_job;
                    State = "Waiting for Job";
                }
                else
                {
                    _state = cutting;
                    State = "Cutting";
                }
            }
            _prev = low_wire_stock;
            break;
    }
}

bool stocked() {    return digitalRead(REFLECTANCE) == HIGH;    }

void cut (double l)
{
    // extrude wire
    unsigned long t_wire = (l+2*_bb_depth)/_speed + 55;
    Serial.println("t_wire = " + String(t_wire));
    analogWrite(PWMA, _pwmspeed);
    analogWrite(PWMB, _pwmspeed);
    delay(t_wire);
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    delay(500);

    // cut wire
    s.write(_close);
    delay(500);
    s.write(_open);
    delay(1000);
}

int queueJob(String job)
{
    Serial.println(job);
    if (job.length() == 0)
        return -1;

    String lengths = job;
    int nWires = findSpaces(job)+1;
    Serial.println(String(nWires) + " wires:");
    int s = 0;
    int e = 0;
    for (int i=0; i<nWires; i++)
    {
        bbWire* curr = new bbWire;
        e = s;
        while (job.substring(e, e+1) != " " && e < job.length()) e++;
        curr->length = atof(job.substring(s, e))*_mm_per_bbu;
        curr->end = false;
        if (e == job.length()) curr->end = true;     
        wires.push(curr);
        s = e+1;
    }   
    printQueue(true);
    return 0;
}

int findSpaces(String str)
{
    int numSpaces = 0;
    for (int i=0; i<str.length(); i++)
        if (str.substring(i,i+1) == " ") numSpaces++;
    return numSpaces;
}

void printQueue(bool sout)
{
    std::queue<bbWire*> pq = wires;
    int i = 1;
    String q = "";
    while (!pq.empty())
    {
        q = q + String(i++) + ": ";
        bbWire curr = *pq.front();
        q = q + "l = " + String(curr.length/_mm_per_bbu*1.0);
        q = curr.end ? q + " - End of job\n" : q + "\n";
        pq.pop();
    }
    q = q + "\n";
    if (sout) Serial.print(q);
    Particle.publish("Queue", q, PUBLIC);
    _t0 = millis();
}