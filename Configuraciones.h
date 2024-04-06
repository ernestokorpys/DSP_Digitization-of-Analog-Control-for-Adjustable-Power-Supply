/********************************************************************/
/* CONFIGURACIÓN DE LOS MÓDULOS UTILIZADOS EN EL PROGRAMA PRINCIPAL */
/* Contiene funciones que son llamadas en el main                   */
/********************************************************************/

//****************************************************************************************************
// Configuración módulo UART (Para transmisión de datos a la PC)
//****************************************************************************************************
void Configura_UART(void)
{
/****************************************************************************************************/
/*    Procedimiento de configuracion sugerido por la hoja de datos:                                 */
/*                                                                                                  */
/*    Steps to follow when setting up a transmission:                                               */
/* 1. Initialize the UxBRG register for the appropriate baud rate (Section 19.3 “UART Baud          */
/*    Rate Generator (BRG)”).                                                                       */
/* 2. Set the number of data bits, number of Stop bits, and parity selection by writing to the      */
/*    PDSEL<1:0> (UxMODE<2:1>) and STSEL (UxMODE<0>) bits.                                          */
/* 3. If transmit interrupts are desired, set the UxTXIE control bit in the corresponding Interrupt */
/*    Enable Control register (IEC). Specify the interrupt priority for the transmit interrupt using*/
/*    the UxTXIP<2:0> control bits in the corresponding Interrupt Priority Control register (IPC).  */
/*    Also, select the Transmit Interrupt mode by writing the UTXISEL (UxSTA<15>) bit.              */
/* 4. Enable the UART module by setting the UARTEN (UxMODE<15>) bit.                                */
/* 5. Enable the transmission by setting the UTXEN (UxSTA<10>) bit, which will also set the         */
/*    UxTXIF bit. The UxTXIF bit should be cleared in the software routine that services the        */
/*    UART transmit interrupt. The operation of the UxTXIF bit is controlled by the UTXISEL         */
/*    control bit.                                                                                  */
/* 6. Load data to the UxTXREG register (starts transmission). If 9-bit transmission has been       */
/*    selected, load a word. If 8-bit transmission is used, load a byte. Data can be loaded into    */
/*    the buffer until the UxTXBF status bit (UxSTA<9>) is set.                                     */
/****************************************************************************************************/

                                // U1BRG = Fcy / (16 * BaudRate) - 1
U1BRG = 0x009B;                 // Transmite a 9600 Baudios
U1MODEbits.PDSEL = 0x00;        // Datos de 8 bits sin paridad
U1MODEbits.STSEL = 0x00;        // Un bit de stop
IEC0bits.U1TXIE = 0x01;         // Habilita interrupcion de transmicion
U1STAbits.UTXISEL = 0x01;       // Seleccion de modo de interrupcion de transmicion
IPC2bits.U1TXIP = 0x01;         // Lo ponemos en la prioridad mas baja
U1MODEbits.UARTEN = 0x01;       // Bit de habilitacion, 1 = habilitado, 0 = deshabilitado
U1STAbits.UTXEN = 0x01;         // Bit para habilitar la transmision: 1 = habalitado, 0 = deshabilitado
U1TXREG = 0xFF;
}

//****************************************************************************************************
// Configuración módulo PWM_MOTOR
//****************************************************************************************************
// PWM Period Calculation for Free Running Count Mode (up counting mode) PTMOD = 00
#define PeriodoPWM 571        // Periodo del PWM calculado con: PTPER = [Fcy/(Fpwm*Prescaler)]-1
							  // Datos: Fcy = 7.15909MHz*16/4 = 28.63636 MHz; Fpwm = 50000Hz; Prescaler = 1
							  // PTPER = 28636360/(50000*1)-1 = 571
// Duty Cycle Calculation for Free Running: Duty Cycle = (PDCx/2)/(PTPER+1)
// Para Duty Cycle 100% el valor de PDCx es 2x(PTPER+1)=1142; Para Duty Cycle de 50% PDCx=PTPER+1=571
void Configura_PWM(void)
{
PTPERbits.PTPER = PeriodoPWM;   // Primero carga el periodo PTPER = PeriodoPWM

// Registro de control PWMCON1:
PWMCON1bits.PEN1L = 0x00;       // Pin PWM 1, parte baja: 1 = habilitado, 0 = deshabilitado
PWMCON1bits.PEN2L = 0x00;       // Pin PWM 2, parte baja: 1 = habilitado, 0 = deshabilitado
PWMCON1bits.PEN3L = 0x01;       // Pin PWM 3, parte baja: 1 = habilitado, 0 = deshabilitado
PWMCON1bits.PEN1H = 0x00;       // Pin PWM 1, parte alta: 1 = habilitado, 0 = deshabilitado
PWMCON1bits.PEN2H = 0x00;       // Pin PWM 2, parte alta: 1 = habilitado, 0 = deshabilitado
PWMCON1bits.PEN3H = 0x00;       // Pin PWM 3, parte alta: 1 = habilitado, 0 = deshabilitado
PWMCON1bits.PMOD1 = 0x01;       // 1 = modo independiente, 0 = modo complementado
PWMCON1bits.PMOD2 = 0x01;       // 1 = modo independiente, 0 = modo complementado
PWMCON1bits.PMOD3 = 0x01;       // 1 = modo independiente, 0 = modo complementado

// Registro de control PWMCON2:
PWMCON2bits.UDIS   = 0x00;      // Actualizacion de registros: 0 = habilitado, 1 = deshabilitado
PWMCON2bits.OSYNC  = 0x00;      // Sincronizacion del Output-Override
PWMCON2bits.IUE    = 0x00;      // Actualizacion inmediata del PDC: 1 = habilitada, 0 = deshabilitado

////////////////////////////////////////////////////CAMBIO
PWMCON2bits.SEVOPS = 0x04;      // Postscaler del Trigger del generador de eventos especiales (1:5)
////////////////////////////////////////////////////CAMBIO DE 00 a 04

// Registro de control DTCON1 (Tiempo Muerto):
DTCON1bits.DTA   = 0x00;        // Valor del tiempo muerto
DTCON1bits.DTAPS = 0x00;        // Prescaler para el tiempo muerto

// Registro de configuracion PTCON (Base de Tiempo):
PTCONbits.PTMOD  = 0x00;        // Bits de configuracion de la base de tiempo en free running mode. 
PTCONbits.PTCKPS = 0x00;        // Prescaler de entrada para la base de tiempo
PTCONbits.PTOPS  = 0x05;        // Postscaler de salida
PTCONbits.PTSIDL = 0x00;        // Si la base de tiempo funciona en "Idle Mode"
PTCONbits.PTEN   = 0x01;        // Habilitacion de la base de tiempo

// Registro de configuracion SEVTCMP (Eventos Especiales)
SEVTCMPbits.SEVTCMP = 0x00FF;   // Valor de la base de tiempo PTMR al cual se produce el evento especial (0x000F)
SEVTCMPbits.SEVTDIR = 0x00;     // Direccion de la base de tiempo PTMR al cual se produce el evento especial (contando hacia arriba)

// Registro de configuracion interrupciones IEC2 (Base de Tiempo):
IEC2bits.PWMIE = 0x00;          // No habilita las interrupciones
}


//****************************************************************************************************
// Configuración módulo ADC_CONVERTER
//****************************************************************************************************

/******************************************************************************************/
/* Procedimiento de configuracion sugerido por la hoja de datos:                          */
/*                                                                                        */
/* The following steps should be followed for performing an A/D conversion:               */
/* 1. Configure the A/D module                                                            */
/* • Select port pins as analog inputs ADPCFG<15:0>                                       */
/* • Select voltage reference source to match expected range on analog inputs             */
/* ADCON2<15:13>                                                                          */
/* • Select the analog conversion clock to match desired data rate with processor clock   */
/* ADCON3<5:0>                                                                            */
/* • Determine how many S/H channels will be used ADCON2<9:8> and ADPCFG<15:0>            */
/* • Determine how sampling will occur ADCON1<3> and ADCSSL<15:0>                         */
/* • Determine how inputs will be allocated to S/H channels ADCHS<15:0>                   */
/* • Select the appropriate sample/conversion sequence ADCON1<7:0> and                    */
/* ADCON3<12:8>                                                                           */
/* • Select how conversion results are presented in the buffer ADCON1<9:8>                */
/* • Select interrupt rate ADCON2<5:9>                                                    */
/* • Turn on A/D module ADCON1<15>                                                        */
/* 2. Configure A/D interrupt (if required)                                               */
/* • Clear ADIF bit                                                                       */
/* • Select A/D interrupt priority                                                        */
/******************************************************************************************/

void Configura_ADC(void)
{
// Configuración de las entradas analógicas (ADPCFG):
ADPCFGbits.PCFG0  = 0x01;   // AN0/RB0 como entrada analógica.
ADPCFGbits.PCFG1  = 0x01;	// AN1/RB1 como I/O digital.
ADPCFGbits.PCFG2  = 0x01;	// AN2/RB2 como I/O digital.
ADPCFGbits.PCFG3  = 0x01;	// AN3/RB3 como I/O digital.
ADPCFGbits.PCFG4  = 0x00;	// AN4/RB4 como I/O digital.
ADPCFGbits.PCFG5  = 0x01;	// AN5/RB5 como I/O digital.
ADPCFGbits.PCFG6  = 0x01;	// AN6/RB6 como I/O digital.
ADPCFGbits.PCFG7  = 0x01;	// AN7/RB7 como I/O digital.
ADPCFGbits.PCFG8  = 0x01;	// AN8/RB8 como I/O digital.

// Registro de configuración del ADC (ADCON1):
ADCON1bits.SSRC   = 0x03;	// Fuente de disparo de la conversión AD (evento especial): 
						    // cada vez se completa un intervalo PWM (formado por , finaliza 
							// muestro e inicia la conversión AD.(Ojo afectado por postscaler de PWCON2 y comparación en SEVTCMP)
ADCON1bits.FORM   = 0x00;   // Formato de salida de la conversión: 00 = Integer (DOUT = 0000 00dd dddd dddd) 
ADCON1bits.ADSIDL = 0x00;   // Continúa funcionamieneto del convresor AD en modo Idle.
ADCON1bits.ASAM   = 0x01;	// La captura comienza automáticamente después de la última conversión

// Registro de selección de de canal de entrada (ADCHS):
ADCHSbits.CH0SA   = 0x04;	// Para la entrada CH0+, selecciona la entrada AN4/RB4.
ADCHSbits.CH0NA   = 0x00;	// Para la entrada CH0-, selecciona a Vref-.

// Registro de configuración del ADC (ADCON3):
ADCON3bits.ADCS   = 0x0E; 	// Selección del Tad: ADCS<5:0> = (2*Tad/Tcy)-1, con: Tadmin = 256.41ns; Fcy = 7.159MHz*16/4 = 28,636 MHz (con PLLx16)
ADCON3bits.ADRC   = 0x00; 	// Clock de conversor AD, derivado del clock del sistema.
ADCON3bits.SAMC   = 0x02; 	// Selección del tiempo de muestreo automático: 2Tad.

// Registro de configuración del ADC (ADCON2):
ADCON2 = 0x00;		// Configura: 
					// - Vref+ = AVdd y Vref- = AVss.
					// - Convierte sólo el CH0.
					// - Si está habilitada la interrupción, interrumpe cada vez que se cumpleta una secuencia muestreo/conversión.
					// - Y otras cosas mas.

ADCON1bits.ADON = 1;    // Se enciende el conversor AD.
IEC0bits.ADIE = 0x01;	// Habilita interrupción por finalización de conversión AD.
}


//****************************************************************************************************
// Configuración de los PUERTOS
//****************************************************************************************************
// El pin RD2 es utilizado como salida, su estado lógico permite verificar interrupción y permite 
// medir el tiempo que dura el calculo del controlador.

void Configura_PUERTOS(void)
{
TRISFbits.TRISF0 = 0x00;		// Configura puerto D todo como salida. 
TRISFbits.TRISF1 = 0x00;		// Configura puerto D todo como salida.
TRISFbits.TRISF4 = 0x00;		// Configura puerto D todo como salida.
TRISFbits.TRISF5 = 0x00;		// Configura puerto D todo como salida.
TRISFbits.TRISF6 = 0x01;		// Configura puerto D todo como entrada.
}
