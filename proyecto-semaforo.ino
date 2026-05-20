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

// Máquina de estados vehicular

void actualizarSemaforos() {

  unsigned long tiempoActual = millis();

  switch (estadoActual) {

    case 0:

      setSemaforoA(false, false, true);
      setSemaforoB(true, false, false);

      // Cruce peatonal permitido
      if (solicitudNS && !monitorCruceOcupado) {
        iniciarCruceNS(tiempoActual);
      }

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

      if (solicitudEO && !monitorCruceOcupado) {
        iniciarCruceEO(tiempoActual);
      }

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


// Control peatonal

void iniciarCruceNS(unsigned long tiempoActual) {

  if (monitorCruceOcupado) return;

  if (!admitirProceso(pPeatonNS)) {

    erroresMemoria++;

    logEvento(
      "ERROR_MEMORIA",
      "PEATON_NS",
      ramUsada,
      "RECHAZADO"
    );

    return;
  }

  monitorCruceOcupado = true;

  cruceNSActivo = true;
  inicioCruceNS = tiempoActual;

  digitalWrite(PEATON_NS_ROJO, LOW);
  digitalWrite(PEATON_NS_VERDE, HIGH);

  logEvento(
    "CRUCE_INICIO",
    "PEATON_NS",
    ramUsada,
    "OK"
  );
}




void iniciarCruceEO(unsigned long tiempoActual) {

  if (monitorCruceOcupado) return;

  if (!admitirProceso(pPeatonEO)) {

    erroresMemoria++;

    logEvento(
      "ERROR_MEMORIA",
      "PEATON_EO",
      ramUsada,
      "RECHAZADO"
    );

    return;
  }

  monitorCruceOcupado = true;

  cruceEOActivo = true;
  inicioCruceEO = tiempoActual;

  digitalWrite(PEATON_EO_ROJO, LOW);
  digitalWrite(PEATON_EO_VERDE, HIGH);

  logEvento(
    "CRUCE_INICIO",
    "PEATON_EO",
    ramUsada,
    "OK"
  );
}

// Actualización de cruces peatonales

void actualizarCruces(unsigned long tiempoActual) {

  // norte - sur
  if (
    cruceNSActivo &&
    tiempoActual - inicioCruceNS >= T_PEATON
  ) {

    digitalWrite(PEATON_NS_VERDE, LOW);
    digitalWrite(PEATON_NS_ROJO, HIGH);

    liberarProceso(pPeatonNS);

    solicitudNS = false;
    bloqueoNS = false;

    cruceNSActivo = false;
    monitorCruceOcupado = false;

    logEvento(
      "CRUCE_FIN",
      "PEATON_NS",
      ramUsada,
      "LIBERADO"
    );
  }

  // este - oeste
  if (
    cruceEOActivo &&
    tiempoActual - inicioCruceEO >= T_PEATON
  ) {

    digitalWrite(PEATON_EO_VERDE, LOW);
    digitalWrite(PEATON_EO_ROJO, HIGH);

    liberarProceso(pPeatonEO);

    solicitudEO = false;
    bloqueoEO = false;

    cruceEOActivo = false;
    monitorCruceOcupado = false;

    logEvento(
      "CRUCE_FIN",
      "PEATON_EO",
      ramUsada,
      "LIBERADO"
    );
  }
}


// ================= MEMORIA + CONTEXT SWITCH =================
Proceso* procesoPorEstado(int estadoVehicular) {
  switch (estadoVehicular) {
    case 0:
      return &pVehA_Verde;
    case 1:
      return &pVehA_Amarillo;
    case 2:
      return &pVehB_Verde;
    case 3:
      return &pVehB_Amarillo;
    default:
      return &pVehA_Verde;
  }
}

bool admitirProceso(Proceso& proceso) {
  if (proceso.activo) {
    return true;
  }

  if (ram_usada_actual + proceso.ram_kb > RAM_TOTAL) {
    return false;
  }

  proceso.activo = true;
  ram_usada_actual += proceso.ram_kb;

  if (ram_usada_actual > ram_maxima_usada) {
    ram_maxima_usada = ram_usada_actual;
  }

  logEvento("ADMITIDO", proceso.id, ram_usada_actual, "OK");
  return true;
}

void liberarProceso(Proceso& proceso) {
  if (!proceso.activo) {
    return;
  }

  proceso.activo = false;
  ram_usada_actual -= proceso.ram_kb;
  if (ram_usada_actual < 0) {
    ram_usada_actual = 0;
  }

  logEvento("LIBERADO", proceso.id, ram_usada_actual, "OK");
}

bool contextSwitchVehicular(int nuevoEstado) {
  Proceso* siguiente = procesoPorEstado(nuevoEstado);

  if (procesoVehicularActual == siguiente) {
    return true;
  }

  int ramLuegoDelCambio = ram_usada_actual;
  if (procesoVehicularActual != 0) {
    ramLuegoDelCambio -= procesoVehicularActual->ram_kb;
  }
  ramLuegoDelCambio += siguiente->ram_kb;

  if (ramLuegoDelCambio > RAM_TOTAL) {
    eventos_espera_memoria++;
    logEvento("ESPERA_MEMORIA", siguiente->id, ram_usada_actual, "CONTEXT_SWITCH_BLOQUEADO");
    return false;
  }

  if (procesoVehicularActual != 0) {
    liberarProceso(*procesoVehicularActual);
  }

  if (!admitirProceso(*siguiente)) {
    eventos_espera_memoria++;
    logEvento("ESPERA_MEMORIA", siguiente->id, ram_usada_actual, "FALLO_ADMISION");
    return false;
  }

  procesoVehicularActual = siguiente;
  logEvento("CONTEXT_SWITCH", siguiente->id, ram_usada_actual, "OK");
  return true;
}

// ================= LOGS Y MÉTRICAS =================
// [TIMESTAMP], [EVENTO], [PROCESO_ID], [RAM_CONSUMIDA], [ESTADO_MEMORIA]
void logEvento(const char* evento, const char* procesoId, int ramConsumida, const char* estadoMemoria) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print("], [");
  Serial.print(evento);
  Serial.print("], [");
  Serial.print(procesoId);
  Serial.print("], [");
  Serial.print(ramConsumida);
  Serial.print("], [");
  Serial.print(estadoMemoria);
  Serial.println("]");

  Serial.print("METRICAS, ram_maxima_usada=");
  Serial.print(ram_maxima_usada);
  Serial.print(", eventos_espera_memoria=");
  Serial.println(eventos_espera_memoria);
}
