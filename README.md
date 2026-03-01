# Control de Semáforo Inteligente con Cruce Peatonal

Sistema de control de tráfico basado en Arduino que permite la interrupción segura por parte de peatones mediante botones con lógica de bloqueo temporal

# Características Principales

Elementos Principales:
- Semaforo A 
- Semaforo B
- Semáforo Peatonal (Botón A)
- Semáforo Peatonal (Botón B)

Funcionamiento:

| Estado | Semáforo A | Semáforo B | Duración |
| :--- | :--- | :--- | :--- |
| 0 | Verde | Rojo | 5s |
| 1 | Amarillo | Rojo | 2s |
| 2 | Rojo | Verde | 5s |
| 3 | Rojo | Amarillo | 2s |
    
- Secuencia Semáforo Peatonal:
    - Se activa luz peatonal verde
    - Se mantiene 4 segundos
    - Se apaga
    - Se limpia la solicitud
    - Se desbloquea el botón

- Restricciones:
    - Al pulsar el botón, el semaforo cambiara principalmente 5 segundos despues. Solo cambiara antes de los 5 segundos si el ciclo actual ya estaba por cambiar.
    - Solo se mantendrá una solicitud a la vez. Si una persona pulsa el botón, se bloquearan los otros hasta que la solicitud termine.
 
# Diagrama de Conexiones:
 
# Instrucciones de instalación:

# Roadmap:
