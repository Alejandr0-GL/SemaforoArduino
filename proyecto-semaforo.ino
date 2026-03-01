// Semaforo A (norte-sur)
const int A_rojo = 2;
const int A_amarillo = 3;
const int A_verde = 4;

// Semaforo B (oriente-occidente) 
const int B_rojo = 5;
const int B_amarillo = 6;
const int B_verde = 7;

// Semaforos peatonales 
const int peat_NS_verde = 9;
const int peat_NS_rojo = 10;

const int peat_OO_verde = 11;
const int peat_OO_rojo = 12;

// BOTONES 
const int boton_NS = 8;
const int boton_OO = 13;

// VARIABLES 
unsigned long tiempoAnterior = 0;

int estado = 0;
// 0=A verde
// 1=A amarillo
// 2=B verde
// 3=B amarillo

bool solicitud_NS = false;
bool solicitud_OO = false;

bool bloqueo_NS = false;
bool bloqueo_OO = false;

const unsigned long tVerde = 5000;
const unsigned long tAmarillo = 2000;
const unsigned long tPeaton = 4000;

unsigned long tiempoPeatonNS = 0;
unsigned long tiempoPeatonEO = 0;

void setup() {

  pinMode(A_rojo, OUTPUT);
  pinMode(A_amarillo, OUTPUT);
  pinMode(A_verde, OUTPUT);

  pinMode(B_rojo, OUTPUT);
  pinMode(B_amarillo, OUTPUT);
  pinMode(B_verde, OUTPUT);

  pinMode(peat_NS_verde, OUTPUT);
  pinMode(peat_NS_rojo, OUTPUT);
  pinMode(peat_OO_verde, OUTPUT);
  pinMode(peat_OO_rojo, OUTPUT);

  pinMode(boton_NS, INPUT_PULLUP);
  pinMode(boton_OO, INPUT_PULLUP);
}

