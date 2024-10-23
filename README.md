# Tank-attack
Autor: Fabricio Mena Mejia y Harold Madriz Cerdas   
Profesor: Luis Alonso Barboza Artavia  
Curso: Datos II
## Requisitos
* Sistema operativo: Ubuntu 24.04.1 LTS
* Editor de codigo: CLion
* Este codigo fue desarrollado en ubuntu por ende es recomendable utilizar este sistema operativo para compilar el proyecto.
* La version de cmake utilizada para el proyecto es la `3.29`, asi que si presenta errores la version actual puede actualizarla mediante los siguientes comandos
  * `sudo apt update`
  * `sudo apt install software-properties-common`
  * `sudo add-apt-repository ppa:kitware/release`
  * `sudo apt update`
  * `sudo apt install cmake` 
* Este juego fue creado utilizando la biblioteca grafica SFML, asi que hay que descargarla mediante el siguiente comando `sudo apt-get install libsfml-dev`.
* Una vez instalada la libreria hay que agregar sus componentes necesarios en el `CMakeLists.txt` mediante `find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)` y `target_link_libraries(Proyecto2 sfml-graphics sfml-window sfml-system)`.

# Compilacion y Ejecucion  
Se creo un programa .cpp por cada algoritmo de ordenamiento solicitado, en cada programa se incluye el algoritmo correspondiente a su nombre del programa, un timer para cada caso de entrada, siendo estos, `mejor caso`, `caso promedio` y `peor caso`, 
ademas de tener un graficador para el comportamiento teorico y para el comportamiento obtenido a partir de los benchmarks. 
En el CMakeLists.txt se crearon ejecutables para cada algoritmo, por lo que es posible correr individualmente cada algoritmo, pero si se llegan a generar problemas por multiples main, es posible comentar los demas mains y dejar solo el main del algoritmo deseado para utilizar.
Hay que esperar un poco ya que la tarea de organizar las listas aleatorias puede variar segun la computadora y el algoritmom, sumado a la funcion de graficarlas puede tomar varios segundos e incluso algunos minutos, por lo que es necesario ser paciente durante su ejecucion. 
Para mi equipo actual que consta de un ryzen 5 5600x, 16 de ram a 3200 mhz y un ssd nvme con capacidad de escritura y lectura de 4000mb/s aproximadamente no pasa de los 45 segundos en correr cada programa, pero otros equipos se pueden ver mas afectados en su tiempo de ejecucion.
Aveces las 2 ventanas de las graficas se abren de forma superpuesta, por lo que es necesario cambiar la ubicacion de una de ellas para poder ver a la que esta debajo.
