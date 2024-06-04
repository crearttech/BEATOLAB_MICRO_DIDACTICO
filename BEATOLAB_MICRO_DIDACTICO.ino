//BEATOLAB MICRO
// Controlador MIDI usando Arduino pro Micro con Atmega 32U4 
//Bibliotecas a utilizar. 
#include <MIDIUSB.h>
#include <usb_rename.h>
/////////////////////////////////////////////
// Declaracion de Variables 
//////////////////////////////////////////
// BOTONES 
const int N_BOTONES = 8; 
const int N_BOTONES_ARDUINO = 8; 
//* numero botones conectados al Arduino
const int BOTON_ARDUINO_PIN[N_BOTONES] = {15,14,16,10,7,8,9,6}; 
//* Pin de los botones del arduino en orden
//Compensación de NOTA LARGA y NOTAS MUY CORTAS : Comparativo de estados
int TIEMPO_MUY_CORTO = 10;
int EstadoActualBoton[N_BOTONES] = {0};        // Valor Actual
int EstadoPasadoBoton[N_BOTONES] = {0};        // Valor Previo
unsigned long tiempo_presionado = 0; // Tiempo que lleva presionado 
unsigned long tiempo_libre = 0; // Tiempo que es liberado
/////////////////////////////////////////////
// PERILLAS
const int N_PER = 4; //
const int POT_ARDUINO_PIN[N_PER] = {A3, A2, A1, A0};
int EstadoActualPerilla[N_PER] = {0}; // ESTADO ACTUAL DE LA PERILLA
int EstadoPasadoPerilla[N_PER] = {0}; // ESTADO PASADO DE LA PERILLA 
int VariacionEstadosPerilla = 0; // Diferencia entre valores 
int EstadoActualMIDI[N_PER] = {0}; // ESTADO ACTUAL MIDI 
int EstadoPasadoMIDI[N_PER] = {0}; // ESTADO PASADO MIDI 
USBRename dummy = USBRename("BEATO", "Creart.tech", "BEAT05");
//Variables para Metodos de Evaluación de movimiento
//* Limite esperado de variacion de señal 
const int LimiteVariacion = 10; 
//* Tiempo máximo de lectura despues de haber superado el Limite de Variacion
const int TiempoMaxLectura =10; 
// Condicional de Movimiento
boolean PerMoviendose = true; 
//Tiempo antes de movimiento de la perilla
unsigned long TiempoAntesMovimiento[N_PER] = {0}; 
// Guarda el tiempo desde que inicio el ciclo de operacion del equipo 
unsigned long temporizador[N_PER] = {0}; 


// velocity
byte velocity[N_BOTONES] = {127};
/////////////////////////////////////////////
// DATOS MIDI A UTILIZAR - CORAZON DEL BEATO
byte CANAL_MIDI = 1; //*CANAL MIDI A UTILIZIAR
byte NOTE = 36; //* NOTA MIDI- MAS BAJA
byte cc=1;
//////////////////////////////////////////////


void setup() {
Serial.begin(115200); //*
  // BOTONES
  for (int i = 0; i < N_BOTONES_ARDUINO; i++) {
    pinMode(BOTON_ARDUINO_PIN[i], INPUT_PULLUP);
     }
// PERILLAS 
  for (int i = 0; i < N_PER; i++) {
    pinMode(POT_ARDUINO_PIN[i], INPUT_PULLUP);
  }
}
////////////////////////////////////////////////////
void loop() {
  
  botones();
  perillas();
}

// FUNCIONES PROPIAS DE LA BIBLIOTECA MIDIUSB.h

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
  }

  void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
  }
 ///// FUNCIONES ASOCIADAS A LAS ENTRADAS 
 /////////////////////////////////////////////
// BOTONES 
void botones() {
  for (int i = 0; i < N_BOTONES; i++) {
    EstadoActualBoton[i] = digitalRead(BOTON_ARDUINO_PIN[i]);
          {
          if (EstadoPasadoBoton[i] != EstadoActualBoton[i]) {
                if (EstadoActualBoton[i] == HIGH); 
                  {
                  noteOn(CANAL_MIDI, NOTE + i, 0);
                  MidiUSB.flush();
                  }
                if(EstadoPasadoBoton[i] == LOW && EstadoActualBoton[i] == HIGH) 
                    //Boton Presionado
                        tiempo_presionado = millis();
                    //Guardar este tiempo y compararlo 
                else if(EstadoPasadoBoton[i] == HIGH && EstadoActualBoton[i] == LOW) { 
                    // Boton Liberado 
                        tiempo_libre= millis();
            long NOTA_LARGA= tiempo_libre - tiempo_presionado;
            if( NOTA_LARGA > TIEMPO_MUY_CORTO ) 
       // ESTRUCTURA DE SALIDA EN LENGUAJE MIDI : Canal , Nota mas baja + i , 0 
       noteOn(CANAL_MIDI, NOTE + i, 127);  //
       MidiUSB.flush();
  }
        EstadoPasadoBoton[i] = EstadoActualBoton[i];
      }
    }
 }
 }
 
/////////////////////////////////////////////
void perillas() {

    for (int i = 0; i < N_PER; i++) {
      EstadoActualPerilla[i] = analogRead(POT_ARDUINO_PIN[i]);
      EstadoActualMIDI[i] = map(EstadoActualPerilla[i], 0, 1023, 0, 127); // FORMATEA / MAPEO el estado actual a idioma MIDI 
      VariacionEstadosPerilla = abs(EstadoActualPerilla[i] - EstadoPasadoPerilla[i]); // Valor absoluto de la diferencia entre estados. 
     if (VariacionEstadosPerilla > LimiteVariacion) // Si es mayor a 5 ms , comencemos a evaluar si hay movimiento. 
      { 
        TiempoAntesMovimiento[i] = millis(); // Usar la funcion millis , para darle precisión a la lectura desde "inicio de ciclo", si es 0 no hay movimiento
      }
      temporizador[i] = millis() - TiempoAntesMovimiento[i];  // Temporizador creado para ver si realmente hay movimiento. SI continua siendo 0 no hay movimiento. 
           
     if (temporizador[i] < TiempoMaxLectura) { //Si el temporizador es mayor al tiempo maximo de lectura quiere decir que AUN se está moviendo 
        PerMoviendose = true;
      }
      else {
        PerMoviendose = false;
      }
      if (PerMoviendose == true) { // Si AUN se está moviendo , envie el valor que nos interesa, EL CONTROL CHANGE. 
        if (EstadoPasadoMIDI[i] != EstadoActualMIDI[i]) {
        controlChange(CANAL_MIDI, cc + i, EstadoActualMIDI[i]); // FORMATO : CANAL , control change y lectura de analog read MAPEADA 
        MidiUSB.flush();
        EstadoPasadoPerilla[i] = EstadoActualPerilla[i]; // Guardar ese estado pasado y continuar comparando
        EstadoPasadoMIDI[i] = EstadoActualMIDI[i];
          }
       }
    }
    }
    
 
