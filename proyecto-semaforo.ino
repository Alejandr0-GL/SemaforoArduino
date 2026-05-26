// Semáforo a (norte - sur)
const int A_ROJO = 2;
const int A_AMARILLO = 3;
const int A_VERDE = 4;

// Semáforo b (este – oeste) 
const int B_ROJO = 5;
const int B_AMARILLO = 6;
const int B_VERDE = 7;

// Peatonales 
const int PEATON_NS_VERDE = 9;
const int PEATON_NS_ROJO = 10;

const int PEATON_EO_VERDE = 11;
const int PEATON_EO_ROJO = 12;

// Botones 
const int BOTON_NS = 8;
const int BOTON_EO = 13;


// Variables del sistema
unsigned long tiempoAnterior = 0;

// Estados del sistema vehicular
/*
 0 = A verde
 1 = A amarillo
 2 = B verde
 3 = B amarillo
*/
int estadoActual = 0;

// Variables peatonales
// Solicitudes realizadas por botones
bool solicitudNS = false;
bool solicitudEO = false;

// Bloqueo anti-spam de botones
bool bloqueoNS = false;
bool bloqueoEO = false;


// Control de cruces activos
bool cruceNSActivo = false;
bool cruceEOActivo = false;

// Solo un cruce peatonal puede usarse a la vez
bool monitorCruceOcupado = false;


// Tiempos del sistema
const unsigned long T_VERDE = 5000;
const unsigned long T_AMARILLO = 2000;
const unsigned long T_PEATON = 4000;


// Control temporal peatonal
unsigned long inicioCruceNS = 0;
unsigned long inicioCruceEO = 0;


// Simulación de memoria ram
const int RAM_TOTAL = 1024;

/*
 Cada estado importante del sistema
 se comporta como un proceso.
*/
struct Proceso {

  const char* nombre;
  int memoriaKB;
  bool activo;
};


// Procesos vehiculares
Proceso pA_Verde     = {"A_VERDE", 180, false};
Proceso pA_Amarillo  = {"A_AMARILLO", 160, false};

Proceso pB_Verde     = {"B_VERDE", 180, false};
Proceso pB_Amarillo  = {"B_AMARILLO", 160, false};


// Procesos peatonales 
Proceso pPeatonNS    = {"PEATON_NS", 220, false};
Proceso pPeatonEO    = {"PEATON_EO", 220, false};

// Proceso vehicular actual
Proceso* procesoActual = NULL;

// Métricas del sistema
int ramUsada = 0;
int ramMaxima = 0;

unsigned long erroresMemoria = 0;

// Prototipos

// Control vehicular
void actualizarSemaforos();
void cambiarEstado(int nuevoEstado);

// Control peatonal
void detectarBotones();
void iniciarCruceNS(unsigned long tiempoActual);
void iniciarCruceEO(unsigned long tiempoActual);
void actualizarCruces(unsigned long tiempoActual);

// Control de LEDs
void setSemaforoA(bool rojo, bool amarillo, bool verde);
void setSemaforoB(bool rojo, bool amarillo, bool verde);

// Gestión de procesos
Proceso* obtenerProceso(int estado);
bool admitirProceso(Proceso& proceso);
void liberarProceso(Proceso& proceso);
bool contextSwitch(int nuevoEstado);

// Logs
void logEvento(
  const char* evento,
  const char* proceso,
  int ram,
  const char* estado
);


// Setup
void setup() {

  // Salidas vehiculares
  pinMode(A_ROJO, OUTPUT);
  pinMode(A_AMARILLO, OUTPUT);
  pinMode(A_VERDE, OUTPUT);

  pinMode(B_ROJO, OUTPUT);
  pinMode(B_AMARILLO, OUTPUT);
  pinMode(B_VERDE, OUTPUT);

  // Salidas peatonales
  pinMode(PEATON_NS_VERDE, OUTPUT);
  pinMode(PEATON_NS_ROJO, OUTPUT);

  pinMode(PEATON_EO_VERDE, OUTPUT);
  pinMode(PEATON_EO_ROJO, OUTPUT);

  // Entradas
  pinMode(BOTON_NS, INPUT_PULLUP);
  pinMode(BOTON_EO, INPUT_PULLUP);

  // Serial
  Serial.begin(9600);

  // Estado inicial peatones
  digitalWrite(PEATON_NS_VERDE, LOW);
  digitalWrite(PEATON_NS_ROJO, HIGH);

  digitalWrite(PEATON_EO_VERDE, LOW);
  digitalWrite(PEATON_EO_ROJO, HIGH);

  // Proceso inicial
  procesoActual = &pA_Verde;
  admitirProceso(*procesoActual);

  tiempoAnterior = millis();

  logEvento("BOOT", "SISTEMA", ramUsada, "INICIALIZADO");
}

// Loop

void loop() {

  unsigned long tiempoActual = millis();

  // Tareas concurrentes
  detectarBotones();
  actualizarCruces(tiempoActual);
  actualizarSemaforos();
}

// Detección de botones peatonales
void detectarBotones() {

  // norte - sur
  if (digitalRead(BOTON_NS) == LOW && !bloqueoNS) {

    solicitudNS = true;
    bloqueoNS = true;

    logEvento(
      "BOTON",
      "PEATON_NS",
      ramUsada,
      "SOLICITUD"
    );
  }

  // este - oeste
  if (digitalRead(BOTON_EO) == LOW && !bloqueoEO) {

    solicitudEO = true;
    bloqueoEO = true;

    logEvento(
      "BOTON",
      "PEATON_EO",
      ramUsada,
      "SOLICITUD"
    );
  }
}

// control peatonal

void iniciarCruceNS(unsigned long tiempoActual) {

  if (monitorCruceOcupado) return;

  monitorCruceOcupado = true;

  cruceNSActivo = true;
  inicioCruceNS = tiempoActual;

  solicitudNS = false;

  logEvento(
    "CRUCE_INICIO",
    "PEATON_NS",
    ramUsada,
    "OK"
  );
}

void iniciarCruceEO(unsigned long tiempoActual) {

  if (monitorCruceOcupado) return;

  monitorCruceOcupado = true;

  cruceEOActivo = true;
  inicioCruceEO = tiempoActual;

  solicitudEO = false;

  logEvento(
    "CRUCE_INICIO",
    "PEATON_EO",
    ramUsada,
    "OK"
  );
}


// Actualización automática de peatones
void actualizarCruces(unsigned long tiempoActual) {

  // norte - sur
  if (estadoActual == 2) {

    digitalWrite(PEATON_NS_VERDE, HIGH);
    digitalWrite(PEATON_NS_ROJO, LOW);

  } else {

    digitalWrite(PEATON_NS_VERDE, LOW);
    digitalWrite(PEATON_NS_ROJO, HIGH);
  }


  // este - oeste
  if (estadoActual == 0) {

    digitalWrite(PEATON_EO_VERDE, HIGH);
    digitalWrite(PEATON_EO_ROJO, LOW);

  } else {

    digitalWrite(PEATON_EO_VERDE, LOW);
    digitalWrite(PEATON_EO_ROJO, HIGH);
  }


  // Finalizar cruce NS
  if (
    cruceNSActivo &&
    tiempoActual - inicioCruceNS >= T_PEATON
  ) {

    cruceNSActivo = false;
    monitorCruceOcupado = false;
    bloqueoNS = false;

    logEvento(
      "CRUCE_FIN",
      "PEATON_NS",
      ramUsada,
      "OK"
    );
  }


  // Finalizar cruce EO
  if (
    cruceEOActivo &&
    tiempoActual - inicioCruceEO >= T_PEATON
  ) {

    cruceEOActivo = false;
    monitorCruceOcupado = false;
    bloqueoEO = false;

    logEvento(
      "CRUCE_FIN",
      "PEATON_EO",
      ramUsada,
      "OK"
    );
  }
}


// Máquina de estados vehicular

void actualizarSemaforos() {

  unsigned long tiempoActual = millis();

  switch (estadoActual) {

    case 0:

      setSemaforoA(false, false, true);
      setSemaforoB(true, false, false);

      // Cambio de estado
      if (
        tiempoActual - tiempoAnterior >= T_VERDE &&
        !monitorCruceOcupado
      ) {

        cambiarEstado(1);
      }

      break;


    case 1:

      setSemaforoA(false, true, false);
      setSemaforoB(true, false, false);
    
      // Activar peatón SOLO cuando termina verde vehicular
	  if (solicitudEO && !monitorCruceOcupado) {
        
        iniciarCruceEO(tiempoActual);
      }

      if (
        tiempoActual - tiempoAnterior >= T_AMARILLO &&
        !monitorCruceOcupado
      ) {

        cambiarEstado(2);
      }

      break;
    case 2:

      setSemaforoB(false, false, true);
      setSemaforoA(true, false, false);

      if (
        tiempoActual - tiempoAnterior >= T_VERDE &&
        !monitorCruceOcupado
      ) {

        cambiarEstado(3);
      }

      break;

    case 3:

      setSemaforoB(false, true, false);
      setSemaforoA(true, false, false);
    
      // Activar peatón SOLO cuando termina verde vehicular
      if (solicitudNS && !monitorCruceOcupado) {
        
        iniciarCruceNS(tiempoActual);
      }

      if (
        tiempoActual - tiempoAnterior >= T_AMARILLO &&
        !monitorCruceOcupado
      ) {

        cambiarEstado(0);
      }

      break;
  }
}


// Cambio de contexto

void cambiarEstado(int nuevoEstado) {

  if (contextSwitch(nuevoEstado)) {

    estadoActual = nuevoEstado;
    tiempoAnterior = millis();
  }
}

// Control de leds

void setSemaforoA(bool rojo, bool amarillo, bool verde) {

  digitalWrite(A_ROJO, rojo);
  digitalWrite(A_AMARILLO, amarillo);
  digitalWrite(A_VERDE, verde);
}

void setSemaforoB(bool rojo, bool amarillo, bool verde) {

  digitalWrite(B_ROJO, rojo);
  digitalWrite(B_AMARILLO, amarillo);
  digitalWrite(B_VERDE, verde);
}

// Gestión de procesos

Proceso* obtenerProceso(int estado) {

  switch (estado) {

    case 0: return &pA_Verde;
    case 1: return &pA_Amarillo;
    case 2: return &pB_Verde;
    case 3: return &pB_Amarillo;

    default: return &pA_Verde;
  }
}


bool admitirProceso(Proceso& proceso) {

  if (proceso.activo) return true;

  if (ramUsada + proceso.memoriaKB > RAM_TOTAL) {
    return false;
  }

  proceso.activo = true;

  ramUsada += proceso.memoriaKB;

  if (ramUsada > ramMaxima) {
    ramMaxima = ramUsada;
  }

  logEvento(
    "PROCESO_ADMITIDO",
    proceso.nombre,
    ramUsada,
    "OK"
  );

  return true;
}


void liberarProceso(Proceso& proceso) {

  if (!proceso.activo) return;

  proceso.activo = false;

  ramUsada -= proceso.memoriaKB;

  if (ramUsada < 0) {
    ramUsada = 0;
  }

  logEvento(
    "PROCESO_LIBERADO",
    proceso.nombre,
    ramUsada,
    "OK"
  );
}

// Context switch

bool contextSwitch(int nuevoEstado) {

  Proceso* siguiente = obtenerProceso(nuevoEstado);

  if (procesoActual == siguiente) {
    return true;
  }

  liberarProceso(*procesoActual);

  if (!admitirProceso(*siguiente)) {

    erroresMemoria++;

    logEvento(
      "CONTEXT_SWITCH_ERROR",
      siguiente->nombre,
      ramUsada,
      "SIN_MEMORIA"
    );

    return false;
  }

  procesoActual = siguiente;

  logEvento(
    "CONTEXT_SWITCH",
    siguiente->nombre,
    ramUsada,
    "OK"
  );

  return true;
}


// Sistema de logs

void logEvento(
  const char* evento,
  const char* proceso,
  int ram,
  const char* estado
) {

  Serial.print("[");

  Serial.print(millis());

  Serial.print("] ");

  Serial.print(evento);

  Serial.print(" | ");

  Serial.print(proceso);

  Serial.print(" | RAM=");

  Serial.print(ram);

  Serial.print("KB | ");

  Serial.println(estado);


  // Métricas

  Serial.print("RAM_MAX=");

  Serial.print(ramMaxima);

  Serial.print("KB");

  Serial.print(" | ");

  Serial.print("ERRORES_MEMORIA=");

  Serial.println(erroresMemoria);
}
