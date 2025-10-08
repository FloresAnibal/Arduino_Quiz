/**
* PROYECTO ARDUINO QUIZ (MÚLTIPLE CHOICE)
* ================================================================================
* 
* Este código implementa un sistema de quiz interactivo para Arduino que utiliza 
* una pantalla LCD 16x2 con interfaz I2C y tres botones para la navegación. 
* El sistema presenta preguntas de opción múltiple (A, B, C) que el usuario puede 
* responder usando los botones de navegación (arriba/abajo) y confirmación (OK). 
* Al finalizar, muestra el puntaje obtenido con retroalimentación personalizada 
* según el rendimiento.
*
* ================================================================================
* 
* Instituto Superior Espíritu Santo
* Profesorado de Educación Secundaria en Informática
* 2025
* 
* Robótica Áulica II - Matías Cardozo
* 
* 4° Año
* 
* Aníbal Flores
* Irupé Lloret
* Walter Schnablegger
*/

// LIBRERÍAS
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// ===== CONFIGURACIÓN DE HARDWARE =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pines de los botones
const int BOTON_OK = 2;
const int BOTON_ARRIBA = 3;
const int BOTON_ABAJO = 4;

// ===== CONFIGURACIÓN DEL QUIZ =====
// Respuestas correctas (0=A, 1=B, 2=C)
const int RESPUESTAS_CORRECTAS[] = {
  1,   // Pregunta 1: Opción B
  0,   // Pregunta 2: Opción A
  2    // Pregunta 3: Opción C
  // Agregar más preguntas aquí
};

//  Calcula cuántos elementos (preguntas) tiene el Array
const int TOTAL_PREGUNTAS = sizeof(RESPUESTAS_CORRECTAS) / sizeof(RESPUESTAS_CORRECTAS[0]);

// Opciones de respuesta para mostrar
const String OPCIONES[] = {"Opcion A", "Opcion B", "Opcion C"};
const int TOTAL_OPCIONES = 3;

// ===== CONFIGURACIÓN DE PANTALLA =====
const bool MOSTRAR_NUMERO_PREGUNTA = true;
const bool MOSTRAR_RESPUESTA_CORRECTA = true;
const bool MOSTRAR_RETROALIMENTACION = true;
const bool MOSTRAR_PORCENTAJE = true;

// ===== MENSAJES =====
const String MSJ_INICIO = "QUIZ ARDUINO";
const String MSJ_INSTRUCCION = "Incorrecta";
const String MSJ_PORCENTAJE_ALTO = "EXCELENTE!";
const String MSJ_PORCENTAJE_MEDIO = "BIEN HECHO";
const String MSJ_PORCENTAJE_BAJO = "CASI LO LOGRAS!";
const String MSJ_FINAL1 = "QUIZ TERMINADO";
const String MSJ_FINAL2 = "";

// ===== ESTADOS DEL JUEGO =====
enum Estado {
  JUGANDO,
  MOSTRANDO_RESULTADO,
  QUIZ_TERMINADO
};

// ===== VARIABLES DEL JUEGO =====
Estado estadoActual = JUGANDO;
int preguntaActual = 0;
int opcionSeleccionada = 0;
int puntajeTotal = 0;
bool esperandoBoton = true;

// Carácter de flecha personalizado
byte flechaChar[] = {B00000, B00100, B00110, B11111, B00110, B00100, B00000, B00000};

// ===== CONFIGURACIÓN INICIAL =====
void setup() {
  // Configurar botones
  pinMode(BOTON_ARRIBA, INPUT_PULLUP);
  pinMode(BOTON_ABAJO, INPUT_PULLUP);
  pinMode(BOTON_OK, INPUT_PULLUP);
  
  // Inicializar pantalla
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, flechaChar);
  
  // Iniciar el juego
  mostrarPantallaBienvenida();

  lcd.clear();
  centrarTexto("Elige la opcion", 0);
  centrarTexto(MSJ_INSTRUCCION, 1);
  delay(2000);

  iniciarJuego();
}

// ===== BUCLE PRINCIPAL =====
void loop() {
  switch (estadoActual) {
    case JUGANDO:
      manejarSeleccionRespuesta();
      break;
    case MOSTRANDO_RESULTADO:
      esperarContinuar();
      break;
    case QUIZ_TERMINADO:
      esperarReinicio();
      break;
  }
}

// ===== FUNCIONES DE PANTALLA =====
void mostrarPantallaBienvenida() {
  lcd.clear();
  centrarTexto(MSJ_INICIO, 0);
  delay(1000);
  centrarTexto("Cargando", 1);

  for(int i = 0; i < 3; i++){
    delay(500);
    lcd.print(".");
  }
  delay(1000);
}

void mostrarPregunta() {
  if (MOSTRAR_NUMERO_PREGUNTA) {
    lcd.clear();
    String numeroPregunta = "Pregunta " + String(preguntaActual + 1) + "/" + String(TOTAL_PREGUNTAS);
    centrarTexto(numeroPregunta, 0);
    delay(1500);
  }
  mostrarOpciones();
}

void mostrarOpciones() {
  lcd.clear();
  
  // Calcular qué opciones mostrar
  int primeraOpcion = 0;
  if (opcionSeleccionada <= 1) {
    primeraOpcion = 0;
  }else{
    primeraOpcion = 1;
  }
  int segundaOpcion = primeraOpcion + 1;
  
  // Mostrar primera línea
  lcd.setCursor(0, 0);
  if (opcionSeleccionada == primeraOpcion) {
    lcd.write(byte(0)); // Mostrar flecha
  } else {
    lcd.print(" ");
  }
  lcd.print(" ");
  lcd.print(OPCIONES[primeraOpcion]);
  
  // Mostrar segunda línea
  lcd.setCursor(0, 1);
  if (opcionSeleccionada == segundaOpcion) {
    lcd.write(byte(0)); // Mostrar flecha
  } else {
    lcd.print(" ");
  }
  lcd.print(" ");
  lcd.print(OPCIONES[segundaOpcion]);
}

void mostrarResultado(bool esCorrecta) {
  lcd.clear();
  
  if (esCorrecta) {
    centrarTexto("CORRECTO!", 0);
  } else {
    centrarTexto("INCORRECTO", 0);
    
    if (MOSTRAR_RESPUESTA_CORRECTA) {
      lcd.setCursor(0, 1);
      switch(RESPUESTAS_CORRECTAS[preguntaActual]) {
        case 0: lcd.print("Era: Opcion A");
        break;
        case 1: lcd.print("Era: Opcion B");
        break; 
        case 2: lcd.print("Era: Opcion C");
        break;
      }  
    }
  }
  
  delay(3000);
  mostrarMensajeContinuar();
}

void mostrarResultadoFinal() {
  lcd.clear();
  centrarTexto(MSJ_FINAL1, 0);
  
  if (MSJ_FINAL2) {
    centrarTexto(MSJ_FINAL2, 1);
  }
  delay(2000);
  
  // Calcular porcentaje
  int porcentaje = (puntajeTotal * 100) / TOTAL_PREGUNTAS;
  
  lcd.clear();
  if (MOSTRAR_PORCENTAJE) {
    String porcentajeTexto = "Aciertos: " + String(porcentaje) + "%";
    centrarTexto(porcentajeTexto, 0);
    delay(1500);
  }

  if (MOSTRAR_RETROALIMENTACION) {
    String mensaje = obtenerMensajeRetroalimentacion(porcentaje);
    centrarTexto(mensaje, 1);
    delay(3000);
    lcd.setCursor(0, 1);
    lcd.print("OK para terminar");
  }

}

void mostrarMensajeContinuar() {
  lcd.clear();
  centrarTexto("Presiona OK", 0);
  centrarTexto("para continuar", 1);
}

// ===== FUNCIONES DE LÓGICA =====
void iniciarJuego() {
  preguntaActual = 0;
  opcionSeleccionada = 0;
  puntajeTotal = 0;
  estadoActual = JUGANDO;
  esperandoBoton = true;
  mostrarPregunta();
}

void manejarSeleccionRespuesta() {
  if (!esperandoBoton) return;
  
  if (digitalRead(BOTON_ARRIBA) == LOW) {
    delay(200);
    cambiarOpcion(-1);
  }
  
  if (digitalRead(BOTON_ABAJO) == LOW) {
    delay(200);
    cambiarOpcion(1);
  }
  
  if (digitalRead(BOTON_OK) == LOW) {
    delay(200);
    procesarRespuesta();
  }
}

void cambiarOpcion(int direccion) {
  opcionSeleccionada += direccion;
  
  // Mantener la opción dentro de los límites
  if (opcionSeleccionada < 0) {
    opcionSeleccionada = TOTAL_OPCIONES - 1;
  } else if (opcionSeleccionada >= TOTAL_OPCIONES) {
    opcionSeleccionada = 0;
  }
  
  mostrarOpciones();
}

void procesarRespuesta() {
  bool esCorrecta = false;
  if (opcionSeleccionada == RESPUESTAS_CORRECTAS[preguntaActual]) {
    esCorrecta = true;
  }
  if (esCorrecta) {
    puntajeTotal++;
  }
  
  mostrarResultado(esCorrecta);
  estadoActual = MOSTRANDO_RESULTADO;
  esperandoBoton = false;
}

void siguientePregunta() {
  preguntaActual++;
  
  if (preguntaActual >= TOTAL_PREGUNTAS) {
    mostrarResultadoFinal();
    estadoActual = QUIZ_TERMINADO;
  } else {
    opcionSeleccionada = 0;
    estadoActual = JUGANDO;
    esperandoBoton = true;
    mostrarPregunta();
  }
}

void esperarContinuar() {
  if (digitalRead(BOTON_OK) == LOW) {
    delay(200);
    siguientePregunta();
  }
}

void esperarReinicio() {
  if (digitalRead(BOTON_OK) == LOW) {
    delay(200);
    mostrarPantallaBienvenida();
    iniciarJuego();
  }
}

// ===== FUNCIONES AUXILIARES =====
void centrarTexto(String texto, int fila) {
  int espacios = (16 - texto.length()) / 2;
  lcd.setCursor(espacios, fila);
  lcd.print(texto);
}

String obtenerMensajeRetroalimentacion(int porcentaje) {
  if (porcentaje >= 80) return MSJ_PORCENTAJE_ALTO;
  if (porcentaje >= 60) return MSJ_PORCENTAJE_MEDIO;
  return MSJ_PORCENTAJE_BAJO;
}
