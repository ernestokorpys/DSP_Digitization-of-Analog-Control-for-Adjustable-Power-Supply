
// ---------------------------------------------

// La tension de salida, que es la variable controlada, pasa por un sensor, cuya salida 
// es adquirida por el canal AN3/RB3. Para una tension de 5V de salida del Buck, 
// la senial a la salida del sensor, debe ser de 4V. 
// Las conversiones del ADC son iniciadas por cada periodo PWM.  
// La finalizacion de cada conversion produce una interrupcion, cuya RSI calcula la accion de
// control y actualiza el ciclo util. 
// La salida RF0(LED1) produce un pulso en alto cuya duracion corresponde al tiempo que demora
// en calcularse la accion de control.
// El DSC opera con un cristal externo de 7.15909MHz, dato esencial para calcular el valor
// a cargar en el registro de periodo del modulo PWM que determina la Fpwm y tambi�n
// la interrupci�n para el c�lculo de la acci�n de control.  
// ****************************************************************************************


// Inclusion de archivos
// ---------------------------------------------------------------------------------------
#include "p30f4011.h"			      // Incluye etiquetas estandar del DSC.
#include "Configuraciones.h"    // Incluye librerias de configuracion correspondiente a:
								                // UART (NO se usa), PWMmotor, ADC, PUERTOS 
#include "LibreriaQ.h"			    // Incluye libreria para el manejo de datos en formato Q.

// Configuracion de fusibles
// ---------------------------------------------------------------------------------------
_FOSC(CSW_FSCM_OFF & XT_PLL16);   // Para el oscilador utiliza cristal externo con PLLx16
_FWDT(WDT_OFF);					  // No se usa el perro guardian (WDT)
_FBORPOR(PBOR_OFF & BORV_42 & PWRT_64 & MCLR_EN & RST_PWMPIN & PWMxH_ACT_LO & PWMxL_ACT_HI); 
// Con la configuracion anterior:
// BOR deshabilitado. Pines PWMxH activos en bajo y pines PWMxL activos en alto

// Etiquetas
// ---------------------------------------------------------------------------------------
// El pin LATF0 sirve para verificar el funcionamiento de la interrupcion y ademas 
// permite medir el tiempo que dura el calculo de la accion de control. El led asociado 
// es LED1 y se enciende con ON (uno).
// El led LED3 indica que se llen� el buffer.

#define ON			1				// Enciende
#define OFF			0          		// Apaga
#define MODE 		LATFbits.LATF0  // Pin asociado al led L1. Se enciende con ON (uno).
#define LOAD 		LATFbits.LATF1  // Pin asociado al led L2. Se enciende con ON (uno).
#define LED3 		LATFbits.LATF4  // Pin asociado al led L3. Se enciende con ON (uno).
#define LED4 		LATFbits.LATF5  // Pin asociado al led L4. Se enciende con ON (uno).
#define BOTON		PORTFbits.RF6	// Pin asociado al pulsador (configurado como entrada digital)

#define TamBuffer 	900				// Tamanio del buffer de almacenamiento de variables.

// --------------------------------ATENCION!!!!!----------------------------------------
// "N_Tpwm" permite alterar la frecuencia de muestreo. En realidad N_Tpwm 
// es el No de periodos PWM (Tpwm) con que se calcula y actualiza la accion de control.  
#define N_Tpwm 		1		// Rampa de 3s, para P, PI, PI-D Y Deadbeat (N_Tpwm = 60)16.

// --------------------------------ATENCION!!!!!----------------------------------------
// "delta_Ref" Este valor permite alterar el incremento de la referencia, 
// para generar la rampa. 
// Calculo: delta_Ref = (N_Tpwm*Tpwm)/Tiempo; Tpwm = 1/Fpwm = 0.001s; Tiempo = Tiempo 
// en el que se desea que la rampa alcance 1pu.
#define delta_Ref	0.002	// Rampa de 2s, para P, PI, PI-D Y Deadbeat (N_Tpwm = 16).						

// Declaracion e inicializacion de variables y constantes en punto fijo (formato Q12)
// ---------------------------------------------------------------------------------------
NumeroQ KbQ12 = 20480;		// Constante para normalizacion y pasaje a formato Q12 de la
                            // la tension medida.KbQ12 es equivalente al KN de los TPs.
// Para esta constante se considera que la salida del sensor es ajustada para que a 
// tension nominal tengamos 4V, ya que el limite maximo para la entrada del ADC es 5V.
// Entonces, considerando 1 p.u. = 4V, en Q12 la constante es KbQ12 = 20480
// Calculo: 
// Para el AD: 4V*1023/5V = 818,4. 
// KbQ12_dec = 2^12/818,4 = 5. 
// KbQ12 = 5*2^12 = 20480 (en Q12).
NumeroQ fm = FloatToFix10(83.333);			// Frecuencia de muestreo en formato Q10
NumeroQ LimiteMaximo = FloatToFix12(0.99);	// Este valor es el limite maximo de la accion 
											// de control (cilo util). 
											// En formato Q12: 2^12*0.99 = 4055.
NumeroQ Referencia = FloatToFix12(0); 		// Referencia interna.
NumeroQ REF0 = FloatToFix12(0); 			// Valor de la referencia al 0% (0pu).  
NumeroQ REF20 = FloatToFix12(0.20); 		// Valor de la referencia al 50% (0.5pu).  
NumeroQ REF50 = FloatToFix12(0.50); 		// Valor de la referencia al 75% (0.75pu). 
NumeroQ REF99 = FloatToFix12(0.99);			// Valor de la referencia al 99% (1pu)										
NumeroQ TensionVo = 0;						// [V(k)] = 0, Tension actual.
NumeroQ TensionVo_km1 = 0;					// [V(k-1)] = 0, Tension anterior.
NumeroQ uk = 0;								// [u(k)] = 0, Accion de control total (ciclo util).
NumeroQ up_k = 0;							// [up(k)] = 0, Accion de control proporcional.
NumeroQ ud_k = 0;							// [ud(k)] = 0, Accion de control derivativa.
NumeroQ fm_error_v = 0;						// Accion derivativa a partir de la senial de salida
NumeroQ upi_k = 0;							// [upi(k)] = 0, Accion de control del PI.
NumeroQ upi_km1 = 0;						// [upi(k-1)] = 0, Accion de control anterior del PI.
NumeroQ upid_k = 0;							// [upid(k)] = 0, Accion de control del PID.
NumeroQ error_k = 0;						// [error(k)] = 0, Error actual (Referencia - Tension)
NumeroQ error_km1 = 0; 						// [error(k-1)] = 0, Error anterior
NumeroQ error_v = 0;						// [error_v(k)] = 0, Error de velocidad.
NumeroQ convertido = 0;						// Almacena el valor convertido por el AD.
NumeroQ udb_k = 0;							// Accion de control deadbeat.
NumeroQ udb_km1 = 0;						// Accion de control deadbeat anterior.

// Parametros del controlador: 
// --------------------------
// --------------------------------ATENCION!!!!!-----------------------------------
// Los siguientes parametros deben ser reemplazados por los valores obtenidos  
// en el proyecto del compensador. 

// Para compensador PI: 
NumeroQ aKpi = FloatToFix12(-0.5198);   //(-0.0715);	// Producto a*Kpi 
NumeroQ Kpi = FloatToFix12(0.6687);     //(0.14793);	    // Ganancia del PI 

// Para compensador deadbeat: 
NumeroQ a = FloatToFix12(4.738);		// ganancia a de deadbeat
NumeroQ b = FloatToFix12(-2.079);	    // Ganancia b de deadbeat 

// Para parte derivativa:
NumeroQ Kdd = FloatToFix12(4);		   // Ganancia del derivador

// Para parte parte proporcional: 
NumeroQ Kpp = FloatToFix12(10.0);	   // Ganancia del compensador proporcional


// Declaracion e inicializacion de variables y constantes de otras variables
// ---------------------------------------------------------------------------------------
int Var_buffer1[TamBuffer];			// Buffer 1 de variables para graficar en Matlab.
//int Var_buffer2[TamBuffer];		// Buffer 2 de variables para graficar en Matlab.
int j = 100;  						// Variable auxiliar para cambio de referencia.
int ContadorBuffer = 0;				// Contador para incremento de llenado de los buffers.
int counter_ref1 = 0;				// Contador para variar la referencia.
int counter_ref2 = 0;				// Contador para variar la referencia.
int counter_1 = 0;					// Contador para alterar la frecuencia de muestreo.

// Declaracion de interrupcion por finalizacion de conversion AD
// ---------------------------------------------------------------------------------------
void __attribute__((interrupt, auto_psv)) _ADCInterrupt(void);  

// --------------------------------------------------------------------------------------- 
// FUNCION PRINCIPAL
// ---------------------------------------------------------------------------------------
int main (void)
{
  Configura_PUERTOS();  // Configura todo el puerto D como salida, utiliza RD2 como "testigo".
  Configura_ADC();      // Configura ADC para efectuar conversion por AN0/RB0. Cada 
						// vez que finaliza la conversion, interrumpe a la CPU.
  Configura_PWM();		// Configura PWMmotor para proporcionar una salida con Fpwm = 50 kHz 
						// por la salida PWM1L/RE0. Tambien se activa el disparo de un 
						// evento especial, que en este caso corresponde al inicio de la 
						// conversion en cada periodo PWM (prescaler 1:1) cada vez que 
						// PTMR = 0x00FF.
  LED1 = OFF;			// Apaga L1.
  LED2 = OFF;			// Apaga L2.
  LED3 = OFF;			// Apaga L3.
  LED4 = OFF;			// Apaga L4.
  Referencia = REF0;	// Referencia en cero.

  while(1){
  	if(BOTON == 0) 
		LED4=1;
  	else 
		LED4=0;
  };				    // Bucle infinito para esperar interrupciones.
}

// ---------------------------------------------------------------------------------------
// Rutina de Servicio a la Interrupcion por final de conversion AD
// ---------------------------------------------------------------------------------------
void __attribute__((interrupt, auto_psv)) _ADCInterrupt(void)
{
counter_1++;
if(counter_1 == N_Tpwm)
{
  LED1 = ON; 				// Cada vez que finaliza la conversion, RD0 en alto (apaga L1).				
  convertido = ADCBUF0; 	// Almacena resultado de la conversion en el registro "convertido". 
							// La conversion AD corresponde a la medicion de la tension.

  TensionVo = MulFix12(convertido,KbQ12);	// Normaliza la conversion y transforma a Q12
											// la tension de salida. 

// Variacion de referencia en rampa ascendente
// ---------------------------------------------------------------------------------------------------
// --------------------------------ATENCION!!!!!------------------------------------------------------
// Esto debe modificarse para obtener la variacion de referencia solicitada en la guia de laboratorio. 
  counter_ref1++;
  counter_ref2++;

  if(Referencia < REF50)								  // Incrementa referencia,
	{ Referencia = Referencia + FloatToFix12(delta_Ref);} // si todavia no alcanzo el 50%.
  else
	{ Referencia = REF99;								  // Referencia al 99%.
      LED2 = ON; }										  // Enciende L2 para indicar que
														  // se alcanzo el 99% de la ref.
// Calculo del error actual
// ---------------------------------------------------------------------------------------
  error_k = Referencia - TensionVo; 	

// Calculo de las acciones de control
// ---------------------------------------------------------------------------------------

// Accion PI, aprox. Forward: upi(k) = upi(k-1) + Kpi*e(k) + a*Kpi*e(k-1)
  upi_k = AddFix(AddFix(MulFix12(error_k,Kpi),MulFix12(error_km1,aKpi)), upi_km1);  

// Valores anteriores para el proximo periodo de muestreo:
// ---------------------------------------------------------------------------------------
  error_km1 = error_k;
  upi_km1 = upi_k;

  uk = upi_k;		 // PI

// Limitacion de la accion de control por valor negativo para no obtener d(t) negativo
// ---------------------------------------------------------------------------------------
  if(uk<0)						// Si es negativa,
	{ uk=0;}					// iguala a "0".

// Calculo del ciclo util a partir de la accion de control
// ---------------------------------------------------------------------------------------
// Almacena el valor en PDC3, registro ciclo util "d(t)" del modulo PWM
// d(t) = (PDCx/2)/(PTPER+1)
// MulFix12 multiplica a uk en formato Q12 por (PTPER+1)
// si uk = 1pu = 4096 entonces MulFix12 = (PTPER+1)
// El valor en PDC3 debe estar en Q0

  PDC3 = MulFix12(uk,(PeriodoPWM + 1)) << 1;  //corrimiento << 1 equivale a multiplicar por 2

// Finaliza el calculo de la acci�n de control

  LED1 = OFF; 	// Pone pin RD0 en bajo para indicar finalizacion del calculo (enciende L1).		

// Almacena en buffer las variables que se desean exportar
// --------------------------------------------------------------------------------
// --------------------------------ATENCION!!!!!-----------------------------------
// Dejar sin comentar las lineas de las variable que se desean almacenar.

  if (ContadorBuffer < TamBuffer)
     {
	   Var_buffer1[ContadorBuffer] = uk;
//	   Var_buffer2[ContadorBuffer] = Referencia;

//	   Var_buffer1[ContadorBuffer] = error_k;
//	   Var_buffer2[ContadorBuffer] = Velocidad;

//	   Var_buffer1[ContadorBuffer] = Referencia;
//     Var_buffer2[ContadorBuffer] = uk;

//	   Var_buffer1[ContadorBuffer] = upi_k;			// Solo para control PI-D
//	   Var_buffer2[ContadorBuffer] = ud_k; 			// Solo para control PI-D

	   ContadorBuffer ++;}
  else
     { LED3 = ON;}			// Enciende el led LED3 para indicar que se llenaron los buffers.

  counter_1 = 0;			// Borra el contador de muestreo, para utilizarlo nuevamente.
}	
  IFS0bits.ADIF = 0x00;     // Resetea el flag de interrupcion del AD.
}	
// FIN DE LA RUTINA DE INTERRUPCION --------------------------------------------------


