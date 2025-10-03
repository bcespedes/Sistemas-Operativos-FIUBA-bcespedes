# shell


### Búsqueda en $PATH

#### Responder:

* ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

    * La diferencia entre exec(3) y execve(2) es que justamente, las funciones de exec(3) al ser wrappers, internamente llaman a execve(2) ademas de hacer cosas extra. Los wrappers en si no son syscalls. Los wrappers marcados con "p" (execlp, execvp, execvpe) buscan el binario en el PATH si no lo encuentran la ruta absoluta. Los wrappers marcados con "l" (execl, execle, execlp) reciben una lista de argumentos y los wrappers marcados con "v" (execv, execvp, execvpe) reciben un vector de argumentos.

#### Responder:

* ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

    * Si, la llamada a exec(3) puede fallar si falla internamente la llamada a execve(2); en ese caso se devuelve el valor -1 y este es atajado por la shell la cual en dicho caso imprime el mensaje de error por la salida de error (stderr) y luego termina el proceso utilizando exit(-1)


### Procesos en segundo plano

* Explicar detalladamente el mecanismo completo utilizado

    * Al ejecutar procesos en segundo plano,  través de la syscall ```sigaction``` podemos manejar la señal SIGCHLD (la cual es enviada por todos los procesos hijos a su proceso padre al finalizar) con un handler. Se llama a la syscall ```waitpid``` con la flag WNOHANG (se transforma la  operación en no bloqueante), esto nos permite seguir ejecutando procesos. Como no todos los procesos se ejecutan en segundo plano, sin embargo la señal sí es enviada por todos, se debe manejar con cuidado, para ello, a los procesos que no se ejecutan en background se les modifica el process group id para diferenciarlos del resto.

#### Responder:
* ¿Por qué es necesario el uso de señales?

    * Las señales son necesarias, pues ellas son las que indican cuando un proceso ha finalizado sin necesidad de bloquear la ejecución. La señal SIGCHLD le notifica al proceso padre (shell) cuando su proceso hijo ha finalizado, lo cual luego puede ser utilizado para el handler para realizar las acciones correspondientes.


### Flujo estándar

#### Responder:
* Investigar el significado de 2>&1, explicar cómo funciona su forma general
    
    * Para entender, primer debemos saber qué significas los números:

        0: Entrada Estandar (stdin)
        1: Salida Estandar (stdout)
        2: Error Estandar (stderr)

    Lo que significa hacer `2>&1` es rredirigir el flujo de stderr, al mismo lugar que el stdout. Con esto podemos enviar tanto la salida estandar como los de error a un mismo lugar.

    * La forma general es `n>&m`, y se lee como "Redirigir el descriptor de archivo `n` al mismo lugar donde está yendo el descriptor `m`".

* Mostrar qué sucede con la salida de cat out.txt en el ejemplo.

        $ ls -C /home /noexiste >out.txt 2>&1
        $ cat out.txt
        ---????---
        
    1) `>out.txt`: El listado mediante `ls`, se va a rederigir el stdout al archivo `out.txt`.
    2) `2>&1`: Si hay algun error (stderr) haciendo `ls`, se va a rederigidir al mismo lugar donde se esta rediriendo el stdout, o sea, en `out.txt`.

    Por lo que, dado los ejemplos anteriores a este (usando patricio), cuando hagamos `cat out.txt`, sería:

        $ cat out.txt`
        /home:
        patricio
        ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio


* Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).
    
    * Dado que ya hicimos `ls -C /home /noexiste >out.txt 2>&1`, pudimos notar que primero el stdout se va a `out.txt`, y luego, el stderr, se va a donde habiamos indicado, o sea, tambien a `out.txt`. Ahora, queremos hacer la inversa, o sea: `ls -C /home /noexiste 2>&1 >out.txt`. Lo que va a pasar acá, son 2 cosas:  

    1) `2>&1`: Se va a redirigir el stderr a donde está yendo el stdout, o sea, a la terminal.
    2) `>out.txt`: Luego el stdou se va a redigiri a `out.txt`.

    Entonces, cuando ejecutemos `ls -C /home /noexiste 2>&1 >out.txt`, obtendremos en la terminal:
        
        $ ls -C /home /noexiste 2>&1 >out.txt
        ls: cannot access '/noexiste': No such file or directory
    
    Y cuando hagamos `cat out.txt`:

        /home:
        patricio

    



### Tuberías múltiples

#### Responder
* Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
* ¿Cambia en algo?
    * Sí, la manera en que la shell maneja los exit codes de los procesos conectados por pipes se guarda en un array interno ($PIPESTATUS en bash), y de esta manera se puede conocer el exit codes de cada proceso. En nuestra implementación no estamos guardando el exit code de los procesos del pipe, por lo que no sabremos si alguno de los procesos del pipe fallo, entonces al cerrar los pipes y esperar los procesos, el programa sale directamente con `exit(0)`.  

* ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.  

    * Para el ejemplo, usaremos: `ls NoExiste | cat NoHayNada.txt`  
    * En Bash:

            $ cat: NoHayNada.txt: No existe el archivo o el directorio
            ls: no se puede acceder a 'NoExiste': No existe el archivo o el directorio
            $ echo $?
            1

    Si antes de hacer un `echo $?`, usammos `echo "${PIPESTATUS[@]}"`, nos aparecerá `2 1`, lo que notamos es que el primer exit code fue el 2 para el `ls`, y luego, el último exit code fue 1 para el `cat`.

    
    * En nuestra implementanción:
    
            $ cat: NoHayNada.txt: No existe el archivo o el directorio
            ls: no se puede acceder a 'NoExiste': No existe el archivo o el directorio
            $ echo $?
            0
        
        
    Tenemos un exit code 0, ya que fue donde terminó la ejecución de los pipes, sin saber si hubo fallos en la ejecución de los procesos, entonces como solo esperó a que terminara los procesos, sale un exit code 0.



### Variables de entorno temporarias

#### Responder:

* ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

    * Dado a que lo que se busca es incorporar nuevas variables de entorno a la ejecución de un programa, es necesario hacerlo luego de la llamada a fork(2) para que estas afecten solo al nuevo proceso que se va a ejecutar, sin tener que modificar el entorno del proceso padre.
    * Entonces, fork(2) crea una copia del proceso actual en un nuevo proceso hijo. Dentro de este ultimo, al llamar a setenv(3) antes de exec(3) se modifican solo las variables de entorno del hijo. 

#### Responder:
* En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
* ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

    * El comportamiento no es el mismo que en el primer caso. 
    * Al usar alguna de las funciones de exec(3) que finalizan con la letra “e” (como execve()), el nuevo proceso reemplaza todo su entorno con el arreglo recibido como tercer argumento, tal como se describe en la sección correspondiente del man exec(3).
    Debido a esto, no se combinan las nuevas variables con el entorno original. En cambio, al utilizar setenv(3) en el hijo antes del exec, las variables nuevas se agregan (o reemplazan) dentro del entorno ya existente.

* Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

    * Para que el comportamiento sea el mismo utilizando una función de la familia exec(3) que permita pasar un arreglo de entorno, se puede plantear:
        1. Copiar el entorno actual usando la variable global environ.
        2. Crear una copia del entorno, agregando o modificando las variables temporales en ella.
        3. Construir un arreglo que contenga todas las variables de entorno (originales + nuevas).
        4. Pasar el arreglo como tercer argumento a la función de exec(3) que finalice con la letra “e”.


### Pseudo-variables

#### Responder
* Investigar al menos otras tres variables mágicas estándar, y describir su propósito.
* Incluir un ejemplo de su uso en bash (u otra terminal similar).

    * La pseudovariable ```$$``` muestra el PID del proceso actual.

    * Ejemplo de uso:

            $ echo PID de la shell: $$
            PID de la shell: 523
    
    * La pseudovariable ```$!``` muestra el PID del último proceso en segundo plano.

    * Ejemplo de uso:

            $ sleep 5 &
            echo PID del último proceso en segundo plano: $!
            PID del último proceso en segundo plano: 814
    
    * La pseudovariable ```$_``` muestra el último argumento del comando anterior.

    * Ejemplo de uso:

            $ cat prueba.txt
            cat: prueba.txt: No such file or directory
            echo Último argumento: $_
            Último argumento: prueba.txt


### Comandos built-in

#### Responder:
* ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

    * El comando pwd se puede implementar sin necesidad de ser built-in, de hecho, si ejecutamos ```which pwd``` en la terminal veremos que está en /usr/bin. Se implementa como built-in debido a que es un comando muy frecuente y de esta manera se gana una eficiencia notoria al no tener que generar un nuevo proceso (mismo caso que con true y false).
    * Además, pwd simplemente imprime el directorio, por lo que no afecta el funcionamiento del proceso general. Por otro lado, cd se debe implementar sí o sí como built-in ya que al cambiar de directorio, queremos que este cambio persista para siguientes ejecuciones en la shell.


### Historial

#### Responder:

* ¿Cuál es la función de los parámetros MIN y TIME del modo no canónico? ¿Qué se logra en el ejemplo dado al establecer a MIN en 1 y a TIME en 0?
