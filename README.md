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
 
# Explicacion del codigo:

Este código controla un sistema de semáforos con paso peatonal usando Arduino. El sistema tiene dos semáforos vehiculares: uno para la vía norte-sur (Semáforo A) y otro para oriente-occidente (Semáforo B), además de dos semáforos peatonales con botones de cruce.

Primero se definen los pines donde están conectadas las luces de cada semáforo (rojo, amarillo y verde) para ambas direcciones. También se asignan pines para los semáforos peatonales, que indican cuándo los peatones pueden cruzar (verde) o deben esperar (rojo).

El código incluye dos botones, uno para cada cruce peatonal. Cuando un peatón presiona el botón, el sistema registra una solicitud de cruce para activarlo en el momento adecuado del ciclo del semáforo.

También se utilizan variables de control para manejar el estado actual del semáforo (verde o amarillo de cada vía), evitar que los botones se activen varias veces seguidas y controlar el tiempo de cada fase.

Los tiempos del sistema están definidos en milisegundos:

Verde: 5 segundos

Amarillo: 2 segundos

Cruce peatonal: 4 segundos

Finalmente, en la función setup() se configuran todos los pines de los LEDs como salidas y los botones como entradas con resistencia INPUT_PULLUP, lo que permite detectar cuándo un peatón presiona el botón sin necesidad de resistencias externas.
      
 
# Mockup

https://drive.google.com/file/d/1QjMIdgousDISngu440odbuZb5858p7xv/view?usp=drivesdk
 
# Diagrama de Conexiones:
 
# Instrucciones de instalación:

# Roadmap:
