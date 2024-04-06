/****************************************************************************************************/
/*    Libreria para el manejo de datos en formato Q.                                                */
/*                                                                                                  */
/*  Esta libreria esta pensada para que funcione en un dsc con una arquitectura de 16 bits.         */
/*                                                                                                  */
/*  La libreria consiste en una serie de macros interpretados por el precompilador y que posibilitan*/
/*  el manejo de numeros en punto fijo y las operaciones entre ellos.                               */
/*                                                                                                  */
/*  Las macros utilizan las siguientes variables:                                                   */
/*                                                                                                  */
/*    a: Primer argumento de la funcion.                                                            */
/*    b: Segundo argumento de la funcion.                                                           */
/*    expA: exponente del argumento a.                                                              */
/*    expB: exponente del argumento b.                                                              */
/*    expC: exponente en el que se quiere que presentar el resultado.                               */
/*                                                                                                  */
/*  Los argumentos en mayusculas representan numeros en formato flotante, mientras que los que estan*/
/* en minusculas, representan numeros en punto fijo.                                                */
/*                                                                                                  */
/****************************************************************************************************/

// Los valores en formato de punto fijo se guardan en registros de 16 bits (short)

#define NumeroQ short

/****************************************************************************************************/
/*                                  Macros de conversion de formatos                                */
/****************************************************************************************************/

// Cambia el exponente de "a", de expA a expB
// Todos los cambios de bases se realizan en 3 ciclos de instruccion, pero hay valores que los esta
// cambiando con mayor error que a otros...
#define ChangeExp(a,expA,expC) (((expC)>(expA)) ? (a)<<((expC)-(expA)) : (a)>>((expA)-(expC)))

// Convierte de flotante a punto fijo
// ----------------------------------
// Los tiempos de ejecucion no tienen una duracion fija... Nose la razon pero varian entre 204 y
// mas de 230 ciclos... Los resultados obtenidos con el simulador muestran que las macros funciona bien.
// No pude lograr hacer una macro generica...
#define FloatToFix15(A) ((int)((-A) * (long)(1<<(15))))
#define FloatToFix12(A) ((int)(( A) * (long)(1<<(12))))
#define FloatToFix10(A) ((int)(( A) * (long)(1<<(10))))
#define FloatToFix5(A) ((int)(( A) * (long)(1<<(5))))

// Convierte de punto fijo a flotante
// ----------------------------------
// Otra vez lo comentarios de arriba, de nuevo lo tiempos de ejecucion no tienen duracion fija y son
// superiores a lo 450 ciclos de maquina.
#define FixToFloat15(a) ((float)(-a)/(float)(1<<(15)))
#define FixToFloat12(a) ((float)( a)/(float)(1<<(12)))


/****************************************************************************************************/
/*                                 Macros de operaciones aritmeticas                                */
/****************************************************************************************************/

/****************************************************************************************************/
/*                                        Macros de suma                                            */
/****************************************************************************************************/

// Suma dos numeros de igual exponente en punto fijo, y el resultado es devuelto en el mismo formato
// Es una operacion sencilla, por lo que la macro es sencilla, y su ejecucion es rapida, toma 4
// ciclos de instruccion.
// A modo de aclaracion vale decir que funciona indistintamente en una base u otra.
#define AddFix(a,b) ((a)+(b))

/****************************************************************************************************/
/*                                        Macros de resta                                           */
/****************************************************************************************************/

// Resta dos numeros de igual exponente en punto fijo, y el resultado es devuelto en el mismo formato
// Los tiempos de ejecucuion, para cualquier formato, y al igual que para la Suma, son de 4 ciclos.
#define SubFix(a,b) ((a)-(b))

/****************************************************************************************************/
/*                                   Macros de multiplicacion                                       */
/****************************************************************************************************/

// Multiplica dos operandos de igual exponente, y devuelve el resultado con el mismo exponente: "expC".
// La macro se ejecuta en un total de 16 ciclos de instruccion
#define MulFix(a,b,expC) (short)(((long)(a)*(long)(b))>>expC)
#define MulFix10(a,b) (short)(((long)(a)*(long)(b))>>10)
#define MulFix12(a,b) (short)(((long)(a)*(long)(b))>>12)

/****************************************************************************************************/
/*                                      Macros de division                                          */
/****************************************************************************************************/

// Divide dos operandos con el exponente "expC" y lo devulve con el mismo exponente.
// La ejecucion tarde mas de 450 ciclos, y es de duracion variable
#define DivFix(a,b,expC) (short)((((long)(a))<<expC)/((long)(b)))
