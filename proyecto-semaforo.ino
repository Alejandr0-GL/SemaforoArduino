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


void setup() {
  pinMode(A_rojo, OUTPUT);
  pinMode(A_amarillo, OUTPUT);
  pinMode(A_verde, OUTPUT);

  pinMode(B_rojo, OUTPUT);
  pinMode(B_amarillo, OUTPUT);
  pinMode(B_verde, OUTPUT);

  pinMode(peat_NS_verde, OUTPUT);
  pinMode(peat_NS_rojo, OUTPUT);
  pinMode(peat_EO_verde, OUTPUT);
  pinMode(peat_EO_rojo, OUTPUT);

  pinMode(boton_NS, INPUT_PULLUP);
  pinMode(boton_EO, INPUT_PULLUP);

  Serial.begin(9600);

  digitalWrite(peat_NS_verde, LOW);
  digitalWrite(peat_NS_rojo, HIGH);
  digitalWrite(peat_EO_verde, LOW);
  digitalWrite(peat_EO_rojo, HIGH);

  procesoVehicularActual = &pVehA_Verde;
  admitirProceso(*procesoVehicularActual);

  tiempoAnterior = millis();
  logEvento("BOOT", "SISTEMA", ram_usada_actual, "INICIALIZADO");
}

void loop() {
  unsigned long tiempoActual = millis();

  // ===== DETECCIÓN BOTONES =====
  if (digitalRead(boton_NS) == LOW && !bloqueo_NS) {
    solicitud_NS = true;
    bloqueo_NS = true;
    logEvento("BOTON", "PEATON_NS", ram_usada_actual, "SOLICITUD_ENCOLADA");
  }

  if (digitalRead(boton_EO) == LOW && !bloqueo_EO) {
    solicitud_EO = true;
    bloqueo_EO = true;
    logEvento("BOTON", "PEATON_EO", ram_usada_actual, "SOLICITUD_ENCOLADA");
  }

  // Tarea cooperativa peatonal (sin delay)
  actualizarCrucePeatonal(tiempoActual);

  // ===== MÁQUINA DE ESTADOS =====
  switch (estado) {
    case 0: // A VERDE - B ROJO
      A_verde_ON();
      B_rojo_ON();

      if (solicitud_NS && !monitorCrucePeatonalOcupado) {
        iniciarCruceNS(tiempoActual);
      }

      if ((tiempoActual - tiempoAnterior >= tVerde) && !monitorCrucePeatonalOcupado) {
        if (contextSwitchVehicular(1)) {
          estado = 1;
          tiempoAnterior = tiempoActual;
        }
      }
      break;

    case 1: // A AMARILLO
      A_amarillo_ON();
      B_rojo_ON();

      if ((tiempoActual - tiempoAnterior >= tAmarillo) && !monitorCrucePeatonalOcupado) {
        if (contextSwitchVehicular(2)) {
          estado = 2;
          tiempoAnterior = tiempoActual;
        }
      }
      break;

    case 2: // B VERDE - A ROJO
      B_verde_ON();
      A_rojo_ON();

      if (solicitud_EO && !monitorCrucePeatonalOcupado) {
        iniciarCruceEO(tiempoActual);
      }

      if ((tiempoActual - tiempoAnterior >= tVerde) && !monitorCrucePeatonalOcupado) {
        if (contextSwitchVehicular(3)) {
          estado = 3;
          tiempoAnterior = tiempoActual;
        }
      }
      break;

    case 3: // B AMARILLO
      B_amarillo_ON();
      A_rojo_ON();

      if ((tiempoActual - tiempoAnterior >= tAmarillo) && !monitorCrucePeatonalOcupado) {
        if (contextSwitchVehicular(0)) {
          estado = 0;
          tiempoAnterior = tiempoActual;
        }
      }
      break;
  }
}

// ================= FUNCIONES VEHICULARES =================
void A_rojo_ON() {
  digitalWrite(A_rojo, HIGH);
  digitalWrite(A_amarillo, LOW);
  digitalWrite(A_verde, LOW);
}

void A_amarillo_ON() {
  digitalWrite(A_rojo, LOW);
  digitalWrite(A_amarillo, HIGH);
  digitalWrite(A_verde, LOW);
}

void A_verde_ON() {
  digitalWrite(A_rojo, LOW);
  digitalWrite(A_amarillo, LOW);
  digitalWrite(A_verde, HIGH);
}

void B_rojo_ON() {
  digitalWrite(B_rojo, HIGH);
  digitalWrite(B_amarillo, LOW);
  digitalWrite(B_verde, LOW);
}

void B_verde_ON() {
  digitalWrite(B_rojo, LOW);
  digitalWrite(B_amarillo, LOW);
  digitalWrite(B_verde, HIGH);
}

void B_amarillo_ON() {
  digitalWrite(B_rojo, LOW);
  digitalWrite(B_amarillo, HIGH);
  digitalWrite(B_verde, LOW);
}

// ================= FUNCIONES PEATONALES =================
void iniciarCruceNS(unsigned long tiempoActual) {
  if (monitorCrucePeatonalOcupado) {
    return;
  }

  if (!admitirProceso(pPeatonNS)) {
    eventos_espera_memoria++;
    logEvento("ESPERA_MEMORIA", "PEATON_NS", ram_usada_actual, "ADMISION_RECHAZADA");
    return;
  }

  monitorCrucePeatonalOcupado = true;
  cruceNSActivo = true;
  cruceEOActivo = false;
  inicioCruceNS = tiempoActual;

  digitalWrite(peat_NS_rojo, LOW);
  digitalWrite(peat_NS_verde, HIGH);
  digitalWrite(peat_EO_verde, LOW);
  digitalWrite(peat_EO_rojo, HIGH);

  logEvento("INICIO_CRUCE", "PEATON_NS", ram_usada_actual, "ADMISION_OK");
}

void iniciarCruceEO(unsigned long tiempoActual) {
  if (monitorCrucePeatonalOcupado) {
    return;
  }

  if (!admitirProceso(pPeatonEO)) {
    eventos_espera_memoria++;
    logEvento("ESPERA_MEMORIA", "PEATON_EO", ram_usada_actual, "ADMISION_RECHAZADA");
    return;
  }

  monitorCrucePeatonalOcupado = true;
  cruceEOActivo = true;
  cruceNSActivo = false;
  inicioCruceEO = tiempoActual;

  digitalWrite(peat_EO_rojo, LOW);
  digitalWrite(peat_EO_verde, HIGH);
  digitalWrite(peat_NS_verde, LOW);
  digitalWrite(peat_NS_rojo, HIGH);

  logEvento("INICIO_CRUCE", "PEATON_EO", ram_usada_actual, "ADMISION_OK");
}

void actualizarCrucePeatonal(unsigned long tiempoActual) {
  if (cruceNSActivo && (tiempoActual - inicioCruceNS >= tPeaton)) {
    digitalWrite(peat_NS_verde, LOW);
    digitalWrite(peat_NS_rojo, HIGH);

    solicitud_NS = false;
    bloqueo_NS = false;
    cruceNSActivo = false;
    monitorCrucePeatonalOcupado = false;

    liberarProceso(pPeatonNS);
    logEvento("FIN_CRUCE", "PEATON_NS", ram_usada_actual, "RECURSOS_LIBERADOS");
  }

  if (cruceEOActivo && (tiempoActual - inicioCruceEO >= tPeaton)) {
    digitalWrite(peat_EO_verde, LOW);
    digitalWrite(peat_EO_rojo, HIGH);

    solicitud_EO = false;
    bloqueo_EO = false;
    cruceEOActivo = false;
    monitorCrucePeatonalOcupado = false;

    liberarProceso(pPeatonEO);
    logEvento("FIN_CRUCE", "PEATON_EO", ram_usada_actual, "RECURSOS_LIBERADOS");
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
